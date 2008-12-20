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
#include "mach3.h"

/*
 * Return statements that specify whether or not we need to do bit translation.
 * I.E. in Mach3MIG, this always returns #if 0, #else, and #endif, indicating
 * that byte swapping is never necessary (IPC on the same host).
 * 
 * The parameter specifies whether to output the if, else or endif
 * (0, 1, 2 respectively)
 */

static const char *if_text
	= "#if 0 /* We don't need to ever do bit translation */";
static const char *else_text
	= "#else /* Always do a bcopy */";
static const char *endif_text
	= "#endif /* bcopy? */";

cast_stmt mach3_mu_state::mu_bit_translation_necessary(int which,
						       mint_ref /*itype*/)
{
	cast_stmt macro = 0;
	
	switch (which) {
	case 0:
		// #if
		macro = cast_new_text(if_text);
		break;
	case 1:
		// #else
		macro = cast_new_text(else_text);
		break;
	case 2:
		// #endif
		macro = cast_new_text(endif_text);
		break;
	default:
		panic(("Invalid value passed to "
		       "`mu_bit_translation_necessary'."));
		break;
	}
	
	return macro;
}

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

cast_expr mach3_mu_state::mu_get_sizeof(mint_ref itype,
					cast_type ctype,
					pres_c_mapping map,
					int *out_size,
					int *out_align_bits)
{
	cast_expr res = 0;
	*out_size = *out_align_bits = -1;

	/* if we don't have a direct mapping, we can't easily
           determine the size of this type */
	if (map->kind == PRES_C_MAPPING_DIRECT)
		/* default case */
		return mem_mu_state::mu_get_sizeof(
			itype, ctype, map, out_size, out_align_bits);
	
	else if (map->kind == PRES_C_MAPPING_REFERENCE) {
		/* object references (ports) in mach are fixed size */
		mint_def *def;
		assert(itype >= 0);
		assert(itype < (signed int) pres->mint.defs.defs_len);
		def = &pres->mint.defs.defs_val[itype];
		
		if (def->kind == MINT_INTERFACE) {
			char *name;
			get_prim_params(itype, out_size, out_align_bits,
					&name);
		}
		
		if (*out_size >= 0) {
			res = cast_new_expr_lit_int(*out_size, 0);
		}
	}
	return res;
}

/* End of file. */

