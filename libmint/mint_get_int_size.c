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

void mint_get_int_size(mint_1 *mint, mint_ref itype,
		       int *out_bits, int *out_is_signed)
{
	mint_def *def;
	mint_integer_def *i;
	int bits, is_signed;

	assert(itype >= 0); assert(itype < (signed int) mint->defs.defs_len);
	def = &(mint->defs.defs_val[itype]);
	assert(def->kind == MINT_INTEGER);
	i = &(def->mint_def_u.integer_def);

	if (i->min >= 0) {
		is_signed = 0;
		if (i->min + i->range <= 255)
			bits = 8;
		else if (i->min + i->range <= 65535)
			bits = 16;
		else
			/* XXX --- What about *really* big ints? */
			bits = 32;
	} else {
		is_signed = 1;
		if ((i->min >= -128) && (i->min + i->range <= 127))
			bits = 8;
		else if ((i->min >= -32768) && (i->min + i->range <= 32767))
			bits = 16;
		else
			/* XXX --- What about *really* big ints? */
			bits = 32;
	}
	
	*out_bits = bits;
	*out_is_signed = is_signed;
}

/* End of file. */

