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

#include <assert.h>
#include <string.h>

#include <mom/compiler.h>
#include <mom/libaoi.h>
#include <mom/libmint.h>
#include <mom/c/pfe.hh>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include "private.hh"

/*****************************************************************************/

void pg_state::p_client_stubs(aoi_interface * /*ai*/)
{
	p_client_stubs_internal(cur_aoi_idx, cur_aoi_idx);
}

void pg_state::p_client_stubs_internal(aoi_ref this_ref, aoi_ref derived_ref)
{
	aoi_interface *this_interface, *derived_interface;
	
	aoi_ref saved_parent_ref, saved_derived_ref;
	
	u_int ops_len;
#ifdef ATTRIBUTE_STUBS
	u_int attribs_len;
#endif
	u_int parents_len;
	
	u_int i;
	
	int flags;
	
	/*
	 * Set `this_interface' and `derived_interface' to point at the AOI
	 * interfaces under consideration.
	 */
	assert((this_ref >= 0)
	       && (this_ref < ((aoi_ref) in_aoi->defs.defs_len)));
	assert(a(this_ref).binding
	       && (a(this_ref).binding->kind == AOI_INTERFACE));
	
	this_interface = &(a(this_ref).binding->aoi_type_u_u.interface_def);
	
	assert((derived_ref >= 0)
	       && (derived_ref < ((aoi_ref) in_aoi->defs.defs_len)));
	assert(a(derived_ref).binding
	       && (a(derived_ref).binding->kind == AOI_INTERFACE));
	
	derived_interface = &(a(derived_ref).binding->aoi_type_u_u.
			      interface_def);
	
	/*
	 * Set the `pg_state' interface reference data members to point at the
	 * interfaces under consideration so that `pg_state::calc_name' can
	 * compute names.
	 */
	saved_parent_ref = parent_interface_ref;
	saved_derived_ref = derived_interface_ref;
	
	parent_interface_ref = this_ref;
	derived_interface_ref = derived_ref;
	
	/* Get the lengths of arrays that are part of `this_interface'. */
	ops_len = this_interface->ops.ops_len;
#ifdef ATTRIBUTE_STUBS
	attribs_len = this_interface->attribs.attribs_len;
#endif
	parents_len = this_interface->parents.parents_len;
	
	/* Generate the send message function for asynchronous messages. */
	if (async_stubs && derived_interface == this_interface) {
		p_send_stub(derived_interface, 1 /* request */);
		p_send_stub(derived_interface, 0 /* reply */);
		p_continue_func_types(derived_interface);
	}
	
	/*
	 * Generate the stubs for the operations defined by `this_interface',
	 * which are inherited by `derived_interface'.
	 */
	for (i = 0; i < ops_len; ++i) {
		aoi_operation *ao = &(this_interface->ops.ops_val[i]);
		
		/*
		 * We must check if we have already generated a stub for this
		 * (interface, operation) pair.  This may occur, for example,
		 * when `this_interface' is inherited through multiple paths
		 * to `derived_interface'.
		 */
		flags = p_interface_table_get_value(derived_interface, ao);
		
		if (!(flags & P_CLIENT_STUB_OP_MARK)) {
			p_interface_table_set_value(derived_interface, ao,
						    (flags
						     | P_CLIENT_STUB_OP_MARK));
			if (async_stubs) {
				p_message_marshal_stub(derived_interface, ao,
						       1 /* client */,
						       1 /* request */);
				p_message_marshal_stub(derived_interface, ao,
						       1 /* client */,
						       0 /* reply */);
				p_receive_func(derived_interface, ao,
					       0 /* reply */);
				p_continue_stub(derived_interface, ao,
						0 /* reply */);
			} else
				p_client_stub(derived_interface, ao);
		}
	}
	
#ifdef ATTRIBUTE_STUBS
	/*
	 * Generate the stubs for the attributes defined by `this_interface',
	 * which are inherited by `derived_interface'.
	 */
	for (i = 0; i < attribs_len; ++i) {
		aoi_attribute *aa = &(this_interface->attribs.attribs_val[i]);
		
		/*
		 * As above, we must screen out attributes that we have already
		 * processed.
		 */
		flags = p_interface_table_get_value(derived_interface, aa);
		
		if (!(flags & P_CLIENT_STUB_OP_MARK)) {
			p_interface_table_set_value(derived_interface, aa,
						    (flags
						     | P_CLIENT_STUB_OP_MARK));
			p_client_attrib_stub(derived_interface, aa);
		}
	}
#endif /* ATTRIBUTE_STUBS */
	
	/*
	 * Generate the stubs for the operations and attributes that are
	 * inherited from parents of `this_interface'.
	 */
	for (i = 0; i < parents_len; ++i) {
		aoi_type parent_val = this_interface->parents.parents_val[i];
		aoi_ref  parent_ref;
		aoi_interface *parent_interface;
		
		/*
		 * All parent references must be through indirects so that
		 * we can find the name to go with the parent interface!
		 */
		assert(parent_val->kind == AOI_INDIRECT);
		
		parent_ref = aoi_deref_fwd(in_aoi, parent_val->aoi_type_u_u.indirect_ref);
		
		parent_interface = &(a(parent_ref).binding->aoi_type_u_u.
				     interface_def);
		
		/*
		 * As above, we check to see if we've already processed this
		 * parent.  (If we didn't do this check we would still generate
		 * correct code because all of the parent's operations and
		 * attributes are marked separately.  But doing this check can
		 * save us time.)
		 */
		flags = p_interface_table_get_value(derived_interface,
						    parent_interface);
		if (!(flags & P_CLIENT_STUB_PARENT_MARK)) {
			p_interface_table_set_value(
				derived_interface,
				parent_interface,
				(flags | P_CLIENT_STUB_PARENT_MARK));
			p_client_stubs_internal(parent_ref,
						derived_ref);
		}
	}
	
	/*
	 * Finally, restore the `pg_state' AOI interface references that we
	 * changed previously.
	 */
	parent_interface_ref = saved_parent_ref;	  
	derived_interface_ref = saved_derived_ref;	  
}


/*****************************************************************************/

/*
 * Generate a client stub presentation for an AOI interface operation.
 *
 * Note that `ao' may not be defined in `ai', but rather in some parent
 * interface of `ai'.
 */
void pg_state::p_client_stub(aoi_interface *ai, aoi_operation *ao)
{
	pres_c_client_stub *cstub;
	int newstub;
	
	char *old_name;
	char *opname;
	int cast_params_len;
	int cr, cdef;
	cast_func_type cfunc;
	
	stub_special_params specials;
	int i, j;
	
	mint_ref request_ref;
	mint_ref reply_ref;
	
	mint_const oper_request;
	mint_const oper_reply;

	cast_scope *scope = (cast_scope *)top_ptr(scope_stack);
	
	/*****/
	
	/*
	 * Some presentation styles dictate that client stubs are created only
	 * for operations that are *defined* within an interface, not for
	 * operations that are *inherited* from parent interfaces.  In such a
	 * presentation, one would invoke the parent's client stub to invoke an
	 * inherited operation on an instance of a derived object type.
	 */
	if ((parent_interface_ref != derived_interface_ref)
	    && !client_stubs_for_inherited_operations)
		return;
	
	if(!lookup_interface_mint_discrims(ai, ao,
					   &oper_request, &oper_reply)) {
		panic("In `pg_state::p_client_stub', "
		      "cannot find MINT request and reply discriminators.");
	}
	
	/*
	 * Find the MINT references to the request and reply types.
	 */
	p_client_stub_find_refs(ai, ao, oper_request, oper_reply,
				&request_ref,
				&reply_ref);
	
	/*
	 * Determine the special parameters for this stub: the target object
	 * reference, the environment reference, etc.
	 */
	p_client_stub_special_params(ao, &specials);
	
	/*
	 * Determine the total number of stub parameters.
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
	name = calc_client_stub_name(ao->name);
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
	pres_c_mapping return_type_mapping;
	cast_type out_ctype_temp;
	
	process_client_params(&cfunc,
			      &specials,
			      request_ref, reply_ref,
			      ao,
			      request_l4_inl, reply_l4_inl,
			      target_inl, 0 /* no client inline */);
	
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
	p_client_stub_return_type(ao,
				  (m(reply_struct_ref).mint_def_u.struct_def.
				   slots.
				   slots_val[m(reply_struct_ref).mint_def_u.
					    struct_def.slots.slots_len - 1]),
				  &out_ctype_temp,
				  &return_type_mapping);
	cfunc.return_type = out_ctype_temp;
	
	/*
	 * Allocate a pres_c_stub for this operation --- UNLESS we have already
	 * created an appropriate client stub.
	 *
	 * Perhaps we are inheriting the current AOI operation from a parent
	 * interface and the user has changed the rule for creating stub names
	 * so that we now have a name conflict.  (In this case, however, the
	 * user should instead tweak the flag that controls the creation of
	 * client stubs for inherited operations.)
	 *
	 * XXX --- We should be more careful about this check.  Checking just
	 * the name is bad news.  We must check for MINT and PRES_C equality as
	 * well!
	 */
	
	cast_scope *deep_scope = scope;
	if (!(ao->flags & (AOI_OP_FLAG_SETTER|AOI_OP_FLAG_GETTER)) &&
	    (cast_find_def(&deep_scope,
			  cast_new_scoped_name(name, NULL),
			  CAST_FUNC_DECL)
	     >= 0)) {
		/*
		 * XXX --- Check that our MINT and PRES_C is equal to that of
		 * the existing client stub.  (Hell, check that `name' refers
		 * to a client stub and not some typedef or something else!)
		 *
		 * In lieu of being smart, warn the user.
		 */
		warn("Suppressing extra definition for client stub `%s'.",
		     name);
		
		/* Restore the name context. */
		name = old_name;
		return;
	}
	
	newstub = p_add_stub(out_pres);
	s(newstub).kind = PRES_C_CLIENT_STUB;
	cstub = &s(newstub).pres_c_stub_u.cstub;
	
	cstub->op_flags = PRES_C_STUB_OP_FLAG_NONE;
	
	/* Determine if the operation is oneway. */
        if (ao->flags & AOI_OP_FLAG_ONEWAY)
		cstub->op_flags |= PRES_C_STUB_OP_FLAG_ONEWAY;

	/*
	 * Set request_itype and reply_itype to the top-level "mom_msg" MINT
	 * union.
	 */
	cstub->request_itype = top_union;
	cstub->reply_itype = top_union;
	
	cfunc.spec = client_func_spec;
	deep_scope = scope;
	cast_scoped_name scn = cast_new_scoped_name(opname, NULL);
	if( ((ao->flags & (AOI_OP_FLAG_SETTER|AOI_OP_FLAG_GETTER)) ||
	     (cr = cast_find_def(&deep_scope,
				 scn,
				 CAST_FUNC_DECL)) == -1) ) {
		cr = cast_add_def(scope,
				  scn,
				  CAST_SC_NONE,
				  CAST_FUNC_DECL,
				  ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
				  current_protection);
		scope->cast_scope_val[cr].u.cast_def_u_u.func_type = cfunc;
		scope->cast_scope_val[cr].u.cast_def_u_u.func_type.spec =
			client_func_spec;
	}
	cfunc.spec &= ~CAST_FUNC_VIRTUAL;
	cdef = cast_add_def(&out_pres->stubs_cast,
			    calc_client_stub_scoped_name(cur_aoi_idx, opname),
			    CAST_SC_NONE,
			    CAST_FUNC_DECL,
			    ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
			    current_protection);
	out_pres->stubs_cast.cast_scope_val[cdef].
		u.cast_def_u_u.func_type = cfunc;
	cstub->c_func = cdef;
	
	/*
	 * Allocate and set the return value slot in the PRES_C `reply_l4_inl'.
	 * The MINT type for the return value is always the last slot in the
	 * reply's MINT struct type.
	 *
	 * Additionally, initialize the request `return_slot' to null.
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
					 return_type_mapping);
	
	request_l4_inl->pres_c_inline_u_u.func_params_i.return_slot = 0;
	
	/*
	 * We need to turn the reply into the union of good, bad, & ugly values
	 */
	p_do_return_union(ao, &reply_l4_inl, reply_ref, cr,
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
	
	/* Assign the unions and param struct to cstub fields. */
	cstub->request_i = request_l1_inl;
	cstub->reply_i = reply_l1_inl;
	
	/* Set the target_i field. */
	cstub->target_i = target_inl;
	cstub->target_itype = out_pres->mint.standard_refs.interface_name_ref;
	
	/* Set the client_i field. */
	cstub->client_i = 0;
	cstub->client_itype = mint_ref_null;
	
	/* Set the error_i field. */

	/* XXX until exception handling is implemented */
	cstub->error_i = reply_l1_inl;
	cstub->error_itype = top_union;
	
	/* Restore the name context. */
	name = old_name;
}


/*****************************************************************************/

/***** Auxiliary functions. *****/

void pg_state::p_client_stub_special_params(aoi_operation *ao,
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
		calc_client_stub_object_param_name(ao->name);
	this_param->ctype = cast_new_type_name(
		calc_client_stub_object_type_name(ao->name));
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
		calc_client_stub_environment_param_name("");
	this_param->ctype = cast_new_type_name(
		/*
		 * XXX --- Don't use `ao->name' until `pg_corba::p_get_env_
		 * struct_type' has access to the operation name, too.
		 */
		calc_client_stub_environment_type_name(""));
	this_param->index = -1;
	
	/*
	 * Initialize the (effective) client SID type and index.  The default
	 * is for client SID to appear after all of the normal parameters if
	 * `gen_sids' is true.  Otherwise, the client SID does not appear.
	 */
	this_param = &(specials->params[stub_special_params::client_sid]);
	
	this_param->name =
		calc_client_stub_client_sid_param_name(ao->name);
	this_param->ctype = cast_new_type_name(
		calc_client_stub_client_sid_type_name(ao->name));
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
		calc_client_stub_required_server_sid_param_name(ao->name);
	this_param->ctype = cast_new_type_name(
		calc_client_stub_server_sid_type_name(ao->name));
	if (gen_sids)
		this_param->index = (ao->params.params_len + 2);
	else
		this_param->index = -1;
	
	/*
	 * Initialize the actual server SID type and index.  The default is for
	 * the actual server SID to appear after the required server SID if
	 * `gen_sids' is true.  Otherwise, the actual server SID does not
	 * appear.
	 */
	this_param = &(specials->params[stub_special_params::
				       actual_server_sid]);
	
	this_param->name =
		calc_client_stub_actual_server_sid_param_name(ao->name);
	this_param->ctype = cast_new_type_name(
		calc_client_stub_server_sid_type_name(ao->name));
	if (gen_sids)
		this_param->index = (ao->params.params_len + 3);
	else
		this_param->index = -1;
	
	/* Finally, we're done! */
}

/*
 * This method determines how `p_client_stub' processes the return type of a
 * client stub.  Some presentation generators override this method.
 */
void pg_state::p_client_stub_return_type(aoi_operation *ao, mint_ref /*mr*/,
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
 * This function digs through MINT to find the MINT references that we need in
 * order to make a client stub.
 *
 * XXX --- The technique for finding these things is gross; we ought to have
 * these MINT references *handed* to us.
 */
void pg_state::p_client_stub_find_refs(aoi_interface *a,
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
		panic("In `pg_state::p_client_stub_find_refs', "
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
		panic("In `pg_state::p_client_stub_find_refs', "
		      " can't find operation request MINT type.");
	if (*reply_ref == mint_ref_null)
		panic("In `pg_state::p_client_stub_find_refs', "
		      " can't find operation reply MINT type.");
}

/* End of file. */

