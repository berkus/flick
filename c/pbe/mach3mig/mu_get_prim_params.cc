/*
 * Copyright (c) 1995, 1996, 1997, 1998, 1999 The University of Utah and
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
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include "mach3.h"

void mach3_mu_state::get_prim_params(mint_ref itype, int *out_size, int *out_align_bits,
				     char **out_name)
{
	/* Booleans are handled as special 32-bit types under Mach 3.  */
	mint_def *def = &pres->mint.defs.defs_val[itype];

	if ((def->kind == MINT_INTEGER)
		   && (def->mint_def_u.integer_def.min == 0)
		   && (def->mint_def_u.integer_def.range == 1)) {
		/*
		 * If we didn't handle booleans specially,pbe_mem would marshal
		 * them in as little space as possible - i.e. one byte. 
		 * However, according to MIG conventions, booleans are 32 bits.
		 */
		*out_size = 4;
		*out_align_bits = 2;
		*out_name = flick_asprintf("flick_%s_%s_boolean",
					   get_be_name(), get_buf_name());
	} else
		/* Do the default thing */
		mem_mu_state::get_prim_params(itype, out_size, out_align_bits, out_name);

	if (!mach3_array) {
		/*
		 * When m/u-ing regular data, the data must take up at least
		 * 4 bytes, plus 4 bytes for the type header.
		 */
		if (*out_size < 4) *out_size = 4;
		*out_size += 4; /* for type header info */
		*out_align_bits = 2;
	}
	
	/*
	 * Yes, we NEVER align to anything larger than 4 bytes.  Reasons:
	 *   a) MIG doesn't do it, so neither should we, and
	 *   b) MACH doesn't allow padding in messages, except after a datum
	 *      (or last datum in an array) such that the encoded length is
	 *      always a multiple of 4 (allowing any following type tag to be
	 *      4-byte aligned).
	 */
	if (*out_align_bits > 2) *out_align_bits = 2;
}

/* End of file. */
