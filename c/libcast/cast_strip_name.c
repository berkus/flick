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

void cast_strip_name(cast_scoped_name *scope,
		     cast_scoped_name *name)
{
	unsigned int i, j;

	for( i = 0; i < scope->cast_scoped_name_len; i++ ) {
		if( strcmp( scope->cast_scoped_name_val[i].name,
			    name->cast_scoped_name_val[i].name ) )
			break;
	}
	for( j = 0; j < (name->cast_scoped_name_len - i); j++ ) {
		name->cast_scoped_name_val[j] =
			name->cast_scoped_name_val[j + i];
	}
	name->cast_scoped_name_len -= i;
}
