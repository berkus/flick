/*
 * Copyright (c) 1995, 1996, 1997, 1998, 1999 The University of Utah and
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

#include <string.h>
#include <assert.h>
#include <mom/compiler.h>
#include <mom/libaoi.h>
#include <mom/libmint.h>
#include <mom/c/pfe.hh>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include "private.hh"

void pg_state::p_server_func_make_decl(aoi_interface */*ai*/,
				       aoi_operation *ao,
				       char *opname,
				       cast_func_type *cfunc)
{
	cast_ref cr;
	cast_scope *scope = (cast_scope *)top_ptr(scope_stack);
	cast_scope *deep_scope = scope;
	cast_scoped_name scn = cast_new_scoped_name(opname, NULL);
	
	if( (ao->flags & (AOI_OP_FLAG_GETTER|AOI_OP_FLAG_SETTER)) ||
	    (cr = cast_find_def(&deep_scope,
				scn,
				CAST_FUNC_DECL)) == -1 ) {
		cr = cast_add_def(scope,
				  scn,
				  CAST_SC_NONE,
				  CAST_FUNC_DECL,
				  ch(cur_aoi_idx, PG_CHANNEL_SERVER_DECL),
				  current_protection);
		scope->cast_scope_val[cr].u.cast_def_u_u.func_type = *cfunc;
		scope->cast_scope_val[cr].u.cast_def_u_u.func_type.spec =
			server_func_spec;
	} else {
		if( out_pres->meta_data.channels.
		    channels_val[scope->cast_scope_val[cr].channel].input !=
		    a(cur_aoi_idx).idl_file )
			scope->cast_scope_val[cr].channel =
				ch(cur_aoi_idx, PG_CHANNEL_SERVER_DECL);
	}
}

/*
 * Generate a server work function presentation for an AOI interface operation.
 */
pres_c_func pg_state::p_server_func(aoi_interface *ai,
				    aoi_operation *ao)
{
	pres_c_func func;
	func.kind = PRES_C_SERVER_FUNC;
	pres_c_server_func *server_func = &func.pres_c_func_u.sfunc;
	
	char *old_name;
	char *opname;
	int cast_params_len;
	int cdef;
	cast_func_type cfunc;
	
	stub_special_params specials;
	int i, j;
	
	mint_ref request_ref;
	mint_ref reply_ref;
	
	mint_const oper_request;
	mint_const oper_reply;
	
	/*****/
	
	if(!lookup_interface_mint_discrims(ai, ao,
					   &oper_request, &oper_reply)) {
		panic("In `pg_state::p_server_func', "
		      "cannot find MINT request and reply discriminators.");
	}
	
	/*
	 * Find the MINT references to the request and reply types.
	 */
	p_server_func_find_refs(ai, ao, oper_request, oper_reply,
				&request_ref,
				&reply_ref);
	
	/*
	 * Determine the special parameters for this function: the target
	 * object reference, the environment reference, etc.
	 */
	p_server_func_special_params(ao, &specials);
	
	/*
	 * Determine the total number of function parameters.
	 */
	cast_params_len = ao->params.params_len;
	for (i = 0;
	     i < stub_special_params::number_of_stub_special_param_kinds;
	     ++i)
		if (specials.params[i].index != -1)
			++cast_params_len;
	
	/*
	 * Verify our set of special parameters.
	 */
	for (i = 0;
	     i < stub_special_params::number_of_stub_special_param_kinds;
	     ++i) {
		if (specials.params[i].index != -1) {
			/*
			 * Assert that this special parameter has a valid
			 * CAST type and a valid, unique index.
			 */
			assert(specials.params[i].ctype != 0);
			assert((specials.params[i].index >= 0)
			       && (specials.params[i].index
				   < cast_params_len));
			
			for (j = i + 1;
			     j < stub_special_params::
				 number_of_stub_special_param_kinds;
			     ++j)
				assert(specials.params[i].index
				       != specials.params[j].index);
		}
	}
	
	/* Save the original name context for when we return. */
	old_name = name;
	name = calc_server_func_name(ao->name);
	opname = name;
	
	/*
	 * Now we are ready to start building the presentation PRES_C and CAST
	 * goo.
	 */
	
	cast_init_function_type(&cfunc, cast_params_len);
	
	/*
	 * Build the group of pres_c_inline structures containing the
	 * parameters, starting with level 4 (params struct) then levels
	 * 3, 2, 1 (unions).
	 *
	 * KBF --- the L4 reply inline is no long a struct, it's a UNION of 3
	 * items:
	 * 0  - the old reply params struct
	 * 1  - union of all user defined exceptions that this operation
	 *      supports
	 * -1 - a system exception (out of memory, etc.)
	 */
	pres_c_inline request_l4_inl = pres_c_new_inline_func_params_struct(0);
	pres_c_inline reply_l4_inl = pres_c_new_inline_func_params_struct(0);
	pres_c_inline target_inl = pres_c_new_inline(PRES_C_INLINE_ATOM);
	pres_c_mapping return_mapping;
	cast_type return_ctype;
	pres_c_mapping alloc_return_mapping;
	
	process_server_params(&cfunc,
			      &specials,
			      request_ref, reply_ref,
			      ao,
			      request_l4_inl, reply_l4_inl,
			      target_inl, 0 /* no client */);
	
	/*
	 * Now process the return type.
	 *
	 * An inline atom index of `-1' indicates a return value.
	 * The MINT type of the return value is stored in the last slot of the
	 * reply structure (thus, the hairy expression for the second argument
	 * in the call below).  See `tam_operation_reply_struct' in the file
	 * `aoi_to_mint.c' for more information.
	 *
	 * KBF - This is now in a UNION before the structure...
	 */
	mint_ref reply_struct_ref = (m(reply_ref).mint_def_u.union_def.
				     cases.cases_val[0].var);
	p_server_func_return_type(ao,
				  (m(reply_struct_ref).mint_def_u.struct_def.
				   slots.
				   slots_val[m(reply_struct_ref).mint_def_u.
					    struct_def.slots.slots_len - 1]),
				  &return_ctype,
				  &return_mapping);
	p_server_func_alloc_return(return_ctype,
				   &alloc_return_mapping);
	cfunc.return_type = return_ctype;
	
	p_server_func_make_decl(ai, ao, opname, &cfunc);
	cdef = cast_add_def(&out_pres->stubs_cast,
			    calc_server_func_scoped_name(cur_aoi_idx, opname),
			    CAST_SC_NONE,
			    CAST_FUNC_DECL,
			    ch(cur_aoi_idx, PG_CHANNEL_SERVER_IMPL),
			    current_protection);
	out_pres->stubs_cast.cast_scope_val[cdef].u.
		cast_def_u_u.func_type = cfunc;
	server_func->c_func = cdef;
	
	server_func->op_flags = PRES_C_STUB_OP_FLAG_NONE;
	
	/* Determine if the operation is oneway. */
        if (ao->flags & AOI_OP_FLAG_ONEWAY)
		server_func->op_flags |= PRES_C_STUB_OP_FLAG_ONEWAY;
	
	/*
	 * Allocate and set the return value slot in the PRES_C `reply_l4_inl'.
	 * The MINT type for the return value is always the last slot in the
	 * reply's MINT struct type.
	 *
	 * Additionally, set the request `return_slot' to a PRES_C tree that
	 * will cause the BE to allocate a ``root'' for the return value.
	 */
	reply_l4_inl->
		pres_c_inline_u_u.func_params_i.return_slot
		= ((pres_c_inline_struct_slot *)
		   mustmalloc(sizeof(*(reply_l4_inl->
				       pres_c_inline_u_u.func_params_i.
				       return_slot))));
	reply_l4_inl->
		pres_c_inline_u_u.func_params_i.return_slot->
		mint_struct_slot_index
		= (m(reply_struct_ref).mint_def_u.struct_def.slots.slots_len
		   - 1);
	reply_l4_inl->
		pres_c_inline_u_u.func_params_i.return_slot->
		inl
		= pres_c_new_inline_atom(pres_c_func_return_index,
					 return_mapping);
	
	request_l4_inl->pres_c_inline_u_u.func_params_i.return_slot
		= ((pres_c_inline_struct_slot *)
		   mustmalloc(sizeof(*(request_l4_inl->
				       pres_c_inline_u_u.func_params_i.
				       return_slot))));
	request_l4_inl->
		pres_c_inline_u_u.func_params_i.return_slot->
		mint_struct_slot_index
		= mint_slot_index_null;
	request_l4_inl->
		pres_c_inline_u_u.func_params_i.return_slot->
		inl
		= pres_c_new_inline_atom(pres_c_func_return_index,
					 alloc_return_mapping);
	
	/*
	 * We need to turn the reply into the union of good, bad, & ugly values
	 */
	p_do_return_union(ao, &reply_l4_inl, reply_ref, cdef,
			  specials.params[stub_special_params::
					 environment_ref].index);
	
	/*
	 * level 3
	 *
	 * XXX - might need to modify the operation request code here,
	 * e.g. add a prefix in the CORBA case (perhaps in overriding
	 * function)
	 */
	pres_c_inline request_l3_inl =
		pres_c_new_inline(PRES_C_INLINE_COLLAPSED_UNION);
	request_l3_inl->pres_c_inline_u_u.collapsed_union.discrim_val =
		oper_request;
	request_l3_inl->pres_c_inline_u_u.collapsed_union.selected_case =
		request_l4_inl;
	
	pres_c_inline reply_l3_inl =
		pres_c_new_inline(PRES_C_INLINE_COLLAPSED_UNION);
	reply_l3_inl->pres_c_inline_u_u.collapsed_union.discrim_val =
		oper_reply;
	reply_l3_inl->pres_c_inline_u_u.collapsed_union.selected_case =
		reply_l4_inl;
	
	/* level 2 */
	pres_c_inline request_l2_inl =
		pres_c_new_inline(PRES_C_INLINE_COLLAPSED_UNION);
	request_l2_inl->pres_c_inline_u_u.collapsed_union.discrim_val =
		mint_new_const_from_aoi_const(ai->code);
	request_l2_inl->pres_c_inline_u_u.collapsed_union.selected_case =
		request_l3_inl;
	
	pres_c_inline reply_l2_inl =
		pres_c_new_inline(PRES_C_INLINE_COLLAPSED_UNION);
	reply_l2_inl->pres_c_inline_u_u.collapsed_union.discrim_val =
		mint_new_const_from_aoi_const(ai->code);
	reply_l2_inl->pres_c_inline_u_u.collapsed_union.selected_case =
		reply_l3_inl;
	
	/* level 1 */
	pres_c_inline request_l1_inl =
		pres_c_new_inline(PRES_C_INLINE_COLLAPSED_UNION);
	request_l1_inl->pres_c_inline_u_u.collapsed_union.discrim_val =
		mint_new_const_int((int)ai->idl);
	request_l1_inl->pres_c_inline_u_u.collapsed_union.selected_case =
		request_l2_inl;
	
	pres_c_inline reply_l1_inl =
		pres_c_new_inline(PRES_C_INLINE_COLLAPSED_UNION);
	reply_l1_inl->pres_c_inline_u_u.collapsed_union.discrim_val =
		mint_new_const_int((int)ai->idl);
	reply_l1_inl->pres_c_inline_u_u.collapsed_union.selected_case =
		reply_l2_inl;
	
	/* Assign the unions and param struct to server_func fields. */
	server_func->request_i = request_l1_inl;
	server_func->reply_i = reply_l1_inl;
	
	/* Set the target_i field. */
	server_func->target_i = target_inl;
	server_func->target_itype =
		out_pres->mint.standard_refs.interface_name_ref;
	
	/* Set the client_i field. */
	server_func->client_i = 0;
	server_func->client_itype = mint_ref_null;
	
	/* Set the error_i field. */
	server_func->error_i = 0;
	server_func->error_itype = mint_ref_null;
	
	/* Restore the name context. */
	name = old_name;
	return func;
}


/*****************************************************************************/

/***** Auxiliary functions. *****/

void pg_state::p_server_func_special_params(aoi_operation *ao,
					    stub_special_params *specials)
{
	stub_special_params::stub_param_info *this_param;
	
	int i;
	
	/*
	 * Initialize all of our special parameter data, just in case we fail
	 * to initialize any individual parameter below.
	 */
	for (i = 0;
	     i < stub_special_params::number_of_stub_special_param_kinds;
	     ++i) {
		specials->params[i].spec = 0;
		specials->params[i].name = 0;
		specials->params[i].ctype = 0;
		specials->params[i].index = -1;
	}
	
	/*
	 * Initialize the object reference type and index.  The default is for
	 * the object reference to appear as the first parameter.
	 */
	this_param = &(specials->params[stub_special_params::object_ref]);
	
	this_param->name =
		calc_server_func_object_param_name(ao->name);
	this_param->ctype = cast_new_type_name(
		calc_server_func_object_type_name(ao->name));
	this_param->index = 0;
	
	/*
	 * Initialize the environment reference type and index.  The default is
	 * for the environment reference not to appear.  We set the CAST type
	 * anyway for the possible benefit of derived presentation generators.
	 */
	this_param = &(specials->params[stub_special_params::environment_ref]);
	
	this_param->name =
		/*
		 * XXX --- Don't use `ao->name' until `pg_corba::p_get_
		 * exception_discrim_name' et al. have access to the operation
		 * name, too.
		 */
		calc_server_func_environment_param_name("");
	this_param->ctype = cast_new_type_name(
		/*
		 * XXX --- Don't use `ao->name' until `pg_corba::p_get_env_
		 * struct_type' has access to the operation name, too.
		 */
		calc_server_func_environment_type_name(""));
	this_param->index = -1;
	
	/*
	 * Initialize the (effective) client SID type and index.  The default
	 * is for client SID to appear after all of the normal parameters if
	 * `gen_sids' is true.  Otherwise, the client SID does not appear.
	 */
	this_param = &(specials->params[stub_special_params::client_sid]);
	
	this_param->name =
		calc_server_func_client_sid_param_name(ao->name);
	this_param->ctype = cast_new_type_name(
		calc_server_func_client_sid_type_name(ao->name));
	if (gen_sids)
		this_param->index = (ao->params.params_len + 1);
	else
		this_param->index = -1;
	
	/*
	 * Initialize the required server SID type and index.  The default is
	 * for the required server SID to appear after the client SID if
	 * `gen_sids' is true.  Otherwise, the required server SID does not
	 * appear.
	 */
	this_param = &(specials->params[stub_special_params::
				       required_server_sid]);
	
	this_param->name =
		calc_server_func_required_server_sid_param_name(ao->name);
	this_param->ctype = cast_new_type_name(
		calc_server_func_server_sid_type_name(ao->name));
	if (gen_sids)
		this_param->index = (ao->params.params_len + 2);
	else
		this_param->index = -1;
	
	/*
	 * Initialize the actual server SID type and index.  The default is for
	 * the actual server SID not to appear --- even if `gen_sids' is
	 * specified.  The underlying transport system, not the server itself,
	 * is responsible for supplying the server's actual SID to the client.
	 */
	this_param = &(specials->params[stub_special_params::
				       actual_server_sid]);
	
	this_param->name =
		calc_server_func_actual_server_sid_param_name(ao->name);
	this_param->ctype = cast_new_type_name(
		calc_server_func_server_sid_type_name(ao->name));
	this_param->index = -1;
	
	/* Finally, we're done! */
}

/*
 * This method determines how `p_server_func' processes the return type of a
 * server work function.  Some presentation generators override this method.
 */
void pg_state::p_server_func_return_type(aoi_operation *ao, mint_ref /*mr*/,
					 cast_type *out_ctype,
					 pres_c_mapping *out_mapping)
{
	p_type_collection *ptc = 0;
	p_type_node *ptn;
	
	/* Compute the basic C type and PRES_C mapping. */
	p_type(ao->return_type, &ptc);
	ptn = ptc->find_type("definition");
	*out_ctype = ptn->get_type();
	*out_mapping = ptn->get_mapping();
	/* Tell the back end that this is the ``root'' of the parameter. */
	pres_c_interpose_param_root(out_mapping, 0, 0);
	/* Tell the back end that this is a return parameter. */
	pres_c_interpose_direction(out_mapping, AOI_DIR_RET);
}

/*
 * This method determines how `p_server_func' will add PRES_C nodes to the
 * request PRES_C tree in order to cause the back end to allocate and perhaps
 * initialize storage for the return value of an operation.
 */
void pg_state::p_server_func_alloc_return(cast_type return_ctype,
					  pres_c_mapping *out_mapping)
{
	cast_type actual_ctype;
	cast_expr zero_cexpr;
	
	/* ``Dereference'' the type, just to be paranoid. */
	actual_ctype
		= cast_find_typedef_type(
			((cast_scope *) top_ptr(scope_stack)),
			return_ctype);
	if (!actual_ctype)
		/* Can't look it up?  Use the `return_ctype' then. */
		actual_ctype = return_ctype;
	
	if (actual_ctype->kind == CAST_TYPE_VOID) {
		/*
		 * Special case: the return type is `void'.  The server should
		 * not allocate storage for the void return value.
		 */
		*out_mapping = pres_c_new_mapping(PRES_C_MAPPING_IGNORE);
		return;
	}
	
	/*
	 * XXX --- Logic stolen from return-value initialization code that used
	 * to be in `mu_state::mu_server_func'.  I copied the logic in order to
	 * reduce the code changes due to the new `PRES_C_MAPPING_PARAM_ROOT'
	 * scheme.  But really, this logic is incomplete.  We should either
	 * steal the more complete logic from `cast_new_expr_assign_to_zero',
	 * or we should bag initialization altogther.
	 *
	 * XXX --- Another problem: the PG is oblivious to any transformations
	 * that the BE may apply to the return type.  So initializing the
	 * return value is suspect in any case.
	 */
	/*
	 * XXX --- If the return type is integer-like or a pointer,
	 * initialize `_return' to zero.  Until we get real error
	 * handling this is better than nothing.
	 */
	zero_cexpr = 0;
	switch (actual_ctype->kind) {
	case CAST_TYPE_PRIMITIVE:
		switch (actual_ctype->cast_type_u_u.primitive_type.kind) {
		case CAST_PRIM_CHAR:
			zero_cexpr = cast_new_expr_lit_char(0, 0);
			break;
		case CAST_PRIM_INT:
			zero_cexpr = cast_new_expr_lit_int(0, 0);
			break;
		case CAST_PRIM_FLOAT:
			zero_cexpr = cast_new_expr_lit_float(0.0);
			break;
		case CAST_PRIM_DOUBLE:
			zero_cexpr = cast_new_expr_lit_double(0.0, 0);
			break;
		default:
			panic("In `pg_state::p_server_func_alloc_return', "
			      "unrecognized CAST primitive type.");
			break;
		}
		break;
		
	case CAST_TYPE_POINTER:
		zero_cexpr = cast_new_expr_lit_int(0, 0);
		break;
		
	default:
		/* Do not initialize the return value. */
		break;
	}
	
	/* Make the basic mapping: initialize the value or do nothing. */
	if (zero_cexpr) {
		*out_mapping = pres_c_new_mapping(PRES_C_MAPPING_INITIALIZE);
		(*out_mapping)->pres_c_mapping_u_u.initialize.value
			= zero_cexpr;
	} else {
		*out_mapping = pres_c_new_mapping(PRES_C_MAPPING_IGNORE);
	}
	
	/* Tell the back end that this is the ``root'' of the parameter. */
	pres_c_interpose_param_root(out_mapping, 0, 0);
	/* Tell the back end that this is a return parameter. */
	pres_c_interpose_direction(out_mapping, AOI_DIR_RET);
}

/*
 * This function digs through MINT to find the MINT references that we need in
 * order to make a server work function.
 *
 * XXX --- The technique for finding these things is gross; we ought to have
 * these MINT references *handed* to us.
 */
void pg_state::p_server_func_find_refs(aoi_interface *a,
				       aoi_operation * /*ao*/,
				       mint_const oper_request_discrim,
				       mint_const oper_reply_discrim,
				       /* OUT */ mint_ref *request_ref,
				       /* OUT */ mint_ref *reply_ref)
{
	mint_ref interface_ref;
	mint_const interface_discrim;
	u_int number_of_interfaces;
	
	u_int i;
	
	/* `interface_ref' starts at the top level MINT union. */
	interface_ref = top_union;
	
	/*
	 * Descend past the IDL type union.
	 * XXX --- This is assuming only one case at the 'IDL type' union.
	 */
	interface_ref = m(interface_ref).mint_def_u.union_def.cases.
			cases_val->var;
	
	/* Now we need to find the current interface value. */
	interface_discrim = mint_new_const_from_aoi_const(a->code);
	number_of_interfaces = m(interface_ref).mint_def_u.union_def.
			       cases.cases_len;
	
	for (i = 0; i < number_of_interfaces; ++i) {
		if (!mint_const_cmp(interface_discrim,
				    (m(interface_ref).mint_def_u.union_def.
				     cases.cases_val[i].val))) {
			interface_ref = m(interface_ref).mint_def_u.
					union_def.cases.cases_val[i].var;
			break;
		}
	}
	if (i >= number_of_interfaces)
		panic("In `pg_state::p_server_func_find_refs', "
		      "can't find interface MINT type.");
	
	/*
	 * Once we've found the interface, finding the request and reply types
	 * is easy.
	 */
	*request_ref = mint_find_union_case(&(out_pres->mint), interface_ref,
					    oper_request_discrim);
	*reply_ref = mint_find_union_case(&(out_pres->mint), interface_ref,
					  oper_reply_discrim);
	
	if (*request_ref == mint_ref_null)
		panic("In `pg_state::p_server_func_find_refs', "
		      " can't find operation request MINT type.");
	if (*reply_ref == mint_ref_null)
		panic("In `pg_state::p_server_func_find_refs', "
		      " can't find operation reply MINT type.");
}

/* End of file. */

