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
#include <string.h>
#include <mom/compiler.h>
#include <mom/c/libcast.h>

#include "fluke.h"

void w_skel(pres_c_1 *pres, int stub_idx)
{
	assert(pres);
	
	pres_c_stub *stub = &pres->stubs.stubs_val[stub_idx];
	assert(stub->kind == PRES_C_SERVER_SKEL
	       || stub->kind == PRES_C_CLIENT_SKEL);
	pres_c_skel *skel;
	if (stub->kind == PRES_C_SERVER_SKEL)
		skel = &stub->pres_c_stub_u.sskel;
	else
		skel = &stub->pres_c_stub_u.cskel;
	assert(skel->c_def >= 0);
	assert(skel->c_def < (signed int)pres->stubs_cast.cast_scope_len);
	cast_def *cdef = &pres->stubs_cast.cast_scope_val[skel->c_def];
	
	mint_ref      simple_request_itype;
	pres_c_inline simple_request_inline;
	
	/* Get an abort context and declare some other stuff for aborts */
	struct mu_abort_block *mab;
	cast_expr error_call;
	cast_stmt reaper_label;
	
	fluke_mu_state must(the_state,
			    (MUST_DECODE | MUST_ALLOCATE | MUST_REQUEST),
			    RPCM_TRUSTED, "server");
	
	emergency_return_value =
		cast_new_expr_name("return DISPATCH_ACK_SEND");
	reaper_label = cast_new_label(
		"_flick_reaper",
		cast_new_return(
			cast_new_expr_name(FLUKE_SERVER_NO_REPLY_VALUE)));
	mab = new mu_abort_block();
	mab->set_kind(MABK_THREAD);
	mab->begin();
	mab->add_stmt(cast_new_return(
		cast_new_expr_name("DISPATCH_ACK_SEND")));
	must.abort_block = mab;
	must.current_span = new mu_msg_span;
	must.current_span->set_kind(MSK_SEQUENTIAL);
	must.current_span->begin();
	
	/* This macro should make a reply with the specified error code
	   and return (if it can). */
	mab->add_stmt(cast_new_stmt_expr(
		error_call = cast_new_expr_call_4(
			cast_new_expr_name(flick_asprintf(
				"flick_%s_server_error", pres->pres_context)),
			cast_new_expr_name(must.get_encode_name()),
			cast_new_expr_name(must.get_be_name()),
			cast_new_expr_name("_flick_reaper"),
			cast_new_expr_name(mab->use_current_label()))));
	reaper_label->cast_stmt_u_u.s_label.users++;
	
	/*
	 * Initialize `simple_request_itype' to the "whole" MINT request type,
	 * so that in case we have no server-side functions, we'll still point
	 * to *some* valid MINT type!
	 */
	simple_request_itype = skel->request_itype;
	
	/*
	 * Build an initial array of `decode_switch_case's for the top level of
	 * decoding.
	 */
	decode_switch_case *dcase;
	
	dcase = (decode_switch_case *)
		mustmalloc(skel->funcs.funcs_len * sizeof(decode_switch_case));
	
	for (unsigned int i = 0; i < skel->funcs.funcs_len; i++) {
		/*
		 * Strip away the ``collapsed union'' goo that encodes IDL and
		 * interface information.  We don't need that information for
		 * Fluke IPC because it is manifest in the object references.
		 *
		 * The astute reader will note that the function call below
		 * only takes care of half of the problem --- what about the
		 * reply?  The IDL and interface codes are removed from the
		 * reply by a Fluke IPC version of `mu_server_func_reply()'.
		 */
		pres_c_inline inl;
		
		switch (skel->funcs.funcs_val[i].kind) {
		case PRES_C_SERVER_FUNC:
			inl = skel->funcs.funcs_val[i].pres_c_func_u.sfunc.
			      request_i;
			break;
		case PRES_C_RECEIVE_FUNC:
			inl = skel->funcs.funcs_val[i].pres_c_func_u.rfunc.
			      msg_i;
			break;
		default:
			panic("Unknown function kind in w_skel()");
		}
		
		remove_idl_and_interface_ids(pres,
					     skel->request_itype,
					     inl,
					     &simple_request_itype,
					     &simple_request_inline);
		
		dcase[i].inl  = simple_request_inline;
		dcase[i].func = &skel->funcs.funcs_val[i];
		dcase[i].skel = skel;
	}

	/* Use `mu_decode_switch' to produce the code to discriminate between
	   the possible set of messages, unmarshal the corresponding data,
	   invoke the server work functions, and marshal the replies.  Yes,
	   from this high up, everything looks simple! */
	must.mu_decode_switch(dcase,
			      skel->funcs.funcs_len,
			      simple_request_itype);
	
	/* Collapse the span tree and commit it */
	must.current_span->end();
	must.current_span->collapse();
	must.current_span->commit();
	
	mab->end();
	mab->set_reaper_label(reaper_label);
	/* Remove unneeded abort code */
	mab->rollback();
	
	/* Output the server dispatch function. */
	w_printf("mom_dispatch_action_t ");
	cast_w_scoped_name(&cdef->name);
	w_printf(("(void *_dispatch_obj, "
		  "mom_dispatch_data_t *_dispatch_data)\n"));
	w_printf("{\n");
	if (must.c_block) {
		w_i_printf(1, "flick_marshal_stream_struct _stream_struct;\n");
		w_i_printf(1, "const flick_marshal_stream_t _stream ");
		w_printf("= &_stream_struct;\n\n");
		w_i_printf(1, "flick_fluke_server_start_decode();\n");
		cast_w_stmt(must.change_stub_state(FLICK_STATE_PROLOGUE), 1);
		cast_w_stmt(must.change_stub_state(FLICK_STATE_UNMARSHAL), 1);
		/*
		 * The matching `end_decode' macro is output by `mu_state::
		 * mu_server_func'.
		 */
		cast_w_stmt(must.c_block, 1);
	} else
		warn("Empty server skeleton!");
	
	cast_w_stmt(
		cast_new_return(
			cast_new_expr_name(
				FLUKE_SERVER_NO_REPLY_VALUE)),
		1);
	if (must.c_block) {
		/* Output the abort block */
		cast_w_stmt(mab->get_block_label(), 1);
		cast_w_stmt(reaper_label, 1);
	}
	w_printf("}\n\n");
}

/* End of file. */

