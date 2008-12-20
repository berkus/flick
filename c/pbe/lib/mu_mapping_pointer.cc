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
 * This method handles ``simple'' pointer indirections --- pointers that are
 * present due to the requirements of the presentation style (e.g., for dynamic
 * memory management, or for passing arguments by reference) but which do not
 * convey any message data.
 *
 * In other words, a MAPPING_POINTER node is used when the *value* of a pointer
 * itself does not embody any information from the RPC.  (Compare this with
 * MAPPING_OPTIONAL_POINTER, which is used when the value of the pointer itself
 * *does* convey message data.)
 *
 * In terms of MINT, this means that there is no MINT representation for the
 * pointer handled here.  The MINT passed into this method (`itype') is
 * therefore the MINT describing the pointer target type, not the pointer type.
 * So, we don't ``descend'' the MINT tree here.
 *
 * This method calls `mu_array' to handle the real dirty work of storage
 * allocation/deallocation, and the marshaling/unmarshaling of the data.
 *
 * Also, compare this method with `mu_mapping_internal_array', which handles
 * presented pointers that are part of certain array presentation styles.
 */
void mu_state::mu_mapping_pointer(
	cast_expr pexpr,
	cast_type ctype,
	mint_ref itype,
	pres_c_mapping_pointer *pmap)
{
	/*
	 * If the CAST type of our pointer is a named type, locate the actual
	 * pointer type.  (Note: The MIG FE/PG makes these kinds of direct
	 * associations between named C types and MAPPING_POINTER nodes.  Other
	 * PG's generally indirect through a MAPPING_STUB, and the named type
	 * is derefenced to a pointer type there.)
	 */
	ctype = cast_find_typedef_type(&(pres->cast), ctype);
	if (!ctype)
		panic("In `mu_state::mu_mapping_pointer', "
		      "can't locate `typedef' for a named type.");
	
	assert(ctype->kind == CAST_TYPE_POINTER);
	cast_type to_ctype = ctype->cast_type_u_u.pointer_type.target;
	
	/* We reset array_data since we really don't have an array. */
	mu_array_data old_array_data = array_data;
	array_data.mint_len_min = 0;
	array_data.mint_len_max = 0;
	array_data.is_valid = 0;
	
	/* Pointers are really a fixed array of one element. */
	mu_array(pexpr, ctype, to_ctype, itype,
		 pmap->target, pmap->arglist_name);
	
	array_data = old_array_data;
}

/* End of file. */

