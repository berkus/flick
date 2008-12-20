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
#include <mom/idl_id.h>
#include <mom/libmint.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include "mach3.h"

#define m(n) (&pres->mint.defs.defs_val[n])

#define MACH_NOTIFY_FIRST               0100

void mach3_mu_state::mu_mapping_simple(cast_expr expr,
				       cast_type ctype,
				       mint_ref itype)
{
	cast_expr id_expr;
	int id_is_int = ((m(itype))->kind == MINT_INTEGER);
	
	if ((id_expected == 1)
	    && id_is_int) {
		/*
		 * When `id_expected == 1', we're expecting an integer to
		 * identify the IDL in use.
		 */
		id_expr = cast_new_expr_name("_buf_start->Head.msgh_id");
		
		if (op & MUST_ENCODE) {
			add_stmt(cast_new_stmt_expr(
				cast_new_expr_assign(id_expr, expr)));
			add_stmt(cast_new_if(
				cast_new_binary_expr(
					CAST_BINARY_NE,
					expr,
					cast_new_expr_lit_int(IDL_MIG, 0)),
				cast_new_stmt_expr(
					cast_new_expr_assign(
						id_expr,
						expr)),
				0));
		} else if (op & MUST_DECODE) {
			add_stmt(cast_new_stmt_expr(
				cast_new_expr_assign(
					expr,
					cast_new_expr_cond(
						cast_new_binary_expr(
							CAST_BINARY_LT,
							id_expr,
							cast_new_expr_lit_int(
								MACH_NOTIFY_FIRST,
								0)),
						id_expr,
						cast_new_expr_lit_int(IDL_MIG,
								      0)))));
		}
		/*
		 * Leave id_expected == 1 for our `mu_union_case' override to
		 * see.
		 */
		
	} else if ((id_expected == 2)
		   && id_is_int) {
		/*
		 * When `id_expected == 2', we're expecting an operation
		 * identifier in the Mach message header.
		 */
		id_expr = cast_new_expr_name("_buf_start->Head.msgh_id");
		
		if (op & MUST_ENCODE) {
			add_stmt(cast_new_stmt_expr(
				cast_new_expr_assign(id_expr, expr)));
		} else if (op & MUST_DECODE) {
			add_stmt(cast_new_stmt_expr(
				cast_new_expr_assign(expr, id_expr))); 
		}
		
		/*
		 * If we are processing a reply, the next thing to be handled
		 * will be the operation reply code.  When we are processing a
		 * request, nothing special is coming up.
		 */
		if (   ((op & MUST_DECODE) && !strcmp(which_stub, "client"))
		    || ((op & MUST_ENCODE) && !strcmp(which_stub, "server"))
		    )
			id_expected = 3;
		else
			id_expected = 0;
		
	} else if ((id_expected == 3)
		   && id_is_int) {
		/*
		 * When `id_expected == 3', we're expecting an operation reply
		 * code (i.e., the discriminator of the reply union, indicating
		 * the operation's success or failure) in the Mach message
		 * header.
		 */
		id_expr = cast_new_expr_name("_buf_start->RetCode");
		
		if (op & MUST_ENCODE) {
			add_stmt(cast_new_stmt_expr(
				cast_new_expr_assign(id_expr, expr)));
		} else if (op & MUST_DECODE) {
			add_stmt(cast_new_stmt_expr(
				cast_new_expr_assign(expr, id_expr))); 
		}
		
		/* Nothing else special coming now. */
		id_expected = 0;
		
	} else {
		if (id_expected && !id_is_int) {
			warn("Could not store non-integer identifier in Mach "
			     "message field.");
			/* No more special message ID processing. */
			id_expected = 0;
		}
		if (!inhibit_marshal)
			mem_mu_state::mu_mapping_simple(expr, ctype, itype);
	}
}

/* End of File */
