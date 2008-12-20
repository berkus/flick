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

#include <mom/c/libcast.h>

#include <mom/c/pbe.hh>

/*
 * Determine the CAST expressions that name the function to be invoked by a
 * Flick-generated server dispatch function.
 *
 * The returned `formal_func_cexpr' and `actual_func_cexpr' must be identical
 * except that wherever a formal argument appears in the `formal_func_cexpr',
 * there must be a null CAST expression at the corresponding point in the
 * `actual_func_cexpr'.  These ``holes'' in the actual expression will be
 * filled in as marshal/unmarshal code is generated.
 */
void mu_state::mu_server_func_get_invocation_names(
	cast_def *cfunc,
	/* OUT */ cast_expr *formal_func_cexpr,
	/* OUT */ cast_expr *actual_func_cexpr)
{
	*formal_func_cexpr = cast_new_expr_scoped_name(cfunc->name);
	*actual_func_cexpr = cast_new_expr_scoped_name(cfunc->name);
}

/*
 * Determine the CAST expressions that invoke the server work function from a
 * Flick-generated server dispatch function.  This method must set the
 * `formal_func_invocation_cexpr' and `actual_func_invocation_cexpr' slots of
 * the `mu_state' object.
 *
 * The `formal_func_invocation_cexpr' and `actual_func_invocation_cexpr' built
 * by this method must be identical except that wherever a formal argument
 * appears in the `formal_func_invocation_cexpr', there must be a null CAST
 * expression at the corresponding point in the `actual_func_invocation_cexpr'.
 * These ``holes'' in the actual expression will be filled in as m/u code is
 * generated.
 *
 * If the server work function returns a value, then the formal expression
 * should contain a formal parameter named `_return', and the actual expression
 * should contain a corresponding hole.  In this case, the formal expression
 * would likely be an assignment expression: `_return = func(...)'.
 */
void mu_state::mu_server_func_set_invocation_cexprs(cast_def *cfunc)
{
	cast_func_type	*cfunct = &(cfunc->u.cast_def_u_u.func_type);
	
	cast_expr	formal_func_cexpr;
	cast_expr	actual_func_cexpr;
	
	int		cfunct_returns_nonvoid;
	
	unsigned int	i;
	
	/*
	 * Set up the formal and actual versions of the expression that will be
	 * output to invoke the work function.
	 */
	cfunct_returns_nonvoid = ((cfunct->return_type != 0) &&
				  (cfunct->return_type->kind != CAST_TYPE_VOID)
				  );
	
	mu_server_func_get_invocation_names(cfunc,
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
	if (cfunct_returns_nonvoid) {
		formal_func_invocation_cexpr
			= cast_new_expr_assign(
				cast_new_expr_name("_return"),
				formal_func_invocation_cexpr);
		actual_func_invocation_cexpr
			= cast_new_expr_assign(
				0,
				actual_func_invocation_cexpr);
	}
}

/*
 * Output the invocation of the server work function that is called from within
 * a Flick-generated server dispatch function.
 */
void mu_state::mu_server_func_call(cast_expr func_call_cexpr)
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
   
   The parameters `inl' and `tn_r' describe the "remaining" part of the itype
   and ctype
   of the incoming request message,
   after mu_decode_switch has "eaten" the part it needed to select a work
   function.
   This routine will generate code to do the rest of the message decoding
   before spitting out the call to the actual C work function.
   
   After the call to the work function,
   this routine generates the necessary code to marshal the reply message,
   using the _entire_ reply itype and ctype extracted directly from `sstub' and
   `sfunc',
   respectively.
   (Reply messages need no decoding on the server side.)
   
   Finally, it calls the mu_server_func_reply() method,
   which more-specific code must provide,
   which does whatever is necessary to finish building the reply message
   and, if appropriate, send it off.
   In some implementations, the reply will be sent off automatically
   after the server stub returns to the runtime support code or to the kernel,
   so nothing special would need to be done here in that case.
   */
void mu_state::mu_server_func(pres_c_inline inl, mint_ref tn_r,
			      pres_c_server_func *sfunc,
			      pres_c_skel *sstub)
{
	cast_def *cfunc = &pres->stubs_cast.cast_scope_val[sfunc->c_func];
	cast_func_type *cfunct = &cfunc->u.cast_def_u_u.func_type;
	struct mu_abort_block *mab_par, *mab_thr;
	
	mab_par = abort_block;
	mab_thr = new mu_abort_block();
	abort_block = mab_thr;
	mab_thr->set_kind(MABK_THREAD);
	mab_thr->begin();
	cfunc->u.cast_def_u_u.func_type = *cfunct;
	
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
	mu_server_func_set_invocation_cexprs(cfunc);
	
	/*
	 * Build the code to locate the target object.
	 */
	mu_server_func_target(sfunc);
	/*
	 * ... and the client object, if present.
	 */
	mu_server_func_client(sfunc);
	
	/*
	 * Build the code to unmarshal the request.  Additionally, this will
	 * also allocate any extra storage required for `out' parameters.
	 */
	mu_func_params(sfunc->c_func, tn_r, inl);
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
	 * Initialize the environment.
	 *
	 * XXX --- This should be part of the PRES_C for unmarshaling the
	 * request --- i.e., to have a PRES_C_MAPPING_INIT or somesuch that
	 * sticks a constant value into the environment.
	 */
	add_stmt(
		cast_new_stmt_expr(
			cast_new_expr_call(
				cast_new_expr_name(
					flick_asprintf(
						"flick_%s_init_environment",
						pres->pres_context)),
				0)));
	
	/*
	 * Build the function call to the server work function.
	 *
	 * The `actual_func_invocation_cexpr' should have been completely
	 * filled out as part of unmarshaling the request.
	 */
	cast_check_expr(actual_func_invocation_cexpr);
	mu_server_func_call(actual_func_invocation_cexpr);
	actual_func_invocation_cexpr = 0;
	
	/*
	 * Build the code to marshal the reply.  Additionally, this will free
	 * storage allocated for `in' parameters.
	 *
	 * XXX --- The freeing of `in' parameters isn't really implemented yet.
	 * But it makes no difference, since the back ends optimize all `in'
	 * parameters to be stack allocated, thus ``automatically'' freed.
	 * (It should be noted that the presentation generators specify only
	 * the allocation semantics required for the parameters, and the back
	 * ends optimize an `always-allocated, always-deallocated' entity that
	 * has no specific allocator (i.e., DONTCARE) into a stack-allocated
	 * entity.  It is actually coincidental that all `in' parameters have
	 * this semantic and are able to be optimized in this way.)
	 *
	 * XXX --- We still need to free storage for `oneway' operations, so we
	 * can't do the following `if' here.
	 */
	if (!(sfunc->op_flags & PRES_C_STUB_OP_FLAG_ONEWAY)) {
		add_stmt(change_stub_state(FLICK_STATE_MARSHAL));
		mu_server_func_reply(sfunc, sstub);
	}
	else
		add_stmt(
			cast_new_return(
				cast_new_expr_name(
					"FLICK_OPERATION_SUCCESS_NOREPLY"
					)));
	
	delete arglist;
	arglist = oldlist;
	add_stmt(cast_new_break());
	abort_block = mab_par;
	mab_thr->end();
	mab_par->add_child(mab_thr, 0);
	add_stmt(mab_thr->get_block_label());
	/* End the new scope we created. */
	cast_stmt new_c_block = c_block;
	c_block = old_c_block;
	add_stmt(new_c_block);
}

/* End of file. */

