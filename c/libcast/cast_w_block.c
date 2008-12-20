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
#include <mom/compiler.h>
#include <mom/c/libcast.h>

void cast_w_block(cast_block *block, int indent)
{
	int i, has_brace = 0;
	
	if( block->scope.cast_scope_len ) {
		w_i_printf(indent, "{\n");
		indent++;
		block->flags &= ~CAST_BLOCK_INLINE;
		has_brace = 1;
	}
	if( block->flags & CAST_BLOCK_INLINE )
		has_brace = 1;
	
	cast_w_scope(&block->scope, indent);
	
	if ((block->scope.cast_scope_len > 0)
	    && (block->stmts.stmts_len > 0))
		w_putc('\n');
	
	if (block->initials.initials_len > 0) {
		/*
		 * Print the initial statements.  These are *NEVER* reversed.
		 */
		assert(block->initials.initials_val);
		for (i = 0; i < (signed int)block->initials.initials_len;
		     i++) {
			assert(block->initials.initials_val[i]);
			if( !has_brace &&
			    (block->initials.initials_val[i]->kind !=
			     CAST_STMT_EMPTY) ) {
				w_i_printf(indent, "{\n");
				indent++;
				has_brace = 1;
			}
			cast_w_stmt(block->initials.initials_val[i],
				    indent);
		}
		w_putc('\n');
	}
	
	if( block->flags & CAST_BLOCK_REVERSE ) {
		/* Print the block in reverse, useful when we have
		   to build something backwards, like the abort code */
		if (block->stmts.stmts_len > 0)
			assert(block->stmts.stmts_val);
		for (i = (signed int)block->stmts.stmts_len - 1;
		     i >= 0; i--) {
			assert(block->stmts.stmts_val[i]);
			if( !has_brace &&
			    (block->stmts.stmts_val[i]->kind !=
			     CAST_STMT_EMPTY) ) {
				w_i_printf(indent, "{\n");
				indent++;
				has_brace = 1;
			}
			cast_w_stmt(block->stmts.stmts_val[i],
				    indent);
		}
	}
	else {
		if (block->stmts.stmts_len > 0)
			assert(block->stmts.stmts_val);
		for (i = 0; i < (signed int)block->stmts.stmts_len;
		     i++) {
			assert(block->stmts.stmts_val[i]);
			if( !has_brace &&
			    (block->stmts.stmts_val[i]->kind !=
			     CAST_STMT_EMPTY) ) {
				w_i_printf(indent, "{\n");
				indent++;
				has_brace = 1;
			}
			cast_w_stmt(block->stmts.stmts_val[i],
				    indent);
		}
	}
	
	if( has_brace && !(block->flags & CAST_BLOCK_INLINE) ) {
		indent--;
		w_i_printf(indent, "}\n");
	}
}

