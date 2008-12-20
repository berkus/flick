/*
 * Copyright (c) 1995, 1996, 1997, 1998, 1999 The University of Utah and
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
#include <mom/libaoi.h>
#include <mom/c/pfe.hh>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include "pg_sun.hh"

/*
 * This method determines how `p_param' processes a parameter type.
 */
void pg_sun::p_param_type(aoi_type at, mint_ref /* mr */, aoi_direction dir,
			  cast_type *out_ctype, pres_c_mapping *out_mapping)
{
	/*
	 * `real_at' is the ``concrete'' AOI type referenced by `at'.  If `at'
	 * is an `AOI_INDIRECT', `real_at' is the type referenced through the
	 * indirection.
	 */
	aoi_type real_at;
	
	/*
	 * `is_string' and `is_array' are flags indicating that the AOI type at
	 * hand is being presented as an C string or array, respectively.
	 */
	int is_string, is_array;
	
	/*********************************************************************/
	
	/*
	 * The library verion of this method checks for `inout' and `out'
	 * parameters at this point.  However, Sun RPC doesn't allow for these
	 * parameter types.
	 */
	assert(dir == AOI_DIR_IN);
	
	/*
	 * Get the basic presentation for this type: the `out_ctype' and the
	 * `out_mapping'.
	 */
	p_type_collection *ptc = 0;
	p_type_node *ptn;
	
	p_type(at, &ptc);
	ptn = ptc->find_type("definition");
	*out_ctype = ptn->get_type();
	*out_mapping = ptn->get_mapping();
	
	/*
	 * Get the actual AOI type (not an AOI_INDIRECT type).  If the actual
	 * AOI type is an AOI_ARRAY, determine if it corresponds to an ONC RPC
	 * string or fixed-length array.
	 */
	real_at = at;
	while (real_at->kind == AOI_INDIRECT)
		real_at = in_aoi->defs.
			defs_val[real_at->aoi_type_u_u.indirect_ref].binding;
	
	if (real_at->kind == AOI_ARRAY) {
		unsigned int min, max;
		
		aoi_get_array_len(in_aoi, &(real_at->aoi_type_u_u.array_def),
				  &min, &max);
		
		is_string   = (real_at->aoi_type_u_u.array_def.flgs
			       == AOI_ARRAY_FLAG_NULL_TERMINATED_STRING);
		is_array    = ((min == max) && !is_string);
	} else {
		is_string = 0;
		is_array = 0;
	}
	
	/*
	 * Now determine the C type of this parameter.
	 */
	if (is_array)
		/*
		 * Do nothing.  IDL arrays are presented as C arrays, which are
		 * automatically passed by reference.
		 */
		;
	else {
		/*
		 * Otherwise, interpose an indirection pointer so that the
		 * parameter will be passed by reference:
		 *   Client side never allocs nor deallocs.
		 *   Server side always allocs and deallocs.
		 *
		 * I think it is surprising that `rpcgen' doesn't omit this
		 * ``extra'' pointer before string types (which are presented
		 * as `char *'s) and optional types (which are presented as
		 * pointers).
		 */
		pres_c_allocation alloc;
		alloc.cases[PRES_C_DIRECTION_IN].allow
			= PRES_C_ALLOCATION_ALLOW;
		/* Let the back end choose the allocator. */
		alloc.cases[PRES_C_DIRECTION_IN].pres_c_allocation_u_u.val.
			allocator.kind = PRES_C_ALLOCATOR_DONTCARE;
		alloc.cases[PRES_C_DIRECTION_IN].pres_c_allocation_u_u.val.
			alloc_init = 0;
		
		if (gen_client) {
			alloc.cases[PRES_C_DIRECTION_IN].
				pres_c_allocation_u_u.val.flags
				= PRES_C_ALLOC_NEVER | PRES_C_DEALLOC_NEVER;
		} else if (gen_server) {
			alloc.cases[PRES_C_DIRECTION_IN].
				pres_c_allocation_u_u.val.flags
				= PRES_C_ALLOC_ALWAYS | PRES_C_DEALLOC_ALWAYS;
		} else
			panic("In `pg_sun::p_param_type', "
			      "generating neither client nor server.");
		
		/* All other allocation cases are invalid. */
		alloc.cases[PRES_C_DIRECTION_UNKNOWN].allow
			= PRES_C_ALLOCATION_INVALID;
		alloc.cases[PRES_C_DIRECTION_INOUT].allow
			= PRES_C_ALLOCATION_INVALID;
		alloc.cases[PRES_C_DIRECTION_OUT].allow
			= PRES_C_ALLOCATION_INVALID;
		alloc.cases[PRES_C_DIRECTION_RETURN].allow
			= PRES_C_ALLOCATION_INVALID;
		
		pres_c_interpose_indirection_pointer(out_ctype, out_mapping,
						     alloc);
	}
	
	/*
	 * Tell the back end that this is the ``root'' of the parameter.
	 */
	pres_c_interpose_param_root(out_mapping, 0, 0);
	
	/*
	 * Finally, wrap the mapping in a `hint' that tells the back end what
	 * kind of parameter this is: `in', `out', etc.
	 */
	pres_c_interpose_direction(out_mapping, dir);
}

/* End of file. */

