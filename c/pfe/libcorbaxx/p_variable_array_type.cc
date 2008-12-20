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

#include <assert.h>
#include <string.h>

#include <mom/compiler.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>
#include <mom/libaoi.h>

#include <mom/c/pg_corbaxx.hh>

static const char * SEQUENCE_NAME_INITIAL_PREFIX	= "flick_sequence_";
static const char * SEQUENCE_NAME			= "sequence_";
static const char * LENGTH				= "_length";
static const char * MAXIMUM				= "_maximum";
static const char * BUFFER				= "_buffer";
static const char * RELEASE				= "_release";

/*
 * First, an auxiliary function to compute the name of a CORBA sequence type.
 */
const char *
pg_corbaxx::sequence_type_string(
	aoi_type at,
	aoi *the_aoi,
	const char *the_name)
{
	const char *s;
	int bits, is_signed;
	
	/* Return a string corresponding to the argument aoi_type. */
	switch (at->kind) {
	case AOI_INDIRECT:
		s = aoi_get_scoped_name(at->aoi_type_u_u.indirect_ref, "_");
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
			panic(("In `pg_corbaxx::sequence_type_string', "
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
			panic(("In `pg_corbaxx::sequence_type_string', "
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
		panic(("In `pg_corbaxx::sequence_type_string', "
		       "invalid AOI type kind %d."),
		      at->kind);
		break;
		
	default:
		panic(("In `pg_corbaxx::sequence_type_string', "
		       "unknown AOI type kind %d."),
		      at->kind);
		break;
	}
	
	return s;
}

/*
 * Here is the CORBA-specific version of `pg_state::p_variable_array_type'.
 */
void pg_corbaxx::p_variable_array_type(aoi_array *array,
				       p_type_collection **out_ptc)
{
	p_type_collection *ptc, *to_ptc, *elem_ptc, *seq_ptc;
	p_scope_node *psn;
	p_type_node *ptn, *array_ptn;
	cast_type ctype, raw_element_ctype, element_ctype;
	pres_c_mapping map, element_map;
	
	cast_type lctype = mint_to_ctype(
		&out_pres->mint,
		out_pres->mint.standard_refs.unsigned32_ref);
	
	/* Create the allocation context node.  */
	pres_c_inline alloc_inl
		= pres_c_new_inline(PRES_C_INLINE_ALLOCATION_CONTEXT);
	pres_c_inline_allocation_context *ac
		= &alloc_inl->pres_c_inline_u_u.acontext;
	
	ac->arglist_name = pres_c_make_arglist_name("vararray");
	
	/*
	 * If there are bounds on the array, set the appropriate minimum and
	 * maximum length for the array.
	 */
	unsigned len_min, len_max;
	aoi_get_array_len(in_aoi, array, &len_min, &len_max);
	assert(len_min != len_max); /* We're doing a *VARIABLE* array! */
	
	/* Check what type of array we're trying to do. */
	if (array->flgs & AOI_ARRAY_FLAG_NULL_TERMINATED_STRING) {
		pres_c_inline managed_alloc_inl
			= pres_c_new_inline(PRES_C_INLINE_ALLOCATION_CONTEXT);
		int lpc;
		
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
		}
		
		/*
		 * Create the allocation semantics for an unmanaged string.
		 * These allocations semantics are for strings that are
		 * passed as parameters and not contained within another object
		 */
		ac->alloc = p_get_allocation();
		if (gen_client) {
			ac->alloc.cases[PRES_C_DIRECTION_INOUT].
				pres_c_allocation_u_u.val.flags
				= PRES_C_ALLOC_ALWAYS | PRES_C_DEALLOC_ALWAYS |
				PRES_C_DEALLOC_ON_FAIL;
		} else if (gen_server) {
			ac->alloc.cases[PRES_C_DIRECTION_OUT].
				pres_c_allocation_u_u.val.flags
				= PRES_C_ALLOC_NEVER | PRES_C_DEALLOC_ALWAYS |
				PRES_C_DEALLOC_ON_FAIL;
			ac->alloc.cases[PRES_C_DIRECTION_RETURN].
				pres_c_allocation_u_u.val.flags
				= PRES_C_ALLOC_NEVER | PRES_C_DEALLOC_ALWAYS |
				PRES_C_DEALLOC_ON_FAIL;
		} else {
			panic("In pg_corba::p_variable_array_type: "
			      "Generating neither client nor server!");
		}
		for( lpc = 0; lpc < PRES_C_DIRECTIONS; lpc++ ) {
			switch( ac->alloc.cases[lpc].allow ) {
			case PRES_C_ALLOCATION_ALLOW:
				ac->alloc.cases[lpc].pres_c_allocation_u_u.val.
					flags &= ~(PRES_C_RUN_CTOR|
						   PRES_C_RUN_DTOR);
				ac->alloc.cases[lpc].pres_c_allocation_u_u.val.
					allocator.pres_c_allocator_u.name =
					ir_strlit("CORBA_string");
				break;
			default:
				break;
			}
		}
		
		/*
		 * Finally, create the map.  We have to get into inline mode
		 * for the allocation context, so we create a singleton node.
		 */
		map = PRES_C_M_SINGLETON,
			PMA_Target, alloc_inl,
			END_PRES_C;
		
		ptc = new p_type_collection;
		psn = new p_scope_node;
		psn->set_name("default");
		psn->set_scope_name(cast_new_scoped_name("CORBA", NULL));
		ptc->add_scope(psn);
		ptc->set_name("String");
		ptc->set_collection_ref(
			prim_collections[PRIM_COLLECTION_STRING]
			);
		
		pres_c_mapping xmap;
		
		xmap = PRES_C_M_XLATE,
			PMA_InternalCType, ctype,
			PMA_InternalMapping, map,
			END_PRES_C;
		
		ptn = ptc->find_type("definition");
		ptn->set_format("%s");
		ptn->set_mapping( xmap );
		
		ptn = ptc->find_type("smart_pointer");
		ptn->set_mapping( xmap );

		
		/*
		 * This is the same as above except we are creating a
		 * different set of allocation semantics.  The semantics
		 * created here apply to strings that are contained within
		 * other objects (e.g. structures).
		 */
		ac = &managed_alloc_inl->pres_c_inline_u_u.acontext;
		ac->arglist_name = pres_c_make_arglist_name("vararray");
		
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
		
		/* Create the allocation semantics. */
		ac->alloc = p_get_allocation();
		for( lpc = 0; lpc < PRES_C_DIRECTIONS; lpc++ ) {
			switch( ac->alloc.cases[lpc].allow ) {
			case PRES_C_ALLOCATION_ALLOW:
				ac->alloc.cases[lpc].pres_c_allocation_u_u.val.
					flags &= ~(PRES_C_DEALLOC_ON_FAIL|
						   PRES_C_DEALLOC_ALWAYS|
						   PRES_C_RUN_CTOR|
						   PRES_C_RUN_DTOR);
				ac->alloc.cases[lpc].pres_c_allocation_u_u.val.
					allocator.pres_c_allocator_u.name =
					ir_strlit("CORBA_string");
				break;
			default:
				break;
			}
		}
		ac->alloc.cases[PRES_C_DIRECTION_IN].pres_c_allocation_u_u.
			val.allocator.kind = PRES_C_ALLOCATOR_NAME;
		
		/*
		 * Finally, create the map.  We have to get into inline mode
		 * for the allocation context, so we create a singleton node.
		 */
		map = PRES_C_M_SINGLETON,
			PMA_Target, managed_alloc_inl,
			END_PRES_C;
		xmap = PRES_C_M_XLATE,
			PMA_InternalCType, ctype,
			PMA_InternalMapping, map,
			PMA_Translator, "string_slot_xlator",
			END_PRES_C;
		
		/*
		 * Add a managed string type to the collection.  The type
		 * is for internal use only so it isn't added to the reference
		 * and it isn't defined.
		 */
		ptn = new p_type_node;
		ptn->set_name("managed_string");
		ptn->set_format("%s");
		ptn->set_flags(PTF_REF_ONLY|PTF_NO_DEF);
		ptn->set_type(ctype);
		ptn->set_mapping(xmap);
		ptc->add_type("default", ptn, 1, 0);
		
		ptn = new p_type_node;
		ptn->set_name("peeled_managed_string");
		ptn->set_format("%s");
		ptn->set_flags(PTF_REF_ONLY|PTF_NO_DEF);
		ptn->set_type(ctype);
		ptn->set_mapping(map);
		ptc->add_type("default", ptn, 1, 0);
		
		if( *out_ptc && (len_max != ~0U) ) {
			add_tag((*out_ptc)->get_tag_list(), "max_len",
				TAG_INTEGER, len_max - 1);
		}
		
	} else {
		array_ptn = new p_type_node;
		array_ptn->set_name("definition");
		array_ptn->set_format("%s");
		/*
		 * We are mapping a CORBA sequence type.  A sequence is a
		 * structure with `_maximum', `_length', and `_buffer' fields.
		 * It also has a release/ownership flag, which shouldn't
		 * really be part of the presentation, but due to current
		 * infrastructure, we must define it here.  We designate it as
		 * `_release'.
		 */
		aoi_type at, target_at;
		
		cast_def_protection last_prot = current_protection;
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
		elem_ptc = 0;
		switch (target_at->kind) {
		case AOI_ENUM:
		case AOI_FWD_INTRFC:
		case AOI_STRUCT:
		case AOI_UNION:
			/*
			 * Use the struct/union name (e.g., ``struct foo'') and
			 * not the raw, unamed struct/union definition (e.g.,
			 * ``struct { ... }''.
			 */
			p_usable_type(at, &elem_ptc,
				      &raw_element_ctype, &element_map);
			element_ctype = raw_element_ctype;
			break;
		case AOI_INTERFACE:
			/*
			 * This is a bit of a hack since the sequence members
			 * aren't managed individually, like in a struct,
			 * rather the sequence members are managed by the
			 * sequence object.  So if a parameter is an 'inout'
			 * setting a member won't automatically free the
			 * previous one, so we have to do it manually.  Since
			 * the setting only happens on the client side we use
			 * the regular semantics there, and the managed on
			 * the server side.  We can't use the regular semantics
			 * on the server side since the sequence object will
			 * take care of freeing objects so theres no point
			 * in duplicating the work.
			 */
			if( gen_server ) {
				p_usable_type(at, &elem_ptc,
					      &raw_element_ctype,
					      &element_map);
				raw_element_ctype = cast_new_pointer_type(
					raw_element_ctype);
				element_ctype = raw_element_ctype;
			} else if( gen_client ) {
				p_type(at, &elem_ptc);
				ptn = elem_ptc->find_type("pointer");
				raw_element_ctype = ptn->get_type();
				element_ctype = raw_element_ctype;
				element_map = ptn->get_mapping();
			}
			break;
		case AOI_ARRAY:
			/* Similar to the above hack with objects */
			if( gen_server &&
			    (at->aoi_type_u_u.array_def.flgs
			     == AOI_ARRAY_FLAG_NULL_TERMINATED_STRING) ) {
				p_type_node *ptn;
				
				p_type(at, &elem_ptc);
				ptn = elem_ptc->
					find_type("peeled_managed_string");
				raw_element_ctype = ptn->get_type();
				element_ctype = ptn->get_type();
				element_map = ptn->get_mapping();
				break;
			}
		default:
			/* Use the actual ``base'' type. */
			p_type(target_at, &elem_ptc);
			ptn = elem_ptc->find_type("definition");
			raw_element_ctype = ptn->get_type();
			element_ctype = raw_element_ctype;
			element_map = ptn->get_mapping();
			break;
		}
		
		/* Create the structure. */
		ctype = cast_new_class_type(0);
		array_struct = &(ctype->cast_type_u_u.agg_type);
		
		cast_scoped_name full_seq_name;
		const char *seq_name;

		if( *out_ptc )
			seq_name = (*out_ptc)->get_name();
		else {
			seq_name
				= flick_asprintf(
					"%s%s%s",
					SEQUENCE_NAME_INITIAL_PREFIX,
					sequence_type_string(
						array->element_type,
						in_aoi,
						name),
					((len_max != ~0U) ?
					 flick_asprintf("_bounded_%d",
							len_max) :
					 "")
					);
		}
		seq_ptc = p_new_type_collection(seq_name);
		full_seq_name = cast_copy_scoped_name(&current_scope_name);
		cast_add_scope_name(&full_seq_name,
				    seq_name,
				    null_template_arg_array);
		array_struct->name = cast_new_scoped_name(seq_name, NULL);
		
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
		
		cdef = cast_add_def(&ctype->cast_type_u_u.agg_type.scope,
				    cast_new_scoped_name(max_name, NULL),
				    CAST_SC_NONE,
				    CAST_VAR_DEF,
				    ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
				    CAST_PROT_PUBLIC);
		ctype->cast_type_u_u.agg_type.scope.cast_scope_val[cdef].u.
			cast_def_u_u.var_def.type = max_ctype;
		cdef = cast_add_def(&ctype->cast_type_u_u.agg_type.scope,
				    cast_new_scoped_name(len_name, NULL),
				    CAST_SC_NONE,
				    CAST_VAR_DEF,
				    ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
				    CAST_PROT_PUBLIC);
		ctype->cast_type_u_u.agg_type.scope.cast_scope_val[cdef].u.
			cast_def_u_u.var_def.type = len_ctype;
		cdef = cast_add_def(&ctype->cast_type_u_u.agg_type.scope,
				    cast_new_scoped_name(val_name, NULL),
				    CAST_SC_NONE,
				    CAST_VAR_DEF,
				    ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
				    CAST_PROT_PUBLIC);
		ctype->cast_type_u_u.agg_type.scope.cast_scope_val[cdef].u.
			cast_def_u_u.var_def.type =
			cast_new_pointer_type(element_ctype);
		cdef = cast_add_def(&ctype->cast_type_u_u.agg_type.scope,
				    cast_new_scoped_name(rel_name, NULL),
				    CAST_SC_NONE,
				    CAST_VAR_DEF,
				    ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
				    CAST_PROT_NONE);
		ctype->cast_type_u_u.agg_type.scope.cast_scope_val[cdef].u.
			cast_def_u_u.var_def.type = rel_ctype;
		
		/* Set up the allocation semantics. */
		ac->alloc = p_get_allocation();
		
		if (len_max != ~0U) {
			ac->alloc.cases[PRES_C_DIRECTION_IN].
				pres_c_allocation_u_u.val.flags &=
				~PRES_C_ALLOC_ALWAYS;
			ac->alloc.cases[PRES_C_DIRECTION_INOUT].
				pres_c_allocation_u_u.val.flags &=
				~PRES_C_ALLOC_ALWAYS;
			ac->alloc.cases[PRES_C_DIRECTION_OUT].
				pres_c_allocation_u_u.val.flags &=
				~PRES_C_ALLOC_ALWAYS;
			ac->alloc.cases[PRES_C_DIRECTION_RETURN].
				pres_c_allocation_u_u.val.flags &=
				~PRES_C_ALLOC_ALWAYS;
		}
		if( gen_client && (len_max == ~0U) ) {
			ac->alloc.cases[PRES_C_DIRECTION_INOUT].
				pres_c_allocation_u_u.val.flags |=
				PRES_C_DEALLOC_ALWAYS|PRES_C_RUN_DTOR;
		}
		if( gen_server ) {
			ac->alloc.cases[PRES_C_DIRECTION_INOUT].
				pres_c_allocation_u_u.val.flags &=
				~PRES_C_DEALLOC_ALWAYS|PRES_C_RUN_DTOR;
		}
		ac->alloc.cases[PRES_C_DIRECTION_RETURN].pres_c_allocation_u_u.
			val.flags &= ~PRES_C_DEALLOC_ALWAYS|PRES_C_RUN_DTOR;
		ac->alloc.cases[PRES_C_DIRECTION_OUT].pres_c_allocation_u_u.
			val.flags &= ~PRES_C_DEALLOC_ALWAYS|PRES_C_RUN_DTOR;
		
		/*
		 * Interpose an ``internal array'' mapping on the basic element
		 * mapping and a CAST pointer on the basic element CAST type.
		 * This is what we need in order to allocate the array storage.
		 */
		pres_c_interpose_internal_array(&element_ctype, &element_map,
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
			 * Bounded arrays are to be allocated to their
			 * ``specified'' (bounded) size.
			 *
			 * This is required by TAO; see the code that TAO
			 * generates for the `allocbuf' and `freebuf' member
			 * functions of bounded sequence types (e.g., bounded
			 * sequence of strings, in TAO's `Param_Test' test).
			 *
			 * XXX --- ``Required'' is perhaps too strong a word.
			 * It's what TAO does, so it's what we do, too.  But if
			 * we changed our code for `allocbuf' and `freebuf'
			 * (and other sequence member functions as necessary,
			 * see `runtime/headers/flick/pres/tao_sequence.scml'),
			 * we could perhaps be more clever and still work with
			 * TAO.  The primary barrier to being clever is that
			 * `allocbuf' and `freebuf' are defined by the CORBA
			 * C++ mapping to be static member functions, so one
			 * can't access the sequence `_length' and `_maximum'
			 * slots in the implementation of those functions!
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
		
		array_ptn->set_type(ctype);
		array_ptn->set_mapping(map);
		seq_ptc->add_type("default", array_ptn);
		
		cast_scope *scope = (cast_scope *)top_ptr(scope_stack);
		current_protection = CAST_PROT_PUBLIC;
		/* if *out_ptc != 0 then this is a typedef'd sequence
		   so we must continue to construct its body.  If it
		   is 0 then we look up in the type collections to
		   see if we've already created a sequence of this
		   name, in which case we can just use that. */
		if( (*out_ptc) ||
		    !(ptc = p_type_collection::
		      find_collection(&type_collections,
				      seq_name,
				      scope)) ) {
			union tag_data_u data;
			tag_data *pt_td;
			aoi_type_u at;
			tag_list *tl;
			
			/* Create a new pres type tag list for the sequence */
			pt_td = &(find_tag(out_pres->pres_attrs,
					   "pres_type")->data);
			if( *out_ptc )
				ptc = *out_ptc;
			else {
				/* Create a named type collection
				   for the sequence */
				ptc = p_new_type_collection(seq_name);
				tl = create_tag_list(0);
				data.tl = tl;
				ptc->set_attr_index(append_tag_data(pt_td,
								    data));
				ptc->set_tag_list(tl);
			}
			/* Build the tag list describing the sequence */
			tl = create_tag_list(0);
			data.tl = tl;
			seq_ptc->set_attr_index(append_tag_data(pt_td, data));
			seq_ptc->set_tag_list(tl);
			ptc->set_collection_ref(seq_ptc);
			add_tag(tl, "name", TAG_STRING, ptc->get_name());
			add_tag(tl, "size", TAG_STRING, "variable");
			add_tag(tl, "idl_type", TAG_STRING, "sequence");
			add_tag(tl, "val", TAG_STRING, val_name);
			add_tag(tl, "max", TAG_STRING, max_name);
			add_tag(tl, "len", TAG_STRING, len_name);
			add_tag(tl, "release", TAG_STRING, rel_name);
			add_tag(tl, "bounded", TAG_BOOL,
				(len_max != (unsigned)~0));
			add_tag(tl, "slice_pres_index", TAG_INTEGER,
				elem_ptc->get_attr_index());
			if( len_max != (unsigned)~0 )
				add_tag(tl, "max_len", TAG_INTEGER, len_max);
			
			/* Add the types for the sequence */
			ptn = new p_type_node;
			ptn->set_name("pointer");
			ptn->set_format("%s_ptr");
			ctype = ptc->find_type("definition")->get_type();
			ptn->set_type(cast_new_pointer_type(ctype));
			ptc->add_type("default", ptn);
			
			ptn = new p_type_node;
			ptn->set_name("array_slice");
			ptn->set_format("");
			ptn->set_type(raw_element_ctype);
			ptc->add_type("default", ptn);
			
			/* Create the C++ infrastructure for the class */
			p_sequence_class(ptc, array);
			at.kind = AOI_ARRAY;
			at.aoi_type_u_u.array_def = *array;
			p_type_var(ptc, &at);
			p_out_class(ptc, &at);
			if( !(*out_ptc) ) {
				/* This sequence wasn't typedef'd so
				   we add it to the list of type_collections,
				   and define its types */
				add_tail(&type_collections, &ptc->link);
				ptc->define_types();
				/* Since this sequence is "anonymous"
				   then we must modify the mapping to be
				   the real thing and not an indirection
				   to a m/u stub */
				ptn = ptc->find_type("definition");
				ptn->set_mapping(map);
			}
		}
		if( *out_ptc )
			ptc = ptc->get_collection_ref();
		current_protection = last_prot;
	}
	
	if( *out_ptc )
		(*out_ptc)->set_collection_ref(ptc);
	else
		*out_ptc = ptc;
}

/* This will create all the functions for a sequence class */
void pg_corbaxx::p_sequence_class(p_type_collection *ptc,
				  aoi_array *aa)
{
	cast_scoped_name scn;
	cast_type raw_type, seq_type, ulong_type, bool_type;
	cast_type raw_elem_type, elem_type;
	tag_list *tl, *pt_tl;
	cast_scope *scope;
	cast_type type, type2;
	const char *seq_name;
	cast_init default_val;
	int is_bounded = 0, string_elem = 0;
	unsigned amin, amax;
	int slice_pres_index;
	tag_item *ti;
	
	aoi_get_array_len(in_aoi,
			  aa,
			  &amin,
			  &amax);
	seq_type = ptc->get_collection_ref()->
		find_type("definition")->get_type();
	pt_tl = ptc->get_tag_list();
	tl = create_tag_list(0);
	if( amax == (unsigned)~0 )
		is_bounded = 0;
	else
		is_bounded = 1;
	add_tag(pt_tl, "main", TAG_TAG_LIST, tl);
	add_tag(tl, "form", TAG_STRING, "class");
	seq_name = ptc->get_name();
	scn = cast_new_scoped_name(seq_name, NULL);
	raw_type = ptc->find_type("definition")->get_type();
	raw_elem_type = ptc->find_type("array_slice")->get_type();
	slice_pres_index = find_tag(ptc->get_collection_ref()->get_tag_list(),
				    "slice_pres_index")->data.tag_data_u.i;
	ti = find_tag(out_pres->pres_attrs, "pres_type");
	ti = find_tag(get_tag_data(&ti->data, slice_pres_index).tl,
		      "idl_type");
	if( !strcmp(ti->data.tag_data_u.str, "string") ) {
		string_elem = 1;
	}
	elem_type = cast_new_pointer_type(raw_elem_type);
	cast_add_scope_name(&current_scope_name,
			    seq_name,
			    null_template_arg_array);
	add_tag(tl, "name", TAG_CAST_SCOPED_NAME,
		cast_copy_scoped_name(&current_scope_name));
	ulong_type = prim_collections[PRIM_COLLECTION_ULONG]->
		find_type("definition")->get_type();
	bool_type = prim_collections[PRIM_COLLECTION_BOOLEAN]->
		find_type("definition")->get_type();
	scope = &seq_type->cast_type_u_u.agg_type.scope;
	add_function(tl,
		     scn,
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "T()",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_Constructor,
		     PFA_Scope, scope,
		     PFA_ReturnType, cast_new_type(CAST_TYPE_NULL),
		     PFA_TAG_DONE);
	default_val = cast_new_init_expr(cast_new_expr_lit_int(0,0));
	if( is_bounded ) {
		add_function(tl,
			     scn,
			     PFA_Protection, current_protection,
			     PFA_FunctionKind, "non_default_constructor",
			     PFA_DeclChannel, ch(cur_aoi_idx,
						 PG_CHANNEL_CLIENT_DECL),
			     PFA_ImplChannel, ch(cur_aoi_idx,
						 PG_CHANNEL_CLIENT_IMPL),
			     PFA_Constructor,
			     PFA_Scope, scope,
			     PFA_ReturnType, cast_new_type(CAST_TYPE_NULL),
			     PFA_Parameter, ulong_type, "length", NULL,
			     PFA_Parameter, elem_type, "buffer", NULL,
			     PFA_Parameter, bool_type, "release", default_val,
			     PFA_TAG_DONE);
	}
	else {
		add_function(tl,
			     scn,
			     PFA_Protection, current_protection,
			     PFA_FunctionKind, "T(ulong)",
			     PFA_DeclChannel, ch(cur_aoi_idx,
						 PG_CHANNEL_CLIENT_DECL),
			     PFA_ImplChannel, ch(cur_aoi_idx,
						 PG_CHANNEL_CLIENT_IMPL),
			     PFA_Constructor,
			     PFA_Scope, scope,
			     PFA_ReturnType, cast_new_type(CAST_TYPE_NULL),
			     PFA_Parameter, ulong_type, "max", NULL,
			     PFA_TAG_DONE);
		add_function(tl,
			     scn,
			     PFA_Protection, current_protection,
			     PFA_FunctionKind, "non_default_constructor",
			     PFA_DeclChannel, ch(cur_aoi_idx,
						 PG_CHANNEL_CLIENT_DECL),
			     PFA_ImplChannel, ch(cur_aoi_idx,
						 PG_CHANNEL_CLIENT_IMPL),
			     PFA_Constructor,
			     PFA_Scope, scope,
			     PFA_ReturnType, cast_new_type(CAST_TYPE_NULL),
			     PFA_Parameter, ulong_type, "max", NULL,
			     PFA_Parameter, ulong_type, "length", NULL,
			     PFA_Parameter, elem_type, "buffer", NULL,
			     PFA_Parameter, bool_type, "release", default_val,
			     PFA_TAG_DONE);
	}
	type = cast_new_qualified_type(raw_type, CAST_TQ_CONST);
	type = cast_new_reference_type(type);
	add_function(tl,
		     scn,
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "T(T &)",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_Constructor,
		     PFA_Scope, scope,
		     PFA_ReturnType, cast_new_type(CAST_TYPE_NULL),
		     PFA_Parameter, type, "o", NULL,
		     PFA_TAG_DONE);
	add_function(tl,
		     cast_new_scoped_name(flick_asprintf("~%s", seq_name),
					  NULL),
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "~T()",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_Scope, scope,
		     PFA_Spec, CAST_FUNC_VIRTUAL,
		     PFA_ReturnType, cast_new_type(CAST_TYPE_NULL),
		     PFA_TAG_DONE);
	
	add_function(tl,
		     cast_new_scoped_name("operator=", NULL),
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "T &operator=(T &)",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_Scope, scope,
		     PFA_ReturnType, cast_new_reference_type(raw_type),
		     PFA_Parameter, type, "rhs", NULL,
		     PFA_TAG_DONE);
	type2 = prim_collections[PRIM_COLLECTION_ULONG]->
		find_type("definition")->get_type();
	if( !string_elem ) {
		type = ptc->find_type("array_slice")->get_type();
		type = cast_new_reference_type(type);
		add_function(tl,
			     cast_new_scoped_name("operator[]", NULL),
			     PFA_Protection, current_protection,
			     PFA_FunctionKind, "T &operator[](ulong)",
			     PFA_DeclChannel, ch(cur_aoi_idx,
						 PG_CHANNEL_CLIENT_DECL),
			     PFA_ImplChannel, ch(cur_aoi_idx,
						 PG_CHANNEL_CLIENT_IMPL),
			     PFA_Scope, scope,
			     PFA_ReturnType, type,
			     PFA_Parameter, type2, "i", NULL,
			     PFA_TAG_DONE);
		type = ptc->find_type("array_slice")->get_type();
		type = cast_new_qualified_type(type, CAST_TQ_CONST);
		type = cast_new_reference_type(type);
		add_function(tl,
			     cast_new_scoped_name("operator[]", NULL),
			     PFA_Protection, current_protection,
			     PFA_FunctionKind,
			     "const T &operator[](ulong) const",
			     PFA_DeclChannel, ch(cur_aoi_idx,
						 PG_CHANNEL_CLIENT_DECL),
			     PFA_ImplChannel, ch(cur_aoi_idx,
						 PG_CHANNEL_CLIENT_IMPL),
			     PFA_Scope, scope,
			     PFA_Spec, CAST_FUNC_CONST,
			     PFA_ReturnType, type,
			     PFA_Parameter, type2, "i", NULL,
			     PFA_TAG_DONE);
	}
	add_function(tl,
		     cast_new_scoped_name("allocbuf", NULL),
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "allocbuf(ulong)",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_Scope, scope,
		     PFA_StorageClass, CAST_SC_STATIC,
		     PFA_ReturnType, elem_type,
		     PFA_Parameter, type2, "size", NULL,
		     PFA_TAG_DONE);
	add_function(tl,
		     cast_new_scoped_name("freebuf", NULL),
		     PFA_FunctionKind, "freebuf(T)",
		     PFA_Protection, current_protection,
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_Scope, scope,
		     PFA_StorageClass, CAST_SC_STATIC,
		     PFA_ReturnType, cast_new_type(CAST_TYPE_VOID),
		     PFA_Parameter, elem_type, "buffer", NULL,
		     PFA_TAG_DONE);
	add_function(tl,
		     cast_new_scoped_name("_allocate_buffer", NULL),
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "_allocate_buffer(ulong)",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_Scope, scope,
		     PFA_Spec, CAST_FUNC_VIRTUAL,
		     PFA_ReturnType, cast_new_type(CAST_TYPE_VOID),
		     PFA_Parameter, type2, "length", NULL,
		     PFA_TAG_DONE);
	add_function(tl,
		     cast_new_scoped_name("_deallocate_buffer", NULL),
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "_deallocate_buffer()",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_Scope, scope,
		     PFA_Spec, CAST_FUNC_VIRTUAL,
		     PFA_ReturnType, cast_new_type(CAST_TYPE_VOID),
		     PFA_TAG_DONE);
	type = prim_collections[PRIM_COLLECTION_BOOLEAN]->
		find_type("definition")->get_type();
	add_function(tl,
		     cast_new_scoped_name("release", NULL),
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "bool release()",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_Scope, scope,
		     PFA_Spec, CAST_FUNC_CONST,
		     PFA_ReturnType, type,
		     PFA_TAG_DONE);
	add_function(tl,
		     cast_new_scoped_name("maximum", NULL),
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "ulong maximum()",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_Scope, scope,
		     PFA_Spec, CAST_FUNC_CONST,
		     PFA_ReturnType, type2,
		     PFA_TAG_DONE);
	add_function(tl,
		     cast_new_scoped_name("length", NULL),
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "ulong length()",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_Scope, scope,
		     PFA_Spec, CAST_FUNC_CONST,
		     PFA_ReturnType, type2,
		     PFA_TAG_DONE);
	add_function(tl,
		     cast_new_scoped_name("length", NULL),
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "length(ulong)",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_Scope, scope,
		     PFA_ReturnType, cast_new_type(CAST_TYPE_VOID),
		     PFA_Parameter, type2, "length", NULL,
		     PFA_TAG_DONE);
	add_function(tl,
		     cast_new_scoped_name("_shrink_buffer", NULL),
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "_shrink_buffer(ulong, ulong)",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_Scope, scope,
		     PFA_Spec, CAST_FUNC_VIRTUAL,
		     PFA_ReturnType, cast_new_type(CAST_TYPE_VOID),
		     PFA_Parameter, type2, "length1", NULL,
		     PFA_Parameter, type2, "length2", NULL,
		     PFA_TAG_DONE);
	add_function(tl,
		     cast_new_scoped_name("get_buffer", NULL),
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "T get_buffer(bool)",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_Scope, scope,
		     PFA_ReturnType, elem_type,
		     PFA_Parameter, type, "orphan",
		     cast_new_init_expr(cast_new_expr_lit_int(0,0)),
		     PFA_TAG_DONE);
	if( raw_elem_type->kind == CAST_TYPE_POINTER ) {
		type = cast_new_qualified_type(raw_elem_type->cast_type_u_u.
					       pointer_type.target,
					       CAST_TQ_CONST);
		type = cast_new_pointer_type(type);
	} else {
		type = cast_new_qualified_type(raw_elem_type, CAST_TQ_CONST);
	}
	type = cast_new_pointer_type(type);
	add_function(tl,
		     cast_new_scoped_name("get_buffer", NULL),
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "const T get_buffer()",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_Scope, scope,
		     PFA_Spec, CAST_FUNC_CONST,
		     PFA_ReturnType, type,
		     PFA_TAG_DONE);
	cast_del_scope_name(&current_scope_name);
}

/* End of file. */

