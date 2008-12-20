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
#include <mom/c/libpres_c.h>

void pres_c_interpose_param_root(
	pres_c_mapping *inout_mapping,
	cast_type ctype,
	cast_init init)
{
	pres_c_mapping old_mapping;
	pres_c_mapping new_mapping;
	
	assert(inout_mapping);
	assert(*inout_mapping);
	
	/*
	 * Wrap the `inout_mapping' inside a `pres_c_mapping_param_root' node.
	 */
	old_mapping = *inout_mapping;
	
	new_mapping = pres_c_new_mapping(PRES_C_MAPPING_PARAM_ROOT);
	
	new_mapping->pres_c_mapping_u_u.param_root.ctype = ctype;
	new_mapping->pres_c_mapping_u_u.param_root.init  = init;
	new_mapping->pres_c_mapping_u_u.param_root.map   = old_mapping;
	
	*inout_mapping = new_mapping;
}

/* End of file. */

