/*
 * Copyright (c) 1996, 1997, 1998, 1999 The University of Utah and
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
#include <mom/libaoi.h>

#include "pg_fluke.hh"

extern mint_1 out_mint;

void pg_fluke::preprocess()
{
	unsigned int a;
	
	pg_corba::preprocess();
	mint_custom_op_const = build_op_const;
	mint_custom_exception_discrim_ref
		= out_mint.standard_refs.unsigned32_ref;
	mint_custom_exception_const = build_exception_const_int;
	
	for (a = 0; a < in_aoi->defs.defs_len; a++) {
		if (in_aoi->defs.defs_val[a].binding->kind == AOI_INTERFACE) {
			aoi_interface *interface = &(in_aoi->defs.defs_val[a].
						     binding->aoi_type_u_u.
						     interface_def);
			
			interface->op_code_type =
				(aoi_type) mustcalloc(sizeof(aoi_type_u));
			interface->op_code_type->
				kind = AOI_INTEGER;
			interface->op_code_type->
				aoi_type_u_u.integer_def.min = 0;
			interface->op_code_type->
				aoi_type_u_u.integer_def.range = ~0;
		}
	}
}

/* End of file. */

