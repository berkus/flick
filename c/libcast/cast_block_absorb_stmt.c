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

#include <mom/compiler.h>
#include <mom/c/libcast.h>

/*
 * Absorb the C statement `from_stmt' into the block statement `into_block'.
 *
 * This function is like `cast_block_add_stmt' except in its handling of
 * blocks.  If `from_stmt' is a block statement, this function ``dissects'' the
 * block and adds its components to `into_block'.  This helps reduce undue
 * nesting in generated code, and has the side effect of widening the scope of
 * the declarations contained within the absorbed `from_stmt' block.
 */
void cast_block_absorb_stmt(cast_block *into_block, cast_stmt from_stmt)
{
	cast_scope	*into_block_scope;
	cast_scope	*from_stmt_scope;
	unsigned int	from_stmt_stmts_len;
	cast_stmt	*from_stmt_stmts_val;
	cast_def	*from_stmt_cdef;
	
	int		cdef_index;
	
	unsigned int	i;
	
	/*****/
	
	if (!from_stmt)
		return;
	
	if (from_stmt->kind != CAST_STMT_BLOCK) {
		cast_block_add_stmt(into_block, from_stmt);
		return;
	}
	
	/*
	 * Copy the parts of the `from_stmt' block statement into `into_block'.
	 */
	
	into_block_scope = &(into_block->scope);
	from_stmt_scope  = &(from_stmt->cast_stmt_u_u.block.scope);
	
	for (i = 0; i < from_stmt_scope->cast_scope_len; ++i) {
		from_stmt_cdef = &(from_stmt_scope->cast_scope_val[i]);
		
		cdef_index = cast_add_def(into_block_scope,
					  from_stmt_cdef->name,
					  from_stmt_cdef->sc,
					  from_stmt_cdef->u.kind,
					  from_stmt_cdef->channel,
					  from_stmt_cdef->protection);
		/* Copy the definition data (a union). */
		into_block_scope->cast_scope_val[cdef_index].u
			= from_stmt_cdef->u;
	}
	
	from_stmt_stmts_len = from_stmt->cast_stmt_u_u.block.stmts.stmts_len;
	from_stmt_stmts_val = from_stmt->cast_stmt_u_u.block.stmts.stmts_val;
	
	if (from_stmt->cast_stmt_u_u.block.flags & CAST_BLOCK_REVERSE) {
		/* Add the statements in reverse order. */
		for (i = from_stmt_stmts_len; i > 0; --i)
			cast_block_add_stmt(into_block,
					    from_stmt_stmts_val[i-1]);
	} else {
		/* Add the statements in the proper order. */
		for (i = 0; i < from_stmt_stmts_len; ++i)
			cast_block_add_stmt(into_block,
					    from_stmt_stmts_val[i]);
	}
}

/* End of file. */

