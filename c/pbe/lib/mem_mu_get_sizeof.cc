/*
 * Copyright (c) 1995, 1996, 1997, 1999 The University of Utah and
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
#include <mom/c/be/mem_mu_state.hh>

/* This function should return either
   a null expression, if the type is NOT fixed,
   or a sizeof expression for the size of the data structure.
   Specifically, this is used from within mu_array
   to see if the memory layout of
   the data structure is identical to
   the buffer layout.
   
   NOTE:  This does NOT require that
   the bit-wise data layout is correct,
   just that the alignment & padding is OK.
   See mu_bit_translation_necessary for stuff regarding bit translation.
   */

cast_expr mem_mu_state::mu_get_sizeof(mint_ref itype,
				      cast_type /*ctype*/,
				      pres_c_mapping map,
				      int *out_size,
				      int *out_align_bits)
{
	char *macro_name;
	
	get_prim_params(itype, out_size, out_align_bits, &macro_name);
	/*
	 * If get_prim_params() can't figure out the name, or we don't have a
	 * direct mapping, we can't easily determine the size of this type.
	 */
	if (!macro_name || (map->kind != PRES_C_MAPPING_DIRECT)) {
		/* Unable to determine size. */
		*out_size = *out_align_bits = -1;
		return 0;
	}
	
	assert(*out_size >= 0);
	return cast_new_expr_lit_int(*out_size, 0);
}

/* End of file. */

