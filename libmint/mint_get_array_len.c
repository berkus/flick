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

#include <mom/libmint.h>

void mint_get_array_len(mint_1 *mint, mint_ref itype,
			unsigned *min, unsigned *max)
{
	mint_def *def;
	mint_array_def *adef;
	mint_def *length_def;

	assert(itype >= 0); assert(itype < (signed int) mint->defs.defs_len);
	def = &(mint->defs.defs_val[itype]);
	assert(def->kind == MINT_ARRAY);
	adef = &(def->mint_def_u.array_def);
	
	length_def = &(mint->defs.defs_val[adef->length_type]);
	assert(length_def->kind == MINT_INTEGER);
	*min = length_def->mint_def_u.integer_def.min;
	*max = *min + length_def->mint_def_u.integer_def.range;
}

/* End of file. */

