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
#include <mom/compiler.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

/*
 * This function is almost identical to `pres_c_interpose_pointer'.  If you
 * change something here, you'll almost certainly need to change it there as
 * well.
 */
void pres_c_interpose_internal_array(cast_type *inout_ctype,
				     pres_c_mapping *inout_mapping,
				     const char *arglist_name)
{
	pres_c_mapping old_mapping, new_mapping;
	
	assert(inout_mapping); assert(*inout_mapping);
	assert(arglist_name);
	
	/*
	 * Interpose a level of pointer indirection before the actual C type,
	 * if the `inout_ctype' was given to us.
	 */
	if (inout_ctype) {
		cast_type old_ctype, new_ctype;
		
		assert(*inout_ctype);
		old_ctype = *inout_ctype;
		new_ctype = cast_new_pointer_type(old_ctype);
		*inout_ctype = new_ctype;
	}
	
	/* Interpose on the mapping correspondingly. */
	old_mapping = *inout_mapping;
	new_mapping = pres_c_new_mapping(PRES_C_MAPPING_INTERNAL_ARRAY);
	new_mapping->pres_c_mapping_u_u.internal_array.element_mapping
		= old_mapping;
	new_mapping->pres_c_mapping_u_u.internal_array.arglist_name
		= flick_asprintf("%s", arglist_name);
	*inout_mapping = new_mapping;
}

/* End of file. */

