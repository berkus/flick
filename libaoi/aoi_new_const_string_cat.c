/*
 * Copyright (c) 1995, 1996, 1999 The University of Utah and
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
#include <mom/libaoi.h>

aoi_const aoi_new_const_string_cat(const char *s1, const char *s2)
{
	const char *p;
	unsigned int l, i;
	aoi_const ac = aoi_new_const(AOI_CONST_ARRAY);
	
	l = strlen(s1) + strlen(s2);
	ac->aoi_const_u_u.const_array.aoi_const_array_val
		= (aoi_const *) mustcalloc(l * sizeof(aoi_const));
	ac->aoi_const_u_u.const_array.aoi_const_array_len
		= l;
	
	i = 0;
	
	/* For each character in s1, add a const_char array element. */
	p = s1;
	while (*p != '\0') {
		ac->aoi_const_u_u.const_array.aoi_const_array_val[i]
			= aoi_new_const_char(*p);
		p++;
		i++;
	}
	
	/* For each character in s2, add a const_char array element. */
	p = s2;
	while (*p != '\0') {
		ac->aoi_const_u_u.const_array.aoi_const_array_val[i]
			= aoi_new_const_char(*p);
		p++;
		i++;
	}
	
	return ac;
}

/* End of file. */

