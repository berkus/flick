/*
 * Copyright (c) 1995, 1996, 1997 The University of Utah and
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

#include "private.h"

/* Can return mint_ref_null if default case is not set
   and selected case is not found. */
mint_ref mint_find_union_case(mint_1 *mint, mint_ref union_r,
			      mint_const const_r)
{
	assert(m(union_r).kind == MINT_UNION);
	mint_union_def *udef = &m(union_r).mint_def_u.union_def;
	
	for (unsigned int i = 0; i < udef->cases.cases_len; i++) {
		if (mint_const_cmp(udef->cases.cases_val[i].val, const_r) == 0)
			return udef->cases.cases_val[i].var;
	}
	
	return udef->dfault;
}

/* End of file. */

