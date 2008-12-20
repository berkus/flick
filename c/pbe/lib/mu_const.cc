/*
 * Copyright (c) 1995, 1996 The University of Utah and
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

#include <mom/compiler.h>
#include <mom/c/libcast.h>
#include <mom/c/pbe.hh>

void 
mu_state::mu_const(mint_const iconst, mint_ref itype)
{
	if (op & MUST_DECODE) {
		cast_expr cexpr = add_temp_var("ismatch",
					       cast_new_prim_type(
						       CAST_PRIM_INT, 0));
		mu_hash_const(&iconst, 1, itype, cexpr);
		
		/* NOTE: Breaking a glob here would cause an excessive
		         number of globs in the common case.  It is not
			 necessary since the error macros have *no* 
			 assumptions about the marshal/unmarshal state. */
		
		/* If we decoded the constant, then the temporary variable will
		   have been set to zero.  (Why zero?  Because `iconst' was
		   element zero of the "array" of values that we passed to
		   `mu_hash_const'.)  However, if we failed to decode the
		   constant, then the temporary variable will have been set to
		   -1. */
		add_stmt(cast_new_if(
			cast_new_binary_expr(CAST_BINARY_NE,
					     cexpr,
					     cast_new_expr_lit_int(0, 0)),
			make_error(FLICK_ERROR_CONSTANT),
			0 /* No `else' statement required. */)
			);
	}
	if (op & MUST_ENCODE)
		mu_encode_const(iconst, itype);
}

