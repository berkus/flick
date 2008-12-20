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

#include <mom/c/libcast.h>

#include <mom/c/pbe.hh>

/*
 * Determine the CAST expressions that name the function to be invoked by a
 * Flick-generated ``receive dispatch'' function.
 *
 * The returned `formal_func_cexpr' and `actual_func_cexpr' must be identical
 * except that wherever a formal argument appears in the `formal_func_cexpr',
 * there must be a null CAST expression at the corresponding point in the
 * `actual_func_cexpr'.  These ``holes'' in the actual expression will be
 * filled in as marshal/unmarshal code is generated.
 */
void mu_state::mu_receive_func_get_invocation_names(
	cast_def *cfunc,
	/* OUT */ cast_expr *formal_func_cexpr,
	/* OUT */ cast_expr *actual_func_cexpr)
{
	*formal_func_cexpr = cast_new_expr_scoped_name(cfunc->name);
	*actual_func_cexpr = cast_new_expr_scoped_name(cfunc->name);
}

/*
 * Determine the CAST expressions that invoke the message-handling function
 * from a Flick-generated ``receive dispatch'' function.  This method must set
 * the `formal_func_invocation_cexpr' and `actual_func_invocation_cexpr' slots
 * of the `mu_state' object.
 *
 * The `formal_func_invocation_cexpr' and `actual_func_invocation_cexpr' built
 * by this method must be identical except that wherever a formal argument
 * appears in the `formal_func_invocation_cexpr', there must be a null CAST
 * expression at the corresponding point in the `actual_func_invocation_cexpr'.
 * These ``holes'' in the actual expression will be filled in as m/u code is
 * generated.
 */
void mu_state::mu_receive_func_set_invocation_cexprs(cast_def *cfunc)
{
	cast_func_type	*cfunct = &(cfunc->u.cast_def_u_u.func_type);
	
	cast_expr	formal_func_cexpr;
	cast_expr	actual_func_cexpr;
	
	unsigned int	i;
	
	/*
	 * Set up the formal and actual versions of the expression that will be
	 * output to invoke the work function.
	 */
	assert((cfunct->return_type != 0) &&
	       (cfunct->return_type->kind == CAST_TYPE_VOID));
	
	mu_receive_func_get_invocation_names(cfunc,
					     &formal_func_cexpr,
					     &actual_func_cexpr);
	assert(formal_func_cexpr);
	assert(actual_func_cexpr);
	
	formal_func_invocation_cexpr
		= cast_new_expr_call(formal_func_cexpr, 0);
	actual_func_invocation_cexpr
		= cast_new_expr_call(actual_func_cexpr, 0);
	
	for (i = 0; i < cfunct->params.params_len; ++i) {
		cast_expr_array *formal_params
			= &(formal_func_invocation_cexpr->cast_expr_u_u.call.
			    params);
		cast_expr_array *actual_params
			= &(actual_func_invocation_cexpr->cast_expr_u_u.call.
			    params);
		
		if (!(cfunct->params.params_val[i].spec
		      & CAST_PARAM_IMPLICIT)) {
			cast_add_expr_array_value(
				formal_params,
				cast_new_expr_name(cfunct->
						   params.params_val[i].name)
				);
			cast_add_expr_array_value(
				actual_params,
				0 /* A placeholder; will be filled by m/u. */
				);
		}
	}
}

/*
 * Output the invocation of the message-handling function that is called from
 * within a Flick-generated ``receive dispatch'' function.
 */
void mu_state::mu_receive_func_call(cast_expr func_call_cexpr)
{
	add_stmt(change_stub_state(FLICK_STATE_FUNCTION_CALL));
	add_stmt(cast_new_stmt_expr(func_call_cexpr));
	add_stmt(change_stub_state(FLICK_STATE_FUNCTION_RETURN));
}

/*****************************************************************************/

/* This routine generates the code to call a single server work function.
   It is generally called (indirectly) from within mu_decode_switch(),
   once the possible choices have been narrowed down to a single server work
   function.
   
   The parameter `inl' describes the "remaining" part of the itype and ctype of
   the incoming message, after mu_decode_switch has "eaten" the part it needed
   to select a work function.
   
   This routine will generate code to package the message and call the
   C work function for processing.  No code is produced to handle replies.
   (Receive functions issue their own reply if one is in order).
*/
void mu_state::mu_receive_func(pres_c_inline inl, mint_ref /*itype*/,
			       pres_c_receive_func *rfunc,
			       pres_c_skel */*skel*/)
{
	cast_def *cfunc = &pres->stubs_cast.cast_scope_val[rfunc->c_func];
	
	/*
	 * Create a new scope containing the variables passed to this server
	 * function.
	 */
	cast_stmt old_c_block = c_block;
	c_block = cast_new_block(0, 0);
	
	/*
	 * Set up the arglist: be ready to catch the various special function
	 * parameters.
	 *
	 * XXX --- We don't *use* any of these arguments here, at least not
	 * yet.  Nevertheless, we must be prepared to capture the information
	 * about any special arguments that the PG may have marked.
	 */
	mu_state_arglist *oldlist = arglist;
	arglist = new mu_state_arglist("params", oldlist);
	arglist->add("params", "object");
	arglist->add("params", "environment");
	arglist->add("params", "return");
	arglist->add("params", "invocation_id");
	arglist->add("params", "client");
	arglist->add("params", "message");
	/*
	 * XXX --- Don't handle these for now.  Would we ever need to?
	 * must.arglist->add("params", "continue_func");
	 * must.arglist->add("params", "continue_data");
	 */
	
	/*
	 * Set up the formal and actual versions of the invocation of the work
	 * function.
	 */
	mu_receive_func_set_invocation_cexprs(cfunc);
	
	/*
	 * Build the code to locate the target object.
	 */
	mu_receive_func_target(rfunc);
	/*
	 * ... and the client object.
	 */
	mu_receive_func_client(rfunc);
	
	/* Build the code to package and identify the message. */
	mu_func_params(rfunc->c_func, rfunc->simple_msg_itype, inl);
	mu_end();
	
	/*
	 * Signal that we are finished with the decode phase.  The matching
	 * `flick_*_server_start_encode' is output by `w_server_skel', which is
	 * defined individually by each back end.
	 */
	add_stmt(
		cast_new_stmt_expr(
			cast_new_expr_call(
				cast_new_expr_name(
					flick_asprintf(
						"flick_%s_server_end_decode",
						get_be_name())),
				0)));
	
	/*
	 * Build the function call to the server work function.
	 *
	 * The `actual_func_invocation_cexpr' should have been completely
	 * filled out as part of unmarshaling the message.
	 */
	cast_check_expr(actual_func_invocation_cexpr);
	mu_receive_func_call(actual_func_invocation_cexpr);
	actual_func_invocation_cexpr = 0;
	
	/*
	 * For receive functions, any necessary reply is handled by the
	 * work function.  Flick is not responsible for generating a reply
	 * when the function returns.
	 */
	add_stmt(
		cast_new_return(
			cast_new_expr_name(
				"FLICK_OPERATION_SUCCESS_NOREPLY"
				)));
	
	/* End the new scope we created. */
	cast_stmt new_c_block = c_block;
	c_block = old_c_block;
	add_stmt(new_c_block);
	
	delete arglist;
	arglist = oldlist;
}

/* End of file. */

