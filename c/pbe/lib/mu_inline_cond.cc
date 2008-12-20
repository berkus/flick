/*
 * Copyright (c) 1995, 1996, 1997, 1999 The University of Utah and
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

/*
 * This routine handles inline conditionals.  It produces a
 * conditional execution of either the true or false branch, depending
 * upon the boolean (at least integerish) value given by the inline
 * index.
 */
void mu_state::mu_inline_cond(inline_state *ist, mint_ref itype,
			      pres_c_inline inl)
{
	pres_c_inline_cond *coinl = &inl->pres_c_inline_u_u.cond;
	
	cast_expr co_expr; cast_type co_ctype;
	ist->slot_access(coinl->index, &co_expr, &co_ctype);
	
	
	if (co_ctype->kind == CAST_TYPE_POINTER)
		co_expr = cast_new_unary_expr(CAST_UNARY_DEREF, co_expr);
	
	cast_stmt if_stmt = cast_new_stmt(CAST_STMT_IF);
	
	if_stmt->cast_stmt_u_u.s_if.test = co_expr;
	
	/* Save off old block, start with a new one. */
	
	cast_stmt old_c_block = c_block;
	
	c_block = 0;
	
	mu_inline(ist, itype, coinl->false_inl);
	
	if_stmt->cast_stmt_u_u.s_if.false_stmt = c_block;
	
	c_block = 0;
	
	mu_inline(ist, itype, coinl->true_inl);
	
	if_stmt->cast_stmt_u_u.s_if.true_stmt = c_block;
	
	c_block = old_c_block;
	
	add_stmt(if_stmt);
}

/* End of file. */

