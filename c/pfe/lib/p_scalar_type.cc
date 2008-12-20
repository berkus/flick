/*
 * Copyright (c) 1997, 1998, 1999 The University of Utah and
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

#include <mom/c/libpres_c.h>
#include <mom/libaoi.h>

#include "private.hh"

void pg_state::p_scalar_type(aoi_scalar *as,
			     p_type_collection **out_ptc)
{
	p_scalar(as->bits,
		 (as->flags != AOI_SCALAR_FLAG_UNSIGNED),
		 out_ptc);
}

void pg_state::p_scalar(int bits,
			int is_signed,
			p_type_collection **out_ptc)
{
	p_type_collection *ptc;
	
	/* XXX machine-dependent */
	switch (bits) {
	case 1:
		ptc = prim_collections[PRIM_COLLECTION_BOOLEAN];
		break;
	case 8:
		ptc = prim_collections[PRIM_COLLECTION_OCTET];
		break;
	case 16:
		if (is_signed)
			ptc = prim_collections[PRIM_COLLECTION_SHORT];
		else
			ptc = prim_collections[PRIM_COLLECTION_USHORT];
		break;
	case 32:
		if (is_signed)
			ptc = prim_collections[PRIM_COLLECTION_LONG];
		else
			ptc = prim_collections[PRIM_COLLECTION_ULONG];
		break;
	case 64:
		if (is_signed)
			ptc = prim_collections[PRIM_COLLECTION_LONGLONG];
		else
			ptc = prim_collections[PRIM_COLLECTION_ULONGLONG];
		break;
	default:
		panic("unknown number of int bits");
	}
	if (*out_ptc)
		(*out_ptc)->set_collection_ref(ptc);
	else
		*out_ptc = ptc;
}

/* End of file. */

