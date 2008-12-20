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

#include <string.h>
#include <mom/compiler.h>
#include <mom/c/libcast.h>

cast_scoped_name cast_set_scoped_name(cast_scoped_name *scname,
				      const char *name,
				      cast_template_arg_array args,
				      ...)
{
	
	cast_scoped_name new_scname;
	va_list args_list;
	const char *name_arg;
	cast_template_arg_array template_arg_args;
	unsigned int i;
	
	if (!scname) {
		scname = &new_scname;
		*scname = null_scope_name;
	}
	name_arg = name;
	template_arg_args = args;
	va_start(args_list, args);
	for (i = 0; name_arg; i++) {
		if (i == scname->cast_scoped_name_len) {
			scname->cast_scoped_name_len += 4;
			scname->cast_scoped_name_val =
				(cast_name_s *)mustrealloc(
					scname->cast_scoped_name_val,
					scname->cast_scoped_name_len *
					sizeof(cast_name_s));
		}
		scname->cast_scoped_name_val[i].name = ir_strlit(name_arg);
		scname->cast_scoped_name_val[i].args = template_arg_args;
		name_arg = va_arg(args_list, const char *);
		template_arg_args = va_arg(args_list, cast_template_arg_array);
	}
	va_end(args_list);
	if (i != scname->cast_scoped_name_len) {
		scname->cast_scoped_name_len = i;
		scname->cast_scoped_name_val =
			(cast_name_s *)mustrealloc(
				scname->cast_scoped_name_val,
				scname->cast_scoped_name_len *
				sizeof(cast_name_s));
	}
	return *scname;
}

/* End of file. */

