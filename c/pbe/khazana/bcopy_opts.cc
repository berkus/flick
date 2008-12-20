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

#include <mom/libmint.h>
#include <mom/c/libcast.h>

#include "khazana.h"

/*
 * Return statements that specify whether or not we need to do bit translation.
 * I.E. in Khazana, this would return
 *   for decoding same endianness, #if 0, #else, and #endif
 *   for decoding opposite endianness, #if 1, #else, and #endif
 *   for encoding, we never have to swap
 * However, single chars never need byte swapped.
 * 
 * The parameter specifies whether to output the if, else or endif
 * (0, 1, 2 respectively)
 */

static const char *decode_if_text
	= "#if %d /* are we in the swapping decode or non-swapping decode */";
static const char *decode_else_text
	= "#else";
static const char *decode_endif_text
	= "#endif";

static const char *encode_if_text
	= "#if 0 /* We never byte swap on encoding */";
static const char *encode_else_text
	= "#else /* Always bcopy */";
static const char *encode_endif_text
	= "#endif /* Always bcopy */";

static const char *never_text
	= "#if 0 /* No need to swap this */";
static const char *always_text
	= "#if 1 /* Data layout differs, can't bcopy */";

cast_stmt khazana_mu_state::mu_bit_translation_necessary(int which,
							 mint_ref itype)
{
	cast_stmt macro = 0;
	int decode = op & MUST_DECODE;
	int msgsize, memsize, align;
	char *name;
	
	/* First, determine the in-message size of the type in question. */
	get_prim_params(itype, &msgsize, &align, &name);
	
	/* Next, determine the in-memory size of the type in question. */
	mem_mu_state::get_prim_params(itype, &memsize, &align, &name);
	
	/* Now determine which statement to return */
	switch (which) {
	case 0:
		// #if
		if (memsize != msgsize)
			/* Never bcopy, since the in-memory size and
			   in-message size are different. */
			macro = cast_new_text(always_text);
		else if (msgsize == 1)
			/* Never have to swap single bytes! */
			macro = cast_new_text(never_text);
		else
			macro = cast_new_text(decode ?
					      flick_asprintf(decode_if_text,
							     should_swap)
					      : encode_if_text);
		break;
	case 1:
		// #else
		macro = cast_new_text(decode ?
				      decode_else_text : encode_else_text);
		break;
	case 2:
		// #endif
		macro = cast_new_text(decode ?
				      decode_endif_text : encode_endif_text);
		break;
	default:
		panic(("Invalid value passed to "
		       "`mu_bit_translation_necessary'."));
		break;
	}
	
	return macro;
}

/* End of file */

