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

#include <mom/c/libcast.h>
#include <mom/c/pbe.hh>

/* Add a C statement to the block of marshaling code
   we're currently building.
   Create a new marshaling code block if necessary.  */
void mu_state::add_stmt(cast_stmt st)
{
	if (!st) return;

	/* Make sure the block exists.  */
	if (!c_block)
		c_block = cast_new_block(0, 0);

	/* Add the statement to the end.  */
	assert(c_block->kind == CAST_STMT_BLOCK);
	cast_block_add_stmt(&c_block->cast_stmt_u_u.block, st);
}

void mu_state::add_initial_stmt(cast_stmt st)
{
	if (!st) return;
	
	/*
	 * If we don't have a block, then the initial statement is
	 * also the last, and we can use the regular add_stmt().
	 */
	if (!c_block)
		c_block = cast_new_block(0, 0);
	
	/* Add the statement to the initial statements. */
	assert(c_block->kind == CAST_STMT_BLOCK);
	cast_block_add_initial_stmt(&c_block->cast_stmt_u_u.block, st);
}

