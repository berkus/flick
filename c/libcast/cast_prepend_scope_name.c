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

void cast_prepend_scope_name(cast_scoped_name *scname,
			     const char *name,
			     cast_template_arg_array args)
{
	unsigned int i;
	int lpc;
	
	assert(name);
	
	i = scname->cast_scoped_name_len++;
	scname->cast_scoped_name_val
		= ((cast_name_s *)
		   mustrealloc(
			   scname->cast_scoped_name_val,
			   scname->cast_scoped_name_len * sizeof(cast_name_s)
			   ));
	for (lpc = i; lpc > 0; lpc--) {
		scname->cast_scoped_name_val[lpc]
			= scname->cast_scoped_name_val[lpc - 1];
	}
	scname->cast_scoped_name_val[0].name = ir_strlit(name);
	scname->cast_scoped_name_val[0].args = args;
}

/* End of file. */

