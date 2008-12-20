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
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include "pg_fluke.hh"

void pg_fluke::p_param_type(aoi_type at, int mr,
			    aoi_direction dir,
			    cast_type *out_ctype, pres_c_mapping *out_mapping)
{
	cast_type target_ctype;
	cast_type const_ctype;
	
	/*
	 * Do the normal CORBA thing, except declare `in' parameters passed
	 * by reference to be `const'.
	 */
	
	pg_corba::p_param_type(at, mr, dir, out_ctype, out_mapping);
	
	if (dir == AOI_DIR_IN) {
		/*
		 * If this `in' parameter is being passed by reference,
		 * declare the target type to be `const'.
		 */
		if ((*out_ctype)->kind == CAST_TYPE_POINTER) {
			target_ctype = (*out_ctype)->
				       cast_type_u_u.pointer_type.target;
			
			const_ctype = cast_new_type(CAST_TYPE_QUALIFIED);
			const_ctype->cast_type_u_u.qualified.qual =
				CAST_TQ_CONST;
			const_ctype->cast_type_u_u.qualified.actual =
				target_ctype;
			
			*out_ctype = cast_new_pointer_type(const_ctype);

		} else {
			/*
			 * If this `in' parameter is an object reference,
			 * `const'ify it.
			 */
			aoi_type param_type = at;
			
			/* Dig through any indirections... */
			while (param_type->kind == AOI_INDIRECT) {
				assert(param_type->aoi_type_u_u.indirect_ref
				       >= 0);
				assert(param_type->aoi_type_u_u.indirect_ref
				       < ((aoi_ref) in_aoi->defs.defs_len));
				
				param_type = a(param_type->
					       aoi_type_u_u.indirect_ref).
					     binding;
				
				assert(param_type);
			}
			
			/* Did we end up at an interface (i.e., an object)? */
			if ((param_type->kind == AOI_INTERFACE)
			    || (param_type->kind == AOI_FWD_INTRFC)) {
				const_ctype =
					cast_new_type(CAST_TYPE_QUALIFIED);
				
				const_ctype->cast_type_u_u.qualified.qual =
					CAST_TQ_CONST;
				const_ctype->cast_type_u_u.qualified.actual =
					(*out_ctype);
				
				*out_ctype = const_ctype;
			}
		}
	}
}

/* End of file. */

