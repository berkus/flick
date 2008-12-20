/*
 * Copyright (c) 1995, 1996, 1997, 1998 The University of Utah and
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
#include <mom/c/libcast.h>
#include <mom/c/pbe.hh>

/*
 * The PRES_C_MAPPING_STRUCT node is the primary ``outlet'' from mapping-flavor
 * (one-itype-to-one-ctype) presentation nodes to inline-flavor (one-itype-to-
 * many-ctype) nodes.
 *
 * The current `ctype' must be a CAST_TYPE_STRUCT, CAST_TYPE_NAME, or CAST_
 * TYPE_STRUCT_NAME.  However, the current `itype' doesn't matter.
 *
 * This routine just builds an inline_state describing the elements of the C
 * struct and then descends into the inline tree to map the itype onto that
 *struct.
 */
void mu_state::mu_mapping_struct(cast_expr expr, cast_type ctype,
				 mint_ref itype, pres_c_inline inl)
{
	cast_aggregate_type *struct_type;
	
	/*
	 * Locate the `cast_struct_type' corresponding to `ctype'.  This will
	 * handle cases in which `ctype' is a named type, a named structure
	 * type, or a ``raw'' structure type.
	 */
	struct_type = cast_find_struct_type(&(pres->cast), ctype);
	if (!struct_type)
		struct_type = cast_find_class_type(&(pres->cast), ctype);
	if (!struct_type)
		panic("In `mu_state::mu_mapping_struct', "
		      "can't determine the structure type.");
	
	struct_inline_state ist(struct_type, expr);
	
	mu_inline(&ist, itype, inl);
}

/* End of file. */

