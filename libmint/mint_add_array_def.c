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

mint_ref mint_add_array_def(mint_1 *mint, mint_ref element_type,
			    unsigned min_len, unsigned max_len)
{
	mint_ref length_type;
	mint_ref i;
	
	/*
	 * Find or produce a datatype representing the possible array lengths.
	 */
	assert(max_len >= min_len);
	length_type = mint_add_integer_def(mint, min_len, max_len - min_len);
	
	/* See if there's already a matching array defined. */
	for (i = 0; i < (signed int) mint->defs.defs_len; i++) {
		if ((m(i).kind == MINT_ARRAY)
		    && (m(i).mint_def_u.array_def.element_type == element_type)
		    && (m(i).mint_def_u.array_def.length_type == length_type))
			return i;
	}
	
	/* If not, create one. */
	i = mint_add_def(mint);
	m(i).kind = MINT_ARRAY;
	m(i).mint_def_u.array_def.element_type = element_type;
	m(i).mint_def_u.array_def.length_type = length_type;
	
	return i;
}

/* End of file. */

