/*
 * Copyright (c) 1999 The University of Utah and
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

#include <mom/c/libcast.h>

#include "sun.h"

void sun_mu_state::mu_array(cast_expr array_expr, cast_type array_ctype,
			    cast_type elem_ctype, mint_ref elem_itype,
			    pres_c_mapping elem_map, char *cname)
{
	assert(!in_packed_array);
	
	/*
	 * If we're processing an array whose XDR stream should be packed,
	 * record that.  These include strings and arrays of `opaque' data
	 * (similar to CORBA `octet').  Opaque data is indentified by an
	 * 8-bit MINT_INTEGER (range of 255, starting at either 0 or -128).
	 */
	if (array_data.is_valid
	    && (mu_array_is_string(cname)
		|| ((pres->mint.defs.defs_val[elem_itype].kind == MINT_INTEGER)
		    && (pres->mint.defs.defs_val[elem_itype].mint_def_u.
			integer_def.range == 255)
		    && ((pres->mint.defs.defs_val[elem_itype].mint_def_u.
			 integer_def.min == 0)
			|| (pres->mint.defs.defs_val[elem_itype].mint_def_u.
			    integer_def.min == -128))))) {
		in_packed_array = 1;
	}
	
	/* Otherwise process the array normally.  */
	mem_mu_state::mu_array(array_expr, array_ctype,
			       elem_ctype, elem_itype, elem_map, cname);
	
	/* we need to pad ourselves to a 4-byte boundary */
	/* XXX we still don't fill the padded bytes with zeroes like the 
           sun specification dictates we ought to. */
	if (in_packed_array && ((align_bits < 2) || (align_ofs & 3)))
                chunk_prim(2, 0);
	
	in_packed_array = 0;
}

/*
 * For Sun/TCP, the encoded length of strings is different from the presented
 * length.  For XDR, encoding the string leaves off the terminator, but the
 * presented array still needs it's terminator (and space allocated for the
 * terminator).  Here we "patch" the length from the arglist by adding 1 to it.
 */
void sun_mu_state::mu_array_get_pres_length(char *cname,
					    cast_expr *len_expr,
					    cast_type *len_ctype)
{
	int gotarg = arglist->getargs(cname, "length", len_expr, len_ctype);
	assert(gotarg);
	assert(len_expr);
	assert(len_ctype);
	*len_expr = cast_new_binary_expr(CAST_BINARY_ADD,
					 *len_expr,
					 cast_new_expr_lit_int(1, 0));
}

/* For Sun/TCP, we don't encode terminators. */
int sun_mu_state::mu_array_encode_terminator(char */*cname*/)
{
	return 0;
}

void sun_mu_state::mu_array_do_msgptr(cast_expr ofs_expr,
				      cast_expr ptr_expr,
				      cast_type ptr_ctype,
				      cast_type target_ctype,
				      cast_expr len_expr,
				      cast_expr size_expr,
				      char *cname)
{
	/*
	 * Special string case: Since XDR strings do not encode the NULL
	 * terminator byte, it may not be possible to do a msgptr optimization
	 * to decode it (adding the terminator to the end may clobber important
	 * data in the stream).
	 */
	if (mu_array_is_string(cname)) {
		cast_expr lexpr;
		cast_type ltype;
		int gotarg = arglist->getargs(cname, "length",
					      &lexpr, &ltype);
		assert(gotarg);assert(lexpr);assert(ltype);
		
		/* Save off the cblock -- we need to catch a regular msgptr and
                   a bcopy into their own CAST blocks. */
		cast_stmt orig_cblock = c_block;
		
		/* Get the code for doing a regular msgptr. */
		c_block = 0;
		mem_mu_state::mu_array_do_msgptr(ofs_expr, ptr_expr,
						 ptr_ctype, target_ctype,
						 len_expr, size_expr, cname);
		cast_stmt msgptr_block = c_block;

		/* Get the code for doing a bcopy. */
		c_block = 0;
		mu_array_do_bcopy(ofs_expr, ptr_expr, ptr_ctype, target_ctype,
				  len_expr, size_expr, cname);
		cast_stmt bcopy_block = c_block;
		
		/* Restore the original CAST block. */
		c_block = orig_cblock;

		/* Add our conditional. */
		add_stmt(cast_new_if(
			cast_new_binary_expr(
				CAST_BINARY_BAND,
				cast_new_binary_expr(
					CAST_BINARY_ADD,
					lexpr,
					cast_new_expr_lit_int(3, 0)),
				cast_new_unary_expr(
					CAST_UNARY_BNOT,
					cast_new_expr_lit_int(3, 0))),
			msgptr_block,
			bcopy_block));
	} else {
		/* Do the default msgptr. */
		mem_mu_state::mu_array_do_msgptr(ofs_expr, ptr_expr,
						 ptr_ctype, target_ctype,
						 len_expr, size_expr, cname);
	}
}

/* End of file. */
