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

#include <mom/libaoi.h>
#include <mom/c/libcast.h>

#include "private.hh"

/*
 * `p_make_ctypename' is called by `p_typedef_def' and `p_indirect_type' in
 * order to create a CAST type expression that refers to a named (typedef'ed)
 * type.
 */
cast_type pg_state::p_make_ctypename(aoi_ref ref)
{
	aoi_type at;
	cast_type ctype_name;
	
	/*****/
	
	assert((ref >= 0) && (ref <= ((aoi_ref) in_aoi->defs.defs_len)));
	assert(a(ref).binding);
	
	at = a(ref).binding;
	
	/*
	 * Don't be confused about the fact that we're making struct types,
	 * union types, and enum types here, even though a typedef'ed name
	 * would normally appear ``unqualified.''
	 *
	 * The assumption here is that if we're dealing with a struct, union,
	 * or enum type, the presentation generator is going to produce code
	 * that looks like: `typedef struct foo { ... } foo;'.  In particular,
	 * the type can be referred to either with or without the initial type
	 * qualifier.
	 * 
	 * Why generate the ``qualified'' versions here?  In AOI, if a client
	 * or server stub mentions a compound type (e.g., a struct), the AOI
	 * for that stub has an indirect (AOI_INDIRECT) to the definition of
	 * that type.  By returning ``qualified'' names here, we make it easier
	 * for Flick to generate code that has forward type references (e.g.,
	 * pointers to structs an in `typedef struct foo *bar;').
	 * 
	 * The above discussion notwithstanding, this method may be overridden
	 * by presentation generators in which the above-stated assumption
	 * (that `{struct,union,enum} foo' == `foo') does not hold for one or
	 * more of these type classes.
	 */
	switch (at->kind) {
	default:
		ctype_name = cast_new_type(CAST_TYPE_NAME);
		ctype_name->cast_type_u_u.name =
			calc_scoped_name_from_ref(ref);
		break;
		
	case AOI_UNION:
		/*
		 * An AOI_UNION is a *discriminated* union, which maps to a
		 * C structure containing the discriminator and a C union.
		 */
		/* FALLTHROUGH */
	case AOI_STRUCT:
		ctype_name = cast_new_type(CAST_TYPE_STRUCT_NAME);
		ctype_name->cast_type_u_u.struct_name =
			calc_scoped_name_from_ref(ref);
		break;
		
	case AOI_ENUM:
		ctype_name = cast_new_type(CAST_TYPE_ENUM_NAME);
		ctype_name->cast_type_u_u.enum_name =
			calc_scoped_name_from_ref(ref);
		break;
	}
	
	return ctype_name;
}

/* End of file. */

