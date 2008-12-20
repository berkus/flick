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

/* Creates and returns a template type with the specified definition type */
cast_type cast_new_template_type(cast_type def_type)
{
	cast_type type = cast_new_type(CAST_TYPE_TEMPLATE);
	assert(def_type);
	type->cast_type_u_u.template_type.flags = 0;
	type->cast_type_u_u.template_type.params.params_val = 0;
	type->cast_type_u_u.template_type.params.params_len = 0;
	type->cast_type_u_u.template_type.def = def_type;
	return type;
}

