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

#include <stdlib.h>
#include <string.h>
#include <mom/compiler.h>
#include <mom/c/libcast.h>

void cast_class_add_parent(cast_type type, cast_parent_flags flags, cast_scoped_name name)
{
	cast_aggregate_type *at;
	int i;
	
	at = &type->cast_type_u_u.agg_type;
	i = at->parents.parents_len++;
	at->parents.parents_val = mustrealloc(at->parents.parents_val,
					      at->parents.parents_len *
					      sizeof(*at->parents.
						     parents_val));
	at->parents.parents_val[i].flags = flags;
	at->parents.parents_val[i].name = name;
}

