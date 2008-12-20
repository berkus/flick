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
#include <mom/compiler.h>
#include <mom/c/libcast.h>

#include "sun.h"

void sun_mu_state::mu_server_func_reply(pres_c_server_func *sfunc,
					pres_c_skel *sskel)
{
	mu_state	*must_out = another(MUST_ENCODE
					    | MUST_DEALLOCATE
					    | MUST_REPLY);
	
	mint_ref	simple_reply_itype;
	pres_c_inline	simple_reply_inline;
	
	/*
	 * Strip away the ``collapsed union'' goo that encodes IDL and
	 * interface information.  We don't need to encode that data in ONC/TCP
	 * reply messages.  Similarly, strip away the ``collapsed union'' goo
	 * that represents the operation's reply code (which is a fixed value
	 * --- NOT an indicator of success or failure).  The client knows what
	 * operation it invoked.
	 */
	remove_idl_and_interface_ids(pres,
				     sskel->reply_itype, sfunc->reply_i,
				     &simple_reply_itype, &simple_reply_inline
		);
	remove_operation_id(pres,
			    simple_reply_itype, simple_reply_inline,
			    &simple_reply_itype, &simple_reply_inline
		);
	
	add_stmt(cast_new_stmt_expr(
		cast_new_expr_call(
			cast_new_expr_name("flick_suntcp_server_start_encode"),
			0)));
	
	/* Build the reply marshal code. */
	must_out->abort_block = abort_block;
	must_out->arglist = arglist;
	must_out->mu_server_func_target(sfunc);
	must_out->mu_func_params(sfunc->c_func,
				 simple_reply_itype, simple_reply_inline);
	must_out->mu_end();
	
	/*
	 * Move the generated code back into our initial `mu_state' object and
	 * delete the now unneeded `must_out'.
	 */
	add_stmt(must_out->c_block);
	delete must_out;
	
	add_stmt(cast_new_stmt_expr(
		cast_new_expr_call(
			cast_new_expr_name("flick_suntcp_server_end_encode"),
			0)));
}

void w_skel(pres_c_1 *pres, int stub_idx)
{
	int assumptions = RPCM_TRUSTED; /* XXX */
	mint_ref simple_request_itype;
	pres_c_inline simple_request_inline;
	struct mu_abort_block *mab;
	cast_stmt reaper_label;
	cast_expr error_call;
	
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
	
	/* return SUCCESS since it should send the reply (which has an error)*/
	emergency_return_value = cast_new_expr_name(
		"return FLICK_OPERATION_SUCCESS");
	reaper_label = cast_new_label(
		"_flick_reaper",
		cast_new_return(
			cast_new_expr_name("FLICK_OPERATION_FAILURE")));
	sun_mu_state must(the_state,
			  MUST_DECODE | MUST_ALLOCATE | MUST_REQUEST,
			  assumptions, "server");
	mab = new mu_abort_block();
	mab->set_kind(MABK_THREAD);
	mab->begin();
	mab->add_stmt(cast_new_return(
		cast_new_expr_name("FLICK_OPERATION_SUCCESS")));
	mab->add_stmt(cast_new_stmt_expr(
		error_call = cast_new_expr_call_4(
			cast_new_expr_name(flick_asprintf(
				"flick_%s_server_error", pres->pres_context)),
			cast_new_expr_name(must.get_encode_name()),
			cast_new_expr_name(must.get_be_name()),
			cast_new_expr_name("_flick_reaper"),
			cast_new_expr_name(mab->use_current_label()))));
	reaper_label->cast_stmt_u_u.s_label.users++;
	
	must.abort_block = mab;
	must.current_span = new mu_msg_span;
	must.current_span->set_kind(MSK_SEQUENTIAL);
	must.current_span->begin();
	
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
		 * interface information.  Our server skeleton does not need to
		 * decode program and version identifiers; this is handled by
		 * Flick's ONC/TCP runtime.
		 *
		 * The astute reader will note that the function call below
		 * only takes care of half of the problem --- what about the
		 * reply?  The IDL and interface codes are removed from the
		 * reply by an ONC/TCP version of `mu_server_func_reply()'.
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
			panic("Unknown function kind in w_server_skel()");
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
	
	/* Descend through the message union tree,
	   producing switch statements that decode the message into 
	   separate procedures.  */
	
	must.mu_decode_switch(dcase,
			      skel->funcs.funcs_len, simple_request_itype);

	/* Collapse the span trees and commit their values */
	must.current_span->end();
	must.current_span->collapse();
	must.current_span->commit();

	mab->end();
	mab->set_reaper_label(reaper_label);
	/* Remove unneeded abort code */
	mab->rollback();
	
	w_printf("flick_operation_success_t ");
	cast_w_scoped_name(&cdef->name);
	w_printf("(FLICK_BUFFER *InHeadP, "
		 "FLICK_BUFFER *OutHeadP)\n");
	w_printf("{\n");

	if (must.c_block) {
		/* Encoding/decoding buffer-related variables.  */
		w_i_printf(1, "FLICK_BUFFER *_stream;\n\n");

		/* Invoke the macro to set up for decoding the
                   received message.  */
		w_i_printf(1, "flick_suntcp_server_start_decode();\n");
		
		cast_w_stmt(must.change_stub_state(FLICK_STATE_PROLOGUE), 1);
		cast_w_stmt(must.change_stub_state(FLICK_STATE_UNMARSHAL), 1);
		/* Output the complete server dispatcher function.  */
		cast_w_stmt(must.c_block, 1);
	} else
		warn("Empty server skeleton!");
	
	w_i_printf(1, "return FLICK_OPERATION_SUCCESS;\n");
	cast_w_stmt(mab->get_block_label(), 1);
	if( reaper_label->cast_stmt_u_u.s_label.users )
		cast_w_stmt(reaper_label, 1);
	w_printf("}\n\n");
}

/* End of file. */

