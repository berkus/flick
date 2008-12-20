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
#include <mom/compiler.h>
#include <mom/c/libcast.h>

#include "iiopxx.h"

void iiopxx_mu_state::mu_server_func(pres_c_inline inl, mint_ref tn_r,
				     pres_c_server_func *sfunc,
				     pres_c_skel *sstub)
{
	cast_def *cfunc = &pres->stubs_cast.cast_scope_val[sfunc->c_func];
	cast_func_type *cfunct = &cfunc->u.cast_def_u_u.func_type;
	struct mu_abort_block *mab_par, *mab_thr;
	int size = 0;
	unsigned int lpc;
	char *plain_name;
	cast_stmt cstmt;
	tag_list *tl;
	tag_item *ti;
	
	tl = create_tag_list(0);
	for( lpc = 0; lpc < cfunc->name.cast_scoped_name_len; lpc++ ) {
		size += strlen(cfunc->name.cast_scoped_name_val[lpc].name) + 1;
	}
	plain_name = (char *)mustmalloc(strlen("POA_") + size + 1);
	strcpy(plain_name, "POA_");
	for( lpc = 0; lpc < cfunc->name.cast_scoped_name_len; lpc++ ) {
		if( lpc > 0 )
			strcat(plain_name, "_");
		strcat(plain_name, cfunc->name.cast_scoped_name_val[lpc].name);
	}
	add_tag(tl, "plain_name", TAG_STRING, plain_name);
	add_tag(tl, "be_name", TAG_STRING, get_be_name());
	ti = add_tag(tl, "my_sections", TAG_STRING_ARRAY, 3);
	ti->data.tag_data_u.str_a.str_a_val[0] = ir_strlit("u");
	ti->data.tag_data_u.str_a.str_a_val[1] = ir_strlit("c");
	ti->data.tag_data_u.str_a.str_a_val[2] = ir_strlit("m");
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

	cstmt = cast_new_stmt_handler("scml-code-cast-handler",
				      "server-stub-code(cycle::decl)",
				      "<cycle::decl name=plain_name sections=my_sections>",
				      ptr_to_tag_ref("struct scml_scope *",
						     state->
						     get_scml_root()),
				      ptr_to_tag_ref("tag_list *", tl),
				      NULL);
	add_stmt(cstmt);
	cstmt = cast_new_stmt_handler("scml-code-cast-handler",
				      "server-stub-code(gtod::decl)",
				      "<gtod::decl name=plain_name sections=my_sections>",
				      ptr_to_tag_ref("struct scml_scope *",
						     state->
						     get_scml_root()),
				      ptr_to_tag_ref("tag_list *", tl),
				      NULL);
	add_stmt(cstmt);
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
	cstmt = cast_new_stmt_handler("scml-code-cast-handler",
				      "server-stub-code(rt::stop)",
				      "<rt::stop name=plain_name>",
				      ptr_to_tag_ref("struct scml_scope *",
						     state->
						     get_scml_root()),
				      ptr_to_tag_ref("tag_list *", tl),
				      NULL);
	add_stmt(cstmt);
	cstmt = cast_new_stmt_handler("scml-code-cast-handler",
				      "server-stub-code(cycle::start)",
				      "<cycle::start name=plain_name>",
				      ptr_to_tag_ref("struct scml_scope *",
						     state->
						     get_scml_root()),
				      ptr_to_tag_ref("tag_list *", tl),
				      NULL);
	add_stmt(cstmt);
	mu_func_params(sfunc->c_func, tn_r, inl);
	cstmt = cast_new_stmt_handler("scml-code-cast-handler",
				      "server-stub-code(cycle::start)",
				      "<cycle::stop name=plain_name section=\"u\">",
				      ptr_to_tag_ref("struct scml_scope *",
						     state->
						     get_scml_root()),
				      ptr_to_tag_ref("tag_list *", tl),
				      NULL);
	add_stmt(cstmt);
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
	cstmt = cast_new_stmt_handler("scml-code-cast-handler",
				      "server-stub-code(cycle::start)",
				      "<cycle::start name=plain_name>",
				      ptr_to_tag_ref("struct scml_scope *",
						     state->
						     get_scml_root()),
				      ptr_to_tag_ref("tag_list *", tl),
				      NULL);
	add_stmt(cstmt);
	cstmt = cast_new_stmt_handler("scml-code-cast-handler",
				      "server-stub-code(gtod::start)",
				      "<gtod::start name=plain_name>",
				      ptr_to_tag_ref("struct scml_scope *",
						     state->
						     get_scml_root()),
				      ptr_to_tag_ref("tag_list *", tl),
				      NULL);
	add_stmt(cstmt);
	mu_server_func_call(actual_func_invocation_cexpr);
	cstmt = cast_new_stmt_handler("scml-code-cast-handler",
				      "server-stub-code(gtod::stop)",
				      "<gtod::stop name=plain_name section=\"c\">",
				      ptr_to_tag_ref("struct scml_scope *",
						     state->
						     get_scml_root()),
				      ptr_to_tag_ref("tag_list *", tl),
				      NULL);
	add_stmt(cstmt);
	cstmt = cast_new_stmt_handler("scml-code-cast-handler",
				      "server-stub-code(cycle::stop)",
				      "<cycle::stop name=plain_name section=\"c\">",
				      ptr_to_tag_ref("struct scml_scope *",
						     state->
						     get_scml_root()),
				      ptr_to_tag_ref("tag_list *", tl),
				      NULL);
	add_stmt(cstmt);
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
		mu_server_func_reply(sfunc, sstub);
		cstmt = cast_new_stmt_handler(
			"scml-code-cast-handler",
			"server-stub-code(cycle::print)",
			"<cycle::print name=plain_name sections=my_sections>",
			ptr_to_tag_ref("struct scml_scope *",
				       state->get_scml_root()),
			ptr_to_tag_ref("tag_list *", tl),
			NULL);
		add_stmt(cstmt);
		cstmt = cast_new_stmt_handler(
			"scml-code-cast-handler",
			"server-stub-code(gtod::print)",
			"<gtod::print name=plain_name sections=my_sections>",
			ptr_to_tag_ref("struct scml_scope *",
				       state->get_scml_root()),
			ptr_to_tag_ref("tag_list *", tl),
			NULL);
		add_stmt(cstmt);
	}
	else
		add_stmt(cast_new_return(cast_new_expr_name("")));
	
	delete arglist;
	arglist = oldlist;
	add_stmt(cast_new_break());
	abort_block = mab_par;
	mab_thr->end();
	mab_thr->set_reaper_label(mab_par->get_current_label());
	mab_thr->rollback();
	add_stmt(mab_thr->get_block_label());
	/* End the new scope we created. */
	cast_stmt new_c_block = c_block;
	c_block = old_c_block;
	add_stmt(new_c_block);
}

void iiopxx_mu_state::mu_server_func_get_invocation_names(
	cast_def *cfunc,
	/* OUT */ cast_expr *formal_func_cexpr,
	/* OUT */ cast_expr *actual_func_cexpr)
{
	cast_scoped_name scname;
	cast_expr cexpr;
	
	/* XXX --- Fix this to look up the implicit param? */
	scname = cast_new_scoped_name(cfunc->name.
				      cast_scoped_name_val[cfunc->name.
							  cast_scoped_name_len
							  - 1].name,
				      NULL);
	cexpr = cast_new_expr_name("this");
	cexpr = cast_new_unary_expr(CAST_UNARY_DEREF, cexpr);
	cexpr = cast_new_expr_sel(cexpr, scname);
	*formal_func_cexpr = cexpr;
	
	*actual_func_cexpr = cast_new_expr_sel(cast_new_unary_expr(CAST_UNARY_DEREF, 0) /* Filled by m/u. */,
					       scname);
}

void iiopxx_mu_state::mu_server_func_call(cast_expr func_call_cexpr)
{
	/*cast_stmt try_block = cast_new_block(0, 0);
	cast_stmt func_try;
	cast_stmt catch_block = cast_new_block(0, 0);
	*/
	
	add_stmt(cast_new_stmt_expr(func_call_cexpr));
	/*cast_block_add_stmt(&(try_block->cast_stmt_u_u.block),
			    cast_new_stmt_expr(func_call_cexpr));
	add_stmt(change_stub_state(FLICK_STATE_FUNCTION_CALL));
	func_try = cast_new_try(try_block);
	add_stmt(func_try);
	cast_block_add_stmt(&catch_block->cast_stmt_u_u.block,
			    cast_new_stmt_expr(
				    cast_new_expr_call_1(
					    cast_new_expr_sel(
						    cast_new_expr_name("_ev"),
						    cast_new_scoped_name(
							    "exception",
							    NULL)),
					    cast_new_expr_name("ex"))));
	cast_try_add_handler(func_try,
			     cast_new_type_scoped_name(
				     cast_new_scoped_name("CORBA",
							  "Exception",
							  NULL)),
			     "ex",
			     catch_block);
			     add_stmt(change_stub_state(
			     	     FLICK_STATE_FUNCTION_RETURN));*/
}

void iiopxx_mu_state::mu_server_func_reply(pres_c_server_func *sfunc,
					   pres_c_skel *sskel)
{
	mu_state      *must_out = another(MUST_ENCODE | MUST_DEALLOCATE
					  | MUST_REPLY);
	
	cast_def *cfunc = &pres->stubs_cast.cast_scope_val[sfunc->c_func];
	mint_ref       simple_reply_itype;
	pres_c_inline  simple_reply_inline;
	cast_stmt cstmt;
	tag_list *tl;
	tag_item *ti;
	char *plain_name;
	int size = 0;
	unsigned int lpc;

	tl = create_tag_list(0);
	for( lpc = 0; lpc < cfunc->name.cast_scoped_name_len; lpc++ ) {
		size += strlen(cfunc->name.cast_scoped_name_val[lpc].name) + 1;
	}
	plain_name = (char *)mustmalloc(strlen("POA_") + size + 1);
	strcpy(plain_name, "POA_");
	for( lpc = 0; lpc < cfunc->name.cast_scoped_name_len; lpc++ ) {
		if( lpc > 0 )
			strcat(plain_name, "_");
		strcat(plain_name, cfunc->name.cast_scoped_name_val[lpc].name);
	}
	add_tag(tl, "plain_name", TAG_STRING, plain_name);
	add_tag(tl, "be_name", TAG_STRING, get_be_name());
	ti = add_tag(tl, "my_sections", TAG_STRING_ARRAY, 3);
	ti->data.tag_data_u.str_a.str_a_val[0] = ir_strlit("u");
	ti->data.tag_data_u.str_a.str_a_val[1] = ir_strlit("c");
	ti->data.tag_data_u.str_a.str_a_val[2] = ir_strlit("m");
	/*
	 * Strip away the ``collapsed union'' goo that encodes IDL and
	 * interface information.  We don't need to encode that data for IIOP
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
			cast_new_expr_name("flick_iiopxx_server_start_encode"),
			0)));
	
	cstmt = cast_new_stmt_handler(
		"scml-code-cast-handler",
		"server-stub-code",
		"<cycle::start name=plain_name>",
		ptr_to_tag_ref("struct scml_scope *",
			       state->get_scml_root()),
		ptr_to_tag_ref("tag_list *", tl),
		NULL);
	add_stmt(cstmt);
	cstmt = cast_new_stmt_handler(
		"scml-code-cast-handler",
		"server-stub-code",
		"<gtod::start name=plain_name>",
		ptr_to_tag_ref("struct scml_scope *",
			       state->get_scml_root()),
		ptr_to_tag_ref("tag_list *", tl),
		NULL);
	add_stmt(cstmt);
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
	
	cstmt = cast_new_stmt_handler(
		"scml-code-cast-handler",
		"server-stub-code",
		"<gtod::stop name=plain_name section=\"m\">",
		ptr_to_tag_ref("struct scml_scope *",
			       state->get_scml_root()),
		ptr_to_tag_ref("tag_list *", tl),
		NULL);
	add_stmt(cstmt);
	cstmt = cast_new_stmt_handler(
		"scml-code-cast-handler",
		"server-stub-code",
		"<cycle::stop name=plain_name section=\"m\">",
		ptr_to_tag_ref("struct scml_scope *",
			       state->get_scml_root()),
		ptr_to_tag_ref("tag_list *", tl),
		NULL);
	add_stmt(cstmt);
	add_stmt(cast_new_stmt_expr(
		cast_new_expr_call(
			cast_new_expr_name("flick_iiopxx_server_end_encode"),
			0)));
}

void w_skel(pres_c_1 *pres, int stub_idx)
{
	int assumptions = RPCM_TRUSTED; /* XXX */
	mint_ref simple_request_itype;
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
	cast_func_type *cfunct = &cdef->u.cast_def_u_u.func_type;
	
	iiopxx_mu_state must(the_state,
			     MUST_DECODE | MUST_ALLOCATE | MUST_REQUEST,
			     assumptions, "server", IIOPXX_NO_SWAP);
	iiopxx_mu_state swap_must(the_state,
				  MUST_DECODE | MUST_ALLOCATE | MUST_REQUEST,
				  assumptions, "server", IIOPXX_SWAP);
	
	/*
	 * This emergency_return_value is used in the case of an error.  It
         * is a success value because the error should already be handled,
         * and the server should continue as normal.
	 */
	emergency_return_value =
		cast_new_expr_name("return");
	reaper_label = cast_new_label(
		"_flick_reaper",
		cast_new_return(
			cast_new_expr_name("")));
	mab = new mu_abort_block();
	mab->set_kind(MABK_THREAD);
	mab->begin();
	mab->add_stmt(cast_new_return(
		cast_new_expr_name("")));
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
	
	must.abort_block = mab;
	must.current_span = new mu_msg_span;
	must.current_span->set_kind(MSK_SEQUENTIAL);
	must.current_span->begin();
	swap_must.abort_block = mab;
	swap_must.current_span = new mu_msg_span;
	swap_must.current_span->set_kind(MSK_SEQUENTIAL);
	swap_must.current_span->begin();
	
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
		 * IIOP because it is manifest in the object references.
		 *
		 * The astute reader will note that the function call below
		 * only takes care of half of the problem --- what about the
		 * reply?  The IDL and interface codes are removed from the
		 * reply by an IIOP version of `mu_server_func_reply()'.
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
	
	/* Descend through the message union tree,
	   producing switch statements that decode the message into 
	   separate procedures.  */
	
	must.mu_decode_switch(dcase, skel->funcs.funcs_len,
			      simple_request_itype);
	swap_must.mu_decode_switch(dcase, skel->funcs.funcs_len,
				   simple_request_itype);
	
	/* Collapse the span trees and commit their values */
	must.current_span->end();
	swap_must.current_span->end();
	must.current_span->collapse();
	must.current_span->commit();
	swap_must.current_span->collapse();
	swap_must.current_span->commit();
	mab->end();
	mab->set_reaper_label(reaper_label);
	/* Remove unneeded abort code */
	mab->rollback();
	cast_w_func_type(cdef->name, cfunct, 0);
	
	if (swap_must.c_block && must.c_block) {
		struct scml_context *sc;
		tag_list *tl;
		
		tl = create_tag_list(0);
		add_tag(tl, "prologue_state_change", TAG_CAST_STMT,
			must.change_stub_state(FLICK_STATE_PROLOGUE));
		add_tag(tl, "unmarshal_state_change", TAG_CAST_STMT,
			must.change_stub_state(FLICK_STATE_UNMARSHAL));
		add_tag(tl, "epilogue_state_change", TAG_CAST_STMT,
			must.change_stub_state(FLICK_STATE_EPILOGUE));
		add_tag(tl, "swap_block", TAG_CAST_STMT, swap_must.c_block);
		add_tag(tl, "out_block", TAG_CAST_STMT, must.c_block);
		add_tag(tl, "abort_block", TAG_CAST_STMT, mab->
			get_block_label());
		add_tag(tl, "reaper_label", TAG_CAST_STMT, reaper_label);
		add_tag(tl, "be_name", TAG_STRING, must.get_be_name());
		sc = new scml_context;
		sc->set_scope(the_state->get_scml_root());
		sc->set_stream_pos(the_state->get_scml_defs_stream());
		sc->exec_cmd("server_skel", tl, 0);
		delete_tag_list(tl);
	} else {
		w_printf("{\n");
		w_i_printf(1, "return;\n");
		w_printf("}\n");
		warn("Empty server skeleton!");
	}
}
	
/* End of file. */

