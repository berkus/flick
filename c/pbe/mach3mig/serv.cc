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

#include "mach3.h"

void mach3_mu_state::mu_server_func_reply(pres_c_server_func *sfunc,
					  pres_c_skel *sstub)
{
	mu_state      *must_out = another(MUST_ENCODE
					  | MUST_DEALLOCATE
					  | MUST_REPLY);
	
	mint_ref       simple_reply_itype;
	pres_c_inline  simple_reply_inline;
	
	/* set the initial maximum message size (for unseen header) */
	((mach3_mu_state *) must_out)->max_msg_size = 32;
	
	/*
	 * Strip away the ``collapsed union'' goo that encodes IDL and
	 * interface information.  We don't need to encode that data for
	 * Mach3MIG IPC because it is manifest in the object (port) references.
	 */
	remove_idl_and_interface_ids(pres,
				     sstub->reply_itype, sfunc->reply_i,
				     &simple_reply_itype, &simple_reply_inline
		);
	/*
	 * Do not call `remove_operation_id' to remove the operation
	 * identifier.  MIG transmits operation reply codes, so we do, too.
	 */
	((mach3_mu_state *) must_out)->set_id_expected(simple_reply_itype);
	
	add_stmt(cast_new_stmt_expr(
		cast_new_expr_call(
			cast_new_expr_name(
				"flick_mach3mig_server_start_encode"),
			0)));
	
	/* Build the reply marshal code. */
	must_out->arglist = arglist;
	must_out->abort_block = abort_block;
	must_out->mu_server_func_target(sfunc);
	must_out->mu_server_func_client(sfunc);
	
	must_out->mu_func_params(sfunc->c_func,
				 simple_reply_itype, simple_reply_inline);
	must_out->mu_end();
	
	/*
	 * Move the generated code back into our initial `mu_state' object and
	 * delete the now unneeded `must_out'.
	 */
	add_stmt(must_out->c_block);
	
	add_stmt(cast_new_stmt_expr(
		cast_new_expr_call(
			cast_new_expr_name("flick_mach3mig_server_end_encode"),
			0)));
	
#if 0
	add_stmt(cast_new_stmt_expr(
		cast_new_expr_assign(
			cast_new_expr_name("_buf_start->Head.msgh_bits"),
			cast_new_expr_name("MACH_MSGH_BITS(19, MACH_MSG_TYPE_MAKE_SEND_ONCE)"))));
#endif
	mach3_mu_state *mout = (mach3_mu_state *)must_out;
	
	if (mout->is_complex == (void *) 1) {
		mout->add_stmt(cast_new_stmt_expr(cast_new_expr_op_assign(
			CAST_BINARY_BOR,
			cast_new_expr_name("_buf_start->Head.msgh_bits"),
			cast_new_expr_name("MACH_MSGH_BITS_COMPLEX"))));
	}
	
	/* Update the maximum message size */
	if (mout->max_msg_size > max_msg_size)
		max_msg_size = mout->max_msg_size;
	
	delete must_out;
}

void w_skel(pres_c_1 *pres, int stub_idx)
{
	int assumptions = RPCM_TRUSTED; /* XXX */
	
	mint_ref      simple_request_itype;
	pres_c_inline simple_request_inline;
	cast_expr error_call;
	struct mu_abort_block *mab;
	cast_stmt reaper_label;
	
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
	
	mach3_mu_state must(the_state,
			    (MUST_DECODE | MUST_ALLOCATE | MUST_REQUEST),
			    assumptions, "server");
	
	emergency_return_value = cast_new_expr_name("return 1");
	
	reaper_label = cast_new_label(
		"_flick_reaper",
		cast_new_return(
			cast_new_expr_lit_int(0,0)));
	mab = new mu_abort_block();
	mab->set_kind(MABK_THREAD);
	mab->begin();
	mab->add_stmt(cast_new_return(
		cast_new_expr_name("FLICK_OPERATION_SUCCESS")));
	/* Setup the abort code */
	mab->add_stmt(cast_new_stmt_expr(
		error_call = cast_new_expr_call_4(
			cast_new_expr_name(flick_asprintf(
				"flick_%s_server_error", pres->pres_context)),
			cast_new_expr_name(must.get_encode_name()),
			cast_new_expr_name(must.get_be_name()),
			cast_new_expr_name("_flick_reaper"),
			cast_new_expr_name(mab->use_current_label()))));
	reaper_label->cast_stmt_u_u.s_label.users++;
	
	/* set the initial maximum message size (for unseen header) */
	must.max_msg_size = 24;
	
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
		 * interface information.  We don't need that information for
		 * Mach3MIG IPC because it is manifest in the object (port)
		 * references.
		 *
		 * The astute reader will note that the function call below
		 * only takes care of half of the problem --- what about the
		 * reply?  The IDL and interface codes are removed from the
		 * reply by a Mach3MIG IPC version of `mu_server_func_reply()'.
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
	
	must.set_id_expected(simple_request_itype);
	
	/* Descend through the message union tree,
	   producing switch statements that decode the message into 
	   separate procedures.  */
	must.mu_decode_switch(dcase,
			      skel->funcs.funcs_len,
			      simple_request_itype);
	
	must.current_span->end();
	must.current_span->collapse();
	must.current_span->commit();
	mab->end();
	mab->set_reaper_label(reaper_label);
	/* Remove unneeded abort code */
	mab->rollback();
	w_printf("boolean_t ");
	cast_w_scoped_name(&cdef->name);
	w_printf("(mach_msg_header_t *InHeadP, "
		 "mach_msg_header_t *OutHeadP)\n");
	w_printf("{\n");
	
	if (must.c_block) {
		/* Encoding/decoding buffer-related variables.  */
		w_i_printf(1, "register mig_reply_header_t *_buf_start;\n");
		w_i_printf(1, "register void *_buf_current;\n\n");
		w_i_printf(1, "struct flick_stub_state _stub_state;\n");
		w_i_printf(1, "flick_mach3mig_server_start_decode();\n");
		
		cast_w_stmt(must.change_stub_state(FLICK_STATE_PROLOGUE), 1);
		cast_w_stmt(must.change_stub_state(FLICK_STATE_UNMARSHAL), 1);
		
		/* Output the complete server dispatcher function.  */
		cast_w_stmt(must.c_block, 1);

		w_i_printf(1, "return 1;\n");
		/* Write out the abort code */
		cast_w_stmt(mab->get_block_label(), 1);
		cast_w_stmt(reaper_label, 1);
	} else {
		warn("Empty server skeleton!");
		w_i_printf(1, "return 0;\n");
	}
	
	w_printf("}\n\n");
}

/* End of file. */

