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

#include <mom/compiler.h>
#include <mom/c/libcast.h>
#include <string.h>

cast_template_arg_array null_template_arg_array = { 0, 0 };

cast_template_arg_array cast_set_template_arg_array(
	cast_template_arg_array *array,
	cast_template_arg template_arg, ...)
{
	cast_template_arg_array new_array;
	va_list args;
	cast_template_arg template_arg_arg;
	unsigned int i;
	
	if (!array) {
		array = &new_array;
		*array = null_template_arg_array;
	}
	template_arg_arg = template_arg;
	va_start(args, template_arg);
	for (i = 0; template_arg_arg; i++) {
		if (i == array->cast_template_arg_array_len) {
			array->cast_template_arg_array_len += 4;
			array->cast_template_arg_array_val =
				(cast_template_arg *)mustrealloc(
					array->cast_template_arg_array_val,
					array->cast_template_arg_array_len *
					sizeof(cast_template_arg));
		}
		array->cast_template_arg_array_val[i] = template_arg_arg;
		template_arg_arg = va_arg(args, cast_template_arg);
	}
	va_end(args);
	if (i != array->cast_template_arg_array_len) {
		array->cast_template_arg_array_len = i;
		array->cast_template_arg_array_val =
			(cast_template_arg *)mustrealloc(
				array->cast_template_arg_array_val,
				array->cast_template_arg_array_len *
				sizeof(cast_template_arg));
	}
	return *array;
}

/* End of file. */

