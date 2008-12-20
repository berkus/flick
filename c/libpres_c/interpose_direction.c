/*
 * Copyright (c) 1997 The University of Utah and
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
#include <mom/c/libpres_c.h>

void pres_c_interpose_direction(pres_c_mapping *inout_mapping,
				aoi_direction aoi_dir)
{
	pres_c_mapping old_mapping;
	pres_c_mapping new_mapping;

	assert(inout_mapping);
	assert(*inout_mapping);
	
	/*
	 * Wrap the `inout_mapping' inside a `pres_c_mapping_direction' node
	 * corresponding to the given AOI direction.  This tells the back end
	 * what kind of parameter it is processing (`in', `out', etc.)
	 */
	old_mapping = *inout_mapping;
	
	new_mapping = pres_c_new_mapping(PRES_C_MAPPING_DIRECTION);
	
	new_mapping->pres_c_mapping_u_u.direction.dir =
		((aoi_dir == AOI_DIR_IN)    ? PRES_C_DIRECTION_IN :
		 (aoi_dir == AOI_DIR_INOUT) ? PRES_C_DIRECTION_INOUT :
		 (aoi_dir == AOI_DIR_OUT)   ? PRES_C_DIRECTION_OUT :
		 (aoi_dir == AOI_DIR_RET)   ? PRES_C_DIRECTION_RETURN :
		 PRES_C_DIRECTION_UNKNOWN);
	
	new_mapping->pres_c_mapping_u_u.direction.mapping = old_mapping;
	
	*inout_mapping = new_mapping;
}

/* End of file. */

