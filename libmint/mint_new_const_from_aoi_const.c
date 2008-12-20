/*
 * Copyright (c) 1995, 1996 The University of Utah and
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

/* Translate an AOI constant to a MINT constant. */
mint_const mint_new_const_from_aoi_const(aoi_const aoic)
{
	mint_const mc = mustcalloc(sizeof(*mc));
	u_int i, array_len, struct_len;
	aoi_const aoi_array_element, aoi_struct_element;

	switch (aoic->kind) {
	case AOI_CONST_INT:
		/* XXX --- Update to support symbolic constants. */
		mc->kind = MINT_CONST_INT;
		mc->mint_const_u_u.const_int.kind = MINT_CONST_LITERAL;
		mc->mint_const_u_u.const_int.mint_const_int_u_u.value =
			aoic->aoi_const_u_u.const_int;
		break;
		
	case AOI_CONST_CHAR:
		/* XXX --- Update to support symbolic constants. */
		mc->kind = MINT_CONST_CHAR;
		mc->mint_const_u_u.const_char.kind = MINT_CONST_LITERAL;
		mc->mint_const_u_u.const_char.mint_const_char_u_u.value =
			aoic->aoi_const_u_u.const_char;
		break;
		
	case AOI_CONST_FLOAT:
		/* XXX --- Update to support symbolic constants. */
		mc->kind = MINT_CONST_FLOAT;
		mc->mint_const_u_u.const_float.kind = MINT_CONST_LITERAL;
		mc->mint_const_u_u.const_float.mint_const_float_u_u.value =
			aoic->aoi_const_u_u.const_float;
		break;
		
	case AOI_CONST_STRUCT:
		mc->kind = MINT_CONST_STRUCT;
		struct_len = aoic->
			     aoi_const_u_u.const_struct.aoi_const_struct_len;
		mc->mint_const_u_u.const_struct.mint_const_struct_val =
			mustmalloc(sizeof(mint_const)*struct_len);
		for (i = 0; i < struct_len; i++) {
			aoi_struct_element = aoic->aoi_const_u_u.const_struct.
					     aoi_const_struct_val[i];
			mc->mint_const_u_u.const_struct.
				mint_const_struct_val[i] =
				mint_new_const_from_aoi_const(
					aoi_struct_element);
		}
		mc->mint_const_u_u.const_struct.mint_const_struct_len =
			struct_len;
		break;
		
	case AOI_CONST_ARRAY:
		mc->kind = MINT_CONST_ARRAY;
		array_len = aoic->
			    aoi_const_u_u.const_array.aoi_const_array_len;
		mc->mint_const_u_u.const_array.mint_const_array_val =
			mustmalloc(sizeof(mint_const)*array_len);
		for (i = 0; i < array_len; i++) {
			aoi_array_element = aoic->aoi_const_u_u.const_array.
					    aoi_const_array_val[i];
			mc->mint_const_u_u.const_array.
				mint_const_array_val[i] =
				mint_new_const_from_aoi_const(
					aoi_array_element);
		}
		mc->mint_const_u_u.const_array.mint_const_array_len =
			array_len;
		break;
		
	default:
		panic("mint_new_const_from_aoi_const: aoi_const_kind (%d) unexpected",
		      aoic->kind);
		break;
	}
	
	return mc;
}

