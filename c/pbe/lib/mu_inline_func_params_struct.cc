/*
 * Copyright (c) 1997 The University of Utah and
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
 * This method handles `PRES_C_INLINE_FUNC_PARAMS_STRUCT' presentation nodes,
 * which are used to link the parameters and return value of a function (as
 * represented by `ist') with a MINT_STRUCT (`itype').  Compare this method
 * with `mu_state::mu_inline_struct'.
 *
 * In this method, we simply iterate over the PRES_C slot definitions; each has
 * a link to its corresponding slot in the `itype' MINT_STRUCT.  The return
 * value, if any, is handled specially.
 *
 * The return value slot is arbitrarily processed before any of the normal
 * parameters, meaning that the encoded return value will come before any
 * encoded parameters in the final message.  (Or, at least, that the generated
 * C code will process the return value first.)  Different back ends may
 * override this method in order to place the encoded return value after any
 * normal parameters, for example.
 *
 * XXX --- We should implement an auxiliary method for deciding whether to
 * process the return value first or last.
 */
void mu_state::mu_inline_func_params_struct(inline_state *ist,
					    mint_ref itype,
					    pres_c_inline inl)
{
	pres_c_inline_func_params_struct *inls
		= &(inl->pres_c_inline_u_u.func_params_i);
	mint_struct_def *sdef = 0;
	
	/*****/
	
	assert(inl->kind == PRES_C_INLINE_FUNC_PARAMS_STRUCT);
	
	/* Find the MINT type (itype) and make sure it matches. */
	if (pres->mint.defs.defs_val[itype].kind == MINT_STRUCT) 
		sdef = &(pres->mint.defs.defs_val[itype].mint_def_u.
			 struct_def);
	assert(sdef);
	
	/*
	 * XXX --- Call the `mu_prefix_params' hook so that certain BE's can do
	 * something special immediately before any parameter data is handled.
	 *
	 * This isn't really The Right Way to do this kind of thing.  What we
	 * truly need is some kind of PRES_C-like IR that translates between
	 * MINT and the actual, on-the-wire message format.
	 */
	mu_prefix_params();
	
	/* Process the return value, if any. */
	if (inls->return_slot) {
		int slot_index = inls->return_slot->mint_struct_slot_index;
		mint_ref slot_type;
		
		if (slot_index != mint_slot_index_null)
			slot_type = sdef->slots.slots_val[slot_index];
		else
			/*
			 * The `return_slot' does not refer to any MINT
			 * structure slot.  In order to keep inlining, we need
			 * to pass down a reference to a MINT_VOID.
			 */
			slot_type = pres->mint.standard_refs.void_ref;
		
		mu_inline(ist, slot_type, inls->return_slot->inl);
	}
	
	/* Now handle the ordinary slots. */
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

