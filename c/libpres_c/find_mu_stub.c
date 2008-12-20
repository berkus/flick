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
#include <mom/c/libpres_c.h>

int pres_c_find_mu_stub(pres_c_1 *pres,
			mint_ref itype, cast_type ctype, pres_c_mapping map,
			pres_c_stub_kind stub_kind)
{
	int i;

	assert(map->kind == PRES_C_MAPPING_STUB);

	/* XXX this currently assumes there's only one ctype for the itype.
	   It might be better for the pres_c_mapping_stub to refer directly
	   to the stubs to be used for marshaling/unmarshaling.  */
	for (i = 0; i < (signed int) pres->stubs.stubs_len; i++)
		if ((pres->stubs.stubs_val[i].kind == stub_kind)
		    && (pres->stubs.stubs_val[i].pres_c_stub_u.mstub.itype
			== itype))
			return i;
	return -1;
}

/* End of file. */

