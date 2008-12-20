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
#include <mom/compiler.h>
#include <mom/libmint.h>
#include <mom/c/be/mem_mu_state.hh>

void mem_mu_state::get_prim_params(mint_ref itype,
				   int *out_size, int *out_align_bits,
				   char **out_name)
{
	mint_def *def = &(pres->mint.defs.defs_val[itype]);
	
	const char *basic_type_name;
	int size;

	switch (def->kind) {
	case MINT_INTEGER: {
		int bits, is_signed;
		
		mint_get_int_size(&pres->mint, itype, &bits, &is_signed);
		basic_type_name = is_signed ? "signed" : "unsigned";
		size = bits / 8;
		
		/*
		 * When processing array lengths, always make them four bytes.
		 * This is the _common case_ so it's in the library
		 * If you want variable array lengths, override this...
		 */
		mu_inline_alloc_context *iac = inline_alloc_context;
		while (iac) {
			/*
			 * If one of the allocation contexts is processing a
			 * length branch, then we must be that length.
			 *
			 * XXX - Is that always true?  We should really make
			 * sure that whatever we're processing is really linked
			 * to the length branch we find here.  Until we're
			 * smarter, this is about all we can do.
			 */
			if (iac->is_length) {
				size = 4;
				break;
			}
			iac = iac->parent_context;
		}
		
		break;
	}
	
	case MINT_SCALAR:
		basic_type_name = (def->mint_def_u.scalar_def.flags !=
				   MINT_SCALAR_FLAG_UNSIGNED) ?
				  "signed" : "unsigned";
		size = def->mint_def_u.scalar_def.bits / 8;
		break;
		
	case MINT_BOOLEAN:
		/* this is the common case, so override this if you
		   want something different */
		basic_type_name = "boolean";
		size = 4;
		break;
		
	case MINT_CHAR:
		basic_type_name = "char";
		size = def->mint_def_u.char_def.bits / 8;
		break;
		
	case MINT_FLOAT:
		basic_type_name = "float";
		size = def->mint_def_u.float_def.bits / 8;
		break;
		
	case MINT_INTERFACE:
		basic_type_name = "port";
		size = 4;
		break;
		
	case MINT_VOID:
		basic_type_name = "void";
		size = 0;
		break;
		
	default:
		/* We used to panic here, but really we should just signal
		   an error to the caller, and let them decide what to do. */
		*out_size = 0;
		*out_align_bits = 0;
		*out_name = 0;
		return;
	}
	
	int natural_align_bits = 0;
	
	while (size > (1 << natural_align_bits))
		natural_align_bits++;
	
	*out_size = size;
	*out_align_bits = natural_align_bits;
	*out_name = flick_asprintf("flick_%s_%s_%s%d",
				   get_encode_name(),
				   get_buf_name(),
				   basic_type_name,
				   size*8);
}

/* End of file. */

