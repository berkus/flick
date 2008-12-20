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

#include <mom/compiler.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include "iiop.h"

void w_client_stub(pres_c_1 *pres, int stub_idx)
{
	assert(pres);
	
	pres_c_stub *stub = &pres->stubs.stubs_val[stub_idx];
	pres_c_client_stub *cstub = &stub->pres_c_stub_u.cstub;
	cast_def *cfunc = &pres->stubs_cast.cast_scope_val[cstub->c_func];
	cast_func_type *cfunct = &cfunc->u.cast_def_u_u.func_type;
	struct mu_abort_block *mab;
	cast_stmt reaper_label;
	char *escape_label;
	
	mint_ref            operation_itype;
	pres_c_inline       operation_inline;
	
	int assumptions = RPCM_TRUSTED; /* XXX */
	const char *emergency_return_string;
	
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
	iiop_target_mu_state must_target(the_state,
					 (MUST_ENCODE | MUST_DEALLOCATE), 0,
					 "client");
	iiop_mu_state must_in(the_state,
			      (MUST_ENCODE | MUST_DEALLOCATE | MUST_REQUEST),
			      assumptions, "client");
	
	/* Start the abort code */
	mab->add_stmt(
		cast_new_stmt_expr(
			cast_new_expr_call_3(
				cast_new_expr_name(
					flick_asprintf("flick_%s_%s_error",
						       pres->pres_context,
						       must_in.which_stub)),
				cast_new_expr_name(must_in.get_encode_name()),
				cast_new_expr_name(must_in.get_be_name()),
				cast_new_expr_name(mab->
						   use_current_label()))));
	
	/* The order of the musts below is crucial to
	   the abort code working properly */
	
	/* Build the target object marshaling code. */
	/* Set the state of the stub.  This is used by error
	   handlers and possibly other things in the future */
	must_target.add_stmt(must_target.
			     change_stub_state(FLICK_STATE_PROLOGUE));
	must_target.add_stmt(must_target.
			     change_stub_state(FLICK_STATE_MARSHAL));
	must_target.abort_block = mab;
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
	
	/* Build the parameter marshal/unmarshal code. */
	must_in.abort_block = mab;
	remove_idl_and_interface_ids(pres,
				     cstub->request_itype, cstub->request_i,
				     &operation_itype, &operation_inline);
	must_in.mu_func_params(cstub->c_func,
			       operation_itype, operation_inline);
	must_in.mu_end();
	
	/* Get an abort label for the printf's below to use */
	escape_label = mab->use_current_label();
	
	struct mu_abort_block *out_mab;
	
	/* If the stub is oneway we don't want to have unmarshaling
	   abort code left around, so we catch it in a dummy context. */
	if (cstub->op_flags & PRES_C_STUB_OP_FLAG_ONEWAY) {
		out_mab = new mu_abort_block();
		out_mab->set_kind(MABK_THREAD);
		out_mab->begin();
	}
	else
		out_mab = mab;
	iiop_mu_state must_out(the_state,
			       (MUST_DECODE | MUST_ALLOCATE | MUST_REPLY),
			       assumptions, "client", IIOP_NO_SWAP);
	
	/* Set up the argument list to catch parameter names we may need. */
	must_out.arglist = new mu_state_arglist("params");
	must_out.arglist->add("params", "environment");
	
	must_out.abort_block = out_mab;
	must_out.add_stmt(must_out.change_stub_state(FLICK_STATE_UNMARSHAL));
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
	
	iiop_mu_state must_swap(the_state,
				(MUST_DECODE | MUST_ALLOCATE | MUST_REPLY),
				assumptions, "client", IIOP_SWAP);
	
	/* Set up the argument list to catch parameter names we may need. */
	must_swap.arglist = new mu_state_arglist("params");
	must_swap.arglist->add("params", "environment");
	
	must_swap.abort_block = out_mab;
	must_swap.add_stmt(must_swap.change_stub_state(FLICK_STATE_UNMARSHAL));
	must_swap.current_span = new mu_msg_span;
	must_swap.current_span->set_kind(MSK_SEQUENTIAL);
	must_swap.current_span->set_block(must_swap.c_block);
	must_swap.current_span->set_abort(must_swap.abort_block);
	must_swap.current_span->begin();
	
	must_swap.mu_func_params(cstub->c_func,
				 operation_itype, operation_inline);
	must_swap.mu_end();
	must_swap.current_span->end();
	must_swap.current_span->collapse();
	must_swap.current_span->commit();
	
	mab->end();
	mab->set_reaper_label(reaper_label);
	/* Remove any unneeded abort code */
	mab->rollback();
	
	/* Start writing the client stub. */
	cast_w_func_type(cfunc->name, cfunct, 0);
	w_printf("\n{\n");
	w_i_printf(1, "FLICK_BUFFER *_stream;\n");
	
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
	
	/* Get the target (object) expression. */
	int gotarg = 0;
	cast_expr target_expr;
	cast_type target_type;
	gotarg = must_target.arglist->getargs("params", "object",
					      &target_expr, &target_type);
	assert(gotarg);assert(target_expr);assert(target_type);
	
#if 0
	/*
	 * XXX --- #if 0'ed out because this code assumes that 0 is a null
	 * object reference.  That's for the presentation generator to decide!
	 * We should do this: `if (flick_<pres>_object_is_nil(...)) ...'.
	 */
	/* Guard against null target object references. */
	cast_w_stmt(
		cast_new_if(
			cast_new_binary_expr(CAST_BINARY_EQ,
					     target_expr,
					     cast_new_expr_lit_int(0, 0)
				),
			must_in.make_error(FLICK_ERROR_INVALID_TARGET),
			0),
		1);
#endif
	
	w_i_printf(1,
		   "flick_%s_client_start_encode();\n",
		   must_in.get_be_name());
	cast_w_stmt(must_target.c_block, 1);
	
	/* Output the general marshaling code. */
	cast_w_stmt(must_in.c_block, 1);
	w_i_printf(1,
		   "flick_%s_client_end_encode();\n",
		   must_in.get_be_name());

	cast_w_stmt(must_in.change_stub_state(FLICK_STATE_SEND), 1);
	/*
	 * If the operation is oneway, only use send_request, and end the
	 * function.
	 */
	w_i_printf(1,
		   "flick_%s_client_set_response_expected(%d);\n",
		   must_in.get_be_name(),
		   ((cstub->op_flags & PRES_C_STUB_OP_FLAG_ONEWAY) ? 0 : 1));
	
	cast_expr obj_boa_in
		= cast_new_expr_sel(
			cast_new_unary_expr(
				CAST_UNARY_DEREF,
				cast_new_expr_sel(
					cast_new_unary_expr(
						CAST_UNARY_DEREF,
						target_expr),
					cast_new_scoped_name("boa", NULL))),
			cast_new_scoped_name("in", NULL));
	cast_expr obj_boa_out
		= cast_new_expr_sel(
			cast_new_unary_expr(
				CAST_UNARY_DEREF,
				cast_new_expr_sel(
					cast_new_unary_expr(
						CAST_UNARY_DEREF,
						target_expr),
					cast_new_scoped_name("boa", NULL))),
			cast_new_scoped_name("out", NULL));
	
	if (cstub->op_flags & PRES_C_STUB_OP_FLAG_ONEWAY) {
		cast_stmt send_stmt = cast_new_if(
			cast_new_unary_expr(
				CAST_UNARY_LNOT,
				cast_new_expr_call_2(
					cast_new_expr_name(
						"flick_client_send_request"),
					target_expr,
					obj_boa_in)),
			cast_new_stmt_expr(cast_new_expr_call_2(
				cast_new_expr_name("flick_stub_error"),
				cast_new_expr_name(
					"FLICK_ERROR_COMMUNICATION"),
				cast_new_expr_name(
					escape_label))),
			0 /* no else clause */);
		cast_w_stmt(send_stmt, 1);
		w_i_printf(1, "%s;\n", emergency_return_string);
		
		/* write out the abort stuff */
		cast_w_stmt(mab->get_block_label(), 1);
		cast_w_stmt(reaper_label, 1);
		w_printf("}\n\n");
		return;
	}

	cast_stmt rpc_stmt = cast_new_if(
		cast_new_binary_expr(
			CAST_BINARY_LOR,
			cast_new_unary_expr(
				CAST_UNARY_LNOT,
				cast_new_expr_call_2(
					cast_new_expr_name(
						"flick_client_send_request"),
					target_expr,
					obj_boa_in)),
			cast_new_unary_expr(
				CAST_UNARY_LNOT,
				cast_new_expr_call_2(
					cast_new_expr_name(
						"flick_client_get_reply"),
					target_expr,
					obj_boa_out))),
		cast_new_stmt_expr(cast_new_expr_call_2(
			cast_new_expr_name(
				flick_asprintf("flick_stub_error",
					       must_in.pres->pres_context)),
			cast_new_expr_name("FLICK_ERROR_COMMUNICATION"),
			cast_new_expr_name(escape_label))),
		0 /* no else clause */);
	cast_w_stmt(rpc_stmt, 1);
	w_i_printf(1,
		   "flick_%s_client_start_decode();\n",
		   must_in.get_be_name());
	
	cast_stmt decode_stmt = cast_new_if(
		cast_new_expr_call_0(
			cast_new_expr_name("flick_cdr_swap")),
		must_swap.c_block,
		must_out.c_block);
	cast_w_stmt(decode_stmt, 1);
	
	w_i_printf(1,
		   "flick_%s_client_end_decode();\n",
		   must_in.get_be_name());
	
	w_i_printf(1, "%s;\n", emergency_return_string);
	
	/* write out the abort stuff */
	cast_w_stmt(mab->get_block_label(), 1);
	cast_w_stmt(reaper_label, 1);
	w_printf("}\n\n");
}

void w_send_stub(pres_c_1 *pres, int stub_idx)
{
	assert(pres);

	pres_c_stub *stub = &pres->stubs.stubs_val[stub_idx];
	assert(stub->kind == PRES_C_SEND_STUB);
	pres_c_msg_stub *sstub = &stub->pres_c_stub_u.send_stub;
	cast_def *cfunc = &pres->stubs_cast.cast_scope_val[sstub->c_func];
	cast_func_type *cfunct = &cfunc->u.cast_def_u_u.func_type;
	struct mu_abort_block *mab;
	cast_stmt reaper_label;
	
	int assumptions = RPCM_TRUSTED; /* XXX */
	const char *emergency_return_string = "return";
	char *escape_label;
	
	assert(cfunct->return_type->kind == CAST_TYPE_VOID);
	emergency_return_value = cast_new_expr_name(emergency_return_string);
	reaper_label = cast_new_label(
		"_flick_reaper",
		cast_new_stmt_expr(emergency_return_value));
	mab = new mu_abort_block();
	mab->set_kind(MABK_THREAD);
	mab->begin();
	
	iiop_target_mu_state must_target(the_state,
					 MUST_ENCODE | MUST_DEALLOCATE,
					 0, "send");
	iiop_mu_state must_send(the_state,
				MUST_ENCODE | MUST_DEALLOCATE,
				assumptions, "send");
	
	/* Start the abort code */
	int gotarg = 0;
	
	mab->add_stmt(
		cast_new_stmt_expr(
			cast_new_expr_call_3(
				cast_new_expr_name(
					flick_asprintf("flick_%s_%s_error",
						       pres->pres_context,
						       must_send.which_stub)),
				cast_new_expr_name(must_send.
						   get_encode_name()),
				cast_new_expr_name(must_send.get_be_name()),
				cast_new_expr_name(mab->
						   use_current_label()))));
	
	/* The order of the musts below is crucial to
	   the abort code working properly */
	
	/* Build the target object marshaling code. */
	must_target.abort_block = mab;
	must_target.arglist = new mu_state_arglist("params");
	must_target.arglist->add("params", "object");
	must_target.mu_func_params(sstub->c_func, sstub->target_itype,
				   sstub->target_i);
	must_target.mu_end();
	
	/* Build the code. */
	/* Set up the argument list to catch the parameter names we need. */
	must_send.arglist = new mu_state_arglist("params");
	must_send.arglist->add("params", "environment");
	must_send.arglist->add("params", "return");
	must_send.arglist->add("params", "invocation_id");
	must_send.arglist->add("params", "client");
	must_send.arglist->add("params", "message");
	
	must_send.abort_block = mab;
	
	must_send.mu_func_params(sstub->c_func,
				 sstub->msg_itype, sstub->msg_i);
	must_send.mu_end();
	
	escape_label = mab->use_current_label();
	
	mab->end();
	mab->set_reaper_label(reaper_label);
	/* Remove any unneeded abort code */
	mab->rollback();
	
	/* Start writing the client stub. */
	cast_w_func_type(cfunc->name, cfunct, 0);
	w_printf("\n{\n");
	
	/* XXX - These are required in order have the stub_state
	   accessible for error handling. */
	w_i_printf(1, "FLICK_BUFFER __stream;\n");
	w_i_printf(1, "FLICK_BUFFER *_stream = &__stream;\n");
	
	/*
	 * To be fully ``correct'', we should actually output the
	 * blocks of code we just generated for the target and stub
	 * body.  As it turns out, though, everything that we need to
	 * do (marshal the target, insert the invocation ID, marshal
	 * the client) is best done in the runtime anyway, not here.
	 */
#if 0
	w_i_printf(1,
		   "flick_%s_send_start_encode();\n",
		   must_send.get_be_name());
	
	cast_w_stmt(must_target.c_block, 1);
	cast_w_stmt(must_send.c_block, 1);
	
	w_i_printf(1,
		   "flick_%s_send_end_encode();\n",
		   must_send.get_be_name());
#endif
	
	cast_w_stmt(must_send.change_stub_state(FLICK_STATE_SEND), 1);
	w_i_printf(1, "if (!flick_send_%s_msg(",
		   (sstub->request)? "request" : "reply");
	cast_expr expr, client_expr, target_expr;
	cast_type type;
	/* Get the client and target names separately. */
	gotarg = must_send.arglist->getargs("params", "client",
					    &client_expr, &type);
	assert(gotarg);assert(client_expr);assert(type);
	gotarg = must_target.arglist->getargs("params", "object",
					      &target_expr, &type);
	assert(gotarg);assert(target_expr);assert(type);
	
	cast_w_expr((sstub->request)? target_expr : client_expr, 0);
	w_printf(", (flick_msg_t) ");
	gotarg = must_send.arglist->getargs("params", "message", &expr, &type);
	assert(gotarg);assert(expr);assert(type);
	cast_w_expr(expr, 0);w_printf(", ");
	gotarg = must_send.arglist->getargs("params", "invocation_id",
					    &expr, &type);
	assert(gotarg);assert(expr);assert(type);
	cast_w_expr(expr, 0);w_printf(", ");
	cast_w_expr((sstub->request)? client_expr : target_expr, 0);
	w_printf("))\n");
	
	w_i_printf(2,
		   "flick_stub_error(FLICK_ERROR_COMMUNICATION, %s);\n",
		   escape_label);
	w_i_printf(1, "%s;\n", emergency_return_string);
	
	/* write out the abort stuff */
	cast_w_stmt(mab->get_block_label(), 1);
	cast_w_stmt(reaper_label, 1);
	w_printf("}\n\n");
}

/* End of file. */

