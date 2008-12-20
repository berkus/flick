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

#include <mom/compiler.h>
#include <mom/c/pbe.hh>

/* A pres_c_inline_atom node represents the transition
   from "inline-flavor" mappings,
   in which one interface (network message) data item
   is mapped to a set of C data items,
   to "mapping-flavor" mappings,
   in which one interface item is mapped to one C item.
   This routine just selects the appropriate C item
   based on the pres_c_inline_atom's index value
   (see inline_state::slot_access()),
   and jumps into the generic mapping-decoding routine.
*/
void mu_state::mu_inline_atom(inline_state *ist,
			      mint_ref itype, pres_c_inline inl)
{
	cast_expr expr;
	cast_type ctype;
	
	assert(itype >= 0);
	assert(itype < (signed int) pres->mint.defs.defs_len);
	
	ist->slot_access(inl->pres_c_inline_u_u.atom.index, &expr, &ctype);
	mu_mapping(expr, ctype, itype, inl->pres_c_inline_u_u.atom.mapping);
}

/* End of file. */

