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
#include <string.h>

#include <mom/compiler.h>
#include <mom/libaoi.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include "pg_sun.hh"

/* XXX The allocation and deallocation flags need to be reexamined ---
   I just guessed as to what they should be.
   I also need to know more about `pres_c_interpose_pointer()' and
   what it does with its fourth argument.
   */

/* This version of `p_variable_array_type()' overrides the one in `c/pfe/lib'.
   This version is specific to Sun RPC.
 */

void pg_sun::p_variable_array_type(aoi_array *array,
				   p_type_collection **out_ptc)
{
	cast_type ctype, element_ctype, array_ctype;
	pres_c_mapping map, element_map;
	p_type_collection *to_ptc, *ptc;
	p_type_node *ptn, *array_ptn;

	cast_type lctype = mint_to_ctype(
		&out_pres->mint,
		out_pres->mint.standard_refs.unsigned32_ref);
	
	ptc = p_new_type_collection(name);
	array_ptn = new p_type_node;
	array_ptn->set_flags(PTF_NAME_REF);
	array_ptn->set_name("definition");
	array_ptn->set_format("%s");
	
	/* Create the allocation context node.  */
	pres_c_inline alloc_inl
		= pres_c_new_inline(PRES_C_INLINE_ALLOCATION_CONTEXT);
	pres_c_inline_allocation_context *ac
		= &alloc_inl->pres_c_inline_u_u.acontext;
	
	ac->arglist_name = pres_c_make_arglist_name("vararray");
	
	/* The default allocation is sufficient. */
	ac->alloc = p_get_allocation();
	
	/*
	 * If there are bounds on the array, set the appropriate minimum and
	 * maximum length for the array.
	 */
	unsigned len_min, len_max;
	aoi_get_array_len(in_aoi, array, &len_min, &len_max);
	assert(len_min != len_max); /* We're doing a *VARIABLE* array! */
	
	/* Create the ctype and mapping for the array's element type. */
	to_ptc = 0;
	p_type(array->element_type, &to_ptc);
	ptn = to_ptc->find_type("definition");
	element_ctype = ptn->get_type();
	element_map = ptn->get_mapping();
	
	/*
	 * Interpose an ``internal array'' mapping on the basic element
	 * mapping and a CAST pointer on the basic element CAST type.
	 * This is what we need in order to allocate the array storage.
	 */
	array_ctype = element_ctype;
	pres_c_interpose_internal_array(&array_ctype, &element_map,
					ac->arglist_name);
	
	/* Check what type of array we're trying to do. */
	if (array->flgs & AOI_ARRAY_FLAG_NULL_TERMINATED_STRING) {
		/* We are mapping a null-terminated array --- most likely, a
		   string.  The Sun RPC mapping for a null-terminated array is
		   straightforward. */
		
		/* Create the array inline/mapping. */
		ctype = array_ctype;
		ac->ptr = pres_c_new_inline_atom(0, element_map);
		
		/* Create the length. */
		ac->length = PRES_C_I_ATOM,
			       PIA_Index, 0,
			       PIA_Mapping, PRES_C_M_TEMPORARY,
			         PMA_Name, "string_len",
			         PMA_CType, lctype,
			         PMA_PreHandler, "stringlen",
 			         PMA_TempType, TEMP_TYPE_ENCODED,
			         PMA_Target, PRES_C_M_ARGUMENT,
			           PMA_ArgList, ac->arglist_name,
			           PMA_Name, "length",
			           PMA_Mapping, PRES_C_M_DIRECT, END_PRES_C,
			           END_PRES_C,
			         END_PRES_C,
			       END_PRES_C;
		
		/* Create the terminator. */
		ac->terminator = PRES_C_I_TEMPORARY,
				   PIA_Name, "string_term",
				   PIA_CType, element_ctype,
				   PIA_Value, cast_new_expr_lit_char(0, 0),
				   PIA_IsConst, 1,
				   PIA_Mapping, PRES_C_M_ARGUMENT,
			             PMA_ArgList, ac->arglist_name,
				     PMA_Name, "terminator",
				     PMA_Mapping, NULL,
				     END_PRES_C,
				   END_PRES_C;
		
		/*
		 * Set the minimum and maximum presented lengths (note that the
		 * *real* presented array has one more byte than what the
		 * IDL/MINT will show, for the terminator).
		 */
		if (len_min != 0 && len_min != ~0U) len_min++;
		if (len_max != ~0U) len_max++;
		if (len_min != 0) {
			ac->min_len = PRES_C_I_TEMPORARY,
				        PIA_Name, "array_min",
				        PIA_CType, lctype, 
				        PIA_Value, cast_new_expr_lit_int(
						len_min, 0),
				        PIA_IsConst, 1,
				        PIA_Mapping, PRES_C_M_ARGUMENT,
				          PMA_ArgList, ac->arglist_name,
				          PMA_Name, "min_len",
				          PMA_Mapping, NULL,
				          END_PRES_C,
				        END_PRES_C;
		}
		if (len_max != ~0U) {
			ac->max_len = PRES_C_I_TEMPORARY,
				        PIA_Name, "array_max",
				        PIA_CType, lctype, 
				        PIA_Value, cast_new_expr_lit_int(
						len_max, 0),
				        PIA_IsConst, 1,
				        PIA_Mapping, PRES_C_M_ARGUMENT,
				          PMA_ArgList, ac->arglist_name,
				          PMA_Name, "max_len",
				          PMA_Mapping, NULL,
				          END_PRES_C,
				        END_PRES_C;
		}
		
		/*
		 * Finally, create the map.  We have to get into inline mode
		 * for the allocation context, so we create a singleton node.
		 */
		map = pres_c_new_mapping(PRES_C_MAPPING_SINGLETON);
		map->pres_c_mapping_u_u.singleton.inl = alloc_inl;
	} else {
		/*
		 * We are mapping a sequence.
		 * This maps onto a structure with `_len' and `_val' fields.
		 */
		cast_aggregate_type *array_struct;
		cast_type len_ctype;
		pres_c_mapping len_map;
		int array_name_len;
		char *len_name, *val_name;
		
		/* Create the structure. */
		ctype = cast_new_struct_type(0);
		array_struct = &(ctype->cast_type_u_u.agg_type);
		/*
		 * The `array_struct->name' field is the name of the structure
		 * type, and we don't want to name the type.  We can't set the
		 * name to NULL, however, because `cast_check_struct_type()'
		 * insists that the name be non-NULL.  So we use an empty
		 * string.
		 */
		array_struct->name = cast_new_scoped_name("", NULL);
		
		/*
		 * Determine the names of the structure fields.  Note that
		 * sizeof() counts the NUL at the end of a string.
		 */
		array_name_len = strlen(name);
		len_name = (char *) mustmalloc(array_name_len +
					       sizeof("_len"));
		val_name = (char *) mustmalloc(array_name_len +
					       sizeof("_val"));
		strcpy(len_name, name);
		strcat(len_name, "_len");
		strcpy(val_name, name);
		strcat(val_name, "_val");
		
		/* Now fill in the structure field definitions. */
		to_ptc = 0;
		p_variable_array_length_type(array, &to_ptc, &len_map,
					     ac->arglist_name);
		ptn = to_ptc->find_type("definition");
		len_ctype = ptn->get_type();
		
		/*
		 * No need to call `p_variable_array_maximum_type'; the ONC RPC
		 * presentation of variable-length arrays does not expose the
		 * allocated array length.  It also does not expose a buffer
		 * ownership/release flag.
		 */
		
		cast_scope *scope = &array_struct->scope;
		int cdef;
		
		cdef = cast_add_def(scope,
				    cast_new_scoped_name(len_name, NULL),
				    CAST_SC_NONE,
				    CAST_VAR_DEF,
				    ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
				    CAST_PROT_NONE);
		scope->cast_scope_val[cdef].u.cast_def_u_u.var_def.type =
			len_ctype;
		cdef = cast_add_def(scope,
				    cast_new_scoped_name(val_name, NULL),
				    CAST_SC_NONE,
				    CAST_VAR_DEF,
				    ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
				    CAST_PROT_NONE);
		scope->cast_scope_val[cdef].u.cast_def_u_u.var_def.type =
			array_ctype;
		
		/* Set up the inlines for the struct members. */
		ac->length = pres_c_new_inline_atom(0, len_map);
		ac->ptr = pres_c_new_inline_atom(1, element_map);
		
		/* Set the minimum and maximum presented lengths. */
		if (len_min != 0) {
			ac->min_len = PRES_C_I_TEMPORARY,
				        PIA_Name, "array_min",
				        PIA_CType, lctype, 
				        PIA_Value,
				          cast_new_expr_lit_int(len_min, 0),
				        PIA_IsConst, 1,
				        PIA_Mapping, PRES_C_M_ARGUMENT,
				          PMA_ArgList, ac->arglist_name,
				          PMA_Name, "min_len",
				          PMA_Mapping, NULL,
				          END_PRES_C,
				        END_PRES_C;
		}
		if (len_max != ~0U) {
			ac->max_len = PRES_C_I_TEMPORARY,
				        PIA_Name, "array_max",
				        PIA_CType, lctype, 
				        PIA_Value,
				          cast_new_expr_lit_int(len_max, 0),
				        PIA_IsConst, 1,
				        PIA_Mapping, PRES_C_M_ARGUMENT,
				          PMA_ArgList, ac->arglist_name,
				          PMA_Name, "max_len",
				          PMA_Mapping, NULL,
				          END_PRES_C,
				        END_PRES_C;
		}
		
		/* Finally, create the map. */
		map = pres_c_new_mapping(PRES_C_MAPPING_STRUCT);
		map->pres_c_mapping_u_u.struct_i = alloc_inl;
	}
	
	array_ptn->set_type(ctype);
	array_ptn->set_mapping(map);
	ptc->add_type("default", array_ptn);
	if( *out_ptc )
		(*out_ptc)->set_collection_ref(ptc);
	else
		*out_ptc = ptc;
}

/* End of file. */

