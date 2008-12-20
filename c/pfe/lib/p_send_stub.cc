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
 * Generate a message send stub presentation for an AOI interface.
 */

void pg_state::p_send_stub(aoi_interface *ai, int request)
{
	pres_c_msg_stub *sstub;
	
	int newstub;
	int cr, cdef;
	
	char *old_name;
	int cast_params_len;
	cast_func_type cfunc;
	
	stub_special_params specials;
	int i, j;
	
	mint_ref m_ref;
	
	cast_scope *scope = (cast_scope *)top_ptr(scope_stack);
	
	/*****/
	
	/*
	 * Determine the special parameters for this stub: the target object
	 * reference, the environment reference, etc.
	 */
	p_msg_send_stub_special_params(ai, &specials, request);
	
	/*
	 * Determine the total number of stub parameters.
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
		name = calc_send_request_stub_name(
			a(cur_aoi_idx).name);
	else
		name = calc_send_reply_stub_name(
			a(cur_aoi_idx).name);
	
	/*
	 * Now we are ready to start building the presentation PRES_C and CAST
	 * goo.
	 */
	
	cast_init_function_type(&cfunc, cast_params_len);
	
	/*
	 * Build MINT for this stub.
	 * Since the asynchronous send stub is solely a presentation entity,
	 * MINT for it does not come from AOI, so we have to ``manufacture''
	 * it here.
	 */
	m_ref = mint_add_def(&out_pres->mint);
	m(m_ref).kind = MINT_STRUCT;
	m(m_ref).mint_def_u.struct_def.slots.slots_len = 0;
	m(m_ref).mint_def_u.struct_def.slots.slots_val = 0;
	
	/*
	 * Build an aoi operation for the send stub.
	 * This enables us to reuse a lot of code to build the PRES_C.
	 */
	aoi_operation ao;
	ao.name = name;
	ao.request_code = 0; /* XXX */
	ao.reply_code = 0; /* XXX */
	ao.flags = 0;
	ao.params.params_len = 0;
	ao.params.params_val = 0;
	ao.return_type = (aoi_type) mustcalloc(sizeof(aoi_type_u));
	ao.return_type->kind = AOI_VOID;
	ao.exceps.exceps_len = 0;
	ao.exceps.exceps_val = 0;
	
	/*
	 * Build the group of pres_c_inline structures containing the
	 * parameters, starting with level 4 (params struct) then levels
	 * 3, 2, 1 (unions).
	 */
	pres_c_inline l3_inl = pres_c_new_inline_func_params_struct(0);
	pres_c_inline target_inl = pres_c_new_inline(PRES_C_INLINE_ATOM);
	process_async_params(&cfunc,
			     &specials,
			     m_ref,
			     &ao, l3_inl,
			     target_inl, 0 /* no client */,
			     1 /* send => encode */, request);
	
	/*
	 * The send stub return type is always void.
	 */
	cfunc.return_type = cast_new_type(CAST_TYPE_VOID);
	
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
			  CAST_FUNC_DECL)
	    >= 0) {
		/*
		 * XXX --- Check that our MINT and PRES_C is equal to that of
		 * the existing client stub.  (Hell, check that `name' refers
		 * to a client stub and not some typedef or something else!)
		 *
		 * In lieu of being smart, warn the user.
		 */
		warn("Suppressing extra definition for message send stub `%s'.",
		     name);
		
		/* Restore the name context. */
		name = old_name;
		return;
	}
	
	newstub = p_add_stub(out_pres);
	s(newstub).kind = PRES_C_SEND_STUB;
	sstub = &s(newstub).pres_c_stub_u.send_stub;
	
	/* Set the message itype to the top-level "mom_msg" MINT union. */
	sstub->msg_itype = m_ref;
	
	deep_scope = scope;
	cast_scoped_name scn = cast_new_scoped_name(name, NULL);
	if( (cr = cast_find_def(&deep_scope,
				scn,
				CAST_FUNC_DECL)) == -1 ) {
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
		request ? calc_send_request_stub_scoped_name(cur_aoi_idx,
							     name) :
		calc_send_reply_stub_scoped_name(cur_aoi_idx,
						 name),
		CAST_SC_NONE,
		CAST_FUNC_DECL,
		ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		current_protection);
	out_pres->stubs_cast.cast_scope_val[cdef].u.
		cast_def_u_u.func_type = cfunc;
	sstub->c_func = cdef;
	
	/*
	 * Initialize the `return_slot' to null.
	 */
	l3_inl->pres_c_inline_u_u.func_params_i.return_slot = 0;
	
	/*
	 * We need to turn the reply into the union of good, bad, & ugly values
	 */
//	if (!request)
//		p_do_return_union(&ao, &l3_inl, m_ref, cr);
	
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
	
	/* Assign the unions and param struct to sstub fields. */
	sstub->msg_i = l3_inl;
	
	/* Set the target_i field. */
	sstub->target_i = target_inl;
	sstub->target_itype = out_pres->mint.standard_refs.interface_name_ref;
	sstub->request = request;
	
	/* Restore the name context. */
	name = old_name;
}


/*****************************************************************************/

/***** Auxiliary functions. *****/
int pg_state::p_msg_send_stub_special_params(aoi_interface */*ai*/,
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
		 * Initialize the object reference type and index.
		 */
		this_param = &(specials->params[stub_special_params
					       ::object_ref]);
		
		this_param->name =
			calc_client_stub_object_param_name(name);
		this_param->ctype = cast_new_type_name(
			calc_client_stub_object_type_name(name));
		this_param->index = newparms;
		newparms++;
	} else {
		/* Reply Send Stub has client object as first param */
		/*
		 * Initialize the client type and index.
		 */
		this_param = &(specials->params[stub_special_params::
					       client_ref]);
		
		this_param->name = "client";
		this_param->ctype
			= cast_new_type_name(
				calc_stub_client_ref_type_name(name));
		this_param->index = newparms;
		newparms++;
	}
	
	/*
	 * Initialize the message reference type and index.
	 * This is only for message unmarshal, send, receive, and continuation
	 * stubs, since the normal presentation never passes around messages.
	 */
	this_param = &(specials->params[stub_special_params::message_ref]);
	
	this_param->name = "_msg";
	if (request) {
		this_param->ctype
			= cast_new_type_name(
				calc_message_request_type_name(name));
	} else {
		this_param->ctype
			= cast_new_type_name(
				calc_message_reply_type_name(name));
	}
	this_param->index = newparms;
	newparms++;
	
	/*
	 * Initialize the invocation identifier type and index.
	 * This is only for message send, receive and continuation stubs.
	 */
	this_param = &(specials->params[stub_special_params::invocation_ref]);
	
	this_param->name = "inv_id";
	this_param->ctype
		= cast_new_type_name(calc_stub_invocation_id_type_name(name));
	this_param->index = newparms;
	newparms++;
	
	if (request) {
		/* Request Send Stub has client object as fourth param */
		/*
		 * Initialize the client type and index.
		 */
		this_param = &(specials->params[stub_special_params::
					       client_ref]);
		
		this_param->name = "client";
		this_param->ctype
			= cast_new_type_name(
				calc_stub_client_ref_type_name(name));
		this_param->index = newparms;
		newparms++;
	} else {
		/* Reply Send Stub has target object as fourth param */
		/*
		 * Initialize the object reference type and index.
		 */
		this_param = &(specials->params[stub_special_params
					       ::object_ref]);
		
		this_param->name =
			calc_client_stub_object_param_name(name);
		this_param->ctype = cast_new_type_name(
			calc_client_stub_object_type_name(name));
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
		/*
		 * XXX --- Don't use `name' until `pg_corba::p_get_
		 * exception_discrim_name' et al. have access to the operation
		 * name, too.
		 */
		calc_client_stub_environment_param_name("");
	this_param->ctype = cast_new_type_name(
		/*
		 * XXX --- Don't use `name' until `pg_corba::p_get_env_
		 * struct_type' has access to the operation name, too.
		 */
		calc_client_stub_environment_type_name(""));
	this_param->index = -1;
	
	/* Finally, we're done! */
	return newparms;
}

/* End of file. */

