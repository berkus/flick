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
#include <mom/c/libpres_c.h>

#include "iiopxx.h"

void w_client_stub(pres_c_1 *pres, int stub_idx)
{
	assert(pres);
	
	pres_c_stub *stub = &pres->stubs.stubs_val[stub_idx];
	pres_c_client_stub *cstub = &stub->pres_c_stub_u.cstub;
	cast_def *cfunc = &pres->stubs_cast.cast_scope_val[cstub->c_func];
	cast_func_type *cfunct = &cfunc->u.cast_def_u_u.func_type;
	struct mu_abort_block *mab;
	cast_stmt reaper_label;
	char *escape_label, *early_escape_label;
	
	mint_ref            operation_itype;
	pres_c_inline       operation_inline;
	
	int assumptions = RPCM_TRUSTED; /* XXX */
	const char *emergency_return_string;
	
	struct scml_context *sc;
	
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
					flick_asprintf("flick_%s_client_error",
						       pres->pres_context)),
				cast_new_expr_name("cdr"),
				cast_new_expr_name("iiopxx"),
				cast_new_expr_name(mab->
						   use_current_label()))));
	early_escape_label = mab->use_current_label();
	/* The order of the musts below is crucial to
	   the abort code working properly */
	
	/* Build the target object marshaling code. */
	iiopxx_target_mu_state must_target(the_state,
					   (MUST_ENCODE | MUST_DEALLOCATE), 0,
					   "client");
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
	iiopxx_mu_state must_in(the_state,
				(MUST_ENCODE | MUST_DEALLOCATE | MUST_REQUEST),
				assumptions, "client");
	must_in.abort_block = mab;
	remove_idl_and_interface_ids(pres,
				     cstub->request_itype, cstub->request_i,
				     &operation_itype, &operation_inline);
	must_in.mu_func_params(cstub->c_func,
			       operation_itype, operation_inline);
	must_in.mu_end();
	
	/* Get an abort label for the statements below to use */
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
	iiopxx_mu_state must_out(the_state,
				 (MUST_DECODE | MUST_ALLOCATE | MUST_REPLY),
				 assumptions, "client", IIOPXX_NO_SWAP);
	
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
	
	iiopxx_mu_state must_swap(the_state,
				  (MUST_DECODE | MUST_ALLOCATE | MUST_REPLY),
				  assumptions, "client", IIOPXX_SWAP);
	
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
	
	cast_expr init_expr = 0;
	if (cfunct->return_type->kind != CAST_TYPE_VOID) {
		/*
		 * Initialize `_return' to be zero.  This eliminates compiler
		 * warnings about returning uninitialized data.
		 */
		init_expr = cast_new_expr_assign_to_zero(
			cast_new_expr_name("_return"),
			cfunct->return_type,
			&(pres->cast)
			);
	}
	
	tag_list *tl;
	char *plain_name;
	int size = 0;
	unsigned int lpc;
	
	tl = create_tag_list(0);
	
	for( lpc = 0; lpc < cfunc->name.cast_scoped_name_len; lpc++ ) {
		size += strlen(cfunc->name.cast_scoped_name_val[lpc].name) + 1;
	}
	plain_name = (char *)mustmalloc(size + 1);
	plain_name[0] = 0;
	for( lpc = 0; lpc < cfunc->name.cast_scoped_name_len; lpc++ ) {
		if( lpc > 0 )
			strcat(plain_name, "_");
		strcat(plain_name, cfunc->name.cast_scoped_name_val[lpc].name);
	}
	add_tag(tl, "plain_name", TAG_STRING, plain_name);
	add_tag(tl, "cfunc", TAG_CAST_DEF, cfunc);
	if( init_expr ) {
		cast_def_t return_def;
		
		return_def = cast_new_def_t(cast_new_scoped_name("_return",
								 NULL),
					    CAST_SC_NONE,
					    CAST_VAR_DECL,
					    PASSTHRU_DATA_CHANNEL,
					    CAST_PROT_NONE);
		return_def->u.cast_def_u_u.var_type = cfunct->return_type;
		add_tag(tl, "init_expr", TAG_CAST_EXPR, init_expr);
		add_tag(tl, "return_def", TAG_CAST_DEF, return_def);
	}
	add_tag(tl, "be_name", TAG_STRING, must_in.get_be_name());
	add_tag(tl, "target_block", TAG_CAST_STMT, must_target.c_block);
	add_tag(tl, "in_block", TAG_CAST_STMT, must_in.c_block);
	add_tag(tl, "out_block", TAG_CAST_STMT, must_out.c_block);
	add_tag(tl, "swap_block", TAG_CAST_STMT, must_swap.c_block);
	add_tag(tl, "send_state_change", TAG_CAST_STMT,
		must_in.change_stub_state(FLICK_STATE_SEND));
	add_tag(tl, "oneway_flag", TAG_BOOL,
		((cstub->op_flags & PRES_C_STUB_OP_FLAG_ONEWAY) ? 1 : 0));
	add_tag(tl, "emergency_return_string", TAG_STRING,
		emergency_return_string);
	add_tag(tl, "abort_block", TAG_CAST_STMT, mab->get_block_label());
	add_tag(tl, "reaper_label", TAG_CAST_STMT, reaper_label);
	add_tag(tl, "escape_label", TAG_STRING, escape_label);
	add_tag(tl, "early_escape_label", TAG_STRING, early_escape_label);
	
	sc = new scml_context;
	sc->set_scope(the_state->get_scml_root());
	cast_w_func_type(cfunc->name, cfunct, 0);
	sc->exec_cmd("client_stub", tl, 0);
	delete_tag_list(tl);
	delete sc;
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
	
	mab->add_stmt(
		cast_new_stmt_expr(
			cast_new_expr_call_3(
				cast_new_expr_name(
					flick_asprintf(
						"flick_%s_client_error",
						pres->pres_context)),
				cast_new_expr_name("cdr"),
				cast_new_expr_name("iiopxx"),
				cast_new_expr_name(mab->
						   use_current_label()))));
	
	/* Build the target object marshaling code. */
	iiopxx_target_mu_state must_target(the_state,
					   MUST_ENCODE | MUST_DEALLOCATE,
					   0, "send");
	must_target.abort_block = mab;
	must_target.mu_func_params(sstub->c_func, sstub->target_itype,
				   sstub->target_i);
	must_target.mu_end();

	/* Build the parameter marshal/unmarshal code. */
	iiopxx_mu_state must_send(the_state,
				  MUST_ENCODE | MUST_DEALLOCATE,
				  assumptions, "send");
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
	int gotarg = 0;
	
	cast_w_func_type(cfunc->name, cfunct, 0);
	w_printf("\n{\n");
	
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

