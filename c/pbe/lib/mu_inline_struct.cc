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

#include <assert.h>

#include <mom/libmint.h>
#include <mom/c/pbe.hh>

/*
 * This routine handles a PRES_C_INLINE_STRUCT inline presentation node, which
 * always corresponds to a MINT_STRUCT node on the interface (itype) side.
 *
 * This function recursively descends into each member of the MINT_STRUCT and
 * separately maps each onto the current inline_state.  This allows the
 * MINT_STRUCT to be "split apart" and rearranged arbitrarily, so the order of
 * the elements in the presented struct can be completely different from those
 * in the over-the-wire struct layout.   (Of course, the members of the
 * MINT_STRUCT can also be split apart and rearranged if desired.)
 */
void mu_state::mu_inline_struct(inline_state *ist,
				mint_ref itype,
				pres_c_inline inl)
{
	pres_c_inline_struct *inls = &(inl->pres_c_inline_u_u.struct_i);
	mint_struct_def *sdef = 0;
	
	/*****/
	
	assert(inl->kind == PRES_C_INLINE_STRUCT);
	
	/* Find the MINT type (itype) and make sure it matches. */
	if (pres->mint.defs.defs_val[itype].kind == MINT_STRUCT) 
		sdef = &(pres->mint.defs.defs_val[itype].mint_def_u.
			 struct_def);
	assert(sdef);
	
	/* Recursively descend through the structure. */
	for (unsigned int i = 0; i < inls->slots.slots_len; ++i) {
		int slot_index = (inls->
				  slots.slots_val[i].mint_struct_slot_index);
		mint_ref slot_type;
		
		if (slot_index != mint_slot_index_null)
			slot_type = sdef->slots.slots_val[slot_index];
		else
			/*
			 * The i'th PRES_C slot does not refer to any MINT
			 * structure slot.  In order to keep inlining, we need
			 * to pass down a reference to a MINT_VOID.
			 */
			slot_type = pres->mint.standard_refs.void_ref;
		
		mu_inline(ist, slot_type, inls->slots.slots_val[i].inl);
	}
}

/* End of file. */

