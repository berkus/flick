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

#include <mom/compiler.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>
#include <mom/libaoi.h>

#include <mom/c/pfe.hh>

/*
 * There is no `pg_state' implementation of `p_variable_array_type' because the
 * mapping of variable-length arrays varies so widely between presentations, so
 * there is no way to provide a useful ``default'' version.
 */

/*****************************************************************************/

/*
 * We can, however, provide useful defaults for the `p_variable_array_type'
 * auxiliary methods.
 */

void pg_state::p_variable_array_length_type(aoi_array */*array*/,
					    p_type_collection **out_ptc,
					    pres_c_mapping *out_map,
					    char *arglist_name)
{
	p_type_node *ptn;
	
	/*
	 * Determine the CAST type and PRES_C mapping for the length of (i.e.,
	 * number of elements in) a variable-length array (e.g., a CORBA
	 * sequence).
	 */
	p_scalar(32, 0, out_ptc);
	ptn = (*out_ptc)->find_type("definition");
	*out_map = ptn->get_mapping();
	
	/*
	 * Identify this datum as a ``length'' for the benefit of back end
	 * code generation.
	 */
	pres_c_interpose_argument(out_map, arglist_name, "length");
}

void pg_state::p_variable_array_maximum_type(aoi_array */*array*/,
					     p_type_collection **out_ptc,
					     pres_c_mapping *out_map,
					     char *arglist_name)
{
	p_type_node *ptn;
	
	/*
	 * Determine the CAST type and PRES_C mapping for the allocated length
	 * (i.e., `malloc'ed number of slots) of a variable-length array (e.g.,
	 * a CORBA sequence).
	 */
	p_scalar(32, 0, out_ptc);
	ptn = (*out_ptc)->find_type("definition");
	*out_map = ptn->get_mapping();
	
	/*
	 * The `out_map' returned by `p_scalar' is correct if we were going to
	 * marshal/unmarshal the allocated length, but that's not what we want.
	 * Rather, we ``ignore'' the value when m/u'ing but identify it as the
	 * ``maximum'' so that the back end can do something special when it
	 * generates code.  (See `mu_state::mu_inline_allocation_context'.)
	 */
	*out_map = pres_c_new_mapping(PRES_C_MAPPING_IGNORE);
	pres_c_interpose_argument(out_map, arglist_name, "alloc_len");
}

void pg_state::p_variable_array_release_type(aoi_array */*array*/,
					     p_type_collection **out_ptc,
					     pres_c_mapping *out_map,
					     char *arglist_name)
{
	p_type_node *ptn;
	
	/*
	 * Determine the CAST type and PRES_C mapping for the release flag
	 * (i.e., whether the buffer can be freed or not) of a variable-length
	 * array (e.g., a CORBA sequence).
	 */
	p_scalar(1, 0, out_ptc);
	ptn = (*out_ptc)->find_type("definition");
	*out_map = ptn->get_mapping();
	
	/*
	 * The `out_map' returned by `p_scalar' is correct if we were going to
	 * marshal/unmarshal the release flag, but that's not what we want.
	 * Rather, we ``ignore'' the value when m/u'ing but identify it as the
	 * ``maximum'' so that the back end can do something special when it
	 * generates code.  (See `mu_state::mu_inline_allocation_context'.)
	 */
	*out_map = pres_c_new_mapping(PRES_C_MAPPING_IGNORE);
	pres_c_interpose_argument(out_map, arglist_name, "release");
}

/* End of file. */

