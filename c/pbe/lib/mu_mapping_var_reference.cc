/*
 * Copyright (c) 1995, 1996, 1998, 1999 The University of Utah and
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
#include <mom/c/pbe.hh>

/*
 * This method handles C++-style reference types.  In all cases, a reference
 * is present only due to the requirements of the presentation style --- the
 * value of the reference itself cannot convey any message data.  (The value of
 * reference itself is unexaminable in C++; only the *referent* storage can be
 * accessed.)
 *
 * In terms of MINT, this means that there is no MINT representation for the
 * reference handled here.  The MINT passed into this method (`itype') is
 * therefore the MINT describing the referent type, not the reference type.
 * So, we don't ``descend'' the MINT tree here.
 *
 * Compare this method with `mu_mapping_pointer', which handles C-style pointer
 * indirections that convey no message data.
 */
void mu_state::mu_mapping_var_reference(
	cast_expr pexpr,
	cast_type ctype,
	mint_ref itype,
	pres_c_mapping_var_reference *pmap)
{
	/*
	 * If the CAST type of our reference is a named type, locate the actual
	 * reference type.  The CORBA C++ PG often makes `_out' types that are
	 * really just typedef'ed reference types.
	 */
	ctype = cast_find_typedef_type(&(pres->cast), ctype);
	if (!ctype)
		panic("In `mu_state::mu_mapping_var_reference', "
		      "can't locate `typedef' for a named type.");
	
	assert(ctype->kind == CAST_TYPE_REFERENCE);
	cast_type to_ctype = ctype->cast_type_u_u.reference_type.target;
	
	mu_mapping(pexpr, to_ctype, itype, pmap->target);
}

/* End of file. */

