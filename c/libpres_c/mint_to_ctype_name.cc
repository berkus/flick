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

#include <mom/compiler.h>
#include <mom/libmint.h>
#include <mom/c/libpres_c.h>

char *mint_to_ctype_name(mint_1 *mint, mint_ref itype)
{
	mint_def *def = &mint->defs.defs_val[itype];
	
	char *name;
	
	switch (def->kind) {
	case MINT_INTEGER: {
		int bits, is_signed;
		mint_get_int_size(mint, itype, &bits, &is_signed);
		name = flick_asprintf("flick_%ssigned%d_t",
				      (is_signed ? "" : "un"),
				      bits);
		break;
	}
	case MINT_CHAR:
		name = flick_asprintf("flick_char%d_t",
				      def->mint_def_u.char_def.bits);
		break;
	case MINT_FLOAT:
		name = flick_asprintf("flick_float%d_t",
				      def->mint_def_u.float_def.bits);
		break;
	default:
		panic("mint_to_ctype_name: unknown mint kind %d\n", def->kind);
	}
	
	return name;
}

/* End of file. */

