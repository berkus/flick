/*
 * Copyright (c) 1995, 1996, 1999 The University of Utah and
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
#include <stdlib.h>

#include <mom/compiler.h>
#include <mom/c/libcast.h>

void cast_block_add_stmt(cast_block *b, cast_stmt st)
{
	/* If the current last statement is a label with a null statement,
	   then just tack the new statement onto that.  */
	if (b->stmts.stmts_len > 0)
	{
		cast_stmt last = b->stmts.stmts_val[b->stmts.stmts_len-1];
		assert(last);
		if ((last->kind == CAST_STMT_LABEL)
		    && (last->cast_stmt_u_u.s_label.stmt == 0))
		{
			last->cast_stmt_u_u.s_label.stmt = st;
			return;
		}
		if ((last->kind == CAST_STMT_CASE)
		    && (last->cast_stmt_u_u.s_case.stmt == 0))
		{
			last->cast_stmt_u_u.s_case.stmt = st;
			return;
		}
		if ((last->kind == CAST_STMT_DEFAULT)
		    && (last->cast_stmt_u_u.default_stmt == 0))
		{
			last->cast_stmt_u_u.default_stmt = st;
			return;
		}
	}

	/* Otherwise, add a new statement to the end of the block.  */
	int i = b->stmts.stmts_len++;
	b->stmts.stmts_val = (cast_stmt*)mustrealloc(
		b->stmts.stmts_val,
		sizeof(cast_stmt)*b->stmts.stmts_len);
	assert(st);
	b->stmts.stmts_val[i] = st;
}

void cast_block_add_initial_stmt(cast_block *b, cast_stmt st)
{
	/*
	 * Add a new statement to the beginning of the block (but after other
	 * initial statements we've already added).
	 */
	int i = b->initials.initials_len++;
	b->initials.initials_val = (cast_stmt*)mustrealloc(
		b->initials.initials_val,
		sizeof(cast_stmt)*b->initials.initials_len);
	assert(st);
	b->initials.initials_val[i] = st;
}

