/*
 * Copyright (c) 1997, 1999 The University of Utah and
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
#include <mom/compiler.h>

void pres_c_interpose_argument(pres_c_mapping *inout_mapping,
			       const char *arglist,
			       const char *name)
{
	pres_c_mapping new_mapping;

	assert(inout_mapping);
	/* This could be a terminal node, ie no current mapping */
	/* assert(*inout_mapping); */
	
	/*
	 * Wrap the `inout_mapping' inside a `pres_c_mapping_argument' node.
	 * This helps pass information between nodes in the pres_c tree,
	 * particularly useful for encoding/decoding an array with a
	 * corresponding length.
	 */

	new_mapping = pres_c_new_mapping(PRES_C_MAPPING_ARGUMENT);
	
	new_mapping->pres_c_mapping_u_u.argument.arglist_name
		= flick_asprintf("%s", arglist);
	new_mapping->pres_c_mapping_u_u.argument.arg_name
		= flick_asprintf("%s", name);

	new_mapping->pres_c_mapping_u_u.argument.map = *inout_mapping;
	
	*inout_mapping = new_mapping;
}

/* End of file. */

