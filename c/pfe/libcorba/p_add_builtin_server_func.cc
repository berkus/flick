/*
 * Copyright (c) 1998, 1999 The University of Utah and
 * the Computer Systems Laboratory at the University of Utah (CSL).
 *
 * This file is part of Flick, the Flexible IDL Compiler Kit.
 *
 * Flick is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Flick is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Flick; see the file COPYING.  If not, write to
 * the Free Software Foundation, 59 Temple Place #330, Boston, MA 02111, USA.
 */

#include <assert.h>

#include <mom/compiler.h>
#include <mom/libaoi.h>
#include <mom/libmint.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include <mom/c/pg_corba.hh>

/* Inject built in functions/interfaces for CORBA */
void pg_corba::p_add_builtin_server_func(aoi_interface *ai,
					 char *ai_name,
					 pres_c_skel *skel)
{
	pg_state::p_add_builtin_server_func(ai, ai_name, skel);
	
	/*
	 * XXX - We should construct decomposed-style built-in functions if
	 * async_stubs is set, instead of just the standard presentation.
	 * Since some things are presented differently (such as the default
	 * allocation semantics for parameters), we MUST NOT use those
	 * settings here.
	 *
	 * We munge the async_stubs flag to indicate that we are doing a
	 * regular presentation for now.
	 */
	int old_async_stubs = async_stubs;
	async_stubs = 0;
	
	cast_type lctype = mint_to_ctype(
		&out_pres->mint,
		out_pres->mint.standard_refs.unsigned32_ref);
	
	mint_const idl_dis, int_dis;
	mint_ref int_ref, my_case, new_int, my_ref;
	mint_union_def *my_udef;
	mint_1 *mint = &out_pres->mint; /* This is used by the
					   MINT_*_REF macros */
	
	int_dis = mint_new_const_from_aoi_const( ai->code );
	/* Get the union of CORBA interfaces */
	assert( m(top_union).mint_def_u.union_def.cases.cases_len );
	idl_dis = mint_new_const_int( 1 );
	int_ref = mint_find_union_case( mint, top_union, idl_dis );
	
	/* Add our own interfaces */
	
	mint_const obj_name, op_rep_dis, op_req_dis;
	mint_ref my_string_ref;
	unsigned int i;
	
	/* Setup MINT stuff */
	my_string_ref = MINT_ARRAY_REF,
		            MDA_ElementType, MINT_CHAR_REF, END_REF,
		            END_REF;
	obj_name = mint_new_const_string( "Object" ); /* Our interface name */
	
	/* Create the interface structures.
	 *
	 * The reason we separate these out as opposed to creating them in one
	 * big chunk is that the evaluation order of parameters in C is not
	 * specified.  Thus, we may actually create MINT in a different order
	 * across different platforms/compilers.  Although not tragic, it makes
	 * our IR data files different across these platforms, and may lead to
	 * some confusion.  We just make the order explicit here, so the IRs
	 * can remian identical.
	 */
	/* The request. */
	mint_ref req_ref = MINT_STRUCT_REF,
			     MDA_Slot, my_string_ref,
			     END_REF;
	/* The [normal] reply. */
	mint_ref rep_ref = MINT_STRUCT_REF,
			     MDA_Slot,
			       MINT_INTEGER_REF,
			         MDA_Min, 0,
			         MDA_Range, 1,
			         END_REF,
			     END_REF;
	/* The reply union (normal and exceptional). */
	mint_ref rep_u_ref = MINT_UNION_REF,
			       MDA_Discrim, MINT_INTEGER_REF, END_REF,
			       MDA_Case,
			         mint_new_symbolic_const(MINT_CONST_INT,
							 "CORBA_NO_EXCEPTION"),
			         rep_ref,
			       MDA_Case,
			         mint_new_symbolic_const(
					 MINT_CONST_INT,
					 "CORBA_SYSTEM_EXCEPTION"),
			         mint->standard_refs.system_exception_ref,
			       END_REF;
	/* The operation. */
	new_int = MINT_UNION_REF,
		MDA_Discrim, my_string_ref,
		MDA_Case,
		    op_req_dis = mint_new_const_string( "_is_a" ),
		    req_ref,
		MDA_Case,
		    op_rep_dis = mint_new_const_string( "$_is_a" ),
		    rep_u_ref,
		END_REF;
	
	/* Add our function cases to all of the MINT interface unions */
	my_ref = mint_find_union_case( mint, int_ref, int_dis );
	for( i = 0;
	     i < m(new_int).mint_def_u.union_def.cases.cases_len;
	     i++ ) {
		/* Check to see if the functions are already there.
		   So either the user put them there or they are
		   there thru inheritance. */
		if( (mint_find_union_case( mint, my_ref,
					   m(new_int).mint_def_u.
					   union_def.cases.
					   cases_val[i].val )) != -1 )
			return;
		my_case = mint_add_union_case( mint, my_ref );
		my_udef = &m(my_ref).mint_def_u.union_def;
		my_udef->cases.cases_val[my_case].val =
			m(new_int).mint_def_u.union_def.cases.
			cases_val[i].val;
		my_udef->cases.cases_val[my_case].var =
			m(new_int).mint_def_u.union_def.cases.
			cases_val[i].var;
	}
	/* Setup PRES_C stuff */
	
	pres_c_inline req_root_inl, rep_root_inl, req_func, target_inl;
	cast_ref c_ref, c_def;
	cast_type c_obj, c_env, c_char, c_bool, c_pp_char;
	cast_expr c_ids_init;
	cast_scope *scope = (cast_scope *)top_ptr(scope_stack);
	
	/* Create CAST structures types that CORBA uses */
	c_obj = cast_new_type_name( "CORBA_Object" );
	c_env = cast_new_type_name( "CORBA_Environment" );
	c_env = cast_new_pointer_type( c_env );
	c_char = cast_new_prim_alias(CAST_PRIM_CHAR,
				     0,
				     cast_new_scoped_name("CORBA_char", NULL));
	c_char = cast_new_pointer_type( c_char );
	c_pp_char = cast_new_prim_type( CAST_PRIM_CHAR, 0 );
	c_pp_char = cast_new_pointer_type( c_pp_char );
	c_pp_char = cast_new_pointer_type( c_pp_char );
	c_bool = cast_new_type_name( "CORBA_boolean" );

	/* Figure out the name of our parent array */
	c_ids_init = cast_new_expr_name(
		flick_asprintf( "flick_interface_parents_%s",
				aoi_get_scoped_name( cur_aoi_idx, "_" ) ) );
	
	/* Create the CAST function definition */
	cast_scope *deep_scope = scope;
	cast_scoped_name scn =
		cast_new_scoped_name("CORBA_Object_is_a_internal", NULL);
	
	if( (c_ref = cast_find_def(&deep_scope,
				   scn,
				   CAST_FUNC_DECL)) == -1 ) {
		c_ref = cast_add_def(scope,
				     scn,
				     CAST_SC_NONE,
				     CAST_FUNC_DECL,
				     ch(cur_aoi_idx, PG_CHANNEL_SERVER_DECL),
				     current_protection);
		cast_init_function_type(&c(c_ref).u.cast_def_u_u.func_type, 4);
		c(c_ref).u.cast_def_u_u.func_type.return_type = c_bool;
		c(c_ref).u.cast_def_u_u.func_type.params.
			params_val[0].name = ir_strlit("_obj");
		c(c_ref).u.cast_def_u_u.func_type.params.
			params_val[0].type = c_obj;
		c(c_ref).u.cast_def_u_u.func_type.params.
			params_val[1].name = ir_strlit("id");
		c(c_ref).u.cast_def_u_u.func_type.params.
			params_val[1].type = c_char;
		c(c_ref).u.cast_def_u_u.func_type.params.
			params_val[2].name = ir_strlit("_par");
		c(c_ref).u.cast_def_u_u.func_type.params.
			params_val[2].type = c_pp_char;
		c(c_ref).u.cast_def_u_u.func_type.params.
			params_val[3].name = ir_strlit("_ev");
		c(c_ref).u.cast_def_u_u.func_type.params.
			params_val[3].type = c_env;
	}
	/* Create an allocation semantic suitable for the environment
	   variable. */
	pres_c_allocation env_alloc;
	
	/* Direction is InOut or Out, so other cases are invalid. */
	env_alloc.cases[PRES_C_DIRECTION_IN].allow
		= PRES_C_ALLOCATION_INVALID;
	env_alloc.cases[PRES_C_DIRECTION_INOUT].allow
		= PRES_C_ALLOCATION_INVALID;
	env_alloc.cases[PRES_C_DIRECTION_OUT].allow
		= PRES_C_ALLOCATION_INVALID;
	env_alloc.cases[PRES_C_DIRECTION_RETURN].allow
		= PRES_C_ALLOCATION_INVALID;
	
	/*
	 * This is an indirection pointer:
	 *   Server side always allocs and deallocs.
	 */
	env_alloc.cases[PRES_C_DIRECTION_UNKNOWN].allow
		= PRES_C_ALLOCATION_ALLOW;
	if (gen_server)
		env_alloc.cases[PRES_C_DIRECTION_UNKNOWN].
			pres_c_allocation_u_u.val.flags
			= PRES_C_ALLOC_ALWAYS | PRES_C_DEALLOC_ALWAYS;
	else
		panic("In pg_corba::p_add_builtin_server_func: "
		      "Not generating server!");
	
	env_alloc.cases[PRES_C_DIRECTION_UNKNOWN].pres_c_allocation_u_u.
		val.allocator.kind = PRES_C_ALLOCATOR_DONTCARE;
	env_alloc.cases[PRES_C_DIRECTION_UNKNOWN].pres_c_allocation_u_u.
		val.alloc_init = 0;
	
	/* Create the PRES_C structures for request.  We don't make the top
	   level unions yet because there will be different discriminators for
	   each interface we have added our functions to (This is just for the
	   server side and not client). */
	char *sname = pres_c_make_arglist_name("vararray");
	char *pname = pres_c_make_arglist_name("indir");
	req_root_inl = PRES_C_I_COLLAPSED_UNION,
		       PIA_DiscrimVal, idl_dis,
		       PIA_SelectedCase, PRES_C_I_COLLAPSED_UNION,
		       PIA_DiscrimVal, int_dis,
		       PIA_SelectedCase, PRES_C_I_COLLAPSED_UNION,
		       PIA_DiscrimVal, op_req_dis,
                       PIA_SelectedCase,
	                 req_func = PRES_C_I_FUNC_PARAMS_STRUCT,
		           PIA_Slot,
		             0,
		             PRES_C_I_ATOM,
		               PIA_Index, 1,
		               PIA_Mapping, PRES_C_M_DIRECTION,
		                 PMA_Dir, PRES_C_DIRECTION_IN,
				 PMA_Mapping, PRES_C_M_PARAM_ROOT,
			           PMA_Mapping, PRES_C_M_SINGLETON,
		                     PMA_Target, PRES_C_I_ALLOCATION_CONTEXT,
		                       PIA_ArgList, sname,
		                       PIA_Len, PRES_C_I_ATOM,
		                         PIA_Index, 0,
		                         PIA_Mapping, PRES_C_M_TEMPORARY,
		                           PMA_Name, "string_len",
		                           PMA_CType, lctype,
		                           PMA_PreHandler, "stringlen",
		                           PMA_TempType, TEMP_TYPE_ENCODED,
		                           PMA_Target, PRES_C_M_ARGUMENT,
		                             PMA_ArgList, sname,
		                             PMA_Name, "length",
		                             PMA_Mapping, PRES_C_M_DIRECT,
		                               END_PRES_C,
		                             END_PRES_C,
		                           END_PRES_C,
		                         END_PRES_C,
		                       PIA_Terminator, PRES_C_I_TEMPORARY,
		                         PIA_Name, "string_term",
		                         PIA_CType, cast_new_prim_type(
						 CAST_PRIM_CHAR, 0),
		                         PIA_Value,
		                           cast_new_expr_lit_char(0, 0),
		                         PIA_IsConst, 1,
		                         PIA_Mapping, PRES_C_M_ARGUMENT,
		                           PMA_ArgList, sname,
		                           PMA_Name, "terminator",
		                           END_PRES_C,
		                         END_PRES_C,
		                       PIA_Alloc, p_get_allocation(),
		                       PIA_Ptr, PRES_C_I_ATOM,
		                         PIA_Index, 0,
		                         PIA_Mapping,
		                           PRES_C_M_INTERNAL_ARRAY,
		                             PMA_ArgList, sname,
		                	     PMA_ElementMapping,
			                       PRES_C_M_DIRECT, END_PRES_C,
			                     END_PRES_C,
			                 END_PRES_C,
		                       END_PRES_C,
		                       END_PRES_C,
		                   END_PRES_C,
		                 END_PRES_C,
		               END_PRES_C,
		           PIA_Slot,
			     -1,
			     PRES_C_I_ATOM,
			       PIA_Index, 3,
			       PIA_Mapping, PRES_C_M_PARAM_ROOT,
				 PMA_Mapping, PRES_C_M_SINGLETON,
		                   PMA_Target, PRES_C_I_ALLOCATION_CONTEXT,
		                     PIA_ArgList, pname,
		                     PIA_Len, PRES_C_I_TEMPORARY,
		                       PIA_Name, "array_len",
		                       PIA_CType, lctype,
		                       PIA_Value,
		                         cast_new_expr_lit_int(1, 0),
		                       PIA_IsConst, 1,
		                       PIA_TempType, TEMP_TYPE_ENCODED,
		                       PIA_Mapping, PRES_C_M_ARGUMENT,
		                         PMA_ArgList, pname,
		                         PMA_Name, "length", END_PRES_C,
		                       END_PRES_C,
		                     PIA_MinLen, PRES_C_I_TEMPORARY,
		                       PIA_Name, "array_len",
		                       PIA_CType, lctype,
		                       PIA_Value,
		                         cast_new_expr_lit_int(1, 0),
		                       PIA_IsConst, 1,
		                       PIA_Mapping, PRES_C_M_ARGUMENT,
		                         PMA_ArgList, pname,
		                         PMA_Name, "min_len", END_PRES_C,
		                       END_PRES_C,
		                     PIA_MaxLen, PRES_C_I_TEMPORARY,
		                       PIA_Name, "array_len",
		                       PIA_CType, lctype,
		                       PIA_Value,
		                         cast_new_expr_lit_int(1, 0),
		                       PIA_IsConst, 1,
		                       PIA_Mapping, PRES_C_M_ARGUMENT,
		                         PMA_ArgList, pname,
		                         PMA_Name, "max_len", END_PRES_C,
		                       END_PRES_C,
		                     PIA_MinAllocLen, PRES_C_I_TEMPORARY,
		                       PIA_Name, "array_len",
		                       PIA_CType, lctype,
		                       PIA_Value,
		                         cast_new_expr_lit_int(1, 0),
		                       PIA_IsConst, 1,
		                       PIA_Mapping, PRES_C_M_ARGUMENT,
		                         PMA_ArgList, pname,
		                         PMA_Name, "min_alloc_len", END_PRES_C,
		                       END_PRES_C,
		                     PIA_Alloc, env_alloc,
		                     PIA_Ptr, PRES_C_I_ATOM,
		                       PIA_Index, 0,
		                       PIA_Mapping, PRES_C_M_POINTER,
		                         PMA_ArgList, pname,
				         PMA_Target, PRES_C_M_IGNORE,
		                           END_PRES_C,
				         END_PRES_C,
		                       END_PRES_C,
		                     END_PRES_C,
		                   END_PRES_C,
				 END_PRES_C,
			       END_PRES_C,
		       /* This parameter is initialized with the
			  variable name of our parent array */
		           PIA_Slot,
			     -1,
			     PRES_C_I_ATOM,
			       PIA_Index, 2,
			       PIA_Mapping, PRES_C_M_PARAM_ROOT,
				 PMA_Mapping, PRES_C_M_INITIALIZE,
			           PMA_Value, c_ids_init,
				   END_PRES_C,
				 END_PRES_C,
			       END_PRES_C,
			   PIA_Return,
			     -1,
			     PRES_C_I_ATOM,
			       PIA_Index, pres_c_func_return_index,
			       PIA_Mapping, PRES_C_M_DIRECTION,
			         PMA_Dir, PRES_C_DIRECTION_RETURN,
				 PMA_Mapping, PRES_C_M_PARAM_ROOT,
				   PMA_Mapping, PRES_C_M_INITIALIZE,
				     PMA_Value, cast_new_expr_lit_int(0, 0),
				     END_PRES_C,
				   END_PRES_C,
			         END_PRES_C,
			       END_PRES_C,
		           END_PRES_C,
		       END_PRES_C,
		       END_PRES_C,
		       END_PRES_C;
	
	/* Create the PRES_C structures for reply */
	char *vaname = pres_c_make_arglist_name("indir");
	char *vuname = pres_c_make_arglist_name("vunion");
	rep_root_inl = PRES_C_I_COLLAPSED_UNION,
		       PIA_DiscrimVal, idl_dis,
		       PIA_SelectedCase, PRES_C_I_COLLAPSED_UNION,
		       PIA_DiscrimVal, int_dis,
		       PIA_SelectedCase, PRES_C_I_COLLAPSED_UNION,
		       PIA_DiscrimVal, op_rep_dis,
		       PIA_SelectedCase, PRES_C_I_VIRTUAL_UNION,
		         PIA_ArgList, vuname,
			 PIA_Discrim, PRES_C_I_ATOM,
		           PIA_Index, 3,
		           PIA_Mapping, PRES_C_M_SINGLETON,
		             PMA_Target, PRES_C_I_ALLOCATION_CONTEXT,
		               PIA_ArgList, vaname,
		               PIA_Len, PRES_C_I_TEMPORARY,
		                 PIA_Name, "array_len",
		                 PIA_CType, lctype, 
		                 PIA_Value, cast_new_expr_lit_int(1, 0),
		                 PIA_IsConst, 1,
		                 PIA_TempType, TEMP_TYPE_ENCODED,
		                 PIA_Mapping, PRES_C_M_ARGUMENT,
		                   PMA_ArgList, vaname,
		                   PMA_Name, "length", END_PRES_C,
		                 END_PRES_C,
		               PIA_MinLen, PRES_C_I_TEMPORARY,
		                 PIA_Name, "array_len",
		                 PIA_CType, lctype,
		                 PIA_Value, cast_new_expr_lit_int(1, 0),
		                 PIA_IsConst, 1,
		                 PIA_Mapping, PRES_C_M_ARGUMENT,
		                   PMA_ArgList, vaname,
		                   PMA_Name, "min_len", END_PRES_C,
		                 END_PRES_C,
		               PIA_MaxLen, PRES_C_I_TEMPORARY,
		                 PIA_Name, "array_len",
		                 PIA_CType, lctype,
		                 PIA_Value, cast_new_expr_lit_int(1, 0),
		                 PIA_IsConst, 1,
		                 PIA_Mapping, PRES_C_M_ARGUMENT,
		                   PMA_ArgList, vaname,
		                   PMA_Name, "max_len", END_PRES_C,
		                 END_PRES_C,
		               PIA_MinAllocLen, PRES_C_I_TEMPORARY,
		                 PIA_Name, "array_len",
		                 PIA_CType, lctype,
		                 PIA_Value, cast_new_expr_lit_int(1, 0),
		                 PIA_IsConst, 1,
		                 PIA_Mapping, PRES_C_M_ARGUMENT,
		                   PMA_ArgList, vaname,
		                   PMA_Name, "min_alloc_len", END_PRES_C,
		                 END_PRES_C,
		               PIA_Alloc, env_alloc,
		               PIA_Ptr, PRES_C_I_ATOM,
		                 PIA_Index, 0,
				 PIA_Mapping, PRES_C_M_POINTER,
		                   PMA_ArgList, vaname,
		                   PMA_Target, PRES_C_M_SELECTOR,
		                     PMA_Index, 0,
		                     PMA_Mapping, PRES_C_M_ARGUMENT,
		                       PMA_ArgList, vuname,
		                       PMA_Name, "discrim",
		                       PMA_Mapping, PRES_C_M_DIRECT,
		                         END_PRES_C,
		                       END_PRES_C,
		                     END_PRES_C,
		                   END_PRES_C,
		                 END_PRES_C,
		               END_PRES_C,
		             END_PRES_C,
		           END_PRES_C,
			 PIA_Case,
		               PRES_C_I_FUNC_PARAMS_STRUCT,
			           PIA_Return,
			               0,
			               PRES_C_I_ATOM,
			                   PIA_Index, -1,
			                   PIA_Mapping, PRES_C_M_DIRECTION,
			                       PMA_Dir,
		                                   PRES_C_DIRECTION_RETURN,
			                       PMA_Mapping,
						   PRES_C_M_PARAM_ROOT,
						   PMA_Mapping,
	                                               PRES_C_M_DIRECT,
			                           END_PRES_C,
				               END_PRES_C,
			                   END_PRES_C,
			               END_PRES_C,
			       END_PRES_C,
		         PIA_Case,
			       PRES_C_I_ATOM,
		                   PIA_Index, 3,
		                   PIA_Mapping,
			               PRES_C_M_SYSTEM_EXCEPTION,
		                   END_PRES_C,
			       END_PRES_C,
			 END_PRES_C,
		       END_PRES_C,
		       END_PRES_C,
		       END_PRES_C;
	
	/* Create the PRES_C structs for target_i */
	target_inl = PRES_C_I_ATOM,
			 PIA_Index, 0,
			 PIA_Mapping, PRES_C_M_ARGUMENT,
		             PMA_ArgList, "params",
		             PMA_Name, "object",
			     PMA_Mapping, PRES_C_M_PARAM_ROOT,
			         PMA_Mapping, PRES_C_M_REFERENCE,
				     PMA_Kind, PRES_C_REFERENCE_COPY,
				     PMA_RefCount, 1,
				     END_PRES_C,
				 END_PRES_C,
			     END_PRES_C,
			 END_PRES_C;
	
	/* We are generating server funcs so we add each
	   of our functions to the server skel */
	pres_c_server_func *stub;
	int j;
	
	/* Loop through all of the server skeletons and add
	   new stub functions with our pres_c stuff */
	j = skel->funcs.funcs_len++;
	skel->funcs.funcs_val = (pres_c_func *)
				mustrealloc(skel->funcs.funcs_val,
					    sizeof(pres_c_func)
					    * (skel->funcs.funcs_len));
	
	/* Setup the values in our
	   new server function */
	skel->funcs.funcs_val[j].kind = PRES_C_SERVER_FUNC;
	stub = &skel->funcs.funcs_val[j].pres_c_func_u.sfunc;
	c_def = cast_add_def(&out_pres->stubs_cast,
			     null_scope_name,
			     CAST_SC_NONE,
			     CAST_FUNC_DECL,
			     ch(cur_aoi_idx, PG_CHANNEL_SERVER_IMPL),
			     CAST_PROT_NONE);
	out_pres->stubs_cast.cast_scope_val[c_def] = c(c_ref);
	stub->c_func = c_def;
	stub->op_flags = 0;
	/* Make the top level collapsed unions with
	   the discriminator for this interface */
	stub->request_i = req_root_inl;
	stub->reply_i = rep_root_inl;
	stub->target_i = target_inl;
	stub->target_itype = out_pres->mint.standard_refs.interface_name_ref;
	stub->client_i = 0;
	stub->client_itype = mint_ref_null;
	stub->error_i = 0;
	stub->error_itype = mint_ref_null;
	
	/* Unmunge the async_stubs flag.  See comment at beginning of func. */
	async_stubs = old_async_stubs;
}

/* End of file. */

