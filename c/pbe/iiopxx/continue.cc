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

void w_continue_stub(pres_c_1 *pres, int stub_idx)
{
	pres_c_continue_stub *stub;
	
	/* Find the `pres_c_continue_stub' that describes this stub. */
	assert(pres);
	assert(pres->stubs.stubs_val[stub_idx].kind == PRES_C_CONTINUE_STUB);
	stub = &(pres->stubs.stubs_val[stub_idx].pres_c_stub_u.continue_stub);
	
	/* Find the CAST declaration of this stub. */
	assert(stub->c_func >= 0);
	assert(stub->c_func < (signed int)pres->stubs_cast.cast_scope_len);
	cast_def *cfunc = &(pres->stubs_cast.cast_scope_val[stub->c_func]);
	
	assert(cfunc->u.kind == CAST_FUNC_DECL);
	cast_func_type *cfunct = &cfunc->u.cast_def_u_u.func_type;
	
	assert(cfunct->return_type->kind == CAST_TYPE_VOID);
	
	emergency_return_value = cast_new_expr_name("return");
	
	/*
	 * Build the parameter marshal/unmarshal code.
	 * NOTE:  We do NOT set MUST_ENCODE, MUST_DECODE, MUST_ALLOCATE, or
	 * MUST_DEALLOCATE because they don't apply to a continuation stub.
	 */
	iiopxx_mu_state must(the_state,
			     (stub->request? MUST_REQUEST : MUST_REPLY),
			     RPCM_TRUSTED,
			     "msg");
	
	/* Set up the argument list to catch the parameter names we need. */
	must.arglist = new mu_state_arglist("params");
	must.arglist->add("params", "object");
	must.arglist->add("params", "environment");
	must.arglist->add("params", "return");
	must.arglist->add("params", "invocation_id");
	must.arglist->add("params", "client");
	must.arglist->add("params", "message");
	must.arglist->add("params", "continue_func");
	must.arglist->add("params", "continue_data");
	
	must.mu_func_params(stub->c_func, stub->itype, stub->i);
	must.mu_end();
	
	/* Create the CAST body of the stub. */
	
	/* Start writing the stub. */
	int gotarg = 0;
	
	cast_w_func_type(cfunc->name, cfunct, 0);
	w_printf("\n{\n");
	
	/* Output the generated code. */
	if (must.c_block)
		cast_w_stmt(must.c_block, 1);
	w_i_printf(1,
		   "flick_continue_%s_msg(",
		   (stub->request)? "request" : "reply");
	cast_expr expr;
	cast_type type;
	gotarg = must.arglist->getargs("params", "client", &expr, &type);
	assert(gotarg);assert(expr);assert(type);
	cast_w_expr(expr, 0);w_printf(", ");
	gotarg = must.arglist->getargs("params", "message", &expr, &type);
	assert(gotarg);assert(expr);assert(type);
	cast_w_expr(expr, 0);w_printf(", ");
	gotarg = must.arglist->getargs("params", "invocation_id",
				       &expr, &type);
	assert(gotarg);assert(expr);assert(type);
	cast_w_expr(expr, 0);w_printf(", ");
	gotarg = must.arglist->getargs("params", "object", &expr, &type);
	assert(gotarg);assert(expr);assert(type);
	cast_w_expr(expr, 0);w_printf(", ");
	gotarg = must.arglist->getargs("params", "continue_func",
				       &expr, &type);
	assert(gotarg);assert(expr);assert(type);
	cast_w_expr(expr, 0);w_printf(", ");
	gotarg = must.arglist->getargs("params", "continue_data",
				       &expr, &type);
	assert(gotarg);assert(expr);assert(type);
	cast_w_expr(expr, 0);w_printf(");\n");
	
	w_printf("}\n\n");
}

/* End of file. */

