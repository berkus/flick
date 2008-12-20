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
#include <mom/c/pbe.hh>

/*
 * This is the general dispatch function for "mapping-flavor" presentation
 * nodes, which map one interface (network) data type to one presentation (C)
 * data type.
 *
 * `mu_mapping' simply switches on the mapping kind and dispatches to the
 * appropriate handler method.
 */
void mu_state::mu_mapping(cast_expr expr, cast_type ctype, mint_ref itype,
			  pres_c_mapping map)
{
	assert(map);
	switch (map->kind) {
	case PRES_C_MAPPING_DIRECT:
		mu_mapping_direct(expr, ctype, itype);
		break;

	case PRES_C_MAPPING_IGNORE:
#if 0
		/*
		 * We should assert that we have a MINT VOID here, but due
		 * to some sloppiness in the code generators, we occasionally
		 * end up with some real MINT.
		 * NOTE: pres_c_check() already asserts this, but there are
		 * some small hidden corners pres_c_check() never sees.
		 */
		assert(itype >= 0);
		assert((unsigned)itype < pres->mint.defs.defs_len);
		assert(pres->mint.defs.defs_val[itype].kind == MINT_VOID);
#endif
		break;
		
	case PRES_C_MAPPING_ELSEWHERE:
		/* The mapping for the CAST & MINT is handled elsewhere;
		   we do not worry about it here. */
		break;
		
	case PRES_C_MAPPING_ILLEGAL:
		mu_mapping_illegal(expr);
		break;
		
	case PRES_C_MAPPING_STUB:
		mu_mapping_stub(expr, ctype, itype, map);
		break;
		
	case PRES_C_MAPPING_POINTER:
		mu_mapping_pointer(expr, ctype, itype,
				   &(map->pres_c_mapping_u_u.pointer));
		break;
		
	case PRES_C_MAPPING_STRUCT:
		mu_mapping_struct(expr, ctype, itype,
				  map->pres_c_mapping_u_u.struct_i);
		break;
		
	case PRES_C_MAPPING_XLATE:
		mu_mapping_xlate(expr, ctype, itype,
				 &(map->pres_c_mapping_u_u.xlate));
		break;
		
	case PRES_C_MAPPING_REFERENCE:
		mu_mapping_reference(expr, ctype, itype,
				     &(map->pres_c_mapping_u_u.ref));
		break;
		
	case PRES_C_MAPPING_TYPE_TAG:
		mu_mapping_type_tag(expr, ctype, itype);
		break;
		
	case PRES_C_MAPPING_TYPED:
		mu_mapping_typed(expr, ctype, itype);
		break;
		
	case PRES_C_MAPPING_INTERNAL_ARRAY:
		mu_mapping_internal_array(expr, ctype, itype,
					  &(map->pres_c_mapping_u_u.
					    internal_array));
		break;
		
	case PRES_C_MAPPING_OPTIONAL_POINTER:
		mu_mapping_optional_pointer(expr, ctype, itype,
					    &(map->pres_c_mapping_u_u.
					      optional_pointer));
		break;
		
	case PRES_C_MAPPING_SYSTEM_EXCEPTION:
		mu_mapping_system_exception(expr, ctype, itype);
		break;
		
	case PRES_C_MAPPING_DIRECTION:
		mu_mapping_direction(expr, ctype, itype,
				     &(map->pres_c_mapping_u_u.direction));
		break;
		
	case PRES_C_MAPPING_SID:
		mu_mapping_sid(expr, ctype, itype,
			       &(map->pres_c_mapping_u_u.sid));
		break;
		
	case PRES_C_MAPPING_ARGUMENT:
		mu_mapping_argument(expr, ctype, itype, map);
		break;
		
	case PRES_C_MAPPING_MESSAGE_ATTRIBUTE:
		mu_mapping_message_attribute(expr, ctype, itype,
					     &(map->pres_c_mapping_u_u.
					       message_attribute));
		break;
		
	case PRES_C_MAPPING_INITIALIZE:
		mu_mapping_initialize(expr, ctype, itype, map);
		break;
		
	case PRES_C_MAPPING_VAR_REFERENCE:
		mu_mapping_var_reference(expr, ctype, itype,
					 &(map->pres_c_mapping_u_u.var_ref));
		break;
		
	case PRES_C_MAPPING_PARAM_ROOT:
		mu_mapping_param_root(expr, ctype, itype,
				      &(map->pres_c_mapping_u_u.param_root));
		break;
		
	case PRES_C_MAPPING_SELECTOR:
		mu_mapping_selector(expr, ctype, itype,
				    &(map->pres_c_mapping_u_u.selector));
		break;
		
	case PRES_C_MAPPING_TEMPORARY:
		mu_mapping_temporary(expr, ctype, itype,
				     &(map->pres_c_mapping_u_u.temp));
		break;
		
	case PRES_C_MAPPING_SINGLETON:
		mu_mapping_singleton(expr, ctype, itype,
				     &(map->pres_c_mapping_u_u.singleton));
		break;
		
	default:
		panic("In `mu_state::mu_mapping', unknown mapping kind %d.",
		      map->kind);
		break;
	}
}

/* End of file. */

