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
#include <mom/c/libcast.h>

#include "fluke.h"

void fluke_mu_state::mu_server_func_target(pres_c_server_func *sfunc)
{
	fluke_target_mu_state *must_target =
		(fluke_target_mu_state *)
		mu_make_target_mu_state(state, op, assumptions, "server");
	
	/*
	 * Make sure that the `target_mu_state' shares the required contextual
	 * structures with the current `mu_state'.
	 */
	must_target->formal_func_invocation_cexpr
		= formal_func_invocation_cexpr;
	must_target->actual_func_invocation_cexpr
		= actual_func_invocation_cexpr;
	
	must_target->arglist = arglist;
	
	must_target->abort_block = abort_block;
	
	/*
	 * Create a CAST expression refering to the function's target object
	 * parameter by ``(un)marshaling'' that parameter.  Our special
	 * `fluke_target_mu_state' puts the CAST in its `target_cast_expr'
	 * slot; see the file `fluke_target_mu_mapping_reference.cc'.
	 */
	must_target->mu_func_params(sfunc->c_func, sfunc->target_itype,
				    sfunc->target_i);
	must_target->mu_end();
	
	if (op & MUST_DECODE) {
		/*
		 * When decoding, output an assignment expression that copies
		 * the server dispatch function's object reference into the
		 * parameter that will be passed to the server work function.
		 * Note that the parameter is a local variable and has already
		 * been declared --- see `mu_state::mu_server_func'.
		 */
		cast_expr expr;
		cast_type type;
		int gotarg = must_target->arglist->getargs("params", "object",
							   &expr, &type);
		assert(gotarg);
		assert(expr);assert(type);
		
		add_stmt(cast_new_stmt_expr(
			cast_new_expr_assign(
				expr,
				cast_new_expr_cast(
					/*
					 * XXX --- Need to un-hardwire
					 * the name of the obj ref!
					 */
					cast_new_expr_name("_dispatch_obj"),
					type
					))));
	}
	absorb_stmt(must_target->c_block);
}

/* End of file. */

