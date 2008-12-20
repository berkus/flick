/*
 * Copyright (c) 1997, 1998, 1999 The University of Utah and
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

#include <mom/c/pg_corba.hh>

/*
 * `p_make_ctypename' is called by `p_typedef_def' and `p_indirect_type' in
 * order to create a CAST type expression that refers to a named (typedef'ed)
 * type.
 */
cast_type pg_corba::p_make_ctypename(aoi_ref ref)
{
	aoi_type at;
	cast_type ctype_name;
	
	/*
	 * This method is identical to `pg_state::p_make_ctypename' (to which
	 * you should refer for more comments), *except* that `enums' are not
	 * qualified.  CORBA IDL enums are not presented as C enumerations; see
	 * `p_enum_type'. 
	 */
	
	/*****/
	
	assert((ref >= 0) && (ref <= ((aoi_ref) in_aoi->defs.defs_len)));
	assert(a(ref).binding);
	
	at = a(ref).binding;
	
	switch (at->kind) {
	default:
		ctype_name = pg_state::p_make_ctypename(ref);
		break;
		
	case AOI_ENUM:
		/* CORBA specific! */
		ctype_name = cast_new_type(CAST_TYPE_NAME);
		ctype_name->cast_type_u_u.name = calc_scoped_name_from_ref(ref);
		break;
	}
	
	return ctype_name;
}

/* End of file. */

