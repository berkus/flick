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
#include <string.h>

#include <mom/compiler.h>
#include <mom/libaoi.h>
#include <mom/libmint.h>
#include <mom/c/pfe.hh>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include "private.hh"

/*****************************************************************************/

/*
 * Generate a message marshal stub presentation for an AOI interface
 * operation. `request' specifies either the request (1) or reply (0).
 *
 * Note that `ao' may not be defined in `ai', but rather in some parent
 * interface of `ai'.
 */
void pg_state::p_message_marshal_stub(aoi_interface *ai, aoi_operation *ao,
				      int client, int request)
{
	pres_c_msg_marshal_stub *mmstub;
	pres_c_msg_marshal_stub *mustub;
	
	int newstub;
	int cr, cdef;
	
	char *old_name;
	char *opname;
	char *optype;
	int cast_params_len;
	cast_func_type cfunc;
	
	stub_special_params specials;
	int i, j;
	
	mint_ref request_ref;
	mint_ref reply_ref;
	mint_ref m_ref;
	
	mint_const oper_request;
	mint_const oper_reply;
	
	cast_scope *scope = (cast_scope *)top_ptr(scope_stack);
	
	/*****/
	
        if (ao->flags & AOI_OP_FLAG_ONEWAY && !request) {
		/* Oneway operations do not need reply stubs */
		return;
	}
	
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
		panic("In `pg_state::p_message_marshal_stub', "
		      "cannot find MINT request and reply discriminators.");
	}
	
	/*
	 * Find the MINT references to the request and reply types.
	 */
	p_client_stub_find_refs(ai, ao, oper_request, oper_reply,
				&request_ref,
				&reply_ref);
	if (request)
		m_ref = request_ref;
	else
		m_ref = reply_ref;
	
	/*
	 * Determine the special parameters for this stub: the target object
	 * reference, the environment reference, etc.
	 */
	p_msg_marshal_stub_special_params(ao, &specials, client, request);
	
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
	int encode;
	if (request) {
		if (client) {
			opname = calc_message_marshal_request_stub_name(
				ao->name);
			encode = 1;
		} else {
			opname = calc_message_unmarshal_request_stub_name(
				ao->name);
			encode = 0;
		}
		optype = calc_message_request_type_name(ao->name);
	} else {
		if (client) {
			opname = calc_message_unmarshal_reply_stub_name(
				ao->name);
			encode = 0;
		} else {
			opname = calc_message_marshal_reply_stub_name(
				ao->name);
			encode = 1;
		}
		optype = calc_message_reply_type_name(ao->name);
	}
	name = opname;
	
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
	pres_c_inline l4_inl = pres_c_new_inline_func_params_struct(0);
	/*
	 * Initialize the `return_slot' to null.  This will be filled in
	 * appropriately by process_async_params() when doing the reply.
	 */
	l4_inl->pres_c_inline_u_u.func_params_i.return_slot = 0;
	
	
	process_async_params(&cfunc,
			     &specials,
			     m_ref,
			     ao, l4_inl,
			     0 /* no target */, 0 /* no client */,
			     encode, request);
	
	/*
	 * Now process the return type.
	 *
	 * Message marshal stubs return a pointer to an interface message type.
	 * Message unmarshal stubs do not return anything directly (void);
	 * they return the decoded information through their parameters.
	 *
	 * XXX - in order to reduce the amount of overhead just to create the
	 * return type for message m/u stubs, this is represented solely in
	 * the CAST (and handled by macros in each back end).  To be `right',
	 * we need to build up the proper PRES_C and link it to the request
	 * and reply inline_func_params_struct's return slot.
	 */
	if (encode) {
		cfunc.return_type
			= cast_new_type_name(optype);
	} else {
		cfunc.return_type = cast_new_type(CAST_TYPE_VOID);
	}
	
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
	if (cast_find_def(&deep_scope, cast_new_scoped_name(name, NULL),
			  CAST_FUNC_DECL)
	    >= 0) {
		/*
		 * XXX --- Check that our MINT and PRES_C is equal to that of
		 * the existing client stub.  (Hell, check that `name' refers
		 * to a client stub and not some typedef or something else!)
		 *
		 * In lieu of being smart, warn the user.
		 */
		warn("Suppressing extra definition "
		     "for message marshal stub `%s'.",
		     name);
		
		/* Restore the name context. */
		name = old_name;
		return;
	}
	
	newstub = p_add_stub(out_pres);
	if (encode) {
		s(newstub).kind = PRES_C_MESSAGE_MARSHAL_STUB;
		mmstub = &s(newstub).pres_c_stub_u.mmstub;
		mmstub->op_flags = PRES_C_STUB_OP_FLAG_NONE;
		mustub = 0;
		
		/*
		 * Determine if the operation is oneway.
		 * This can be either a oneway request,
		 * or a reply to a request.
		 */
		if (ao->flags & AOI_OP_FLAG_ONEWAY
		    || !request)
			mmstub->op_flags |= PRES_C_STUB_OP_FLAG_ONEWAY;
		
		/* Set the message itype to the top-level "mom_msg"
                   MINT union. */
		mmstub->itype = top_union;
		
	} else {
		s(newstub).kind = PRES_C_MESSAGE_UNMARSHAL_STUB;
		mustub = &s(newstub).pres_c_stub_u.mustub;
		mustub->op_flags = PRES_C_STUB_OP_FLAG_NONE;
		mmstub = 0;
		
		/*
		 * Determine if the operation is oneway.
		 * This can be either a oneway request,
		 * or a reply to a request.
		 */
		if (ao->flags & AOI_OP_FLAG_ONEWAY
		    || !request)
			mustub->op_flags |= PRES_C_STUB_OP_FLAG_ONEWAY;
		
		/* Set the message itype to the top-level "mom_msg"
                   MINT union. */
		mustub->itype = top_union;
		
	}
	assert((mmstub && !mustub && encode)
	       || (!mmstub && mustub && !encode));
	
	deep_scope = scope;
	cast_scoped_name scn = cast_new_scoped_name(opname, NULL);
	if( (cr = cast_find_def(&deep_scope,
				scn,
				CAST_FUNC_DECL) == -1) ) {
		cr = cast_add_def(scope,
				  scn,
				  CAST_SC_NONE,
				  CAST_FUNC_DECL,
				  ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
				  current_protection);
		scope->cast_scope_val[cr].u.cast_def_u_u.func_type = cfunc;
	}
	cdef = cast_add_def(
		&out_pres->stubs_cast,
		mmstub ? request ?
		calc_message_marshal_request_stub_scoped_name(
			cur_aoi_idx, opname)
		: calc_message_marshal_reply_stub_scoped_name(
			cur_aoi_idx, opname) :
		request ?
		calc_message_unmarshal_request_stub_scoped_name(
			cur_aoi_idx, opname)
		: calc_message_unmarshal_reply_stub_scoped_name(
			cur_aoi_idx, opname),
		CAST_SC_NONE,
		CAST_FUNC_DECL,
		ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		current_protection);
	out_pres->stubs_cast.cast_scope_val[cdef].
		u.cast_def_u_u.func_type = cfunc;
	if (mmstub)
		mmstub->c_func = cdef;
	else
		mustub->c_func = cdef;
	
	/*
	 * We need to turn the reply into the union of good, bad, & ugly values
	 */
	if (!request) {
		pres_c_inline inl = l4_inl;
		/*
		 * This interposes a union on the top of the inline tree, and
		 * potentially changes some MINT_CONST's for the union cases.
		 * (Which is why we do it unconditionally.)
		 */
		p_do_return_union(ao, &inl, m_ref, cr,
				  specials.params[stub_special_params::
						 environment_ref].index);
		
		if (encode) {
			/*
			 * On encode, we only need to work with the message
			 * (no exceptions).  Instead of using the union
			 * provided by p_do_return_union(), we create a
			 * COLLAPSED_UNION for the normal reply case.
			 * Exceptions are handled by a different message
			 * marshal function.
			 */
			unsigned int i;
			assert(m(m_ref).kind == MINT_UNION);
			mint_union_def *mu = &(m(m_ref).mint_def_u.union_def);
			inl = pres_c_new_inline(
				PRES_C_INLINE_COLLAPSED_UNION);
			for (i = 0; i < mu->cases.cases_len; i++) {
				if (p_is_normal_return_case(mu, i)) {
					break;
				}
			}
			assert(i < mu->cases.cases_len);
			inl->pres_c_inline_u_u.collapsed_union.
				discrim_val = mu->cases.cases_val[i].val;
			inl->pres_c_inline_u_u.collapsed_union.
				selected_case = l4_inl;
		}
		l4_inl = inl;
	}
	
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
		(request)? oper_request : oper_reply;
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
	
	/* Assign the unions and param struct to cstub fields. */
	if (mmstub) {
		mmstub->i = l1_inl;
		
		/* set the message marshal stub flags */
		mmstub->client = client;
		mmstub->request = request;
		mmstub->reply_handler_name
			= calc_receive_reply_func_name(ao->name);
	} else {
		mustub->i = l1_inl;
		
		/* set the message marshal stub flags */
		mustub->client = client;
		mustub->request = request;
		mustub->reply_handler_name = ir_strlit("");
	}
	
	if (encode && !request) {
		/* We need to create a marshal stub for an exceptional
                   (error) message. */
		p_message_marshal_exception_stub(ai, ao);
	}
	
	/* Restore the name context. */
	name = old_name;
}


/*****************************************************************************/

/*
 * Generate an exceptional (error) message marshal stub presentation for
 * an AOI interface operation.
 *
 * Note that `ao' may not be defined in `ai', but rather in some parent
 * interface of `ai'.
 */
void pg_state::p_message_marshal_exception_stub(aoi_interface *ai,
						aoi_operation *orig_ao)
{
	pres_c_msg_marshal_stub *mmstub;
	
	int newstub;
	int cr, cdef;
	
	char *old_name;
	char *opname;
	char *optype;
	int cast_params_len;
	cast_func_type cfunc;
	
	stub_special_params specials;
	
	mint_ref request_ref;
	mint_ref reply_ref;
	mint_ref m_ref;
	
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
	
	if(!lookup_interface_mint_discrims(ai, orig_ao,
					   &oper_request, &oper_reply)) {
		panic("In `pg_state::p_message_marshal_stub', "
		      "cannot find MINT request and reply discriminators.");
	}
	
	/*
	 * Find the MINT references to the request and reply types.
	 */
	p_client_stub_find_refs(ai, orig_ao, oper_request, oper_reply,
				&request_ref,
				&reply_ref);
	m_ref = reply_ref;
	
	/*
	 * This is a very specialized stub we are generating.  Rather than
	 * coercing the regular functions to generate the stub for us, we
	 * simply create the stub here at this level.  The main problem stems
	 * from having a reply environment.  It is wrong to make it a special
	 * parameter (we already have so many, and it contains what we really
	 * want to place in the message, not just an auxiliary parameter that
	 * is only useful for the presentation).  It is also wrong to create
	 * AOI and make the parameter an AOI_EXCEPTION, since it is really
	 * just an environment that can (must) contain an exception, but not
	 * necessarily any specific one.  We get exactly what we need by
	 * creating everything directly here.
	 */
	cast_params_len = 2;
	
	/* Save the original name context for when we return. */
	old_name = name;
	opname = calc_message_marshal_except_stub_name(orig_ao->name);
	optype = calc_message_reply_type_name(orig_ao->name);
	name = opname;
	
	/*
	 * Now we are ready to start building the presentation PRES_C and CAST
	 * goo.
	 */
	
	cast_init_function_type(&cfunc, cast_params_len);
	
	cfunc.params.params_len = cast_params_len;
	cfunc.params.params_val 
		= (cast_func_param *) mustcalloc(cast_params_len *
						 sizeof(cast_func_param));
	
	/*
	 * Determine the special parameters for this stub: the target object
	 * reference, the environment reference, etc.
	 * NOTE: We do not use this in the normal sense!  In general, this is
	 * passed to process_async_params(), and used to generated the param
	 * list.  Since this is a special stub with known special parameters,
	 * we generate the param list directly.  But we still need the CAST
	 * for the environment given to us by (an overridden) special_params().
	 */
	p_msg_marshal_stub_special_params(orig_ao, &specials,
					  0 /* server */, 0 /* reply */);
	
	/* This is param 1: the reply environment parameter. */
	int reply_ev_cast_idx = 0;
	int reg_ev_cast_idx = 1;
	
	cfunc.params.params_val[reply_ev_cast_idx].name
		= flick_asprintf("_reply%s",
				 calc_client_stub_environment_param_name(""));
	cfunc.params.params_val[reply_ev_cast_idx].type
		= specials.params[stub_special_params::environment_ref].ctype;
	cfunc.params.params_val[reply_ev_cast_idx].spec = 0;
	cfunc.params.params_val[reply_ev_cast_idx].default_value = 0;
	
	/* This is param 2: the regular environment parameter.
	   This is essentially what process_async_params() would do. */
	cfunc.params.params_val[reg_ev_cast_idx].name
		= calc_client_stub_environment_param_name("");
	cfunc.params.params_val[reg_ev_cast_idx].type
		= specials.params[stub_special_params::environment_ref].ctype;
	cfunc.params.params_val[reg_ev_cast_idx].spec = 0;
	cfunc.params.params_val[reg_ev_cast_idx].default_value = 0;
	
	/*
	 * Now process the return type.
	 *
	 * Message marshal stubs return a pointer to an interface message type.
	 *
	 * XXX - in order to reduce the amount of overhead just to create the
	 * return type for message m/u stubs, this is represented solely in
	 * the CAST (and handled by macros in each back end).  To be `right',
	 * we need to build up the proper PRES_C and link it to the request
	 * and reply inline_func_params_struct's return slot.
	 */
	cfunc.return_type = cast_new_type_name(optype);
	
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
	if (cast_find_def(&deep_scope,
			  cast_new_scoped_name(name, NULL),
			  CAST_FUNC_DECL) >= 0) {
		/*
		 * XXX --- Check that our MINT and PRES_C is equal to that of
		 * the existing client stub.  (Hell, check that `name' refers
		 * to a client stub and not some typedef or something else!)
		 *
		 * In lieu of being smart, warn the user.
		 */
		warn("Suppressing extra definition "
		     "for message marshal stub `%s'.",
		     name);
		
		/* Restore the name context. */
		name = old_name;
		return;
	}
	
	newstub = p_add_stub(out_pres);
	s(newstub).kind = PRES_C_MESSAGE_MARSHAL_STUB;
	mmstub = &s(newstub).pres_c_stub_u.mmstub;
	mmstub->op_flags = PRES_C_STUB_OP_FLAG_NONE;
	
	/* The operation is oneway (it is a reply to a request). */
	mmstub->op_flags |= PRES_C_STUB_OP_FLAG_ONEWAY;
	
	/* Set the message itype to the top-level "mom_msg"
	   MINT union. */
	mmstub->itype = top_union;
	
	deep_scope = scope;
	cast_scoped_name scn = cast_new_scoped_name(opname, NULL);
	if( (cr = cast_find_def(&deep_scope,
				scn,
				CAST_FUNC_DECL)) == -1 ) {
		cr = cast_add_def(scope,
				  cast_new_scoped_name(opname, NULL),
				  CAST_SC_NONE,
				  CAST_FUNC_DECL,
				  ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
				  current_protection);
		scope->cast_scope_val[cr].u.cast_def_u_u.func_type = cfunc;
	}
	cdef = cast_add_def(&out_pres->stubs_cast,
			    calc_message_marshal_except_stub_scoped_name(
					  cur_aoi_idx, opname),
			    CAST_SC_NONE,
			    CAST_FUNC_DECL,
			    ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
			    current_protection);
	out_pres->stubs_cast.cast_scope_val[cdef].u.
		cast_def_u_u.func_type = cfunc;
	mmstub->c_func = cdef;
	
	/*
	 * Build the group of pres_c_inline structures containing the
	 * parameters, starting with level 4 (params struct) then levels
	 * 3, 2, 1 (unions).
	 *
	 * XXX - Instead of the normal INLINE_FUNC_PARAMS_STRUCT, we make an
	 * INLINE_ILLEGAL to indicate the normal reply case isn't valid here.
	 * Perhaps there needs to be a better way to inhibit the production
	 * of a union case, even though the MINT and PRES exist for it.
	 */
	pres_c_inline l4_inl = pres_c_new_inline(PRES_C_INLINE_ILLEGAL);
	
	/*
	 * We need to turn the reply into the union of good, bad, & ugly values
	 */
	p_do_return_union(orig_ao, &l4_inl, m_ref, cr, reply_ev_cast_idx);
	
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
		oper_reply;
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
	
	/* Assign the unions and param struct to cstub fields. */
	mmstub->i = l1_inl;
	
	/* set the message marshal stub flags */
	mmstub->client = 0; /* server */
	mmstub->request = 0; /* reply */
	mmstub->reply_handler_name
		= calc_receive_reply_func_name(orig_ao->name);
	
	/* Restore the name context. */
	name = old_name;
}


/*****************************************************************************/

/***** Auxiliary functions. *****/
int pg_state::p_msg_marshal_stub_special_params(aoi_operation *ao,
						stub_special_params *specials,
						int client,
						int request)
{
	int newparms = 0;
	stub_special_params::stub_param_info *this_param;
	p_type_collection *ptc;
	p_type_node *ptn;
	
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
	 * Initialize the object reference type and index.  The default is
	 * for the object reference not to appear.  We set the CAST type
	 * anyway for the possible benefit of derived presentation generators.
	 */
	this_param = &(specials->params[stub_special_params::object_ref]);
	
	this_param->name =
		calc_client_stub_object_param_name(ao->name);
	this_param->ctype = cast_new_type_name(
		calc_client_stub_object_type_name(ao->name));
	this_param->index = -1;
	
	/*
	 * Initialize the message reference type and index.
	 * This is only for message unmarshal stubs, since the normal
	 * presentation never passes around messages.
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
	if ((client && !request) || (!client && request)) {
		this_param->index = newparms;
		newparms++;
	} else
		this_param->index = -1;
	
	/*
	 * Initialize the return reference type and index.
	 * This is only for message marshal/unmarshal stubs, since the
	 * normal presentation uses the regular function return value.
	 * Also, only replies have return values.
	 */
	this_param = &(specials->params[stub_special_params::return_ref]);
	
	this_param->name = "_return_code";
	this_param->index = -1; /* explicitly defualt to no return code */
	ptc = 0;
	p_type(ao->return_type, &ptc);
	ptn = ptc->find_type("definition");
	this_param->ctype = ptn->get_type();
	if (!request) {
		this_param->index = newparms;
		newparms++;
	}
	
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
	if (gen_sids) {
		this_param->index = ao->params.params_len + newparms;
		newparms++;
	} else
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
	if (gen_sids) {
		this_param->index = ao->params.params_len + newparms;
		newparms++;
	} else
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
	if (gen_sids) {
		this_param->index = ao->params.params_len + newparms;
		newparms++;
	} else
		this_param->index = -1;
	
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
	
	/* Finally, we're done! */
	return newparms;
}

/* End of file. */

