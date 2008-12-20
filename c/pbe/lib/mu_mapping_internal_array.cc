/*
 * Copyright (c) 1998, 1999 The University of Utah and
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
#include <mom/c/libcast.h>
#include <mom/c/pbe.hh>

/*
 * This method handles pointer indirections that are part of a larger array
 * presentation.  A `pres_c_mapping_internal_array' is associated with the data
 * buffer that is part of an array processed via an inline allocation context.
 *
 * The contextual information required to process the array data must already
 * be stored in the `mu_state' object before this method is reached.  See the
 * `mu_state::mu_inline_*_array' methods.
 *
 * Like the pointers managed by `pres_c_mapping_pointer', the pointers managed
 * by `pres_c_mapping_internal_array' are presentation effects only.  That is,
 * the pointer is required by the presentation style, but the particular value
 * of the pointer conveys no information from the message.  In terms of MINT,
 * this means that there is no MINT representation of the pointer.  The MINT
 * passed into this method (`elem_itype') is therefore the MINT describing
 * the array element type, not the pointer type.  So, we don't ``descend'' the
 * MINT tree here.
 *
 * This method calls `mu_array' to handle the real dirty work of storage
 * allocation/deallocation, and the marshaling/unmarshaling of the data.
 */
void mu_state::mu_mapping_internal_array(
	cast_expr cexpr,
	cast_type ctype,
	mint_ref itype,
	pres_c_mapping_internal_array *amap)
{
	/* Get the array length. */
	assert(itype >= 0);
	assert(itype < (signed int) pres->mint.defs.defs_len);
	mint_def *idef = &(pres->mint.defs.defs_val[itype]);
	assert((idef->kind == MINT_ARRAY) || (idef->kind == MINT_VOID));
	
	mint_ref elem_itype = itype;  // MINT_VOID, otherwise it's set below.
	cast_type elem_ctype;

	/*
	 * Here we set up the `array_data' slot to communicate MINT array
	 * bounds information down to mu_array, mu_array_elem, etc.
	 */
	mu_array_data old_array_data = array_data;
	if (idef->kind == MINT_ARRAY) {
		mint_get_array_len(&pres->mint, itype,
				   &array_data.mint_len_min,
				   &array_data.mint_len_max);
		array_data.is_valid = 1;
		elem_itype = idef->mint_def_u.array_def.element_type;
	} else {
		/* We set the min & max just in case. */
		array_data.mint_len_min = 0;
		array_data.mint_len_max = 0;
		array_data.is_valid = 0;
	}
	
	/*
	 * If the CAST type of our ``internal array'' is a named type, locate
	 * the actual type.  (Note: The MIG FE/PG makes these kinds of direct
	 * associations between named C types and MAPPING_INTERNAL_ARRAY nodes.
	 * Other PG's generally indirect through a MAPPING_STUB, and the named
	 * type is derefenced to a pointer type there.)
	 */
	ctype = cast_find_typedef_type(&(pres->cast), ctype);
	if (!ctype)
		panic("In `mu_state::mu_mapping_internal_array', "
		      "can't locate `typedef' for a named type.");
	
	/*
	 * Strip off CAST_TYPE_QUALIFIED nodes from the array (pointer) type,
	 * so we can find the array's element type.
	 */
	cast_type uq_ctype = ctype;
	while(uq_ctype->kind == CAST_TYPE_QUALIFIED)
		uq_ctype = uq_ctype->cast_type_u_u.qualified.actual;
	
	switch (uq_ctype->kind) {
	case CAST_TYPE_POINTER:
		elem_ctype = uq_ctype->cast_type_u_u.pointer_type.target;
		break;
	case CAST_TYPE_ARRAY:
		elem_ctype = uq_ctype->cast_type_u_u.array_type.element_type;
		break;
	default:
		panic("In `mu_state::mu_mapping_internal_array', "
		      "CAST type is neither a pointer nor an array.");
		break;
	}
	
	mu_array(cexpr, ctype, elem_ctype, elem_itype, amap->element_mapping,
		 amap->arglist_name);
	
	/* Restore the `array_data' slot. */
	array_data = old_array_data;
}

/* End of file. */

