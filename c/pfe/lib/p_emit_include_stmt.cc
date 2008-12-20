/*
 * Copyright (c) 1997, 1998, 1999 The University of Utah and
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

#include <string.h>

#include <mom/c/libcast.h>

#include <mom/c/pfe.hh>

void pg_state::p_emit_include_stmt(const char *filename, int system_only)
{
	u_int i;
	cast_scope *scope = (cast_scope *)top_ptr(scope_stack);
	
	/*
	 * Check the output CAST to see if we have already output the desired
	 * `#include' statement.  If so, just return.
	 */
	for (i = 0; i < scope->cast_scope_len; ++i)
		if ((c(i).u.kind == CAST_INCLUDE)
		    && !strcmp(filename, c(i).u.cast_def_u_u.include.filename)
		    && (c(i).u.cast_def_u_u.include.system_only
			== system_only))
			return;
	
	/* Otherwise... */
	i = cast_add_def(scope,
			 null_scope_name,
			 CAST_SC_NONE,
			 CAST_INCLUDE,
			 PASSTHRU_DATA_CHANNEL,
			 CAST_PROT_NONE);
	
	c(i).u.cast_def_u_u.include.filename = ir_strlit(filename);
	c(i).u.cast_def_u_u.include.system_only = system_only;
}

/* End of file. */

