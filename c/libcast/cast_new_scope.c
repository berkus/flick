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

#include <mom/compiler.h>
#include <mom/c/libcast.h>
#include <string.h>

cast_scope cast_new_scope(int defs)
{
	cast_scope retval;
	
	if (defs) {
		int i;
		
		retval.cast_scope_len = defs;
		retval.cast_scope_val = mustcalloc(defs * sizeof(cast_def));
		memset(retval.cast_scope_val,
		       0,
		       defs * sizeof(cast_def));
		for (i = 0; i < defs; i++ ) {
			retval.cast_scope_val[i].name = empty_scope_name;
			retval.cast_scope_val[i].channel = -1;
		}
	} else {
		retval.cast_scope_len = 0;
		retval.cast_scope_val = 0;
	}
	
	return retval;
}
