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
#include <mom/libaoi.h>

/*
 * Return 1 if the constants are equal and 0 of they are not.
 */
int
aoi_const_eq(aoi_const c1, aoi_const c2)
{
	assert(c1);
	assert(c2);
	
	if (c1->kind != c2->kind)
		return 0;
	
	switch (c1->kind) {
	case AOI_CONST_INT:
		return (   c1->aoi_const_u_u.const_int
			== c2->aoi_const_u_u.const_int);
		break;
	
	case AOI_CONST_CHAR:
		return (   c1->aoi_const_u_u.const_char
			== c2->aoi_const_u_u.const_char);
		break;
		
	case AOI_CONST_FLOAT:
		return (   c1->aoi_const_u_u.const_float
			== c2->aoi_const_u_u.const_float);
		break;
		
	case AOI_CONST_STRUCT: {
		aoi_const_struct *cs1 = &(c1->aoi_const_u_u.const_struct);
		aoi_const_struct *cs2 = &(c2->aoi_const_u_u.const_struct);
		unsigned int i;
		
		if (cs1->aoi_const_struct_len != cs2->aoi_const_struct_len)
			return 0;
		
		for (i = 0; i < cs1->aoi_const_struct_len; i++) {
			if (!aoi_const_eq(cs1->aoi_const_struct_val[i],
					  cs2->aoi_const_struct_val[i]))
				return 0;
		}
		return 1;
	}
	break;
	
	case AOI_CONST_ARRAY: {
		aoi_const_array *ca1 = &(c1->aoi_const_u_u.const_array);
		aoi_const_array *ca2 = &(c2->aoi_const_u_u.const_array);
		unsigned int i;
		
		if (ca1->aoi_const_array_len != ca2->aoi_const_array_len)
			return 0;
		
		for (i = 0; i < ca1->aoi_const_array_len; ++i) {
			if (!aoi_const_eq(ca1->aoi_const_array_val[i],
					  ca2->aoi_const_array_val[i]))
				return 0;
		}
		return 1;
	}
	break;
	
	default:
		panic("In `aoi_const_eq', unknown aoi_const_kind %d.",
		      c1->kind);
		break;
	}
	
	panic("In `aoi_const_eq', should have returned in switch statement.");
	return 0;
}

/* End of file. */

