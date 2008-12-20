/*
 * Copyright (c) 1999 The University of Utah and
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

#include <mom/compiler.h>
#include <mom/libaoi.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include <mom/c/pg_corba.hh>

void pg_corba::p_typed_type(aoi_typed *at,
			    p_type_collection **out_ptc)
{
	/*
	 * Verify that `at' refers to the AOI representation of a CORBA
	 * IDL-defined `any' type: i.e., that the type tag is an `AOI_TYPE_TAG'
	 * and the typed value is an `AOI_ANY'.  We don't know how to present
	 * anything else.
	 */
	if (!(at->tag)
	    || (at->tag->kind != AOI_TYPE_TAG))
		panic("In `pg_corba::p_typed_type', "
		      "the `tag' field of the `AOI_TYPED' node is not an "
		      "`AOI_TYPE_TAG'.");
	if (!(at->type)
	    || (at->type->kind != AOI_ANY))
		panic("In `pg_corba::p_typed_type', "
		      "the `type' field of the `AOI_TYPED' node is not an "
		      "`AOI_ANY'.");
	
	/*
	 * Return the precomputed type collection from `prim_collections'.
	 */
	if (*out_ptc)
		(*out_ptc)->set_collection_ref(
			prim_collections[PRIM_COLLECTION_TYPED_ANY]
			);
	else
		*out_ptc = prim_collections[PRIM_COLLECTION_TYPED_ANY];
}

/* End of file. */

