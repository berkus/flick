/*
 * Copyright (c) 1995, 1996, 1997 The University of Utah and
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

#include <mom/libmint.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>
#include <mom/c/pbe.hh>

void mu_state::mu_inline_assign(inline_state *ist,
				mint_ref itype, pres_c_inline inl)
{
	pres_c_inline_assign *asinl = &inl->pres_c_inline_u_u.assign;
	
	cast_expr as_expr;
	cast_type as_ctype;
	
	assert(itype >= 0);
	assert(itype < (signed int) pres->mint.defs.defs_len);
	
	/* Find the assign to expression and C type.  */
	ist->slot_access(asinl->index, &as_expr, &as_ctype);
	
	/* Add a assign stmt, assigning the as_expr to the value. */
	if (as_ctype->kind == CAST_TYPE_POINTER)
		as_expr = cast_new_unary_expr(CAST_UNARY_DEREF, as_expr);
	
	add_stmt(cast_new_stmt_expr(
		cast_new_expr_assign(as_expr, asinl->value)
		));
	
	/* Descend into the sub inline. */
	mu_inline(ist, itype, asinl->sub);				    
}

/* End of file. */

