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

#include <assert.h>

#include <mom/compiler.h>
#include <mom/c/libcast.h>

cast_name_s empty_name_s = { "" };
cast_scoped_name null_scope_name = { 0, 0 };
cast_scoped_name empty_scope_name = { 1, &empty_name_s };

cast_scoped_name cast_new_scoped_name(const char *name, ...)
{
	cast_scoped_name retval;
	va_list args;
	const char *name_arg;
	
	assert(name);
	
	retval.cast_scoped_name_len = 1;
	retval.cast_scoped_name_val = (cast_name_s *)
				      mustcalloc(sizeof(cast_name_s));
	retval.cast_scoped_name_val[0].name = ir_strlit(name);
	retval.cast_scoped_name_val[0].args = null_template_arg_array;
	
	va_start(args, name);
	while ((name_arg = va_arg(args, const char *)) != 0) {
		cast_add_scope_name(&retval,
				    name_arg,
				    null_template_arg_array);
	}
	va_end(args);
	
	return retval;
}

/* End of file. */

