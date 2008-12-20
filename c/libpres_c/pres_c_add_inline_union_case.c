/*
 * Copyright (c) 1998 The University of Utah and
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

#include <memory.h>
#include <mom/compiler.h>
#include <mom/c/libpres_c.h>

int pres_c_add_inline_union_case(pres_c_inline inl)
{
	int i;
	
	/*
	 * Because we use anonymous sequence types in the PRES_C structure
	 * definitions, we must handle each kind of inline_union separately.
	 *
	 * The alternative would be for `pres_c.x' to include a typedef for the
	 * case sequence type.  We could then remove redundant code here, but
	 * the price would be less readable code everywhere else in Flick,
	 * because `rpcgen' would use the typedef name to create the names of
	 * the `_len' and `_val' sequence slots!
	 */
	switch (inl->kind) {
	default:
		panic("In `pres_c_add_inline_union_case', unexpected "
		      "`pres_c_inline' kind.");
		break;
		
	case PRES_C_INLINE_STRUCT_UNION: {
		pres_c_inline_struct_union *u_inl
			= &(inl->pres_c_inline_u_u.struct_union);
		
		i = u_inl->cases.cases_len++;
		
		u_inl->cases.cases_val
			= mustrealloc(u_inl->cases.cases_val,
				      (u_inl->cases.cases_len
				       * sizeof(*(u_inl->cases.cases_val))
				       ));
		
		memset(&(u_inl->cases.cases_val[i]),
		       0,
		       sizeof(u_inl->cases.cases_val[i]));
		
		break;
	}
	case PRES_C_INLINE_VOID_UNION: {
		pres_c_inline_void_union *u_inl
			= &(inl->pres_c_inline_u_u.void_union);
		
		i = u_inl->cases.cases_len++;
		
		u_inl->cases.cases_val
			= mustrealloc(u_inl->cases.cases_val,
				      (u_inl->cases.cases_len
				       * sizeof(*(u_inl->cases.cases_val))
				       ));
		
		memset(&(u_inl->cases.cases_val[i]),
		       0,
		       sizeof(u_inl->cases.cases_val[i]));
		
		break;
	}
	case PRES_C_INLINE_EXPANDED_UNION: {
		pres_c_inline_expanded_union *u_inl
			= &(inl->pres_c_inline_u_u.expanded_union);
		
		i = u_inl->cases.cases_len++;
		
		u_inl->cases.cases_val
			= mustrealloc(u_inl->cases.cases_val,
				      (u_inl->cases.cases_len
				       * sizeof(*(u_inl->cases.cases_val))
				       ));
		
		memset(&(u_inl->cases.cases_val[i]),
		       0,
		       sizeof(u_inl->cases.cases_val[i]));
		
		break;
	}
	case PRES_C_INLINE_VIRTUAL_UNION: {
		pres_c_inline_virtual_union *u_inl
			= &(inl->pres_c_inline_u_u.virtual_union);
		
		i = u_inl->cases.cases_len++;
		
		u_inl->cases.cases_val
			= mustrealloc(u_inl->cases.cases_val,
				      (u_inl->cases.cases_len
				       * sizeof(*(u_inl->cases.cases_val))
				       ));
		
		memset(&(u_inl->cases.cases_val[i]),
		       0,
		       sizeof(u_inl->cases.cases_val[i]));
		
		break;
	}
	
	}
	
	return i;
}

/* End of file. */

