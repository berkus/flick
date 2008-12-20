/*
 * Copyright (c) 1999 The University of Utah and
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

/* This will find a `cast_label' with the specified `cast_block'. */
int cast_find_label(cast_block *block, const char *label)
{
	cast_stmt curr;
	int retval = -1;
	unsigned int i;
	
	for (i = 0; (i < block->stmts.stmts_len) && (retval == -1); ++i) {
		curr = block->stmts.stmts_val[i];
		if ((curr->kind == CAST_STMT_LABEL)
		    && !strcmp(label, curr->cast_stmt_u_u.s_label.label)) {
			retval = i;
		}
	}
	return retval;
}

/* End of file. */

