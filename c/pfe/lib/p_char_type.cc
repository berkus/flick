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

#include "private.hh"

void pg_state::p_char_type(aoi_char *ac,
			   p_type_collection **out_ptc)
{
	p_type_collection *ptc;
	
	assert(ac->bits == 8); /* XXX */
	
	if (ac->flags == AOI_CHAR_FLAG_NONE)
		ptc = prim_collections[PRIM_COLLECTION_CHAR];
	else if (ac->flags == AOI_CHAR_FLAG_SIGNED)
		ptc = prim_collections[PRIM_COLLECTION_SCHAR];
	else if (ac->flags == AOI_CHAR_FLAG_UNSIGNED)
		ptc = prim_collections[PRIM_COLLECTION_UCHAR];
	else
		panic("unrecognized aoi_char flags in pg_state::p_char_type");
	
	if (*out_ptc)
		(*out_ptc)->set_collection_ref(ptc);
	else
		*out_ptc = ptc;
}

/* End of file. */

