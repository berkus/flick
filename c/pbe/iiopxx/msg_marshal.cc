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
#include <mom/c/libcast.h>

#include "iiopxx.h"

void w_msg_marshal_stub(pres_c_1 *pres, int stub_idx)
{
	pres_c_msg_marshal_stub *stub;
	cast_stmt stub_body;
	struct mu_abort_block *mab;
	cast_stmt reaper_label;
	
	/* Find the `pres_c_msg_marshal_stub' that describes this stub. */
	assert(pres);
	assert(pres->stubs.stubs_val[stub_idx].kind
	       == PRES_C_MESSAGE_MARSHAL_STUB);
	stub = &(pres->stubs.stubs_val[stub_idx].pres_c_stub_u.mmstub);
	
	/* Find the CAST declaration of this stub. */
	assert(stub->c_func >= 0);
	assert(stub->c_func < (signed int)pres->stubs_cast.cast_scope_len);
	cast_def *cfunc = &(pres->stubs_cast.cast_scope_val[stub->c_func]);
	
	assert(cfunc->u.kind == CAST_FUNC_DECL);
	cast_func_type *cfunct = &cfunc->u.cast_def_u_u.func_type;
	
	assert(cfunct->return_type->kind != CAST_TYPE_VOID);
	
	emergency_return_value = cast_new_expr_name("return _return");
	
	mint_ref            operation_itype;
	pres_c_inline       operation_inline;
	
	/* Build the parameter marshal/unmarshal code. */
	iiopxx_mu_state must(the_state,
			     (MUST_ENCODE | MUST_DEALLOCATE
			      | ((stub->request)? MUST_REQUEST
				 : MUST_REPLY)),
			     RPCM_TRUSTED,
			     "msg");
	
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
						"flick_%s_%s_error",
						pres->pres_context,
						must.which_stub)),
				cast_new_expr_name(must.get_encode_name()),
				cast_new_expr_name(must.get_be_name()),
				cast_new_expr_name(mab->
						   use_current_label()))));
	
	/*
	 * Set up the argument list to catch the parameter names we need.
	 * We actually don't need them here, but we must create places
	 * for them so the code generator can set them correctly.
	 */
	must.arglist = new mu_state_arglist("params");
	must.arglist->add("params", "object");
	must.arglist->add("params", "environment");
	must.arglist->add("params", "return");
	must.arglist->add("params", "invocation_id");
	must.arglist->add("params", "client");
	must.arglist->add("params", "message");
	
	must.abort_block = mab;
	
	/*
	 * Strip away the ``collapsed_union'' that represents the IDL
	 * and interface.  IIOPXX doesn't require this information.
	 */
	remove_idl_and_interface_ids(pres,
				     stub->itype, stub->i,
				     &operation_itype, &operation_inline);
	
	/* The reply doesn't need  the operation ID either */
	if (!stub->request || !stub->client) {
		remove_operation_id(pres,
				    operation_itype, operation_inline,
				    &operation_itype, &operation_inline);
	}
	
	must.mu_func_params(stub->c_func,
			    operation_itype, operation_inline);
	must.mu_end();
	
	/* Create the CAST body of the stub. */
	if (must.c_block
	    && (must.c_block->kind == CAST_STMT_BLOCK))
		stub_body = must.c_block;
	else {
		stub_body = cast_new_block(0, 0);
		if (must.c_block)
			cast_block_add_stmt(&(stub_body->
					      cast_stmt_u_u.block),
					    must.c_block);
	}
	
	/* Start writing the stub. */
	cast_w_func_type(cfunc->name, cfunct, 0);
	w_printf("\n{\n");
	w_i_printf(1, "FLICK_BUFFER __stream;\n");
	w_i_printf(1, "FLICK_BUFFER *_stream = &__stream;\n");
	
	/*
	 * Initialize `_return' to be zero.  This eliminates compiler
	 * warnings about returning uninitialized data.
	 */
	cast_expr init_expr
		= cast_new_expr_assign(
			cast_new_expr_name("_return"),
			cast_new_expr_lit_int(0, 0));
	
	w_indent(1);
	cast_w_type(cast_new_scoped_name("_return", NULL),
		    cfunct->return_type, 1);
	w_printf(";\n");
	
	w_printf("\n");
	cast_w_stmt(cast_new_stmt_expr(init_expr), 1);
	
	w_i_printf(1,
		   "flick_%s_msg_start_encode(%d);\n",
		   must.get_be_name(),
		   !stub->request);
	
	/* Output the general marshaling code. */
	cast_w_stmt(stub_body, 1);
	w_i_printf(1,
		   "flick_%s_msg_end_encode();\n",
		   must.get_be_name());
	
	w_i_printf(1, "flick_%s_%s_stream_to_msg(_return);\n",
		   must.get_be_name(),
		   must.get_buf_name());
	/* For the request, set the ``response_expected'' field. */
	if (stub->request) {
		if (stub->op_flags & PRES_C_STUB_OP_FLAG_ONEWAY) {
			w_i_printf(1,
				   "flick_%s_msg_set_response_expected"
				   "(0, _return);\n",
				   must.get_be_name());
		} else {
			w_i_printf(1,
				   "flick_%s_msg_set_response_expected"
				   "(1, _return);\n",
				   must.get_be_name());
			w_i_printf(1,
				   "flick_%s_msg_set_reply_handler"
				   "(%s, _return);\n",
				   must.get_be_name(),
				   stub->reply_handler_name);
		}
		w_i_printf(1,
			   "flick_%s_msg_set_principal_mark(%d, _return);\n",
			   must.get_be_name(),
			   must.principal_mark);
	}
	
	cast_w_stmt(cast_new_stmt_expr(emergency_return_value), 1);
	
	mab->end();
	mab->set_reaper_label(reaper_label);
	/* Remove any unneeded abort code */
	mab->rollback();
	
	/* write out the abort stuff */
	cast_w_stmt(mab->get_block_label(), 1);
	cast_w_stmt(reaper_label, 1);
	w_printf("}\n\n");
}

void w_msg_unmarshal_stub(pres_c_1 *pres, int stub_idx)
{
	pres_c_msg_marshal_stub *stub;
	struct mu_abort_block *mab;
	cast_stmt reaper_label;
	
	/* Find the `pres_c_msg_marshal_stub' that describes this stub. */
	assert(pres);
	assert(pres->stubs.stubs_val[stub_idx].kind
	       == PRES_C_MESSAGE_UNMARSHAL_STUB);
	stub = &(pres->stubs.stubs_val[stub_idx].pres_c_stub_u.mustub);
	
	/* Find the CAST declaration of this stub. */
	assert(stub->c_func >= 0);
	assert(stub->c_func < (signed int)pres->stubs_cast.cast_scope_len);
	cast_def *cfunc = &(pres->stubs_cast.cast_scope_val[stub->c_func]);
	
	assert(cfunc->u.kind == CAST_FUNC_DECL);
	cast_func_type *cfunct = &cfunc->u.cast_def_u_u.func_type;
	
	assert(cfunct->return_type->kind == CAST_TYPE_VOID);
	
	emergency_return_value = cast_new_expr_name("return");
	
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
						"flick_%s_msg_error",
						pres->pres_context)),
				cast_new_expr_name("cdr"),
				cast_new_expr_name("iiopxx"),
				cast_new_expr_name(mab->
						   use_current_label()))));
	
	mint_ref            operation_itype;
	pres_c_inline       operation_inline;
	
	/* Build the parameter marshal/unmarshal code. */
	iiopxx_mu_state must(the_state,
			     (MUST_DECODE | MUST_ALLOCATE
			      | ((stub->request)? MUST_REQUEST : MUST_REPLY)),
			     RPCM_TRUSTED, "msg", IIOPXX_NO_SWAP);
	iiopxx_mu_state swap_must(the_state,
				  (MUST_DECODE | MUST_ALLOCATE
				   | ((stub->request)?
				      MUST_REQUEST : MUST_REPLY)),
				  RPCM_TRUSTED, "msg", IIOPXX_SWAP);
	
	/*
	 * Set up the argument list to catch the parameter names we need.
	 * We actually don't need them here, but we must create places
	 * for them so the code generator can set them correctly.
	 */
	must.arglist = new mu_state_arglist("params");
	must.arglist->add("params", "object");
	must.arglist->add("params", "environment");
	must.arglist->add("params", "return");
	must.arglist->add("params", "invocation_id");
	must.arglist->add("params", "client");
	must.arglist->add("params", "message");
	must.abort_block = mab;
	must.current_span = new mu_msg_span;
	must.current_span->set_kind(MSK_SEQUENTIAL);
	must.current_span->begin();
	
	swap_must.arglist = new mu_state_arglist("params");
	swap_must.arglist->add("params", "object");
	swap_must.arglist->add("params", "environment");
	swap_must.arglist->add("params", "return");
	swap_must.arglist->add("params", "invocation_id");
	swap_must.arglist->add("params", "client");
	swap_must.arglist->add("params", "message");
	swap_must.abort_block = mab;
	swap_must.current_span = new mu_msg_span;
	swap_must.current_span->set_kind(MSK_SEQUENTIAL);
	swap_must.current_span->begin();
	
	/*
	 * Set the alignment to 0
	 * XXX -- We actually can determine the alignment here, if we
	 * dig through enough information to find the operation length
	 * added to the known alignment before the operation name, but
	 * to be simple, just force a runtime alignment.
	 */
	must.align_bits = 0;
	must.align_ofs = 0;
	swap_must.align_bits = 0;
	swap_must.align_ofs = 0;
	
	/*
	 * Strip away the ``collapsed_union'' that represents the IDL
	 * and interface.  IIOP doesn't require this information.
	 */
	remove_idl_and_interface_ids(pres,
				     stub->itype, stub->i,
				     &operation_itype, &operation_inline);
	
	/* The reply doesn't need  the operation ID either */
	if (!stub->request || !stub->client) {
		remove_operation_id(pres,
				    operation_itype, operation_inline,
				    &operation_itype, &operation_inline);
	}
	
	must.mu_func_params(stub->c_func,
			    operation_itype, operation_inline);
	must.mu_end();
	swap_must.mu_func_params(stub->c_func,
			    operation_itype, operation_inline);
	swap_must.mu_end();
	
	/* Collapse the span trees and commit their values */
	must.current_span->end();
	swap_must.current_span->end();
	must.current_span->collapse();
	must.current_span->commit();
	swap_must.current_span->collapse();
	swap_must.current_span->commit();
	
	mab->end();
	mab->set_reaper_label(reaper_label);
	/* Remove any unneeded abort code */
	mab->rollback();
	
	/* Start writing the stub. */
	cast_w_func_type(cfunc->name, cfunct, 0);
	w_printf("\n{\n");
	w_i_printf(1, "FLICK_BUFFER __stream;\n");
	w_i_printf(1, "FLICK_BUFFER *_stream = &__stream;\n");
	
	w_printf("\n");
	
	cast_expr msg_expr;
	cast_type msg_type;
	int gotarg = must.arglist->getargs("params", "message",
					   &msg_expr, &msg_type);
	assert(gotarg);assert(msg_expr);assert(msg_type);
	cast_w_stmt(
		cast_new_stmt_expr(
			cast_new_expr_call_1(
				cast_new_expr_name(
					flick_asprintf(
						"flick_%s_%s_msg_to_stream",
						must.get_be_name(),
						must.get_buf_name())),
				msg_expr)),
		1);
	cast_w_stmt(
		cast_new_stmt_expr(
			cast_new_expr_call_1(
				cast_new_expr_name(
					flick_asprintf(
						"flick_%s_msg_start_decode",
						must.get_be_name())),
				cast_new_expr_lit_int(!stub->request, 0))),
		1);
	// We need to emit an if (swap) to determine which code is run...
	cast_w_stmt(
		cast_new_if(
			cast_new_expr_call_0(
				cast_new_expr_name("flick_cdr_swap")),
			swap_must.c_block,
			must.c_block),
		1);
	cast_w_stmt(
		cast_new_stmt_expr(
			cast_new_expr_call_0(
				cast_new_expr_name(
					flick_asprintf(
						"flick_%s_msg_end_decode",
						must.get_be_name())))),
		1);
	cast_w_stmt(cast_new_stmt_expr(emergency_return_value), 1);
	
	/* write out the abort stuff */
	cast_w_stmt(mab->get_block_label(), 1);
	cast_w_stmt(reaper_label, 1);
	w_printf("}\n\n");
}

/* End of file. */

