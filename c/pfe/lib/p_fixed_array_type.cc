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
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include "private.hh"

void pg_state::p_fixed_array_type(aoi_array *aa,
				  p_type_collection **out_ptc)
{
	/* Create the ctype and mapping for the array's target. */
	p_type_collection *ptc, *to_ptc = 0;
	p_type_node *ptn;
	cast_type to_ctype;
	pres_c_mapping to_map;
	
	cast_type lctype = mint_to_ctype(
		&out_pres->mint,
		out_pres->mint.standard_refs.unsigned32_ref);
	
	p_type(aa->element_type, &to_ptc);
	ptn = to_ptc->find_type("definition");
	to_ctype = ptn->get_type();
	to_map = ptn->get_mapping();
	
	/* Find the length of the array. */
	unsigned length, max;
	aoi_get_array_len(in_aoi, aa, &length, &max);
	assert(length == max);
	
	/* Create the array ctype. */
	cast_type ctype = cast_new_type(CAST_TYPE_ARRAY);
	ctype->cast_type_u_u.array_type.length = cast_new_expr_lit_int(length,
								       0);
	ctype->cast_type_u_u.array_type.element_type = to_ctype;
	
	ptc = p_new_type_collection(calc_name_from_ref(cur_aoi_idx));
	ptn = new p_type_node;
	ptn->set_name("definition");
	ptn->set_format("%s");
	ptn->set_type(ctype);
	
	/* Create the array mapping. */
	/*
	 * This must be done in three steps:
	 *   1. Create a singleton mapping, so we can return to inline mode
	 *      for handling the allocation context.
	 *   2. Create an allocation context node to describe important
	 *      aspects of the array and its associated allocation.
	 *   3. Create the array mapping.
	 */
	pres_c_mapping singleton;
	pres_c_inline alloc_inl;
	pres_c_mapping array_map;
	
	singleton = pres_c_new_mapping(PRES_C_MAPPING_SINGLETON);
	
	singleton->pres_c_mapping_u_u.singleton.inl
		= alloc_inl
		= pres_c_new_inline(PRES_C_INLINE_ALLOCATION_CONTEXT);
	pres_c_inline_allocation_context *ac
		= &alloc_inl->pres_c_inline_u_u.acontext;
	
	ac->arglist_name = pres_c_make_arglist_name("fixedarray");
	
	ac->length
		= PRES_C_I_TEMPORARY,
		    PIA_Name, "array_len",
		    PIA_CType, lctype,
		    PIA_Value, ctype->cast_type_u_u.array_type.length,
		    PIA_IsConst, 1,
		    PIA_TempType, TEMP_TYPE_ENCODED,
		    PIA_Mapping, PRES_C_M_ARGUMENT,
		      PMA_ArgList, ac->arglist_name,
		      PMA_Name, "length",
		      PMA_Mapping, NULL,
		      END_PRES_C,
		    END_PRES_C;
	
	ac->min_len
		= PRES_C_I_TEMPORARY,
		    PIA_Name, "array_len",
		    PIA_CType, lctype,
		    PIA_Value, ctype->cast_type_u_u.array_type.length,
		    PIA_IsConst, 1,
		    PIA_Mapping, PRES_C_M_ARGUMENT,
		      PMA_ArgList, ac->arglist_name,
		      PMA_Name, "min_len",
		      PMA_Mapping, NULL,
		      END_PRES_C,
		    END_PRES_C;
	
	ac->max_len
		= PRES_C_I_TEMPORARY,
		    PIA_Name, "array_len",
		    PIA_CType, lctype, 
		    PIA_Value, ctype->cast_type_u_u.array_type.length,
		    PIA_IsConst, 1,
		    PIA_Mapping, PRES_C_M_ARGUMENT,
		      PMA_ArgList, ac->arglist_name,
		      PMA_Name, "max_len",
		      PMA_Mapping, NULL,
		      END_PRES_C,
		    END_PRES_C;
	
	ac->min_alloc_len = PRES_C_I_TEMPORARY,
			      PIA_Name, "array_max",
			      PIA_CType, lctype, 
			      PIA_Value,
			        ctype->cast_type_u_u.array_type.length,
			      PIA_IsConst, 1,
			      PIA_Mapping, PRES_C_M_ARGUMENT,
			        PMA_ArgList, ac->arglist_name,
			        PMA_Name, "min_alloc_len",
			        PMA_Mapping, NULL,
			        END_PRES_C,
			      END_PRES_C;
	
	/*
	 * We never need to allocate storage when the corresponding C type is
	 * an array, but we *do* need to allocate storage when the C type is a
	 * pointer (pointer-to-array-element).  The allocation flags in
	 * `array_map' only apply when the C type is a pointer type.  See the
	 * function `mu_state::mu_mapping_fixed_array' in the back end.
	 *
	 * You might be confused because we just set our presented C type to be
	 * an array type.  But some later code (e.g., `pg_corba::p_param_type'
	 * or `mu_state::mu_server_func') may change the type to be a pointer
	 * type.
	 */
	
	/* Use the default semantics for unknown and return directions. */
	ac->alloc = p_get_allocation();
	
	/*
	 * Fixed arrays are entirely allocated by the caller (user or
	 * server dispatch), thus never allocated/deallocated by the
	 * client, but always allocated/deallocated by the server.
	 */
	if (gen_client) {
		
		ac->alloc.cases[PRES_C_DIRECTION_IN].
			pres_c_allocation_u_u.val.flags
			= PRES_C_ALLOC_NEVER | PRES_C_DEALLOC_NEVER;
		ac->alloc.cases[PRES_C_DIRECTION_INOUT].
			pres_c_allocation_u_u.val.flags
			= PRES_C_ALLOC_NEVER | PRES_C_DEALLOC_NEVER;
		ac->alloc.cases[PRES_C_DIRECTION_OUT].
			pres_c_allocation_u_u.val.flags
			= PRES_C_ALLOC_NEVER | PRES_C_DEALLOC_NEVER;
		
	} else if (gen_server) {
		
		ac->alloc.cases[PRES_C_DIRECTION_IN].
			pres_c_allocation_u_u.val.flags
			= PRES_C_ALLOC_ALWAYS | PRES_C_DEALLOC_ALWAYS;
		ac->alloc.cases[PRES_C_DIRECTION_INOUT].
			pres_c_allocation_u_u.val.flags
			= PRES_C_ALLOC_ALWAYS | PRES_C_DEALLOC_ALWAYS;
		ac->alloc.cases[PRES_C_DIRECTION_OUT].
			pres_c_allocation_u_u.val.flags
			= PRES_C_ALLOC_ALWAYS | PRES_C_DEALLOC_ALWAYS;
		
		/*
		 * Further, fixed arrays are not reallocated by the callee
		 * (client stub or server work function).  Thus, we don't
		 * have to use the default presentation-supplied allocator.
		 */
		ac->alloc.cases[PRES_C_DIRECTION_IN].
			pres_c_allocation_u_u.val.allocator.kind
			= PRES_C_ALLOCATOR_DONTCARE;
		ac->alloc.cases[PRES_C_DIRECTION_INOUT].
			pres_c_allocation_u_u.val.allocator.kind
			= PRES_C_ALLOCATOR_DONTCARE;
		ac->alloc.cases[PRES_C_DIRECTION_OUT].
			pres_c_allocation_u_u.val.allocator.kind
			= PRES_C_ALLOCATOR_DONTCARE;
		
	}
	
	ac->ptr = pres_c_new_inline_atom(
		0,
		(array_map
		 = pres_c_new_mapping(PRES_C_MAPPING_INTERNAL_ARRAY)));
	
	array_map->pres_c_mapping_u_u.internal_array.element_mapping = to_map;
	array_map->pres_c_mapping_u_u.internal_array.arglist_name
		= ac->arglist_name;
	
	ptn->set_mapping(singleton);
	ptc->add_type("default", ptn);
	if( *out_ptc )
		(*out_ptc)->set_collection_ref(ptc);
	else
		*out_ptc = ptc;
}

/* End of file. */

