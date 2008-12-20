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

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <mom/compiler.h>
#include <mom/libmint.h>
#include <mom/c/pfe.hh>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include "private.hh"

void pg_state::p_do_return_union(aoi_operation * /*ao*/,
				 pres_c_inline* reply_l4_inl,
				 mint_ref reply_ref,
				 cast_ref /*cfunc*/,
				 pres_c_inline_index discrim_idx)
{
	/*
	 * WARNING:
	 * DO NOT make this make any assumptions about the mint union it
	 * receives, except that there is some function that determines if
	 * it's a function reply.  The union passed into the function may
	 * need modification for different exception presentations.
	 */
	unsigned int i;
	mint_union_def *mu = &(m(reply_ref).mint_def_u.union_def);
	
	/*
	 * We need to create space in the virtual union for as many elements
	 * as there are in the MINT union.  This should always be 2 (normal
	 * and system error).  User exceptions fall in the 'default' slot.
	 * However, it is conceivable that there could be more or less.
	 */
	pres_c_inline new_return
		= pres_c_new_inline_virtual_union(mu->cases.cases_len);
	pres_c_inline_virtual_union *vuinl
		= &new_return->pres_c_inline_u_u.virtual_union;
	
	/* Build the descriminator PRES_C. */
	vuinl->arglist_name = pres_c_make_arglist_name("vuinl");
	vuinl->discrim = pres_c_new_inline_atom(
		discrim_idx,
		p_make_exception_discrim_map(vuinl->arglist_name));
	
	for (i = 0; i < mu->cases.cases_len; i++) {
		// We need to special case the normal return value
		if (p_is_normal_return_case(mu, i)) {
			vuinl->cases.cases_val[i] = *reply_l4_inl;
		} else
			// Build the appropriate mapping of an exception in
			// the current presentation
			p_do_exceptional_case(&vuinl->cases.cases_val[i],
					      &mu->cases.cases_val[i],
					      mu->cases.cases_val[i].var,
					      discrim_idx);
	}
	
	// There should generally be a default case here.
	// We treat it like any other, except the call to do_exceptional_case
	// doesn't get a mint_union_case value...
	if (mu->dfault != -1) {
		p_do_exceptional_case(&vuinl->dfault, 0, mu->dfault,
				      discrim_idx);
	} else
		// If there is no default, we just don't put one on the
		// virtual union
		vuinl->dfault = 0;
	*reply_l4_inl = new_return;
}
