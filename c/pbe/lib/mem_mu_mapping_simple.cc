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

#include <mom/compiler.h>
#include <mom/libmint.h>
#include <mom/libaoi.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>
#include <mom/c/be/mem_mu_state.hh>

/* This is the default implementation of m/u code generation
   for simple values (ints, chars, etc.).
   It assumes that the appropriate runtime support header file
   defines a set of macros named `mom_mem_<op>_<endian>_<type>'.
   <op> is either `encode' or `decode' (marshal or unmarshal).
   <endian> is `big', `little', or `local',
   indicating the endianness of the message stream being processed.
   (`local' means the standard endianness for the target architecture.)
   <type> is the name of a simple C type
   whose size and shape matches that of the appropriate atom
   in the message stream we're marshaling.

   mom_mem_decode_*() macros are expected to take no parameters,
   but return the value of the next atom in the message,
   converted appropriately to native format for the target machine
   
   mom_mem_encode_*() macros are experted to take one argument:
   the value to convert and marshal into the message stream.
*/
void mem_mu_state::mu_mapping_simple(cast_expr expr,
				     cast_type ctype,
				     mint_ref itype)
{
	/* Find the marshaling parameters for this primitive type. */
	int prim_size, prim_align_bits;
	char *macro_name;
	
	get_prim_params(itype, &prim_size, &prim_align_bits, &macro_name);
	if (!macro_name) {
		panic(("In `mem_mu_state::mu_mapping_simple', "
		       "invalid MINT type (%d) encountered."),
		      itype);
	}
	
	cast_expr cex;
	cast_expr macro_expr = cast_new_expr_name(macro_name);
	cast_expr ofs_expr = cast_new_expr_lit_int(chunk_prim(prim_align_bits,
							      prim_size),
						   0);
	cast_expr type_expr = cast_new_expr_type(ctype);
	
	/* Spit out the marshal/unmarshal macro call. */
	cex = cast_new_expr_call_3(macro_expr,
				   ofs_expr,
				   expr,
				   type_expr);
	
	add_stmt(cast_new_stmt_expr(cex));
}

/* End of file. */

