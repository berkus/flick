/*
 * Copyright (c) 1999 The University of Utah and
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
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

/*
 * Although similar in name to `pres_c_interpose_pointer', this function is
 * concerned with constructing a SPECIFIC TYPE of pointer, namely an
 * indirection pointer.  These types of pointers have simple and similar
 * allocation contexts, (barring the actual allocation semantics described in
 * a pres_c_allocation struct).  For this reason, we not only interpose the
 * pointer's CAST and mapping, but we also create the necessary allocation
 * context (with temporaries).  Unlike `pres_c_interpose_pointer', this
 * function also takes as input `alloc', the allocation semantics for the
 * indirection pointer.
 */
void pres_c_interpose_indirection_pointer(cast_type *inout_ctype,
					  pres_c_mapping *inout_mapping,
					  pres_c_allocation ptr_alloc)
{
	/*
	 * In order to interpose this pointer, we also need to interpose an
	 * allocation context inline and temporary mappings to hold imporant
	 * contextual information.
	 */
	pres_c_mapping singleton;
	pres_c_inline alloc_inl;
	cast_type u32 = cast_new_prim_type(CAST_PRIM_INT,
					   CAST_MOD_UNSIGNED);
	
	singleton = pres_c_new_mapping(PRES_C_MAPPING_SINGLETON);
	singleton->pres_c_mapping_u_u.singleton.inl
		= alloc_inl
		= pres_c_new_inline(PRES_C_INLINE_ALLOCATION_CONTEXT);
	pres_c_inline_allocation_context *ac
		= &alloc_inl->pres_c_inline_u_u.acontext;
	
	ac->arglist_name = pres_c_make_arglist_name("indir");
	ac->length
		= PRES_C_I_TEMPORARY,
		    PIA_Name, "array_len",
		    PIA_CType, u32,
		    PIA_Value, cast_new_expr_lit_int(1, 0),
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
		    PIA_CType, u32,
		    PIA_Value, cast_new_expr_lit_int(1, 0),
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
		    PIA_CType, u32,
		    PIA_Value, cast_new_expr_lit_int(1, 0),
		    PIA_IsConst, 1,
		    PIA_Mapping, PRES_C_M_ARGUMENT,
		      PMA_ArgList, ac->arglist_name,
		      PMA_Name, "max_len",
		      PMA_Mapping, NULL,
		      END_PRES_C,
		    END_PRES_C;
	
	ac->min_alloc_len = PRES_C_I_TEMPORARY,
			      PIA_Name, "array_len",
			      PIA_CType, u32, 
			      PIA_Value, cast_new_expr_lit_int(1, 0),
			      PIA_IsConst, 1,
			      PIA_Mapping, PRES_C_M_ARGUMENT,
			        PMA_ArgList, ac->arglist_name,
			        PMA_Name, "min_alloc_len",
			        PMA_Mapping, NULL,
			        END_PRES_C,
			      END_PRES_C;
	
	ac->alloc = ptr_alloc;
	
	pres_c_interpose_pointer(inout_ctype, inout_mapping, ac->arglist_name);
	ac->ptr = pres_c_new_inline_atom(0, *inout_mapping);
	*inout_mapping = singleton;
}

/* End of file. */

