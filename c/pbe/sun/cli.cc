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

#include "sun.h"

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
	/* Start the abort code */
	mab->add_stmt(
		cast_new_stmt_expr(
			cast_new_expr_call_3(
				cast_new_expr_name(
					flick_asprintf(
						"flick_%s_client_error",
						pres->pres_context)),
				cast_new_expr_name("xdr"),
				cast_new_expr_name("sun"),
				cast_new_expr_name(mab->
						   use_current_label()))));
	
	/* Build the target object marshaling code. */
	sun_target_mu_state must_target(the_state,
					(MUST_ENCODE | MUST_DEALLOCATE), 0,
					"client");
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
	sun_mu_state must_in(the_state,
			     (MUST_ENCODE | MUST_DEALLOCATE | MUST_REQUEST),
			     assumptions, "client");
	must_in.abort_block = mab;
	remove_idl_and_interface_ids(pres,
				     cstub->request_itype, cstub->request_i,
				     &operation_itype, &operation_inline);
	must_in.mu_func_params(cstub->c_func,
			       operation_itype, operation_inline);
	must_in.mu_end();
	
	escape_label = mab->use_current_label();
	
	struct mu_abort_block *out_mab;
	
	if (cstub->op_flags & PRES_C_STUB_OP_FLAG_ONEWAY) {
		out_mab = new mu_abort_block();
		out_mab->set_kind(MABK_THREAD);
		out_mab->begin();
	}
	else
		out_mab = mab;
	sun_mu_state must_out(the_state,
			      (MUST_DECODE | MUST_ALLOCATE | MUST_REPLY),
			      assumptions, "client");
	
	/* Set up the argument list to catch parameter names we may need. */
	must_out.arglist = new mu_state_arglist("params");
	must_out.arglist->add("params", "environment");
	
	must_out.add_stmt(must_out.change_stub_state(FLICK_STATE_UNMARSHAL));
	must_out.abort_block = out_mab;
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
	must_out.current_span->end();
	must_out.current_span->collapse();
	must_out.current_span->commit();
	
	mab->end();
	mab->set_reaper_label(reaper_label);
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
	if (cstub->op_flags & PRES_C_STUB_OP_FLAG_ONEWAY) {
		w_i_printf(1,
			   "if (!flick_client_send_request(_obj, _obj->in))"
			   "\n");
		w_i_printf(2,
			   "flick_stub_error(FLICK_ERROR_COMMUNICATION, "
			   "%s);\n",
			   escape_label);
		if (cfunct->return_type->kind != CAST_TYPE_VOID)
			w_i_printf(1, "return _return;\n");
		else
			w_i_printf(1, "return;\n");
		cast_w_stmt(mab->get_block_label(), 1);
		cast_w_stmt(reaper_label, 1);
		w_printf("}\n\n");
		return;
	}
	
	w_i_printf(1,
		   "if (!flick_client_send_request(_obj, _obj->in))\n");
	w_i_printf(2,
		   "flick_stub_error(FLICK_ERROR_COMMUNICATION, %s);\n",
		   escape_label);
	cast_w_stmt(must_in.change_stub_state(FLICK_STATE_RECEIVE), 1);
	w_i_printf(1, "if (!flick_client_get_reply(_obj, _obj->out))\n");
	w_i_printf(2,
		   "flick_stub_error(FLICK_ERROR_COMMUNICATION, %s);\n",
		   escape_label);
	w_i_printf(1,
		   "flick_%s_client_start_decode();\n",
		   must_in.get_be_name());
	
	/* Output the general unmarshaling code. */
	cast_w_stmt(must_out.c_block, 2);
	
	w_i_printf(1,
		   "flick_%s_client_end_decode();\n",
		   must_in.get_be_name());
	
	if (cfunct->return_type->kind != CAST_TYPE_VOID)
		w_i_printf(1, "return _return;\n");
	else
		w_i_printf(1, "return;\n");
	/* Add in the abort code */
	cast_w_stmt(mab->get_block_label(), 1);
	cast_w_stmt(reaper_label, 1);
	w_printf("}\n\n");
}

/* End of file. */

