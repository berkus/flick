/*
 * Copyright (c) 1995, 1996, 1998, 1999 The University of Utah and
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

/* Declare a new variable in the current m/u code block.
   The variable will be available (in scope)
   throughout the current code block, but not outside of it.

   `name' is the name to give the variable.
   The caller is responsible for ensuring
   that is does not conflict with the names of other
   variables, functions, parameters, etc.
   as dictated by C's scoping rules.

   `type' is the C type to give the temporary variable.

   `sc' is the storage class for the variable -
   e.g. CAST_SC_NONE for a normal local variable,
   CAST_SC_STATIC for a static variable,
   or CAST_SC_EXTERN for a reference to an external global variable.
*/
void mu_state::add_var(const char *name, cast_type type,
		       cast_init init, cast_storage_class sc)
{
	assert(name);
	assert(type);
	
	/* Make sure the block exists. */
	if (!c_block)
		c_block = cast_new_block(0, 0);
	
	/* Add the new variable to the scope. */
	assert(c_block->kind == CAST_STMT_BLOCK);
	cast_block_add_var(&(c_block->cast_stmt_u_u.block),
			   name,
			   type, init, sc);
}

/* End of file. */

