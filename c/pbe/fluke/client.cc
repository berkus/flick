/*
 * Copyright (c) 1996, 1997, 1998, 1999 The University of Utah and
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
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include "fluke.h"

/*
 *
 */

void w_client_stub(pres_c_1 *pres, int stub_idx)
{
	assert(pres);
	
	pres_c_stub        *stub   = &pres->stubs.stubs_val[stub_idx];
	pres_c_client_stub *cstub  = &stub->pres_c_stub_u.cstub;
	cast_def           *cfunc  = &pres->stubs_cast.
				     cast_scope_val[cstub->c_func];
	cast_func_type     *cfunct = &cfunc->u.cast_def_u_u.func_type;
	
	mint_ref            operation_itype;
	pres_c_inline       operation_inline;
	
	int                 assumptions = RPCM_TRUSTED; /* XXX */
	const char	   *emergency_return_string;
	
	cast_stmt           rpc_cast_stmt;
	
	struct mu_abort_block *mab;
	cast_stmt reaper_label;
	char *escape_label;
	
	/*****/
	
	if (cfunct->return_type->kind != CAST_TYPE_VOID)
		emergency_return_string = "return _return";
	else
		emergency_return_string = "return";
	emergency_return_value = cast_new_expr_name(emergency_return_string);
	
	reaper_label = cast_new_label(
		"_flick_reaper",
		cast_new_stmt_expr(emergency_return_value));
	mab = new mu_abort_block();
	mab->set_kind(MABK_THREAD);
	mab->begin();
	
	/* Start the abort code */
	mab->add_stmt(
		cast_new_stmt_expr(
			cast_new_expr_call_3(
				cast_new_expr_name(
					flick_asprintf(
						"flick_%s_client_error",
						pres->pres_context)),
				cast_new_expr_name("fluke"),
				cast_new_expr_name("fluke"),
				cast_new_expr_name(mab->
						   use_current_label()))));
	/* The order of the musts below is crucial to
	   the abort code working properly */
	
	/*
	 * Figure out which of the stub function parameters is the object
	 * reference and create a CAST expression that names it.
	 *
	 * We do this by using a `fluke_target_mu_state' to ``marshal'' the
	 * target object reference.  This process doesn't create any marshaling
	 * code, however.  The `fluke_taregt_mu_state' knows that when it would
	 * ordinarily marshal an object reference (to the target object), it
	 * should instead simply store the CAST name of the object in its
	 * `target_cast_expr' slot.  Clever, eh?
	 */
	fluke_target_mu_state must_target(the_state,
					  (MUST_ENCODE),
					  assumptions, "client");
	
	/* Set the stub state for error handling
	   and anything else that needs to know
	   where the stub is currently at in execution. */
	must_target.add_stmt(must_target.
			     change_stub_state(FLICK_STATE_PROLOGUE));
	must_target.add_stmt(must_target.
			     change_stub_state(FLICK_STATE_MARSHAL));
	must_target.abort_block = mab;
	
	/* Set up the argument list to catch parameter names we may need. */
	must_target.arglist = new mu_state_arglist("params");
	must_target.arglist->add("params", "object");
	
	must_target.mu_func_params(cstub->c_func, cstub->target_itype,
				   cstub->target_i);
	must_target.mu_end();
	
	/* Build the client object marshaling code. */
	/* XXX --- Not yet supported. */
	if (cstub->client_i) {
		warn("This back end doesn't support client references.");
	}
	
	/*
	 * Build the request marshaling/deallocation code.  We strip away the
	 * ``collapsed union'' goo that encodes the IDL and interface IDs.  We
	 * don't need this information for Fluke IPC because it is manifest in
	 * the object references.
	 */
	fluke_mu_state must_in(the_state,
			       (MUST_ENCODE | MUST_DEALLOCATE | MUST_REQUEST),
			       assumptions, "client");
	
	must_in.abort_block = mab;
	
	remove_idl_and_interface_ids(pres,
				     cstub->request_itype, cstub->request_i,
				     &operation_itype, &operation_inline);
	must_in.mu_func_params(cstub->c_func,
			       operation_itype, operation_inline);
	must_in.mu_end();
	
	/* Get an abort label for the printf's down below to use */
	escape_label = mab->use_current_label();
	
	struct mu_abort_block *out_abort_block;

	/* If its a oneway stub we don't want to have output
	   musts making abort code that won't be used.  So
	   we make a new context and pass it to the musts. */
	if (cstub->op_flags & PRES_C_STUB_OP_FLAG_ONEWAY) {
		out_abort_block = new mu_abort_block();
		out_abort_block->set_kind(MABK_THREAD);
		out_abort_block->begin();
	}
	else
		out_abort_block = mab;
	/*
	 * Build the reply unmarshaling/allocation code.  Again, we strip away
	 * all of the ``collapsed union'' IDL and interface goo.  This time we
	 * strip away the operation reply identifier, too.  (Note that this is
	 * just a fixed number indicating what operation is being replied to.
	 * Any success/failure code is a separate message component!)
	 */
	fluke_mu_state must_out(the_state,
				(MUST_DECODE | MUST_ALLOCATE | MUST_REPLY),
				assumptions, "client");
	
	/* Set up the argument list to catch parameter names we may need. */
	must_out.arglist = new mu_state_arglist("params");
	must_out.arglist->add("params", "environment");
	
	must_out.change_stub_state(FLICK_STATE_UNMARSHAL);
	must_out.abort_block = out_abort_block;
	/* Make a span node to check for message size requirements */
	must_out.current_span = new mu_msg_span;
	must_out.current_span->set_kind(MSK_SEQUENTIAL);
	must_out.current_span->begin();
	
	remove_idl_and_interface_ids(pres,
				     cstub->reply_itype, cstub->reply_i,
				     &operation_itype, &operation_inline);
	remove_operation_id(pres,
			    operation_itype, operation_inline,
			    &operation_itype, &operation_inline);
	must_out.mu_func_params(cstub->c_func,
				operation_itype, operation_inline);
	must_out.mu_end();
	/* Collapse the span tree and commit the values */
	must_out.current_span->end();
	must_out.current_span->collapse();
	must_out.current_span->commit();
	
	mab->end();
	mab->set_reaper_label(reaper_label);
	/* Remove any unneeded abort code */
	mab->rollback();
	
	/* Output the client stub. */
	
	cast_w_func_type(cfunc->name, cfunct, 0);
	w_printf("\n{\n");
	w_i_printf(1, "flick_marshal_stream_struct _stream_struct;\n");
	w_i_printf(1, "const flick_marshal_stream_t _stream ");
	w_printf("= &_stream_struct;\n\n");
	w_i_printf(1, "fluke_error_t _rc;\n");
	
	if (cfunct->return_type->kind != CAST_TYPE_VOID) {
		/*
		 * Initialize `_return' to be zero.  This eliminates compiler
		 * warnings about returning uninitialized data.
		 */
		cast_expr init_expr
			= cast_new_expr_assign_to_zero(
				cast_new_expr_name("_return"),
				cfunct->return_type,
				&(pres->cast)
				);
		
		w_indent(1);
		cast_w_type(cast_new_scoped_name("_return", NULL),
			    cfunct->return_type, 1);
		w_printf(";\n");
		
		w_printf("\n");
		cast_w_stmt(cast_new_stmt_expr(init_expr), 1);
	} else
		w_printf("\n");
	
	w_i_printf(1,
		   "if( !flick_%s_client_start_encode() ) {\n",
		   must_in.get_be_name());
	/* Output the request marshaling code. */
	cast_w_stmt(must_in.c_block, 2);
	w_i_printf(2,
		   "flick_%s_client_end_encode();\n",
		   must_in.get_be_name());
	
	/*
	 * Make the RPC.  If the client stub parametr list includes SIDs, use
	 * the secure version of the RPC function.
	 */
	/*
	 * Until December 1997, secure Flask stubs used server SID parameters.
	 * But now, server SIDs are not presented by the MOM/Fluke PG and are
	 * not supported by the MOM-on-Fluke runtime.  Warn if the server SIDs
	 * parameters are present; the old assertions are `#if 0'ed out in the
	 * code below.
	 */
	if (must_in.server_sid_cexpr || must_out.server_sid_cexpr) {
		warn("the Fluke transport does not support server SID "
		     "arguments to stubs.");
		warn("ignoring server SID parameters to client stub `%s'.",
		     cfunc->name);
	}
	
	if (!must_in.client_sid_cexpr) {
		/*
		 * The client stub does not include the client SID in its
		 * parameter list.  Make sure that it doesn't include any other
		 * SIDs either, and make the normal RPC call.
		 */
#if 0
		/* See above; server SID params are no longer used in Flask. */
		assert(!must_in.server_sid_cexpr
		       && !must_out.server_sid_cexpr);
#endif
		
		rpc_cast_stmt =
			cast_new_stmt_expr(
				cast_new_expr_assign(
					cast_new_expr_name("_rc"),
					cast_new_expr_call_2(
						cast_new_expr_name(
							"flick_client_send_"
							"request_get_reply"
							),
						/* arg 0 */
						must_target.target_cast_expr,
						/* arg 1 */
						cast_new_expr_name("_stream")
						)));
	} else {
		/*
		 * The client stub does include the client SID in its
		 * parameter list.  Make the secure RPC call.
		 */
#if 0
		/* See above; server SID params are no longer used in Flask. */
		assert(must_in.server_sid_cexpr
		       && must_out.server_sid_cexpr);
#endif
		
		rpc_cast_stmt =
			cast_new_stmt_expr(
				cast_new_expr_assign(
					cast_new_expr_name("_rc"),
					cast_new_expr_call_3(
						cast_new_expr_name(
							"flick_client_send_"
							"request_get_reply_"
							"secure"
							),
						/* arg 0 */
						must_target.target_cast_expr,
						/* arg 1 */
						cast_new_expr_name("_stream"),
						/* arg 2 */
						must_in.client_sid_cexpr
#if 0
						/* Not used; see above. */
						/* arg 3 */
						must_in.server_sid_cexpr,
						/* arg 4 */
						must_out.server_sid_cexpr
#endif
						)));
	}
	cast_w_stmt(must_in.change_stub_state(FLICK_STATE_SEND_RECEIVE), 2);
	cast_w_stmt(rpc_cast_stmt, 2);
	
	/* Check for errors from the IPC operation. */
	w_i_printf(2,
		   "if ((_rc != FLUKE_SUCCESS) "
		   "&& (_rc != FLUKE_IPC_RECV_DISCONNECTED))\n");
	w_i_printf(3,
		   "flick_stub_error(FLICK_ERROR_COMMUNICATION, %s);\n",
		   escape_label);
	
	/* Output the reply unmarshaling code. */
	if (cstub->op_flags & PRES_C_STUB_OP_FLAG_ONEWAY) {
		w_i_printf(1,
			   "}\n");
		w_i_printf(1,
			   "else\n");
		w_i_printf(2,
			   "flick_stub_error(FLICK_ERROR_NO_MEMORY, %s);\n",
			   escape_label);
		w_i_printf(1, "return;\n");
		/* Write the abort code out */
		cast_w_stmt(mab->get_block_label(), 1);
		w_printf("}\n\n");
		return;
	}
	
	w_i_printf(2,
		   "flick_%s_client_start_decode();\n",
		   must_in.get_be_name());
	cast_w_stmt(must_out.c_block, 2);
	w_i_printf(2,
		   "flick_%s_client_end_decode();\n",
		   must_in.get_be_name());
	
	w_i_printf(1,
		   "}\n");
	w_i_printf(1,
		   "else\n");
	w_i_printf(2,
		   "flick_stub_error(FLICK_ERROR_NO_MEMORY, %s);\n",
		   escape_label);
	if (cfunct->return_type->kind != CAST_TYPE_VOID)
		w_i_printf(1, "return _return;\n");
	else
		w_i_printf(1, "return;\n");
	/* Write the abort code out */
	cast_w_stmt(mab->get_block_label(), 1);
	cast_w_stmt(reaper_label, 1);
	w_printf("}\n\n");
}

/* End of file. */

