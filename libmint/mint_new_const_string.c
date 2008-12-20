/*
 * Copyright (c) 1997, 1999 The University of Utah and
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

mint_const mint_new_const_string(const char *val)
{
	unsigned int i;
	mint_const elem;
	mint_const mc = mint_new_const(MINT_CONST_ARRAY);
	unsigned int len = strlen(val) + 1;
	
	mc->mint_const_u_u.const_array.mint_const_array_len = len;
	mc->mint_const_u_u.const_array.mint_const_array_val =
		mustmalloc(sizeof(mint_const_u) * len);
	for (i = 0; i < len; i++) {
		elem = mint_new_const(MINT_CONST_CHAR);
		elem->mint_const_u_u.const_char.kind = MINT_CONST_LITERAL;
		elem->mint_const_u_u.const_char.mint_const_char_u_u.value =
			val[i];
		mc->mint_const_u_u.const_array.mint_const_array_val[i] = elem;
	}
	return mc;
}

/* End of file. */

