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

#include "mach3.h"

/* MAX_STATIC_MESSAGE_SIZE defines the largest message size that is
 * allowed on the stack.  If a message could potentially be larger (in
 * particular, unbounded), then a dynamically-allocated message buffer
 * is used (supplied by the runtime library).
 *
 * XXX - Note that this does not specify the maximum size of a message
 * received by the server.  Whatever creates the receive buffer for
 * the server designates the maximum message allowed to be passed by
 * the system.
 */
#define MAX_STATIC_MESSAGE_SIZE 65536

static void do_stub(pres_c_1 *pres, cast_ref c_func,
		    mint_ref request_itype, mint_ref reply_itype,
		    pres_c_inline request_i, pres_c_inline reply_i,
		    mint_ref target_itype, mint_ref client_itype,
		    pres_c_inline target_i, pres_c_inline client_i,
		    pres_c_stub_op_flags op_flags)
{
	assert(pres);
	
	cast_def *cfunc = &pres->stubs_cast.cast_scope_val[c_func];
	cast_func_type *cfunct = &cfunc->u.cast_def_u_u.func_type;
	struct mu_abort_block *mab;
	cast_stmt reaper_label;
	char *escape_label;
	
	mint_ref            operation_itype;
	pres_c_inline       operation_inline;
	
	int assumptions = RPCM_TRUSTED; /* XXX */
	const char *emergency_return_string;
	
	/*****/
	
	if (cfunct->return_type->kind != CAST_TYPE_VOID)
		emergency_return_string = "return _return";
	else
		emergency_return_string = "return";
	emergency_return_value = cast_new_expr_name(emergency_return_string);

	cast_stmt reaper_stmt;
	/* Start the abort code */
	if( !strcmp( "mig", pres->pres_context ) &&
	    cfunct->return_type->kind != CAST_TYPE_VOID ) {
		reaper_stmt = cast_new_if(
			cast_new_binary_expr(CAST_BINARY_NE,
					     cast_new_expr_name(
						     "MACH_MSG_SUCCESS"),
					     cast_new_expr_name(
						     "_ipc_return")),
			cast_new_return(cast_new_expr_name("_ipc_return")),
			cast_new_return(cast_new_expr_name("_return")));
	}
	else if( cfunct->return_type->kind != CAST_TYPE_VOID ) {
		reaper_stmt = cast_new_return(cast_new_expr_name("_return"));
	}
	else
		reaper_stmt = cast_new_stmt_expr(cast_new_expr_name("return"));
	
	reaper_label = cast_new_label("_flick_reaper", reaper_stmt);
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
				cast_new_expr_name("mach3mig"),
				cast_new_expr_name("mig"),
				cast_new_expr_name(mab->
						   use_current_label()))));
	/* The order of the musts below is crucial to
	   the abort code working properly */
	
	/* Build the client object marshaling code. */
	mach3_client_mu_state must_client(the_state,
					  MUST_ENCODE, 0,
					  "client");
	must_client.add_stmt(must_client.
			     change_stub_state(FLICK_STATE_PROLOGUE));
	must_client.add_stmt(must_client.
			     change_stub_state(FLICK_STATE_MARSHAL));
	must_client.abort_block = mab;
	if (client_i) {
		must_client.arglist = new mu_state_arglist("params");
		must_client.arglist->add("params", "client");
		must_client.mu_func_params(c_func, client_itype, client_i);
		must_client.mu_end();
	} else {
		must_client.c_block = cast_new_stmt_expr(cast_new_expr_call_4(
			cast_new_expr_name(
				flick_asprintf("flick_%s_%s_%s_client",
					       must_client.get_be_name(),
					       must_client.get_which_stub(),
					       must_client.get_buf_name())),
			cast_new_expr_name(
				(op_flags & PRES_C_STUB_OP_FLAG_ONEWAY) ?
				"MACH_PORT_NULL" :
				"mig_get_reply_port()"),
			cast_new_expr_lit_int(0, 0),
			cast_new_expr_lit_int(0, 0),
			cast_new_expr_name(must_client.get_encode_name())));
	}
	
	/* Build the target object marshaling code. */
	mach3_target_mu_state must_target(the_state,
					  MUST_ENCODE, 0,
					  "client");
	must_target.abort_block = mab;;
	must_target.arglist = new mu_state_arglist("params");
	must_target.arglist->add("params", "object");
	must_target.mu_func_params(c_func, target_itype, target_i);
	must_target.mu_end();
	
        /* Build the parameter marshal/deallocation code. */
	mach3_mu_state must_in(the_state,
			       (MUST_ENCODE | MUST_DEALLOCATE | MUST_REQUEST),
			       assumptions, "client");
	/* set the initial maximum message size (for unseen header) */
	must_in.max_msg_size = 24;
	
	must_in.abort_block = mab;
	remove_idl_and_interface_ids(pres,
				     request_itype, request_i,
				     &operation_itype, &operation_inline);
	must_in.set_id_expected(operation_itype);
	
	must_in.mu_func_params(c_func, operation_itype, operation_inline);
	must_in.add_stmt(must_in.change_stub_state(FLICK_STATE_SEND));
	must_in.mu_end();
	
	escape_label = mab->use_current_label();
	
	struct mu_abort_block *out_mab;
	
	/* If the stub is oneway we need to dump any abort code from
	   the unmarshaling must's into a dummy abort context.
	   Otherwise, we will have a bunch of unused abort code. */
	if (op_flags & PRES_C_STUB_OP_FLAG_ONEWAY) {
		out_mab = new mu_abort_block();
		out_mab->set_kind(MABK_THREAD);
		out_mab->begin();
	}
	else
		out_mab = mab;
	
	/* Build the unmarshal code, but only if this is a two-way stub. */
	mach3_mu_state must_out(the_state,
				(MUST_DECODE | MUST_ALLOCATE | MUST_REPLY),
				assumptions, "client");
	
	/* Set up the argument list to catch parameter names we may need. */
	must_out.arglist = new mu_state_arglist("params");
	must_out.arglist->add("params", "environment");
	
	/* set the initial maximum message size (for unseen header) */
	must_out.max_msg_size = 32;
	must_out.add_stmt(must_out.change_stub_state(FLICK_STATE_UNMARSHAL));
	must_out.abort_block = out_mab;
	must_out.current_span = new mu_msg_span;
	must_out.current_span->set_kind(MSK_SEQUENTIAL);
	must_out.current_span->begin();
	if (reply_i) {
		remove_idl_and_interface_ids(pres,
					     reply_itype, reply_i,
					     &operation_itype,
					     &operation_inline);
		/*
		 * Do not call `remove_operation_id' to remove the operation
		 * identifier.  MIG transmits operation reply codes, so we do,
		 * too.
		 */
		must_out.set_id_expected(operation_itype);
		must_out.mu_func_params(c_func,
					operation_itype, operation_inline);
		must_out.mu_end();
		
		/* Minimize the span tree and commit the values in it. */
		must_out.current_span->end();
		must_out.current_span->collapse();
		must_out.current_span->commit();
	} else {
		assert(must_out.max_msg_size == 32);
	}
	
	mab->end();
	mab->set_reaper_label(reaper_label);
	/* Remove any unneeded abort code */
	mab->rollback();
	
	/* Start writing the client stub.  */
	cast_w_func_type(cfunc->name, cfunct, 0);
	w_printf("\n{\n");
	if (must_in.max_msg_size < MAX_STATIC_MESSAGE_SIZE &&
	    must_out.max_msg_size < MAX_STATIC_MESSAGE_SIZE) {
		w_i_printf(1, "char _global_buf_start[%d];\n",
			   ((must_in.max_msg_size > must_out.max_msg_size) ?
			    must_in.max_msg_size:
			    must_out.max_msg_size));
		w_i_printf(1, "void *_buf_end = (void *)&(_global_buf_start[%d]);\n",
			   ((must_in.max_msg_size > must_out.max_msg_size) ?
			    must_in.max_msg_size:
			    must_out.max_msg_size));
	} else {
		if (must_in.msg_option_expr) {
			must_in.msg_option_expr =
				cast_new_binary_expr(
					CAST_BINARY_BOR,
					must_in.msg_option_expr,
					cast_new_expr_name("MACH_RCV_LARGE"));
		} else {
			must_in.msg_option_expr =
				cast_new_expr_name("MACH_RCV_LARGE");
		}
	}
	w_i_printf(1, "register mig_reply_header_t *_buf_start;\n");
	w_i_printf(1, "register void *_buf_current;\n");
	w_i_printf(1, "struct flick_stub_state _stub_state;\n");
	w_i_printf(1, "kern_return_t _ipc_return = KERN_SUCCESS;\n");

	switch (cfunct->return_type->kind) {
	case CAST_TYPE_VOID:
		w_printf("\n");
		break;
		
	case CAST_TYPE_NAME:
		if (!strcmp(cfunct->return_type->cast_type_u_u.name.
			    cast_scoped_name_val[0].name,
			    "kern_return_t")) {
			w_indent(1);
			cast_w_type(cast_new_scoped_name("_return", NULL),
				    cfunct->return_type, 1);
			w_printf(" = 0;\n\n");
			break;
		}
		
	default:
		w_indent(1);
		cast_w_type(cast_new_scoped_name("_return", NULL),
			    cfunct->return_type, 1);
		w_printf(";\n\n");
		cast_w_stmt(
			cast_new_stmt_expr(
				cast_new_expr_assign_to_zero(
					cast_new_expr_name("_return"),
					cfunct->return_type,
					&(pres->cast))),
			1);
	}
	
	w_i_printf(1,
		   "flick_%s_client_start_encode();\n",
		   must_in.get_be_name());
	
	cast_w_stmt(must_client.c_block, 1);
	cast_w_stmt(must_target.c_block, 1);
	
	/* Output the general marshaling code. */
	cast_w_stmt(must_in.c_block, 1);
	w_i_printf(1,
		   "flick_%s_client_end_encode();\n",
		   must_in.get_be_name());
	
	if (must_in.is_complex == (void *) 1) {
		cast_w_stmt(cast_new_stmt_expr(cast_new_expr_op_assign(
			CAST_BINARY_BOR,
			cast_new_expr_name("_buf_start->Head.msgh_bits"),
			cast_new_expr_name("MACH_MSGH_BITS_COMPLEX"))),
			1);
	}
	
	/* Invoke the appropriate runtime function to perform the RPC. */
	cast_expr rpc;
	rpc = cast_new_expr_call_2(
		cast_new_expr_name(
			flick_asprintf(
				"flick_%s_%s",
				must_in.get_be_name(),
				((op_flags & PRES_C_STUB_OP_FLAG_ONEWAY) ?
				 "send" :
				 ((must_in.max_msg_size <
				   MAX_STATIC_MESSAGE_SIZE &&
				   must_out.max_msg_size <
				   MAX_STATIC_MESSAGE_SIZE)?
				  "rpc_static" : "rpc_dynamic")))),
		must_in.msg_option_expr ?
		must_in.msg_option_expr :
		cast_new_expr_name("MACH_MSG_OPTION_NONE"),
		must_in.timeout_expr ?
		must_in.timeout_expr :
		cast_new_expr_name("MACH_MSG_TIMEOUT_NONE"));

	cast_w_stmt(must_in.change_stub_state(FLICK_STATE_SEND_RECEIVE),
		    1);
	cast_w_stmt(
		cast_new_stmt_expr(
			cast_new_expr_assign(
				cast_new_expr_name("_ipc_return"),
				rpc)),
		1);
	
	/* Check for errors from the IPC operation. */
	w_i_printf(1,
		   "if (_ipc_return != MACH_MSG_SUCCESS)\n");
	w_i_printf(2,
		   "flick_stub_error(FLICK_ERROR_COMMUNICATION, %s);\n",
		   escape_label);
	
	/* Output the reply unmarshaling code. */
	if (reply_i) {
		if (op_flags & PRES_C_STUB_OP_FLAG_ONEWAY) {
			/* XXX - we may still need to alloc/dealloc space */
			/* XXX - wrong to do this: */
			/* cast_w_stmt(must_out.c_block, 1);*/
		} else {
			w_i_printf(1,
				   "flick_%s_client_start_decode();\n",
				   must_in.get_be_name());
			cast_w_stmt(must_out.c_block, 1);
			w_i_printf(1,
				   "flick_%s_client_end_decode();\n",
				   must_in.get_be_name());
		}
	}
	
	if (cfunct->return_type->kind != CAST_TYPE_VOID)
		w_i_printf(1, "return _return;\n");
	else
		w_i_printf(1, "return;\n" );
	/* Write out the abort code */
	cast_w_stmt(mab->get_block_label(), 1);
	cast_w_stmt(reaper_label, 1);
	w_printf("}\n\n");
}

/* Start the client stub */
void w_client_stub(pres_c_1 *pres, int stub_idx)
{
	assert(pres);

	pres_c_stub *stub = &pres->stubs.stubs_val[stub_idx];
	assert(stub->kind == PRES_C_CLIENT_STUB);
	pres_c_client_stub *cstub = &stub->pres_c_stub_u.cstub;

	assert(cstub->request_i);
	assert(cstub->reply_i);

	do_stub(pres, cstub->c_func,
		cstub->request_itype, cstub->reply_itype,
		cstub->request_i, cstub->reply_i,
		cstub->target_itype, cstub->client_itype,
		cstub->target_i, cstub->client_i,
		cstub->op_flags);
}

#if 0
void w_send_stub(pres_c_1 *pres, int stub_idx)
{
	assert(pres);

	pres_c_stub *stub = &pres->stubs.stubs_val[stub_idx];
	assert(stub->kind == PRES_C_SEND_STUB);
	pres_c_msg_stub *sstub = &stub->pres_c_stub_u.send_stub;

	do_stub(pres, sstub->c_func,
		sstub->msg_itype, mint_ref_null,
		sstub->msg_i, 0,
		sstub->target_itype, mint_ref_null,
		sstub->target_i, 0,
		PRES_C_STUB_OP_FLAG_ONEWAY /* XXX --- Right? */);
}
#endif

/* End of file. */

