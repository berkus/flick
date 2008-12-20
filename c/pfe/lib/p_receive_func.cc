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

#include <string.h>
#include <assert.h>
#include <mom/compiler.h>
#include <mom/libaoi.h>
#include <mom/libmint.h>
#include <mom/c/pfe.hh>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include "private.hh"

/*
 * Generate a server work function presentation for an AOI interface operation.
 */
pres_c_func pg_state::p_receive_func(aoi_interface *ai,
				     aoi_operation *ao,
				     int request)
{
	pres_c_func func;
	func.kind = PRES_C_RECEIVE_FUNC;
	pres_c_receive_func *receive_func = &func.pres_c_func_u.rfunc;
	
	char *old_name;
	char *opname;
	int cast_params_len;
	int cr, cdef;
	cast_func_type cfunc;
	
	stub_special_params specials;
	int i, j;
	
	mint_ref msg_ref;
	
	mint_const oper_request;
	mint_const oper_reply;
	
	cast_scope *scope = (cast_scope *)top_ptr(scope_stack);
	
	/*****/
	
        if (ao->flags & AOI_OP_FLAG_ONEWAY && !request) {
		/* Oneway operations do not need reply stubs */
		func.kind = (pres_c_func_kind) -1; /* invalid stub kind */
		return func;
	}
	
	if(!lookup_interface_mint_discrims(ai, ao,
					   &oper_request, &oper_reply)) {
		panic("In `pg_state::p_receive_func', "
		      "cannot find MINT request and reply discriminators.");
	}
	
	/*
	 * Determine the special parameters for this function: the target
	 * object reference, the environment reference, etc.
	 */
	p_receive_func_special_params(ao, &specials, request);
	
	/*
	 * Determine the total number of function parameters.
	 */
	cast_params_len = 0;
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
	if (request)
		name = calc_receive_request_func_name(ao->name);
	else
		name = calc_receive_reply_func_name(ao->name);
	opname = name;
	
	/* Construct MINT for this function */
	p_receive_func_make_refs(ai, ao,
				 (request ? oper_request : oper_reply),
				 request, &msg_ref);
	
	/*
	 * Now we are ready to start building the presentation PRES_C and CAST
	 * goo.
	 */
	
	cast_init_function_type(&cfunc, cast_params_len);
	
	/*
	 * Build the group of pres_c_inline structures containing the
	 * parameters, starting with level 4 (params struct) then levels
	 * 3, 2, 1 (unions).
	 */
	pres_c_inline l4_inl
		= pres_c_new_inline(PRES_C_INLINE_HANDLER_FUNC);
	pres_c_inline target_inl = pres_c_new_inline(PRES_C_INLINE_ATOM);
	pres_c_inline client_inl = pres_c_new_inline(PRES_C_INLINE_ATOM);
	
	process_async_params(&cfunc,
			     &specials,
			     msg_ref,// reply_ref,
			     ao,
			     l4_inl,// reply_l4_inl,
			     target_inl, client_inl,
			     0 /* decode */, 1 /* request */);
	
	/* Now process the return type. */
	cfunc.return_type = cast_new_type(CAST_TYPE_VOID);
	
	cast_scope *deep_scope = scope;
	cast_scoped_name scn = cast_new_scoped_name(opname, NULL);
	if( (cr = cast_find_def(&deep_scope,
				scn,
				CAST_FUNC_DECL)) == -1 ) {
		cr = cast_add_def(scope,
				  scn,
				  CAST_SC_NONE,
				  CAST_FUNC_DECL,
				  ch(cur_aoi_idx, PG_CHANNEL_SERVER_DECL),
				  current_protection);
		scope->cast_scope_val[cr].u.cast_def_u_u.func_type = cfunc;
	}
	cdef = cast_add_def(
		&out_pres->stubs_cast,
		request ? calc_receive_request_func_scoped_name(
			cur_aoi_idx, opname) :
		calc_receive_reply_func_scoped_name(
			cur_aoi_idx, opname),
		CAST_SC_NONE,
		CAST_FUNC_DECL,
		ch(cur_aoi_idx, PG_CHANNEL_SERVER_IMPL),
		current_protection);
	out_pres->stubs_cast.cast_scope_val[cdef].u.
		cast_def_u_u.func_type = cfunc;
	receive_func->c_func = cdef;

	/* Receive funcs are essentailly oneways */
	receive_func->op_flags = PRES_C_STUB_OP_FLAG_ONEWAY;
	
	/*
	 * level 3
	 *
	 * XXX - might need to modify the operation request code here,
	 * e.g. add a prefix in the CORBA case (perhaps in overriding
	 * function)
	 */
	pres_c_inline l3_inl =
		pres_c_new_inline(PRES_C_INLINE_COLLAPSED_UNION);
	l3_inl->pres_c_inline_u_u.collapsed_union.discrim_val =
		(request ? oper_request : oper_reply);
	l3_inl->pres_c_inline_u_u.collapsed_union.selected_case =
		l4_inl;
	
	/* level 2 */
	pres_c_inline l2_inl =
		pres_c_new_inline(PRES_C_INLINE_COLLAPSED_UNION);
	l2_inl->pres_c_inline_u_u.collapsed_union.discrim_val =
		mint_new_const_from_aoi_const(ai->code);
	l2_inl->pres_c_inline_u_u.collapsed_union.selected_case =
		l3_inl;
	
	/* level 1 */
	pres_c_inline l1_inl =
		pres_c_new_inline(PRES_C_INLINE_COLLAPSED_UNION);
	l1_inl->pres_c_inline_u_u.collapsed_union.discrim_val =
		mint_new_const_int((int)ai->idl);
	l1_inl->pres_c_inline_u_u.collapsed_union.selected_case =
		l2_inl;
	
	/* Assign the unions and param struct to server_func fields. */
	receive_func->msg_i = l1_inl;
	/*
	 * The original MINT (from the server skel) will take care of
	 * the top inlines.  Here we specify the MINT for the
	 * decomposed message.
	 */
	receive_func->simple_msg_itype = msg_ref;
	
	/* Set the target_i field. */
	receive_func->target_i = target_inl;
	receive_func->target_itype =
		out_pres->mint.standard_refs.interface_name_ref;
	
	/* Set the client_i field. */
	receive_func->client_i = client_inl;
	receive_func->client_itype =
		out_pres->mint.standard_refs.interface_name_ref;
	
	/* Set the error_i field. */
	receive_func->error_i = 0;
	receive_func->error_itype = mint_ref_null;
	
	/* Restore the name context. */
	name = old_name;
	return func;
}


/*****************************************************************************/

/***** Auxiliary functions. *****/

int pg_state::p_receive_func_special_params(aoi_operation *ao,
					    stub_special_params *specials,
					    int request)
{
	int newparms = 0;
	stub_special_params::stub_param_info *this_param;
//	char *name = getscopedname(cur_aoi_idx);
	
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
	
	if (request) {
		/* Request Send Stub has target object as first param */
		/*
		 * Initialize the object reference type and index.  The default
		 * is for the object reference not to appear.  We set the CAST
		 * type anyway for the possible benefit of derived presentation
		 * generators.
		 */
		this_param = &(specials->params[stub_special_params
					       ::object_ref]);
		
		this_param->name =
			calc_client_stub_object_param_name(ao->name);
		this_param->ctype = cast_new_type_name(
			calc_client_stub_object_type_name(ao->name));
		this_param->index = newparms;
		newparms++;
	} else {
		/* Reply Send Stub has client object as first param */
		/*
		 * Initialize the invocation identifier type and index.
		 * This is only for message send and receive stubs.
		 */
		this_param = &(specials->params[stub_special_params::
					       client_ref]);
		
		this_param->name = "client";
		this_param->ctype
			= cast_new_type_name(
				calc_stub_client_ref_type_name(ao->name));
		this_param->index = newparms;
		newparms++;
	}
	
	/*
	 * Initialize the message reference type and index.
	 * This is only for message unmarshal, send, and receive stubs,
	 * since the normal presentation never passes around messages.
	 */
	this_param = &(specials->params[stub_special_params::message_ref]);
	
	this_param->name = "_msg";
	if (request) {
		this_param->ctype
			= cast_new_type_name(
				calc_message_request_type_name(ao->name));
	} else {
		this_param->ctype
			= cast_new_type_name(
				calc_message_reply_type_name(ao->name));
	}
	this_param->index = newparms;
	newparms++;
	
	/*
	 * Initialize the invocation identifier type and index.
	 * This is only for message send and receive stubs.
	 */
	this_param = &(specials->params[stub_special_params::invocation_ref]);
	
	this_param->name = "inv_id";
	this_param->ctype
		= cast_new_type_name(calc_stub_invocation_id_type_name(ao->name));
	this_param->index = newparms;
	newparms++;
	
	if (request) {
		/* Request Send Stub has client object as fourth param */
		/*
		 * Initialize the invocation identifier type and index.
		 * This is only for message send and receive stubs.
		 */
		this_param = &(specials->params[stub_special_params::
					       client_ref]);
		
		this_param->name = "client";
		this_param->ctype
			= cast_new_type_name(
				calc_stub_client_ref_type_name(ao->name));
		this_param->index = newparms;
		newparms++;
	} else {
		/* Reply Send Stub has target object as fourth param */
		/*
		 * Initialize the object reference type and index.  The default
		 * is for the object reference not to appear.  We set the CAST
		 * type anyway for the possible benefit of derived presentation
		 * generators.
		 */
		this_param = &(specials->params[stub_special_params
					       ::object_ref]);
		
		this_param->name =
			calc_client_stub_object_param_name(ao->name);
		this_param->ctype = cast_new_type_name(
			calc_client_stub_object_type_name(ao->name));
		this_param->index = newparms;
		newparms++;
	}
	
	/*
	 * Initialize the environment reference type and index.  The default is
	 * for the environment reference not to appear.  We set the CAST type
	 * anyway for the possible benefit of derived presentation generators.
	 */
	this_param = &(specials->params[stub_special_params::environment_ref]);
	
	this_param->name =
		calc_client_stub_environment_param_name("");
	this_param->ctype = cast_new_type_name(
		calc_client_stub_environment_type_name(""));
	this_param->index = -1;
	
	/* Finally, we're done! */
	return newparms;
}

/*
 * This function digs through MINT to find the MINT references that we need in
 * order to make a server work function.
 *
 * XXX --- The technique for finding these things is gross; we ought to have
 * these MINT references *handed* to us.
 */
void pg_state::p_receive_func_make_refs(aoi_interface *a,
					aoi_operation * /*ao*/,
					mint_const oper_discrim,
					int /*request*/,
					/* OUT */ mint_ref *m_ref)
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
		panic("In `pg_state::p_receive_func_make_refs', "
		      "can't find interface MINT type.");
	
	/*
	 * Once we've found the interface, we make a copy of the union
	 * of operations.
	 * 
	 * The receive_func stilll needs to be connected with the original
	 * request and reply MINT (since the message is the same), but
	 * it can't use this to decode the information it needs to
	 * present it as a decomposed stub.  Thus, we make a new union
	 * of operations (with the operation we want as the only case),
	 * and will later link it directly the the receive_func structure.
	 */
	assert(m(interface_ref).kind == MINT_UNION);
	mint_ref u_ref = mint_add_def(&out_pres->mint);
	*m_ref = mint_add_def(&out_pres->mint);
	mint_union_def *udef = &m(interface_ref).mint_def_u.union_def;

	m(u_ref).kind = MINT_UNION;
	m(u_ref).mint_def_u.union_def.discrim = udef->discrim;
	m(u_ref).mint_def_u.union_def.dfault = udef->dfault;
	m(u_ref).mint_def_u.union_def.cases.cases_len = 1;
	m(u_ref).mint_def_u.union_def.cases.cases_val
		= (mint_union_case *)
		mustmalloc(sizeof(mint_union_case));
	m(u_ref).mint_def_u.union_def.cases.cases_val[0].val
		= oper_discrim;
	m(u_ref).mint_def_u.union_def.cases.cases_val[0].var
		= *m_ref;
	m(*m_ref).kind = MINT_STRUCT;
	m(*m_ref).mint_def_u.struct_def.slots.slots_len = 0;
	m(*m_ref).mint_def_u.struct_def.slots.slots_val = 0;
}

/* End of file. */

