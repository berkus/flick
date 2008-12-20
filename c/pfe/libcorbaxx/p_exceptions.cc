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

#include <mom/c/pg_corbaxx.hh>

void pg_corbaxx::p_except_type(aoi_exception *ae,
			       p_type_collection **out_ptc)
{
	p_type_collection *ptc, *ptc_ref;
	union tag_data_u data;
	cast_scoped_name scn;
	p_type_node *ptn;
	cast_type ctype;
	tag_list *tl;
	tag_data *pt_td;
	char *ex_name;
	
	/* Setup the type collection for our exception */
	ex_name = calc_name_from_ref(cur_aoi_idx);
	pt_td = &(find_tag(out_pres->pres_attrs, "pres_type")->data);
	tl = create_tag_list(0);
	ptc = *out_ptc;
	ptc->set_tag_list(tl);
	data.tl = tl;
	ptc->set_attr_index(append_tag_data(pt_td, data));
	ptc->set_id(get_repository_id(cur_aoi_idx));
	add_tag(tl, "idl_type", TAG_STRING, "exception");
	add_tag(tl, "name", TAG_STRING, ptc->get_name());
	
	/* Construct the actual exception type */
	ctype = cast_new_aggregate_type(except_aggregate_type, 0);
	ctype->cast_type_u_u.agg_type.name = cast_new_scoped_name(name, NULL);
	
	/* we make a new collection to hold the types, while the
	   collection passed in holds fully named types. */
	ptc_ref = p_new_type_collection(ex_name);
	ptc->set_collection_ref(ptc_ref);
	
	ptn = new p_type_node;
	ptn->set_name("definition");
	ptn->set_format("%s");
	ptn->set_type(ctype);
	pres_c_mapping map = pres_c_new_mapping(PRES_C_MAPPING_STRUCT);
	ptn->set_mapping(map);
	ptc->add_type("default", ptn);
	
	/* Build up the functions of the exception class */
	p_exception_class(ptc);
	
	push_ptr(scope_stack, &ctype->cast_type_u_u.agg_type.scope);
	cast_add_scope_name(&current_scope_name,
			    ex_name,
			    null_template_arg_array);
	
	map->pres_c_mapping_u_u.struct_i = p_inline_exception(ae,
							      *out_ptc,
							      ctype);
	
	pop_ptr(scope_stack);
	cast_del_scope_name(&current_scope_name);
	
	scn = cast_new_scoped_name(ex_name, NULL);
	
	ptn = new p_type_node;
	ptn->set_name("forward_declaration");
	ptn->set_format(0);
	ctype = cast_new_type(CAST_TYPE_CLASS_NAME);
	ctype->cast_type_u_u.class_name = scn;
	ptn->set_type(ctype);
	ptc->add_type("default", ptn, 0);
}

/* This function will construct all of the functions
   that normally go in an exception class.  */
void pg_corbaxx::p_exception_class(p_type_collection *ptc)
{
	cast_def_protection last_prot = current_protection;
	cast_type ex_type, raw_type, type;
	cast_scoped_name scn;
	cast_scope *scope;
	tag_list *tl, *pt_tl;
	const char *ex_name;
	
	pt_tl = ptc->get_tag_list();
	tl = create_tag_list(0);
	add_tag(pt_tl, "main", TAG_TAG_LIST, tl);
	add_tag(tl, "form", TAG_STRING, "class");
	ex_type = ptc->get_collection_ref()->
		find_type("definition")->get_type();
	cast_class_add_parent(ex_type,
			      CAST_PARENT_PUBLIC,
			      cast_new_scoped_name("CORBA",
						   "UserException",
						   NULL));
	raw_type = ptc->find_type("definition")->get_type();
	add_tag(pt_tl, "type", TAG_CAST_TYPE, raw_type);
	ex_name = ptc->get_name();
	cast_add_scope_name(&current_scope_name,
			    ex_name,
			    null_template_arg_array);
	add_tag(tl, "name", TAG_CAST_SCOPED_NAME,
		cast_copy_scoped_name(&current_scope_name));
	scn = cast_new_scoped_name(ex_name, NULL);
	scope = &ex_type->cast_type_u_u.agg_type.scope;
	current_protection = CAST_PROT_PUBLIC;
	add_function(tl,
		     scn,
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "T()",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_Constructor,
		     PFA_Scope, scope,
		     PFA_ReturnType, cast_new_type(CAST_TYPE_NULL),
		     PFA_TAG_DONE);
	type = cast_new_qualified_type(raw_type, CAST_TQ_CONST);
	type = cast_new_reference_type(type);
	add_function(tl,
		     scn,
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "T(T &)",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_Constructor,
		     PFA_Scope, scope,
		     PFA_ReturnType, cast_new_type(CAST_TYPE_NULL),
		     PFA_Parameter, type, "o", NULL,
		     PFA_TAG_DONE);
	add_function(tl,
		     cast_new_scoped_name(flick_asprintf("~%s", ex_name), NULL),
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "~T()",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_Scope, scope,
		     PFA_ReturnType, cast_new_type(CAST_TYPE_NULL),
		     PFA_TAG_DONE);
	add_function(tl,
		     cast_new_scoped_name("operator=", NULL),
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "T &operator=(T &)",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_Scope, scope,
		     PFA_ReturnType, cast_new_reference_type(raw_type),
		     PFA_Parameter, type, "o", NULL,
		     PFA_TAG_DONE);
	add_function(tl,
		     cast_new_scoped_name("_raise", NULL),
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "_raise()",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_Scope, scope,
		     PFA_Spec, CAST_FUNC_VIRTUAL,
		     PFA_ReturnType, cast_new_type(CAST_TYPE_VOID),
		     PFA_TAG_DONE);
	scn = cast_new_scoped_name("CORBA", "Exception", NULL);
	type = cast_new_type_scoped_name(scn);
	type = cast_new_pointer_type(type);
	add_function(tl,
		     cast_new_scoped_name("_narrow", NULL),
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "_narrow()",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_Scope, scope,
		     PFA_StorageClass, CAST_SC_STATIC,
		     PFA_ReturnType, cast_new_pointer_type(raw_type),
		     PFA_Parameter, type, "e", NULL,
		     PFA_TAG_DONE);
	add_function(tl,
		     cast_new_scoped_name("_alloc", NULL),
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "_alloc()",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_Scope, scope,
		     PFA_StorageClass, CAST_SC_STATIC,
		     PFA_ReturnType, type,
		     PFA_TAG_DONE);
	cast_del_scope_name(&current_scope_name);
	current_protection = last_prot;
}

cast_type pg_corbaxx::p_get_env_struct_type() 
{
	static cast_type env = 0;
	int cdef;
	if (env)
		return env;
	env = cast_new_struct_type(0);
	
	cdef = cast_add_def(&env->cast_type_u_u.agg_type.scope,
			    cast_new_scoped_name("_major", NULL),
			    CAST_SC_NONE,
			    CAST_VAR_DEF,
			    pg_channel_maps[PG_CHANNEL_CLIENT_DECL]
			    [builtin_file],
			    CAST_PROT_NONE);
	env->cast_type_u_u.agg_type.scope.cast_scope_val[cdef].
		u.cast_def_u_u.var_def.type =
		cast_new_type_scoped_name(cast_new_scoped_name("CORBA",
							       "ExceptionType",
							       NULL));
#if 0
	/* XXX - This is what we should do, but the pres_c doesn't handle it */
	except.name = "_value";
	except.type = cast_new_union_type(2);
	cast_union_add_case(except, "_system_except", cast_new_type_name(
		"flick_system_exception"));
	cast_union_add_case(except, "_user_except", cast_new_pointer_type(
		cast_new_type(CAST_TYPE_VOID)));
#else
	/* XXX - This is what we do now */
	cdef = cast_add_def(&env->cast_type_u_u.agg_type.scope,
			    cast_new_scoped_name("_user_except", NULL),
			    CAST_SC_NONE,
			    CAST_VAR_DEF,
			    pg_channel_maps[PG_CHANNEL_CLIENT_DECL]
			    [builtin_file],
			    CAST_PROT_NONE);
	env->cast_type_u_u.agg_type.scope.cast_scope_val[cdef].
		u.cast_def_u_u.var_def.type =
		cast_new_pointer_type(cast_new_type(CAST_TYPE_VOID));
#endif

	cdef = cast_add_def(&env->cast_type_u_u.agg_type.scope,
			    cast_new_scoped_name("_id", NULL),
			    CAST_SC_NONE,
			    CAST_VAR_DEF,
			    pg_channel_maps[PG_CHANNEL_CLIENT_DECL]
			    [builtin_file],
			    CAST_PROT_NONE);
	env->cast_type_u_u.agg_type.scope.cast_scope_val[cdef].
		u.cast_def_u_u.var_def.type =
		cast_new_pointer_type(cast_new_prim_type(CAST_PRIM_CHAR, 0));
	
	if (gen_client)
		env->cast_type_u_u.agg_type.name
			= cast_new_scoped_name("flick_corbaxx_env", NULL);
	else if (gen_server)
		env->cast_type_u_u.agg_type.name
			= cast_new_scoped_name("flick_corbaxx_env", NULL);
	else
		panic("In `pg_corbaxx::p_get_env_struct_type', "
		      "generating neither client nor server.");
	
	return env;
}

int pg_corbaxx::p_get_exception_discrim()
{
	return 0;
}

int pg_corbaxx::p_get_exception_void()
{
	return 1;
}

pres_c_inline_atom pg_corbaxx::p_get_user_discrim() 
{
#define ASSIGN_DISCRIM_STATICALLY
#ifdef ASSIGN_DISCRIM_STATICALLY
#undef ASSIGN_DISCRIM_STATICALLY
	/* The discriminator won't be unmarshaled.  It will be assigned the
	   exact literal value upon entering the chosen branch of the union. */
	pres_c_mapping ta
		= pres_c_new_mapping(PRES_C_MAPPING_ELSEWHERE);
#else
	pres_c_mapping ta
		= pres_c_new_mapping(PRES_C_MAPPING_TERMINATED_ARRAY);
	
	ta->pres_c_mapping_u_u.terminated_array.terminator
		= 0;
	ta->pres_c_mapping_u_u.terminated_array.max
		= 0;
	ta->pres_c_mapping_u_u.terminated_array.element_mapping
		= pres_c_new_mapping(PRES_C_MAPPING_DIRECT);
	/*
	 * When we encode, we don't free the string since we assume it to
	 * be a literal string (e.g. ex_CORBA_NO_MEM).  When we decode,
	 * we have to allocate space in order to process it, but we should
	 * quickly deallocate it after locating a static (literal) copy of
	 * that id in a table.  XXX - lookup not done yet.
	 */
	ta->pres_c_mapping_u_u.terminated_array.alloc
		= p_get_allocation();
#endif
	
	// Note the assumption that the identifier is in position 2!!!
	return pres_c_new_inline_atom(2, ta)->pres_c_inline_u_u.atom;
}

pres_c_inline_void_union_case *pg_corbaxx::p_build_user_exceptions(int icase) 
{
	assert(m(icase).kind == MINT_UNION);
	
	mint_union_def *mu = &(m(icase).mint_def_u.union_def);
	int size = mu->cases.cases_len, i;
	aoi_ref j;
	
	pres_c_inline_void_union_case *vuc =
		(pres_c_inline_void_union_case *)
		mustcalloc(sizeof(pres_c_inline_void_union_case) * size);
	
	for (i = 0; i < size; i++) {
		pres_c_allocation alloc;
		int lpc;
		
		mint_ref cur = mu->cases.cases_val[i].var;
		
		vuc[i].mapping
			= pres_c_new_mapping(PRES_C_MAPPING_STUB);
		
		int stub_idx = pres_c_find_mu_stub(out_pres, cur, 0,
						   vuc[i].mapping,
						   PRES_C_MARSHAL_STUB);
		
		vuc[i].mapping->pres_c_mapping_u_u.mapping_stub.
			mapping_stub_index
			= stub_idx;
		
		/*
		 * Now build the type of the exception (just a name for now).
		 */
		for (j = 0; j < (signed int)in_aoi->defs.defs_len; j++) {
			if (aoi_to_mint_association[j] == cur)
				break;
		}
		assert(j < (signed int)in_aoi->defs.defs_len);
		vuc[i].type = p_make_ctypename(j);
		
		alloc = p_get_allocation();
		for( lpc = 0; lpc < PRES_C_DIRECTIONS; lpc++ ) {
			switch( alloc.cases[lpc].allow ) {
			case PRES_C_ALLOCATION_ALLOW:
				alloc.cases[lpc].pres_c_allocation_u_u.val.
					flags |= PRES_C_RUN_CTOR;
				alloc.cases[lpc].pres_c_allocation_u_u.val.
					flags &= ~(PRES_C_DEALLOC_ALWAYS|
						   PRES_C_RUN_DTOR);
				break;
			default:
				break;
			}
		}
		/* Interpose an indirection pointer (w/ allocation context). */
		pres_c_interpose_indirection_pointer(&vuc[i].type,
						     &vuc[i].mapping,
						     alloc);
		
		/* Convert the MINT discriminator value for this branch. */
		vuc[i].case_value =
			p_mint_exception_id_const_to_cast(
				mu->cases.cases_val[i].val);
	}
	
	return vuc;
}

int pg_corbaxx::p_count_user_exceptions(int icase)
{
	mint_union_def *mu = &(m(icase).mint_def_u.union_def);
	return mu->cases.cases_len;
}

void pg_corbaxx::p_do_exceptional_case(
	pres_c_inline_virtual_union_case *vucase,
	mint_union_case *ucase, int icase,
	pres_c_inline_index env_idx) 
{
	pres_c_mapping map = 0;
	cast_type ctype;
	
	mint_ref system_exception_ref
		= out_pres->mint.standard_refs.system_exception_ref;
	
	assert(vucase);
	if (!ucase
	    || (ucase->var != system_exception_ref)) {
		/*
		 * We have the default case, so this is the void union for the
		 * user exceptions.
		 */
		pres_c_inline void_inl
			= pres_c_new_inline(PRES_C_INLINE_VOID_UNION);
		
		void_inl->pres_c_inline_u_u.void_union.discrim
			= p_get_user_discrim();
		void_inl->pres_c_inline_u_u.void_union.void_index
			= p_get_exception_void();
		void_inl->pres_c_inline_u_u.void_union.cases.cases_val
			= p_build_user_exceptions(icase);
		void_inl->pres_c_inline_u_u.void_union.cases.cases_len
			= p_count_user_exceptions(icase);
		void_inl->pres_c_inline_u_u.void_union.dfault
			= 0;
		map = pres_c_new_mapping(PRES_C_MAPPING_STRUCT);
		map->pres_c_mapping_u_u.struct_i = void_inl;
		ctype = cast_new_type_name("flick_corbaxx_env");
		pres_c_interpose_temporary(ctype,
					   "env",
					   0 /* no initialization expr */,
					   "env_to_exception",
					   "exception_to_env",
					   0 /* not constant */,
					   TEMP_TYPE_PRESENTED,
					   &map);
	} else if (ucase && (ucase->var == system_exception_ref)) {
		map = pres_c_new_mapping(PRES_C_MAPPING_SYSTEM_EXCEPTION);
	} else
		panic("Don't know how to deal with a user exception that is "
		      "not in the default slot!");
	
	*vucase = pres_c_new_inline_atom(env_idx, map);
}

pres_c_mapping pg_corbaxx::p_make_exception_discrim_map(char *arglist_name)
{
	pres_c_mapping map = pres_c_new_mapping(PRES_C_MAPPING_DIRECT);
	cast_type ctype;
	
	pres_c_interpose_argument(&map, arglist_name, "discrim");
	
	ctype = cast_new_prim_type(CAST_PRIM_INT, 0);
	pres_c_interpose_temporary(ctype,
				   "major",
				   0 /* no initialization expr */,
				   gen_client ? "" : "get_major",
				   0 /* no post-handler */,
				   0 /* not constant */,
				   TEMP_TYPE_PRESENTED,
				   &map);
	
	return map;
}

void pg_corbaxx::p_do_return_union(aoi_operation *ao,
				   pres_c_inline* reply_l4_inl,
				   mint_ref reply_ref,
				   cast_ref cfunc,
				   pres_c_inline_index discrim_idx)
{
	mint_union_def *mu = &(m(reply_ref).mint_def_u.union_def);
	
	assert(mu->cases.cases_len == 2);
	
	mu->cases.cases_val[0].val
		= mint_new_symbolic_const(MINT_CONST_INT,
					  "CORBA::NO_EXCEPTION");
	mu->cases.cases_val[1].val
		= mint_new_symbolic_const(MINT_CONST_INT,
					  "CORBA::SYSTEM_EXCEPTION");
	
	pg_state::p_do_return_union(ao, reply_l4_inl, reply_ref, cfunc,
				    discrim_idx);
}

/* End of file. */

