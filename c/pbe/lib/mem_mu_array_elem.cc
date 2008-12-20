/*
 * Copyright (c) 1995, 1996, 1999 The University of Utah and
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
#include <mom/c/be/mem_mu_state.hh>

#ifdef min
#undef min
#endif

#define min(a, b) ((a) < (b)) ? (a) : (b)

static void reduce_bits(int *first_bits, int *first_ofs,
			int *final_bits, int *final_ofs)
{
	// We water the alignment bits and alignment offset until they're compatible
	// This could, potentially be better, if the encoding is such that all elements
	// should be laid out identically, but CDR doesn't specify that, so you can't
	// pad before the array...
	int bits = min(*first_bits, *final_bits);
	do {
		int mask = (1 << bits) - 1;
		*first_bits = *final_bits = bits;
		*first_ofs &= mask;
		*final_ofs &= mask;
		bits--;
	} while (*first_bits != *final_bits ||
		 *first_ofs != *final_ofs);
}

void mem_mu_state::mu_array_elem(cast_expr elem_expr, cast_type elem_ctype,
				 mint_ref elem_itype, pres_c_mapping elem_mapping,
				 unsigned long len_min, unsigned long len_max)
{
	/* Take a dry run in a cloned state.  */
	mem_mu_state *sub = (mem_mu_state*)clone();
	sub->abort_block = new mu_abort_block();
	sub->abort_block->set_kind(MABK_THREAD);
	sub->abort_block->begin();
	sub->current_span = 0;
	sub->mu_state::mu_array_elem(elem_expr, elem_ctype,
				     elem_itype, elem_mapping,
				     len_min, len_max);
	
	/* Reduce the initial alignment knownness if necessary
	   to be compatible with the alignment at the end. */
	reduce_bits(&align_bits, &align_ofs,
		    &sub->align_bits, &sub->align_ofs);
	
	int prev_chunk_count = chunk_count;
	
	/* Now take the live run, with the final alignment we computed.  */
	mu_state::mu_array_elem(elem_expr, elem_ctype, elem_itype,
				elem_mapping, len_min, len_max);

	/* If len_min == 0, then no elements at all might be marshaled -
	   in that case, the final align_bits had better be no more than the initial.  */
	if (len_min == 0)
		reduce_bits(&align_bits, &align_ofs,
			    &sub->align_bits, &sub->align_ofs);
	
	elem_one_chunk = (chunk_count - prev_chunk_count <= 1);
	
	/* Enforce the one-chunk-per-array-element rule.  */
	if (len_max > 1)
		break_chunk();

	/* If we're marshaling each element into a separate glob,
	   enforce that too.  */
	if (!array_one_glob)
		break_glob();
	sub->abort_block->end();
	delete sub;
}

