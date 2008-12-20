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
#include <mom/libaoi.h>

aoi_type aoi_indir_1(aoi *a, aoi_type t)
{
	assert(a);
	assert(t);
	
	if (t->kind == AOI_INDIRECT) {
		assert(t->aoi_type_u_u.indirect_ref >= 0);
		assert(t->aoi_type_u_u.indirect_ref <
		       (signed int) a->defs.defs_len);
		
		t = a->defs.defs_val[t->aoi_type_u_u.indirect_ref].binding;
		assert(t);
	}
	
	return t;
}

/* End of file. */

