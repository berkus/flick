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

#include <mom/compiler.h>
#include <mom/c/pbe.hh>

/*
 * A `pres_c_mapping_direction' node contains information for the back end: is
 * the data below this node `in', `inout', `out', `return', or unknown?  The
 * back end sometimes needs this information in order to optimize allocation,
 * etc.  (E.g., `in' parameters can be allocated on the runtime stack.)
 *
 * Fundamentally, the need for `direction' nodes points to a shortcoming in
 * the presentation generator: the PG phase is driven almost exclusively by
 * types, not by how the data is being used.  The role of the data, however,
 * determines certain things such as allocation semantics.  If the presentation
 * generator was more careful to consider the roles of data when generating
 * PRES_C, it could generate PRES_C without resorting to `direction' nodes
 * which need to be interpreted by the back end.
 */
void mu_state::mu_mapping_direction(cast_expr expr,
				    cast_type ctype,
				    mint_ref itype,
				    pres_c_mapping_direction *dir_map)
{
	pres_c_direction old_dir;
	
	old_dir = current_param_dir;
	if ((old_dir != PRES_C_DIRECTION_UNKNOWN)
	    || (dir_map->dir == PRES_C_DIRECTION_UNKNOWN))
		warn("Changing from parameter direction %s to %s.",
		     pres_c_dir_name(old_dir),
		     pres_c_dir_name(dir_map->dir));
	
	current_param_dir = dir_map->dir;
	mu_mapping(expr, ctype, itype, dir_map->mapping);
	current_param_dir = old_dir;
}

/* End of file. */

