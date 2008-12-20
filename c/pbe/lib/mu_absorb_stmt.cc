/*
 * Copyright (c) 1998 The University of Utah and
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

#include <mom/c/libcast.h>
#include <mom/c/pbe.hh>

/*
 * Absorb a C statement into the block of code being built by this `mu_state'.
 *
 * This method is like `mu_state::add_stmt' except in its handling of blocks.
 * Given a block statement, this method ``dissects'' the block and adds its
 * components to this `mu_state's block.  This helps reduce undue nesting in
 * the generated code, and has the side effect of widening the scope of the
 * declarations contained within the absorbed block.
 *
 * NOTE: If you are calling this simply to widen the scope of local variable
 * declarations, you should consider using `mu_state::add_var_to_cdecl_block'
 * to declare your variables in the first place.
 */
void mu_state::absorb_stmt(cast_stmt st)
{
	if (!st) return;
	
	/* Make sure our `cblock' exists. */
	if (!c_block)
		c_block = cast_new_block(0, 0);
	
	/* Absorb the statement at the end of our `cblock'. */
	cast_block_absorb_stmt(&(c_block->cast_stmt_u_u.block), st);
}

/* End of file. */

