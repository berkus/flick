/*
 * Copyright (c) 1995, 1996, 1997, 1998 The University of Utah and
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
#include <mom/c/libpres_c.h>

pres_c_inline pres_c_new_inline_expanded_union(int cases)
{
	pres_c_inline inl = pres_c_new_inline(PRES_C_INLINE_EXPANDED_UNION);
	
	inl->pres_c_inline_u_u.expanded_union.cases.cases_len = cases;
	if (cases > 0)
		inl->pres_c_inline_u_u.expanded_union.cases.cases_val =
			mustcalloc(cases *
				   sizeof(pres_c_inline));
	else
		inl->pres_c_inline_u_u.expanded_union.cases.cases_val =
			0;
	
	inl->pres_c_inline_u_u.expanded_union.dfault = 0;
	
	return inl;
}

/* End of file. */

