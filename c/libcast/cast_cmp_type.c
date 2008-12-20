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

int cast_cmp_type(cast_type a, cast_type b)
{
	int diff;

	/* Easy things first */
	if (a == b)
		return 0;
	assert(a);
	assert(b);
	if (a->kind != b->kind)
		return (int)a->kind - (int)b->kind;
	switch (a->kind)
	{
		case CAST_TYPE_NAME:
		case CAST_TYPE_STRUCT_NAME:
		case CAST_TYPE_UNION_NAME:
		case CAST_TYPE_ENUM_NAME:
		case CAST_TYPE_CLASS_NAME:
		case CAST_TYPE_TYPENAME:
			return cast_cmp_scoped_names(&a->cast_type_u_u.name,
						     &b->cast_type_u_u.name);
		case CAST_TYPE_PRIMITIVE:
			if ((diff = (int)a->cast_type_u_u.primitive_type.kind
				 - (int)b->cast_type_u_u.primitive_type.kind))
				return diff;
			return (int)a->cast_type_u_u.primitive_type.mod
			     - (int)b->cast_type_u_u.primitive_type.mod;
		case CAST_TYPE_REFERENCE:
		case CAST_TYPE_POINTER:
			return cast_cmp_type(a->cast_type_u_u.pointer_type.target,
					     b->cast_type_u_u.pointer_type.target);
		case CAST_TYPE_VOID:
			return 0;
		default:
			panic("cast_cmp_type: unknown cast_type kind %d", a->kind);
	}
	panic("cast_cmp_type: should have returned in switch statement\n");
	return 0;
}

