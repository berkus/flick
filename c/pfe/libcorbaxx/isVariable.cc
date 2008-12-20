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

#include <mom/libaoi.h>

#include <mom/c/pg_corbaxx.hh>

/*
 * `isVariable' returns 1 if the given AOI type is variable-length, as defined
 * by Section 14.8 of the CORBA 2.0 specification.  If the AOI type is not
 * variable-length, `isVariable' returns 0.
 */
int pg_corbaxx::isVariable(aoi_type t)
{
	unsigned int i;
	
	switch (t->kind) {
	case AOI_ANY:		/* ``Untyped'' any values, not in CORBA. */
	case AOI_TYPE_TAG:	/* `CORBA::TypeCode' values. */
	case AOI_TYPED:		/* Type-tagged `any' values. */
	case AOI_INTERFACE:
	case AOI_FWD_INTRFC:
		return 1;
		break;
		
	case AOI_ARRAY: {
		unsigned min, max;
		aoi_array *aa = &(t->aoi_type_u_u.array_def);
		
		if (aa->flgs & AOI_ARRAY_FLAG_NULL_TERMINATED_STRING)
			/* IT's a bounded or unbounded string. */
			return 1;
		aoi_get_array_len(in_aoi, aa, &min, &max);
		if (min == max)
			/* It's an array. */
			return isVariable(aa->element_type);
		else
			/* It's a sequence. */
			return 1;
		break;
	}
	
	case AOI_STRUCT:
		for (i = 0;
		     i < t->aoi_type_u_u.struct_def.slots.slots_len;
		     ++i)
			if (isVariable(t->aoi_type_u_u.struct_def.slots.
				       slots_val[i].type))
				return 1;
		break;
		
	case AOI_UNION:
		for (i = 0;
		     i < t->aoi_type_u_u.union_def.cases.cases_len;
		     ++i)
			if (isVariable(t->aoi_type_u_u.union_def.cases.
				       cases_val[i].var.type))
				return 1;
		if (t->aoi_type_u_u.union_def.dfault)
			if (isVariable(t->aoi_type_u_u.union_def.dfault->type))
				return 1;
		break;
		
	case AOI_INDIRECT:
		return isVariable(in_aoi->
				  defs.defs_val[t->aoi_type_u_u.indirect_ref].
				  binding);
		break;
		
	default:
		break;
	}
	
	return 0;
}

/* End of file. */

