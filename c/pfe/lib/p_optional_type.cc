/*
 * Copyright (c) 1995, 1996, 1998, 1999 The University of Utah and
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
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include "private.hh"

void pg_state::p_optional_type(aoi_optional *ao,
			       p_type_collection **out_ptc)
{
	p_type_collection *ptc, *to_ptc = 0;
	p_type_node *ptn;
	cast_type to_ctype, ctype;
	pres_c_mapping to_map;
	
	cast_type lctype = mint_to_ctype(
		&out_pres->mint,
		out_pres->mint.standard_refs.unsigned32_ref);
	
	/* Create the ctype and mapping for the optional target. */
	p_type(ao->type, &to_ptc);
	
	ptn = to_ptc->find_type("definition");
	to_ctype = ptn->get_type();
	to_map = ptn->get_mapping();
	
	/* Create the pointer ctype. */
	ctype = cast_new_pointer_type(to_ctype);
	
	ptc = p_new_type_collection("optional pointer");
	ptn = new p_type_node;
	ptn->set_name("definition");
	ptn->set_format("%s");
	ptn->set_type(ctype);
	
	/*
	 * Create the mapping.  An optional maps as an optional pointer, which
	 * corresponds to a MINT array (a counted array with zero or one
	 * elements).  See `aoi_to_mint()' for more information.  We do this in
	 * three steps:
	 *   1. Create a temporary mapping, so we can create temporary
	 *      variables for values needed by the allocation context.
	 *   2. Create an allocation context node to describe important
	 *      aspects of the array and its associated allocation.
	 *   3. Create the optional pointer mapping.
	 */
	pres_c_mapping singleton;
	pres_c_inline alloc_inl;
	pres_c_mapping optr_map;
	
	singleton = pres_c_new_mapping(PRES_C_MAPPING_SINGLETON);
	
	singleton->pres_c_mapping_u_u.singleton.inl
		= alloc_inl
		= pres_c_new_inline(PRES_C_INLINE_ALLOCATION_CONTEXT);
	pres_c_inline_allocation_context *ac
		= &alloc_inl->pres_c_inline_u_u.acontext;
	
	ac->arglist_name = pres_c_make_arglist_name("optptr");
	
	ac->length
		= PRES_C_I_ATOM,
		    PIA_Index, 0,
		    PIA_Mapping, PRES_C_M_TEMPORARY,
		      PMA_Name, "array_len",
		      PMA_CType, lctype,
		      PMA_PreHandler, "ptr_not_nil",
		      PMA_TempType, TEMP_TYPE_PRESENTED,
		      PMA_Target, PRES_C_M_ARGUMENT,
		        PMA_ArgList, ac->arglist_name,
		        PMA_Name, "length",
		        PMA_Mapping, PRES_C_M_DIRECT, END_PRES_C,
		        END_PRES_C,
		      END_PRES_C,
		    END_PRES_C;
	
	ac->max_len
		= PRES_C_I_TEMPORARY,
		    PIA_Name, "array_max",
		    PIA_CType, lctype,
		    PIA_Value, cast_new_expr_lit_int(1, 0),
		    PIA_IsConst, 1,
		    PIA_Mapping, PRES_C_M_ARGUMENT,
		      PMA_ArgList, ac->arglist_name,
		      PMA_Name, "max_len",
		      PMA_Mapping, NULL,
		      END_PRES_C,
		    END_PRES_C;
	
	/*
	 * Although the pointer is ``optional'', if we allocate, we MUST
	 * allocate one.  If we don't have one, we don't allocate anyway.
	 * Thus, the minimum length for the allocation is fixed at 1.
	 */
	ac->min_alloc_len
		= PRES_C_I_TEMPORARY,
		    PIA_Name, "array_max",
		    PIA_CType, lctype,
		    PIA_Value, cast_new_expr_lit_int(1, 0),
		    PIA_IsConst, 1,
		    PIA_Mapping, PRES_C_M_ARGUMENT,
		      PMA_ArgList, ac->arglist_name,
		      PMA_Name, "min_alloc_len",
		      PMA_Mapping, NULL,
		      END_PRES_C,
		    END_PRES_C;
	
	/* Use the default allocation semantics. */
	ac->alloc = p_get_allocation();
	
	ac->ptr = pres_c_new_inline_atom(
		0,
		(optr_map = pres_c_new_mapping(
			PRES_C_MAPPING_OPTIONAL_POINTER)));
	optr_map->pres_c_mapping_u_u.optional_pointer.target = to_map;
	optr_map->pres_c_mapping_u_u.optional_pointer.arglist_name
		= ac->arglist_name;

	ptn->set_mapping(singleton);
	ptc->add_type("default", ptn);
	
	if (*out_ptc)
		(*out_ptc)->set_collection_ref(ptc);
	else
		*out_ptc = ptc;
}

/* End of file. */

