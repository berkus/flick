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

#include <assert.h>

#include <mom/compiler.h>
#include <mom/c/libcast.h>

void cast_block_add_var(cast_block *b,
			const char *name, cast_type type,
			cast_init init, cast_storage_class sc)
{
	assert(name);
	assert(type);
	assert(b);
	
	/* Add the new variable to the scope.  */
	int cdef = cast_add_def(&b->scope,
				cast_new_scoped_name(name, NULL),
				sc,
				CAST_VAR_DEF,
				PASSTHRU_DATA_CHANNEL,
				CAST_PROT_NONE);
	b->scope.cast_scope_val[cdef].u.cast_def_u_u.var_def.type = type;
	b->scope.cast_scope_val[cdef].u.cast_def_u_u.var_def.init = init;
}

/* End of file. */

