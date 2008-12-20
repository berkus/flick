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

#include <assert.h>
#include <mom/compiler.h>
#include <mom/c/pbe.hh>

/*
 * Decode the current `pres_c_inline' node `inl', which describes how the
 * current network message type `itype' maps onto the C slots/parameters/return
 * values described in the current `inline_state' `ist'.
 *
 * Basically, this is just a switch that fires off various routines for the
 * specific kinds of `pres_c_inline' nodes.
 */
void mu_state::mu_inline(inline_state *ist, mint_ref itype, pres_c_inline inl)
{
	/*
	 * If the ptype and itype we're trying to handle are void, just stop
	 * here.
	 */
	if (inl == 0) {
		assert(itype == mint_ref_null);
		return;
	}
	
	assert(itype >= 0);
	assert(itype < (signed int) pres->mint.defs.defs_len);
	
	switch (inl->kind) {
	case PRES_C_INLINE_ATOM:
		mu_inline_atom(ist, itype, inl);
		break;
	case PRES_C_INLINE_STRUCT:
		mu_inline_struct(ist, itype, inl);
		break;
	case PRES_C_INLINE_FUNC_PARAMS_STRUCT:
		mu_inline_func_params_struct(ist, itype, inl);
		break;
	case PRES_C_INLINE_HANDLER_FUNC:
		mu_inline_handler_func(ist, itype, inl);
		break;
	case PRES_C_INLINE_STRUCT_UNION:
		mu_inline_struct_union(ist, itype, inl);
		break;
	case PRES_C_INLINE_COLLAPSED_UNION:
		mu_inline_collapsed_union(ist, itype, inl);
		break;
	case PRES_C_INLINE_TYPED:
		mu_inline_typed(ist, itype, inl);
		break;
	case PRES_C_INLINE_XLATE:
		mu_inline_xlate(ist, itype, inl);
		break;
	case PRES_C_INLINE_COND:
		mu_inline_cond(ist, itype, inl);
		break;
	case PRES_C_INLINE_ASSIGN:
		mu_inline_assign(ist, itype, inl);
		break;
	case PRES_C_INLINE_VIRTUAL_UNION:
		mu_inline_virtual_union(ist, itype, inl);
		break;
	case PRES_C_INLINE_VOID_UNION:
		mu_inline_void_union(ist, itype, inl);
		break;
	case PRES_C_INLINE_MESSAGE_ATTRIBUTE:
		mu_inline_message_attribute(ist, itype, inl);
		break;
	case PRES_C_INLINE_ALLOCATION_CONTEXT:
		mu_inline_allocation_context(ist, itype, inl);
		break;
	case PRES_C_INLINE_TEMPORARY:
		mu_inline_temporary(ist, itype, inl);
		break;
		
	case PRES_C_INLINE_ILLEGAL:
		mu_inline_illegal(ist, itype, inl);
		break;
	default:
		panic("In `mu_state::mu_inline', unknown inline kind %d.",
		      inl->kind);
		break;
	}
}

/* End of file. */

