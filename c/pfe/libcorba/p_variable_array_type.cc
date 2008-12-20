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
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>
#include <mom/libaoi.h>

#include <mom/c/pg_corba.hh>

static const char * SEQUENCE_NAME_INITIAL_PREFIX	= "CORBA_sequence_";
static const char * SEQUENCE_NAME			= "sequence_";
static const char * LENGTH				= "_length";
static const char * MAXIMUM				= "_maximum";
static const char * BUFFER				= "_buffer";
static const char * RELEASE				= "_release";

/*
 * First, an auxiliary function to compute the name of a CORBA sequence type.
 */
const char *
pg_corba::sequence_type_string(
	aoi_type at,
	aoi *the_aoi,
	const char *the_name)
{
	const char *s;
	int bits, is_signed;
	aoi_type target_at;
	
	/* Return a string corresponding to the argument aoi_type. */
	switch (at->kind) {
	case AOI_INDIRECT:
		/*
		 * Section 17.11 of the CORBA 2.1 specification: ``The type
		 * name used in the C mapping is the type name of the effective
		 * type...'' meaning that one digs down to the base type.
		 */
		target_at = aoi_indir_1(the_aoi, at);
		while (target_at->kind == AOI_INDIRECT) {
			at = target_at;
			target_at = aoi_indir_1(the_aoi, at);
		}
		switch (target_at->kind) {
		case AOI_STRUCT:
		case AOI_UNION:
			s = calc_name_from_ref(at->aoi_type_u_u.
					       indirect_ref);
			break;
			
		default:
			s = sequence_type_string(target_at, the_aoi, the_name);
			break;
		}
		break;
		
	case AOI_INTEGER:
	case AOI_SCALAR:
		if (at->kind == AOI_INTEGER)
			aoi_get_int_size(&(at->aoi_type_u_u.integer_def),
					 &bits, &is_signed);
		else {
			bits = at->aoi_type_u_u.scalar_def.bits;
			is_signed = !(at->aoi_type_u_u.scalar_def.flags
				      & AOI_SCALAR_FLAG_UNSIGNED);
		}
		
		switch (bits) {
		case 1:
			assert(!is_signed);
			s = "boolean";
			break;
		case 8:
			assert(!is_signed);
			s = "octet";
			break;
		case 16:
			s = (is_signed ? "short" : "unsigned_short");
			break;
		case 32:
			s = (is_signed ? "long" : "unsigned_long");
			break;
		case 64:
			s = (is_signed ? "long_long" : "unsigned_long_long");
			break;
		default:
			panic(("In `pg_corba::sequence_type_string', "
			       "unrecognized number of integer bits: %d."),
			      bits);
			break;
		}
		break;
		
	case AOI_ENUM:
		s = "enum";
		break;
		
	case AOI_FLOAT:
		bits = at->aoi_type_u_u.float_def.bits;
		switch (bits) {
		case 32:
			s = "float";
			break;
		case 64:
			s = "double";
			break;
		default:
			panic(("In `pg_corba::sequence_type_string', "
			       "unrecognized number of float bits: %d."),
			      bits);
			break;
		}
		break;
		
	case AOI_CHAR:
		s = "char";
		break;
		
	case AOI_ARRAY:
		/* Recognize string types. */
		if (at->aoi_type_u_u.array_def.element_type->kind == AOI_CHAR)
			s = "string";
		else
			s = flick_asprintf(
				"%s%s",
				SEQUENCE_NAME,
				sequence_type_string((at->aoi_type_u_u.
						      array_def.element_type),
						     the_aoi,
						     the_name)
				);
		break;
		
	case AOI_TYPE_TAG:
		/*
		 * `TypeCode's are pseudo-objects, meaning that they do not
		 * implicitly inherit from `CORBA::Object', and so must be
		 * handled separately.
		 */
		s = "TypeCode";
		break;
		
	case AOI_TYPED:
		s = "any";
		break;
		
	case AOI_INTERFACE:
	case AOI_FWD_INTRFC:
		s = "Object";
		break;
		
	case AOI_STRUCT:
	case AOI_UNION:
	case AOI_EXCEPTION:
	case AOI_VOID:
	case AOI_CONST:
	case AOI_NAMESPACE:
	case AOI_OPTIONAL:
	case AOI_ANY:
		panic(("In `pg_corba::sequence_type_string', "
		       "invalid AOI type kind %d."),
		      at->kind);
		break;
		
	default:
		panic(("In `pg_corba::sequence_type_string', "
		       "unknown AOI type kind %d."),
		      at->kind);
		break;
	}
	
	return s;
}

/*
 * Here is the CORBA-specific version of `pg_state::p_variable_array_type'.
 */
void pg_corba::p_variable_array_type(aoi_array *array,
				     p_type_collection **out_ptc)
{
	cast_type ctype, element_ctype;
	pres_c_mapping map, element_map;
	p_type_collection *ptc, *to_ptc;
	p_type_node *ptn, *array_ptn;
	
	cast_type lctype = mint_to_ctype(
		&out_pres->mint,
		out_pres->mint.standard_refs.unsigned32_ref);
	
	ptc = p_new_type_collection(a(cur_aoi_idx).name);
	array_ptn = new p_type_node;
	array_ptn->set_name("definition");
	array_ptn->set_format("%s");
	
	/* Create the allocation context node.  */
	pres_c_inline alloc_inl
		= pres_c_new_inline(PRES_C_INLINE_ALLOCATION_CONTEXT);
	pres_c_inline_allocation_context *ac
		= &alloc_inl->pres_c_inline_u_u.acontext;
	
	ac->arglist_name = pres_c_make_arglist_name("vararray");
	
	/*
	 * The normal allocation case is almost sufficient.  However, we need
	 * to fix the following cases:
	 *   On the client:
	 *     INOUT case, we *do* want to dealloc variable-sized pointers.
	 *   On the server:
	 *     OUT and RETURN cases, we don't want/need to allocate space.
	 *
	 * XXX - What we really should do is dealloc only if we can't reuse
	 * the buffer.  Though a flag exists for this (DEALLOC_IF_TOO_SMALL),
	 * nothing is implemented for it yet.
	 */
	ac->alloc = p_get_allocation();
	if (gen_client) {
		ac->alloc.cases[PRES_C_DIRECTION_INOUT].pres_c_allocation_u_u.
			val.flags
			= PRES_C_ALLOC_ALWAYS | PRES_C_DEALLOC_ALWAYS |
			PRES_C_DEALLOC_ON_FAIL;
	} else if (gen_server) {
		ac->alloc.cases[PRES_C_DIRECTION_OUT].pres_c_allocation_u_u.
			val.flags
			= PRES_C_ALLOC_NEVER | PRES_C_DEALLOC_ALWAYS |
			PRES_C_DEALLOC_ON_FAIL;
		ac->alloc.cases[PRES_C_DIRECTION_RETURN].pres_c_allocation_u_u.
			val.flags
			= PRES_C_ALLOC_NEVER | PRES_C_DEALLOC_ALWAYS |
			PRES_C_DEALLOC_ON_FAIL;
	} else {
		panic("In pg_corba::p_variable_array_type: "
		      "Generating neither client nor server!");
	}
	
	/*
	 * If there are bounds on the array, set the appropriate minimum and
	 * maximum length for the array.
	 */
	unsigned len_min, len_max;
	aoi_get_array_len(in_aoi, array, &len_min, &len_max);
	assert(len_min != len_max); /* We're doing a *VARIABLE* array! */
	
	/* Check what type of array we're trying to do. */
	if (array->flgs & AOI_ARRAY_FLAG_NULL_TERMINATED_STRING) {
		/*
		 * We are mapping a null-terminated array --- most likely, a
		 * string.  The CORBA mapping for a null-terminated array is
		 * straightforward.
		 */
		
		/* Create the ctype and mapping for the array element type. */
		to_ptc = 0;
		p_type(array->element_type, &to_ptc);
		ptn = to_ptc->find_type("definition");
		element_ctype = ptn->get_type();
		element_map = ptn->get_mapping();
		
		/* Create the array inline/mapping. */
		ctype = element_ctype;
		pres_c_interpose_internal_array(&ctype, &element_map,
						ac->arglist_name);
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
			/*
			 * CORBA 2.2 spec section 17.12: Bounded strings are to
			 * be allocated to their ``specified'' size.  In some
			 * cases this is likely dumb, but hey, it's the spec.
			 */
			ac->min_alloc_len = PRES_C_I_TEMPORARY,
					      PIA_Name, "array_max",
					      PIA_CType, lctype, 
					      PIA_Value, cast_new_expr_lit_int(
						      len_max, 0),
					      PIA_IsConst, 1,
					      PIA_Mapping, PRES_C_M_ARGUMENT,
					        PMA_ArgList, ac->arglist_name,
					        PMA_Name, "min_alloc_len",
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
		 * We are mapping a CORBA sequence type.  A sequence is a
		 * structure with `_maximum', `_length', and `_buffer' fields.
		 * It also has a release/ownership flag, which shouldn't
		 * really be part of the presentation, but due to current
		 * infrastructure, we must define it here.  We designate it as
		 * `_release'.
		 */
		aoi_type at, target_at;
		
		cast_aggregate_type *array_struct;
		cast_type max_ctype, len_ctype, rel_ctype;
		pres_c_mapping max_map, len_map, rel_map;
		const char *max_name, *len_name, *val_name, *rel_name;
		
		int cdef;
		
		/*
		 * Create the ctype and mapping for the array element type.
		 *
		 * Section 17.11 or the CORBA 2.1 spec says that the type of
		 * the sequence buffer elements is the ``type name of the
		 * effective type,'' so we have to dig down through the AOI.
		 */
		at = array->element_type;
		target_at = aoi_indir_1(in_aoi, at);
		while (target_at->kind == AOI_INDIRECT) {
			at = target_at;
			target_at = aoi_indir_1(in_aoi, at);
		}
		to_ptc = 0;
		switch (target_at->kind) {
		case AOI_STRUCT:
		case AOI_UNION:
			/*
			 * Use the struct/union name (e.g., ``struct foo'') and
			 * not the raw, unamed struct/union definition (e.g.,
			 * ``struct { ... }''.
			 */
			p_type(at, &to_ptc);
			break;
			
		default:
			/* Use the actual ``base'' type. */
			p_type(target_at, &to_ptc);
			break;
		}
		
		ptn = to_ptc->find_type("definition");
		element_ctype = ptn->get_type();
		element_map = ptn->get_mapping();
		
		/* Create the structure. */
		ctype = cast_new_struct_type(0);
		array_struct = &(ctype->cast_type_u_u.agg_type);
		array_struct->name
			= cast_new_scoped_name(
				flick_asprintf(
					"%s%s",
					SEQUENCE_NAME_INITIAL_PREFIX,
					sequence_type_string(
						array->element_type,
						in_aoi,
						name)
					),
				NULL);
		
		/*
		 * Determine the names of the structure fields.
		 */
		max_name = MAXIMUM;
		len_name = LENGTH;
		val_name = BUFFER;
		rel_name = RELEASE;
		
		/* Now fill in the structure field definitions. */
		to_ptc = 0;
		p_variable_array_maximum_type(array, &to_ptc, &max_map,
					      ac->arglist_name);
		ptn = to_ptc->find_type("definition");
		max_ctype = ptn->get_type();
		to_ptc = 0;
		p_variable_array_length_type(array, &to_ptc, &len_map,
					     ac->arglist_name);
		ptn = to_ptc->find_type("definition");
		len_ctype = ptn->get_type();
		to_ptc = 0;
		p_variable_array_release_type(array, &to_ptc, &rel_map,
					      ac->arglist_name);
		ptn = to_ptc->find_type("definition");
		rel_ctype = ptn->get_type();

		/*
		 * XXX - The CORBA C mapping doesn't offer inheritance, and the
		 * release flag is not specified as a member of a sequence
		 * structure, so the only way to ensure the runtime functions
		 * for getting and setting the release flag work properly is if
		 * the release flag is at a known offset from the beginning of
		 * the structure.  For optimal compatibility, we can't assume
		 * that the release flag is part of the normal (defined)
		 * sequence type (such as at offset 0), since the user may wish
		 * to view/set it's internals solely by offset (or overlaying a
		 * standard sequence type over ours).  Thus, we put the release
		 * flag as the fourth member of the structure.
		 *
		 * XXX - Note that runtime/headers/flick/pres/corba.h currently
		 * predefines two sequence types (necessary for the runtime).
		 * THESE MUST CONFORM TO THE SAME LAYOUT AS GIVEN HERE!!
		 * They must also conform to the CORBA functions for getting
		 * and setting the release flag.
		 */
		cdef = cast_add_def(&ctype->cast_type_u_u.agg_type.scope,
				    cast_new_scoped_name(max_name, NULL),
				    CAST_SC_NONE,
				    CAST_VAR_DEF,
				    PASSTHRU_DATA_CHANNEL,
				    CAST_PROT_NONE);
		ctype->cast_type_u_u.agg_type.scope.cast_scope_val[cdef].u.
			cast_def_u_u.var_def.type = max_ctype;
		cdef = cast_add_def(&ctype->cast_type_u_u.agg_type.scope,
				    cast_new_scoped_name(len_name, NULL),
				    CAST_SC_NONE,
				    CAST_VAR_DEF,
				    PASSTHRU_DATA_CHANNEL,
				    CAST_PROT_NONE);
		ctype->cast_type_u_u.agg_type.scope.cast_scope_val[cdef].u.
			cast_def_u_u.var_def.type = len_ctype;
		cdef = cast_add_def(&ctype->cast_type_u_u.agg_type.scope,
				    cast_new_scoped_name(val_name, NULL),
				    CAST_SC_NONE,
				    CAST_VAR_DEF,
				    PASSTHRU_DATA_CHANNEL,
				    CAST_PROT_NONE);
		ctype->cast_type_u_u.agg_type.scope.cast_scope_val[cdef].u.
			cast_def_u_u.var_def.type =
			cast_new_pointer_type(element_ctype);
		cdef = cast_add_def(&ctype->cast_type_u_u.agg_type.scope,
				    cast_new_scoped_name(rel_name, NULL),
				    CAST_SC_NONE,
				    CAST_VAR_DEF,
				    PASSTHRU_DATA_CHANNEL,
				    CAST_PROT_NONE);
		ctype->cast_type_u_u.agg_type.scope.cast_scope_val[cdef].u.
			cast_def_u_u.var_def.type = rel_ctype;
		
		/*
		 * Interpose an ``internal array'' mapping on the basic element
		 * mapping and a CAST pointer on the basic element CAST type.
		 * This is what we need in order to allocate the array storage.
		 */
		pres_c_interpose_internal_array(&element_ctype,
						&element_map,
						ac->arglist_name);
		
		/* Set up the inlines for the struct members. */
		ac->alloc_len = pres_c_new_inline_atom(0, max_map);
		ac->length = pres_c_new_inline_atom(1, len_map);
		ac->ptr = pres_c_new_inline_atom(2, element_map);
		ac->release = pres_c_new_inline_atom(3, rel_map);
		
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
			/*
			 * CORBA 2.2 spec section 17.11: Bounded arrays are to
			 * be allocated to their ``specified'' size.  In some
			 * cases this is likely dumb, but hey, it's the spec.
			 */
			ac->min_alloc_len = PRES_C_I_TEMPORARY,
					      PIA_Name, "array_max",
					      PIA_CType, lctype, 
					      PIA_Value, cast_new_expr_lit_int(
						      len_max, 0),
					      PIA_IsConst, 1,
					      PIA_Mapping, PRES_C_M_ARGUMENT,
					        PMA_ArgList, ac->arglist_name,
					        PMA_Name, "min_alloc_len",
					        PMA_Mapping, NULL,
					        END_PRES_C,
					      END_PRES_C;
		}
		
		/* Finally, create the map. */
		map = pres_c_new_mapping(PRES_C_MAPPING_STRUCT);
		map->pres_c_mapping_u_u.struct_i = alloc_inl;
		
		/* Emit a `typedef' for this sequence type. */
		cast_scope *deep_scope = root_scope;
		if( cast_find_def(&deep_scope,
				  array_struct->name,
				  CAST_TYPEDEF) == -1 ) {
			cdef = cast_add_def(root_scope,
					    array_struct->name,
					    CAST_SC_NONE,
					    CAST_TYPEDEF,
					    ch(cur_aoi_idx,
					       PG_CHANNEL_CLIENT_DECL),
					    CAST_PROT_NONE);
			root_scope->cast_scope_val[cdef].u.cast_def_u_u.
				typedef_type = ctype;
		}
		
		ctype = cast_new_type(CAST_TYPE_STRUCT_NAME);
		ctype->cast_type_u_u.struct_name = array_struct->name;
	}
	
	array_ptn->set_type(ctype);
	array_ptn->set_mapping(map);
	ptc->add_type("default", array_ptn);
	if (*out_ptc)
		(*out_ptc)->set_collection_ref(ptc);
	else
		*out_ptc = ptc;
}

/* End of file. */

