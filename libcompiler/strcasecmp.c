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

#include <ctype.h>

/*
 * A Flick version of `strcasecmp'.  Since `strcasecmp' is not an ANSI-standard
 * function, we provide this function for ourselves.
 */
int
flick_strcasecmp(const char *s1, const char *s2)
{
	int s1_elem;
	int s2_elem;
	
	while (*s1 && *s2) {
		s1_elem = tolower(*s1);
		s2_elem = tolower(*s2);
		if (s1_elem != s2_elem)
			return (s1_elem - s2_elem);
		++s1;
		++s2;
	}
	if (*s1 != *s2)
		return (*s1 ? 1 : -1);
	return 0;
}

/* End of file. */

