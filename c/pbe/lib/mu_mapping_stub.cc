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

#include <assert.h>

#include <mom/compiler.h>
#include <mom/c/libpres_c.h>
#include <mom/c/pbe.hh>

/* This routine handles PRES_C_MAPPING_STUB presentations, which (at least
   logically) cause the current marshal/unmarshal stub to call _another_
   marshal/unmarshal stub at runtime to perform the actual marshaling or
   unmarshaling of the current data item.  As an optimization, this call can be
   eliminated in some cases by inlining the sub-stub's marshal/unmarshal code
   directly into this stub.  (Note that this is "code inlining" --- completely
   different from the "data inlining" done by PRES_C_INLINE_* nodes.)

   However, the sub-stub cannot be inlined if it cannot be found in the list of
   stubs we're producing.  (This may occur, for example, if the sub-stub is to
   be provided manually by the user in a separately-linked object file.)
   Inlining must also be broken if we encounter a recursive type (a type that,
   directly or indirectly, may contain a member of itself); otherwise code
   generation would get into an infinite loop trying to produce an infinitely
   deep marshal/unmarshal stub.
*/
void mu_state::mu_mapping_stub(cast_expr expr, cast_type ctype, mint_ref itype,
			       pres_c_mapping map)
{
	int stub_index;
	
	/* Determine the index of the stub we need.  This is an index into
	   `pres->stubs.stubs_val[]'. */
	stub_index = pres_c_find_mu_stub(pres, itype, ctype, map,
					 ((op & MUST_ENCODE) ?
					  PRES_C_MARSHAL_STUB :
					  PRES_C_UNMARSHAL_STUB));
	
	/* Decide if we should inline the stub code or simply output a call to
	   the marshal/unmarshal stub. */
	if ((stub_index >= 0) &&
	    (stub_inline_depth[stub_index] < MAX_STUB_RECURSIVE_INLINE_DEPTH)
	    ) {
		/* Increment the "depth" of this stub's inlining around the
		   call to `mu_mapping_stub_inline'.  This is what keeps us
		   from infinitely inlining the code for self-referential data
		   types! */
		++stub_inline_depth[stub_index];
		mu_mapping_stub_inline(expr, ctype, itype, map);
		--stub_inline_depth[stub_index];
	} else {
		/* We *must* break the current glob before calling a marshal or
		   unmarshal stub.  Each stub assumes that it starts with a
		   "clean plate." */
		break_glob();
		mu_mapping_stub_call(expr, ctype, itype, map);
	}
}

