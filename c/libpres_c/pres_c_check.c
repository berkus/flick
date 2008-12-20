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
#include <stdio.h>
#include <string.h>
#include <mom/libmeta.h>
#include <mom/compiler.h>
#include <mom/libmint.h>
#include <mom/libaoi.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#define CHECK_MINT_REF(ref)					\
	assert((ref) >= 0);					\
	assert((ref) < (signed int) pres->mint.defs.defs_len);

#define CHECK_CAST_REF(ref)					\
	assert((ref) >= 0);					\
	assert((ref) < (signed int) pres->cast.cast_scope_len);

#define CHECK_STUB_CAST_REF(ref)				\
	assert((ref) >= 0);					\
	assert((ref) < (signed int) pres->stubs_cast.cast_scope_len);

#define CHECK_OPT_STRING(str) assert((str) != 0);

#define CHECK_STRING(str) assert((str) != 0); assert(strlen((str)) > 0);

struct inline_state
{
	enum {
		AGGREGATE,
		FUNC,
		SINGLE
	} which;
	union {
		cast_aggregate_type *stype;
		struct cast_func_type *ftype;
		cast_type *ttype;
	} u;
};

static void check_mapping(pres_c_1 *pres, pres_c_mapping map,
			  cast_type ctype, mint_ref itype);

static cast_type *is_type(struct inline_state *is, int slot)
{
	switch (is->which) {
	case FUNC:
		if (slot >= 0) {
			assert(slot < (signed int)
			       is->u.ftype->params.params_len);
			assert(is->u.ftype->params.params_val);
			return &is->u.ftype->params.params_val[slot].type;
		} else {
			assert(slot == pres_c_func_return_index);
			return &is->u.ftype->return_type;
		}
		
	case AGGREGATE:
		assert(slot >= 0);
		assert(slot < (signed int)is->u.stype->scope.cast_scope_len);
		assert(is->u.stype->scope.cast_scope_val);
		return &is->u.stype->scope.cast_scope_val[slot].u.
			cast_def_u_u.var_def.type;

	case SINGLE:
		assert(slot == 0);
		return is->u.ttype;
		
	default:
		panic("In is_type(), invalid inline state (discrim = %d).",
		      slot);
	}
}

static cast_type is_get_type(struct inline_state *is, int slot)
{
	cast_type ctype = *is_type(is, slot);
	assert(ctype != 0);
	return ctype;
}

static void check_alloc_flags(pres_c_alloc_flags flags)
{
}

static void check_alloc(pres_c_allocation *alloc)
{
	int i;
	for (i = 0; i < PRES_C_DIRECTIONS; i++) {
		if (alloc->cases[i].allow == PRES_C_ALLOCATION_ALLOW) {
			pres_c_allocation_case *acase
				= &(alloc->cases[i].pres_c_allocation_u_u.val);
			check_alloc_flags(acase->flags);
			switch (acase->allocator.kind) {
			case PRES_C_ALLOCATOR_DONTCARE:
			case PRES_C_ALLOCATOR_STATIC:
				break;
			case PRES_C_ALLOCATOR_OUTOFLINE:
				CHECK_STRING(acase->allocator.
					     pres_c_allocator_u.ool_name);
				break;
			case PRES_C_ALLOCATOR_NAME:
				CHECK_STRING(acase->allocator.
					     pres_c_allocator_u.name);
				break;
			default:
				panic("In check_alloc: "
				      "Invalid allocator kind detected.");
			}
			if (acase->alloc_init)
				cast_check_init(acase->alloc_init);
		}
	}
}

static void check_stub_op_flags(pres_c_stub_op_flags op_flags)
{
	u_int valid_bits = (  PRES_C_STUB_OP_FLAG_ONEWAY
			    | PRES_C_STUB_OP_FLAG_IDEMPOTENT
			   );
	
	/* Assert that no invalid flag bits are set. */
	assert((op_flags & ~valid_bits) == 0);
}

static void check_inline(pres_c_1 *pres, pres_c_inline inl,
			 struct inline_state *is, mint_ref itype)
{
	mint_def *idef;

	assert(inl);
	CHECK_MINT_REF(itype);
	idef = &(pres->mint.defs.defs_val[itype]);
	
	switch (inl->kind) {
	case PRES_C_INLINE_ATOM: {
		cast_type ctype
			= is_get_type(is, inl->pres_c_inline_u_u.atom.index);
		
		check_mapping(pres,
			      inl->pres_c_inline_u_u.atom.mapping,
			      ctype,
			      itype);
		break;
	}
	
	case PRES_C_INLINE_STRUCT: {
		pres_c_inline_struct *sinl
			= &(inl->pres_c_inline_u_u.struct_i);
		mint_struct_def *istruct
			= &(idef->mint_def_u.struct_def);
		
		int		slot_index;
		int		pres_c_slot_found;
		mint_ref	this_mint_ref;
		
		unsigned int	i, j;
		
		/*
		 * We require that the associated MINT node be a struct type,
		 * and that every slot in that MINT be managed by exactly one
		 * PRES_C_INLINE_STRUCT slot.
		 *
		 * We may have more PRES_C_INLINE_STRUCT slots than MINT_STRUCT
		 * slots; in that case, the ``extra'' PRES_C slots will
		 * necessarily correspond to no MINT representation.
		 */
		assert(idef->kind == MINT_STRUCT);
		
		for (i = 0; i < istruct->slots.slots_len; ++i) {
			/*
			 * Find the PRES_C slot for the i'th MINT slot.
			 */
			pres_c_slot_found = 0;
			
			for (j = 0;
			     j < sinl->slots.slots_len;
			     ++j) {
				/*
				 * Verify that the j'th PRES_C slot refers to a
				 * valid MINT struct slot, or to no slot.
				 */
				slot_index = (sinl->slots.slots_val[j].
					      mint_struct_slot_index);
				assert(((slot_index >= 0)
					&& (slot_index
					    < ((signed int)
					       istruct->slots.slots_len)))
				       ||
				       (slot_index == mint_slot_index_null));
				
				if (slot_index == (signed int) i) {
					/*
					 * The j'th PRES_C slot corresponds to
					 * the i'th MINT slot.  Assert that we
					 * haven't yet made a connection to the
					 * i'th MINT slot, and then set the
					 * flag (so that we'll die if we find
					 * another association).
					 */
					assert(!pres_c_slot_found);
					pres_c_slot_found = 1;
				}
			}
			/*
			 * Assert that we found a (single) PRES_C <--> MINT
			 * slot correspondence.
			 */
			assert(pres_c_slot_found);
		}
		
		/*
		 * Now that we know the PRES_C <--> MINT slot associations are
		 * OK, we can descend and check the associated PRES_C inlines.
		 */
		for (i = 0; i < sinl->slots.slots_len; ++i) {
			assert(sinl->slots.slots_val[i].inl);
			if (sinl->slots.slots_val[i].mint_struct_slot_index
			    != mint_slot_index_null)
				/*
				 * Default: The PRES_C slot refers to a MINT
				 * structure slot.
				 */
				this_mint_ref
					= (istruct->
					   slots.slots_val[
						   sinl->
						   slots.slots_val[i].
						   mint_struct_slot_index]);
			else
				/*
				 * Special case: The PRES_C slot refers to no
				 * underlying MINT structure slot.  We'll check
				 * the PRES_C inline anyway, by passing down a
				 * reference to our basic MINT_VOID node.
				 */
				this_mint_ref = pres->
						mint.standard_refs.void_ref;
			
			check_inline(pres,
				     sinl->slots.slots_val[i].inl,
				     is,
				     this_mint_ref);
		}
		
		break;
	}
	
	case PRES_C_INLINE_FUNC_PARAMS_STRUCT: {
		pres_c_inline_func_params_struct *fpinl
			= &(inl->pres_c_inline_u_u.func_params_i);
		mint_struct_def *istruct
			= &(idef->mint_def_u.struct_def);
		
		int		slot_index;
		int		pres_c_slot_found;
		mint_ref	this_mint_ref;
		
		unsigned int	i, j;
		
		/*
		 * We require that the associated MINT node be a struct type,
		 * and that every slot in that MINT be managed by exactly one
		 * PRES_C_INLINE_FUNC_PARAMS_STRUCT slot.
		 *
		 * We may have more PRES_C_INLINE_FUNC_PARAMS_STRUCT slots than
		 * MINT_STRUCT slots; in that case, the ``extra'' PRES_C slots
		 * will necessarily correspond to no MINT representation.
		 */
		assert(idef->kind == MINT_STRUCT);
		
		for (i = 0; i < istruct->slots.slots_len; ++i) {
			/*
			 * Find the PRES_C slot for the i'th MINT slot.
			 */
			pres_c_slot_found = 0;
			
			for (j = 0;
			     j < fpinl->slots.slots_len;
			     ++j) {
				/*
				 * Verify that the j'th PRES_C slot refers to a
				 * valid MINT struct slot, or to no slot.
				 */
				slot_index = (fpinl->slots.slots_val[j].
					      mint_struct_slot_index);
				assert(((slot_index >= 0)
					&& (slot_index
					    < ((signed int)
					       istruct->slots.slots_len)))
				       ||
				       (slot_index == mint_slot_index_null));
				
				if (slot_index == (signed int) i) {
					/*
					 * The j'th PRES_C slot corresponds to
					 * the i'th MINT slot.  Assert that we
					 * haven't yet made a connection to the
					 * i'th MINT slot, and then set the
					 * flag (so that we'll die if we find
					 * another association).
					 */
					assert(!pres_c_slot_found);
					pres_c_slot_found = 1;
				}
			}
			
			/*
			 * PRES_C_INLINE_FUNC_PARAMS_STRUCTs are more compli-
			 * cated than ordinary PRES_C_INLINE_STRUCTs, because
			 * the former has a retun slot that must be checked in
			 * addition to the ``ordinary'' slots.
			 */
			if (fpinl->return_slot) {
				/*
				 * Verify that the `return_slot' refers to a
				 * valid MINT struct slot, or to no slot.
				 */
				slot_index = (fpinl->return_slot->
					      mint_struct_slot_index);
				assert(((slot_index >= 0)
					&& (slot_index
					    < ((signed int)
					       istruct->slots.slots_len)))
				       ||
				       (slot_index == mint_slot_index_null));
				
				if (slot_index == (signed int) i) {
					/*
					 * The `return_slot' corresponds to the
					 * i'th MINT slot.  Assert that we
					 * haven't yet made a connection to the
					 * i'th MINT slot, and then set the
					 * flag (so that we'll die if we find
					 * another association).
					 */
					assert(!pres_c_slot_found);
					pres_c_slot_found = 1;
				}
			}
			
			/*
			 * Assert that we found a (single) PRES_C <--> MINT
			 * slot correspondence.
			 */
			assert(pres_c_slot_found);
		}
		
		/*
		 * Now that we know the PRES_C <--> MINT slot associations are
		 * OK, we can descend and check the associated PRES_C inlines.
		 */
		for (i = 0; i < fpinl->slots.slots_len; ++i) {
			assert(fpinl->slots.slots_val[i].inl);
			if (fpinl->slots.slots_val[i].mint_struct_slot_index
			    != mint_slot_index_null)
				/*
				 * Default: The PRES_C slot refers to a MINT
				 * structure slot.
				 */
				this_mint_ref
					= (istruct->
					   slots.slots_val[
						   fpinl->
						   slots.slots_val[i].
						   mint_struct_slot_index]);
			else
				/*
				 * Special case: The PRES_C slot refers to no
				 * underlying MINT structure slot.  We'll check
				 * the PRES_C inline anyway, by passing down a
				 * reference to our basic MINT_VOID node.
				 */
				this_mint_ref = pres->
						mint.standard_refs.void_ref;
			
			check_inline(pres,
				     fpinl->slots.slots_val[i].inl,
				     is,
				     this_mint_ref);
		}
		if (fpinl->return_slot) {
			assert(fpinl->return_slot->inl);
			if (fpinl->return_slot->mint_struct_slot_index
			    != mint_slot_index_null)
				/*
				 * Default: The PRES_C slot refers to a MINT
				 * structure slot.
				 */
				this_mint_ref
					= (istruct->
					   slots.slots_val[
						   fpinl->return_slot->
						   mint_struct_slot_index]);
			else
				/*
				 * Special case: The PRES_C slot refers to no
				 * underlying MINT structure slot.
				 */
				this_mint_ref = pres->
						mint.standard_refs.void_ref;
			
			check_inline(pres,
				     fpinl->return_slot->inl,
				     is,
				     this_mint_ref);
		}
		
		break;
	}
	
	case PRES_C_INLINE_HANDLER_FUNC: {
		pres_c_inline_handler_func *hfinl =
			&(inl->pres_c_inline_u_u.handler_i);
		unsigned int i;
		
		/*
		 * We require that the associated MINT node be a struct type
		 * for the request or reply parameters and return value.
		 * However, the PRES_C_HANDLER_FUNC does not process the MINT
		 * further.
		 *
		 * The PRES_C_INLINE_STRUCT slots here MUST NOT correspond to
		 * any MINT_STRUCT slots; they are presentation-only entities.
		 */
		assert(idef->kind == MINT_STRUCT);
		
		/*
		 * We descend and check the associated PRES_C inlines.
		 */
		for (i = 0; i < hfinl->slots.slots_len; ++i) {
			assert(hfinl->slots.slots_val[i].inl);
			assert(hfinl->slots.slots_val[i].mint_struct_slot_index
			       == mint_slot_index_null);
			
			check_inline(pres,
				     hfinl->slots.slots_val[i].inl,
				     is,
				     pres->mint.standard_refs.void_ref);
		}
		
		break;
	}
	
	case PRES_C_INLINE_VIRTUAL_UNION: {
		pres_c_inline_virtual_union *vuinl =
			&(inl->pres_c_inline_u_u.virtual_union);
		mint_union_def *iunion = &idef->mint_def_u.union_def;
		unsigned int i;
		assert(idef->kind == MINT_UNION);
		assert(vuinl->cases.cases_len == iunion->cases.cases_len);
		
		check_inline(pres, vuinl->discrim, is, iunion->discrim);
		for (i = 0; i < vuinl->cases.cases_len; i++) {
			check_inline(pres, vuinl->cases.cases_val[i],
				     is, iunion->cases.cases_val[i].var);
		}
		if (vuinl->dfault)
			check_inline(pres, vuinl->dfault, is, iunion->dfault);
		
		CHECK_STRING(vuinl->arglist_name);
		break;
	}				     
	
	case PRES_C_INLINE_STRUCT_UNION: {
		pres_c_inline_struct_union *suinl =
			&(inl->pres_c_inline_u_u.struct_union);
		
		int mint_union_cases_len;
		cast_type discrim_ctype, union_ctype;
		int case_union_ctype_index;
		
		int i;
		
		/* This mapping must correspond to a MINT_UNION. */
		assert(idef->kind == MINT_UNION);
		
		/* Check the discriminator. */
		discrim_ctype = is_get_type(is, suinl->discrim.index);
		check_mapping(pres,
			      suinl->discrim.mapping,
			      discrim_ctype,
			      idef->mint_def_u.union_def.discrim);
		
		/* Check the number of cases.  Note that the CAST union def may
		   have fewer cases, because some ot the variants of the MINT
		   union may be void. */
		mint_union_cases_len = idef->
				       mint_def_u.union_def.cases.cases_len;
		assert(mint_union_cases_len
		       == (signed int)suinl->cases.cases_len);
		
		/* Check each case. */
		union_ctype = is_get_type(is, suinl->union_index);
		for (i = 0; i < mint_union_cases_len; ++i) {
			/* Check the MINT constant for this case. */
			mint_const_check(idef->mint_def_u.union_def.cases.
					 cases_val[i].val);
			
			/* Can't check the CAST constant - it could be nil */
			
			/* Get the index into the CAST union for this case. */
			case_union_ctype_index = suinl->
						 cases.cases_val[i].index;
			
			/* Check that the index into the CAST union is OK. */
			assert((case_union_ctype_index == -1) ||
			       ((case_union_ctype_index >= 0) &&
				(case_union_ctype_index <
				 (signed int)(union_ctype->
				  cast_type_u_u.agg_type.scope.
					      cast_scope_len))
				       ));
			
			/* Check the mapping for this case. */
			if (case_union_ctype_index == -1) {
				/* Do nothing.  Perhaps we should check that
				   the MINT type for this case is void.  But
				   even if it's not, we won't generate bad
				   code. */
			} else
				check_mapping(pres,
					      (suinl->
					       cases.cases_val[i].mapping),
					      (union_ctype->
					       cast_type_u_u.agg_type.scope.
					       cast_scope_val
					       [case_union_ctype_index].u.
					       cast_def_u_u.var_def.type),
					      (idef->
					       mint_def_u.union_def.cases.
					       cases_val[i].var)
					);
		}
		
		/* Check the default case, too. */
		if (suinl->dfault) {
			/* Assert that we have a default case in the MINT. */
			assert(idef->mint_def_u.union_def.dfault != -1);
			
			/* Get the index into the CAST union for this case. */
			case_union_ctype_index = suinl->dfault->index;
			
			/* Check that the index into the CAST union is OK. */
			assert((case_union_ctype_index == -1) ||
			       ((case_union_ctype_index >= 0) &&
				(case_union_ctype_index <
				 (signed int)(union_ctype->
				  cast_type_u_u.agg_type.scope.
					      cast_scope_len))
				       ));
			
			/* Check the mapping for this case. */
			if (case_union_ctype_index == -1) {
				/* Do nothing.  Perhaps we should check that
				   the MINT type for this case is void.  But
				   even if it's not, we won't generate bad
				   code. */
			} else
				check_mapping(pres,
					      suinl->dfault->mapping,
					      (union_ctype->
					       cast_type_u_u.agg_type.scope.
					       cast_scope_val
					       [case_union_ctype_index].u.
					       cast_def_u_u.var_def.type),
					      idef->mint_def_u.union_def.dfault
					);
		} else
			/* No default case in the inline, so assert that there
			   is no default case in the MINT either. */
			assert(idef->mint_def_u.union_def.dfault == -1);
		
		break;
	}
	
	case PRES_C_INLINE_VOID_UNION: {
		pres_c_inline_void_union *vuinl =
			&inl->pres_c_inline_u_u.void_union;

		int mint_union_cases_len;
		cast_type discrim_ctype;
		
		int i;
		
		/* This mapping must correspond to a MINT_UNION. */
		assert(idef->kind == MINT_UNION);
		
		/* Check the discriminator. */
		discrim_ctype = is_get_type(is, vuinl->discrim.index);
		check_mapping(pres,
			      vuinl->discrim.mapping,
			      discrim_ctype,
			      idef->mint_def_u.union_def.discrim);
		
		/* Check the number of cases.  Note that the CAST union def may
		   have fewer cases, because some ot the variants of the MINT
		   union may be void. */
		mint_union_cases_len = idef->
				       mint_def_u.union_def.cases.cases_len;
		assert(mint_union_cases_len
		       == (signed int)vuinl->cases.cases_len);
		
		/* Check each case. */
		for (i = 0; i < mint_union_cases_len; ++i) {
			/* Check the MINT constant for this case. */
			mint_const_check(idef->mint_def_u.union_def.cases.
					 cases_val[i].val);
			
			if (vuinl->cases.cases_val[i].case_value)
				cast_check_expr(vuinl->cases.cases_val[i].
						case_value);
			
			/* Check the mapping for this case. */
			check_mapping(pres,
				      vuinl->cases.cases_val[i].mapping,
				      vuinl->cases.cases_val[i].type,
				      (idef->
				       mint_def_u.union_def.cases.
				       cases_val[i].var));
		}
		
		/* Check the default case, too. */
		if (vuinl->dfault) {
			/* Assert that we have a default case in the MINT. */
			assert(idef->mint_def_u.union_def.dfault != -1);
			
			cast_check_expr(vuinl->dfault->case_value);
			
			check_mapping(pres,
				      vuinl->dfault->mapping,
				      vuinl->dfault->type,
				      idef->mint_def_u.union_def.dfault);
		} else
			/* No default case in the inline, so assert that there
			   is no default case in the MINT either. */
			assert(idef->mint_def_u.union_def.dfault == -1);
		
		break;
	}
	
	case PRES_C_INLINE_COLLAPSED_UNION: {
		pres_c_inline_collapsed_union *cuinl =
			&inl->pres_c_inline_u_u.collapsed_union;
		mint_ref case_r;
		
		assert(idef->kind == MINT_UNION);
		mint_const_check(cuinl->discrim_val);
		case_r = mint_find_union_case(&pres->mint, itype,
					      cuinl->discrim_val);
		CHECK_MINT_REF(case_r);
		check_inline(pres, cuinl->selected_case, is, case_r);
		break;
	}
	
	case PRES_C_INLINE_TYPED: {
		pres_c_inline_typed *tinl =
			&inl->pres_c_inline_u_u.typed;
		mint_typed_def *ityped = &idef->mint_def_u.typed_def;
		
		assert(idef->kind == MINT_TYPED);
		
		/* check the tag and the data's inlines */
		check_inline(pres, tinl->tag, is, ityped->tag);
		check_inline(pres, tinl->inl, is, ityped->ref);
		break;
	}
	
	case PRES_C_INLINE_XLATE: {
		pres_c_inline_xlate *xinl = &inl->pres_c_inline_u_u.xlate;
		cast_type *ctypep = is_type(is, xinl->index);
		cast_type octype = *ctypep;
		
		assert(xinl->internal_ctype);
		*ctypep = xinl->internal_ctype;
		check_inline(pres, xinl->sub, is, itype);
		*ctypep = octype;
		
		CHECK_OPT_STRING(xinl->translator);
		CHECK_OPT_STRING(xinl->destructor);
		break;
	}
	
	case PRES_C_INLINE_ASSIGN: {
		pres_c_inline_assign *ainl = &inl->pres_c_inline_u_u.assign;
		cast_type ctype;
		
		check_inline(pres, ainl->sub, is, itype);
		
		ctype = is_get_type(is, ainl->index);
		assert((ctype->kind == CAST_TYPE_PRIMITIVE)
		       || (ctype->kind == CAST_TYPE_NAME));
		
		cast_check_expr(ainl->value);
		break;
	}
	
	case PRES_C_INLINE_COND: {
		pres_c_inline_cond *cinl = &inl->pres_c_inline_u_u.cond;
		cast_type ctype;
		
		ctype = is_get_type(is, cinl->index);
		assert((ctype->kind == CAST_TYPE_PRIMITIVE)
		       || (ctype->kind == CAST_TYPE_NAME));
		
		check_inline(pres, cinl->true_inl, is, itype);
		check_inline(pres, cinl->false_inl, is, itype);
		
		break;
	}
	
	case PRES_C_INLINE_MESSAGE_ATTRIBUTE: {
		pres_c_inline_message_attribute *msg_attr =
			&(inl->pres_c_inline_u_u.msg_attr);
		assert((msg_attr->kind == PRES_C_MESSAGE_ATTRIBUTE_FLAGS)
		       || (msg_attr->kind == PRES_C_MESSAGE_ATTRIBUTE_TIMEOUT)
		       || (msg_attr->kind
			   == PRES_C_MESSAGE_ATTRIBUTE_SEQUENCE_RECEIVED)
		       || (msg_attr->kind
			   == PRES_C_MESSAGE_ATTRIBUTE_CLIENT_REFERENCE)
		       || (msg_attr->kind
			   == PRES_C_MESSAGE_ATTRIBUTE_SERVERCOPY));
		break;
	}
	
	case PRES_C_INLINE_ALLOCATION_CONTEXT: {
		pres_c_inline_allocation_context *acontext =
			&(inl->pres_c_inline_u_u.acontext);
		/* Default length itype = unsigned 32-bit integer. */
		mint_ref lenref = pres->mint.standard_refs.unsigned32_ref;
		mint_ref boolref = pres->mint.standard_refs.bool_ref;
		/* Default element itype = current itype. */
		mint_ref elemref = itype;
		
		/* For arrays, we can get the real length & element types. */
		if (idef->kind == MINT_ARRAY) {
			lenref = idef->mint_def_u.array_def.length_type;
			elemref = idef->mint_def_u.array_def.element_type;
		}
		
		/*
		 * For MINT_VOIDs, this is just a alloc/dealloc pass,
		 * so all itypes we pass down should be void.
		 */
		if (idef->kind == MINT_VOID) {
			lenref = pres->mint.standard_refs.void_ref;
			boolref = pres->mint.standard_refs.void_ref;
			elemref = pres->mint.standard_refs.void_ref;
		}
		
		/* Check our name. */
		CHECK_STRING(acontext->arglist_name);
		
		/* Check the inlines that we have. */
		if (acontext->offset)
			check_inline(pres, acontext->offset, is, lenref);
		if (acontext->length)
			check_inline(pres, acontext->length, is, lenref);
		if (acontext->min_len)
			check_inline(pres, acontext->min_len, is, lenref);
		if (acontext->max_len)
			check_inline(pres, acontext->max_len, is, lenref);
		if (acontext->alloc_len)
			check_inline(pres, acontext->alloc_len, is, lenref);
		if (acontext->min_alloc_len)
			check_inline(pres, acontext->min_alloc_len, is,
				     lenref);
		if (acontext->max_alloc_len)
			check_inline(pres, acontext->max_alloc_len, is,
				     lenref);
		if (acontext->release)
			check_inline(pres, acontext->release, is, boolref);
		if (acontext->terminator)
			check_inline(pres, acontext->terminator, is, elemref);
		if (acontext->mustcopy)
			check_inline(pres, acontext->mustcopy, is, boolref);
		
		/* Check the allocation node */
		check_alloc(&acontext->alloc);
		
		/*
		 * XXX - The following are not currently implemented, so we
		 * also make sure they aren't being used or set.
		 * See pres_c.x for more details.
		 */
		assert(acontext->offset == 0);
		assert(acontext->max_alloc_len == 0);
		assert(acontext->overwrite == 0);
		assert(acontext->owner == 0);
		
		check_inline(pres, acontext->ptr, is, itype);
		break;
	}
		
	case PRES_C_INLINE_TEMPORARY: {
		pres_c_temporary *temp = &inl->pres_c_inline_u_u.temp;
		
		CHECK_OPT_STRING(temp->name);
		assert(temp->ctype);
		assert(temp->is_const == 0 || temp->is_const == 1);
		CHECK_OPT_STRING(temp->prehandler);
		CHECK_OPT_STRING(temp->posthandler);
		check_mapping(pres, temp->map, temp->ctype, itype);
		
		break;
	}

	case PRES_C_INLINE_ILLEGAL:
		/*
		 * This node indicates that this inline (most likely a case
		 * of some union) should not occur -- i.e., it is an illegal
		 * inline at this point.
		 * Any associated MINT and CAST are ignored.
		 */
		break;
		
	default:
		panic("check_inline: unknown pres_c_inline_kind %d",
		      inl->kind);
	}
}

static void check_mapping(pres_c_1 *pres, pres_c_mapping map,
			  cast_type ctype, mint_ref itype)
{
	mint_def *idef;
	
	assert(map);
	assert(ctype);
	CHECK_MINT_REF(itype);
	idef = &(pres->mint.defs.defs_val[itype]);
	
	/* Dig down through the type qualifiers. */
	while (ctype->kind == CAST_TYPE_QUALIFIED)
		ctype = ctype->cast_type_u_u.qualified.actual;
	
	switch (map->kind) {
	case PRES_C_MAPPING_DIRECT:
		if (idef->kind == MINT_VOID) {
			assert(ctype->kind == CAST_TYPE_VOID);
			break;
		}
		assert((idef->kind == MINT_INTEGER) ||
		       (idef->kind == MINT_FLOAT) ||
		       (idef->kind == MINT_CHAR) ||
		       (idef->kind == MINT_SCALAR));
		assert(ctype->kind == CAST_TYPE_PRIMITIVE ||
		       ctype->kind == CAST_TYPE_NAME);
		break;
		
	case PRES_C_MAPPING_IGNORE:
		/*
		 * It is wrong to ignore non-void message data.
		 * We need to marshal/unmarshal the data even
		 * if it is not going to be presented.
		 */
		assert(idef->kind == MINT_VOID);
		break;
		
	case PRES_C_MAPPING_POINTER: {
		pres_c_mapping_pointer *pmap =
			&map->pres_c_mapping_u_u.pointer;
		ctype = cast_find_typedef_type(&(pres->cast), ctype);
		assert(ctype);
		
		assert(ctype->kind == CAST_TYPE_POINTER ||
		       ctype->kind == CAST_TYPE_ARRAY);
		if (ctype->kind == CAST_TYPE_ARRAY)
			check_mapping(pres, pmap->target,
				      (ctype->cast_type_u_u.array_type.
				       element_type),
				      itype);
		else
			check_mapping(pres, pmap->target,
				      ctype->cast_type_u_u.pointer_type.target,
				      itype);
		
		CHECK_STRING(pmap->arglist_name);
		break;
	}
	
	case PRES_C_MAPPING_VAR_REFERENCE: {
		pres_c_mapping_var_reference *rmap =
			&map->pres_c_mapping_u_u.var_ref;
		ctype = cast_find_typedef_type(&(pres->cast), ctype);
		assert(ctype);
		
		assert(ctype->kind == CAST_TYPE_REFERENCE);
		check_mapping(pres, rmap->target,
			      ctype->cast_type_u_u.reference_type.target,
			      itype);
		break;
	}
	
	case PRES_C_MAPPING_INTERNAL_ARRAY: {
		/*
		 * XXX --- We don't enforce the restriction that this node
		 * can only appear as a child (direct or indirect) of an
		 * `inline_*_array' node.
		 */
		pres_c_mapping_internal_array *amap
			= &(map->pres_c_mapping_u_u.internal_array);
		
		ctype = cast_find_typedef_type(&(pres->cast), ctype);
		assert(ctype);
		
		assert((ctype->kind == CAST_TYPE_POINTER)
		       || (ctype->kind == CAST_TYPE_ARRAY));
		
		if (idef->kind != MINT_VOID) {
			assert(idef->kind == MINT_ARRAY);
			itype = idef->mint_def_u.array_def.element_type;
		}
		
		if (ctype->kind == CAST_TYPE_ARRAY)
			check_mapping(pres,
				      amap->element_mapping,
				      (ctype->cast_type_u_u.array_type.
				       element_type),
				      itype);
		else
			check_mapping(pres,
				      amap->element_mapping,
				      ctype->cast_type_u_u.pointer_type.target,
				      itype);
		
		CHECK_STRING(amap->arglist_name);
		break;
	}
	
	case PRES_C_MAPPING_STRUCT: {
		struct inline_state is;
		
		ctype = cast_find_typedef_type(&(pres->cast), ctype);
		assert(ctype);
		
		assert(ctype->kind == CAST_TYPE_AGGREGATE);
		is.which = AGGREGATE;
		is.u.stype = &ctype->cast_type_u_u.agg_type;
		check_inline(pres, map->pres_c_mapping_u_u.struct_i, &is,
			     itype);
		break;
	}
	
	case PRES_C_MAPPING_XLATE: {
		pres_c_mapping_xlate *xmap = &map->pres_c_mapping_u_u.xlate;
		
		check_mapping(pres, xmap->internal_mapping,
			      xmap->internal_ctype, itype);
		
		CHECK_OPT_STRING(xmap->translator);
		CHECK_OPT_STRING(xmap->destructor);
		break;
	}
	
	case PRES_C_MAPPING_REFERENCE: {
		assert(idef->kind == MINT_INTERFACE);
		
		/*
		 * XXX --- I (Eric Eide) think that requiring a named type at
		 * this point is unnecessary.  Why can't object references be
		 * essentially anything: pointers, integers (indices),
		 * structures, ...?
		 */
		/* assert(ctype->kind == CAST_TYPE_NAME); */
		/*assert(ctype->cast_type_u_u.name.
		  cast_scoped_name_val[0].name != 0);*/
		switch (map->pres_c_mapping_u_u.ref.kind) {
		case PRES_C_REFERENCE_COPY:
		case PRES_C_REFERENCE_MOVE:
		case PRES_C_REFERENCE_COPY_AND_CONVERT:
			break;
		default:
			panic("pres_c_check: Invalid mapping reference!");
		}
		/* this means how many am I sending/receiving - can't be 0 */
		assert(map->pres_c_mapping_u_u.ref.ref_count > 0);
		CHECK_OPT_STRING(map->pres_c_mapping_u_u.ref.arglist_name);
		break;
	}
	
	case PRES_C_MAPPING_TYPE_TAG:
		assert(idef->kind == MINT_TYPE_TAG);
		break;
		
	case PRES_C_MAPPING_TYPED: {
		mint_typed_def *typed_def = &(idef->mint_def_u.typed_def);
		mint_def *tag_def;
		mint_def *ref_def;
		
		assert(idef->kind == MINT_TYPED);
		/*
		 * Although the `MINT_TYPED' node has substructure, we can't
		 * really check it, because we don't have any more PRES_C.  In
		 * other words, whatever MINT structure exists here must be
		 * handled by the implementation of `mu_mapping_typed' in the
		 * back end.
		 *
		 * Really, all we should do here is check that the `MINT_TYPED'
		 * substructure is valid (well-formed MINT of any kind).  We
		 * try to do a little better, though, by checking for the only
		 * MINT that we currently use with `PRES_C_MAPPING_TYPED': a
		 * `MINT_TYPED' containing a `MINT_TYPE_TAG' and a `MINT_ANY'.
		 */
		CHECK_MINT_REF(typed_def->tag);
		CHECK_MINT_REF(typed_def->ref);
		
		tag_def = &(pres->mint.defs.defs_val[typed_def->tag]);
		ref_def = &(pres->mint.defs.defs_val[typed_def->ref]);
		
		if (tag_def->kind != MINT_TYPE_TAG)
			panic(("In `check_mapping', unexpected MINT kind (%d) "
			       "for `tag' field of a MINT_TYPED node."),
			      tag_def->kind);
		if (ref_def->kind != MINT_ANY)
			panic(("In `check_mapping', unexpected MINT kind (%d) "
			       "for `ref' field of a MINT_TYPED node."),
			      ref_def->kind);
		break;
	}
	
	case PRES_C_MAPPING_STUB: {
		while (ctype->kind == CAST_TYPE_QUALIFIED)
			ctype = ctype->cast_type_u_u.qualified.actual;
		if (ctype->kind == CAST_TYPE_POINTER) {
			ctype = ctype->cast_type_u_u.pointer_type.target;
			assert(ctype);
			while (ctype->kind == CAST_TYPE_QUALIFIED)
				ctype = ctype->cast_type_u_u.qualified.actual;
		}
		assert((ctype->kind == CAST_TYPE_NAME) ||
		       (ctype->kind == CAST_TYPE_STRUCT_NAME) ||
		       (ctype->kind == CAST_TYPE_UNION_NAME) ||
		       (ctype->kind == CAST_TYPE_ENUM_NAME));
		break;
	}
	
	case PRES_C_MAPPING_OPTIONAL_POINTER: {
		pres_c_mapping_optional_pointer *pmap =
			&(map->pres_c_mapping_u_u.optional_pointer);
		mint_array_def *iarray = &(idef->mint_def_u.array_def);
		mint_def *length_def;
		
		/* On the CAST side, this mapping corresponds to a pointer. */
		ctype = cast_find_typedef_type(&(pres->cast), ctype);
		assert(ctype);
		assert(ctype->kind == CAST_TYPE_POINTER);
		/* On the MINT side, we have a MINT array (a counted array
		   containing zero or one elements). */
		assert(idef->kind == MINT_ARRAY);
		
		/* Check that the length of the MINT array is an integer that
		   may be zero or one. */
		CHECK_MINT_REF(iarray->element_type);
		length_def = &(pres->mint.defs.defs_val[iarray->length_type]);
		assert(length_def->kind == MINT_INTEGER);
		assert(length_def->mint_def_u.integer_def.min == 0);
		assert(length_def->mint_def_u.integer_def.range == 1);
		
		/* Check the mapping of this pointer's target type. */
		check_mapping(pres,
			      pmap->target,
			      ctype->cast_type_u_u.pointer_type.target,
			      iarray->element_type);
		
		CHECK_STRING(pmap->arglist_name);
		break;
	}
	
	case PRES_C_MAPPING_SYSTEM_EXCEPTION:
		assert(idef->kind == MINT_SYSTEM_EXCEPTION);
		break;
		
	case PRES_C_MAPPING_DIRECTION: {
		pres_c_mapping_direction *dir_map =
			&(map->pres_c_mapping_u_u.direction);
		
		/* We should never see a direction mapping of unknown! */
		assert((dir_map->dir == PRES_C_DIRECTION_IN)
		       || (dir_map->dir == PRES_C_DIRECTION_INOUT)
		       || (dir_map->dir == PRES_C_DIRECTION_OUT)
		       || (dir_map->dir == PRES_C_DIRECTION_RETURN));
		
		check_mapping(pres,
			      dir_map->mapping,
			      ctype,
			      itype);
		break;
	}
	
	case PRES_C_MAPPING_SID:
		/*
		 * The `ctype' corresponding to a SID could be anything, so we
		 * can't really check anything about it here.
		 *
		 * SIDs are not part of an IDL-defined message and therefore
		 * don't have MINT representations.  So we can't say anything
		 * about the `itype' either.
		 */
		break;

	case PRES_C_MAPPING_ARGUMENT:
		CHECK_STRING(map->pres_c_mapping_u_u.argument.arglist_name);
		CHECK_STRING(map->pres_c_mapping_u_u.argument.arg_name);
		if (map->pres_c_mapping_u_u.argument.map)
			check_mapping(pres,
				      map->pres_c_mapping_u_u.argument.map,
				      ctype, itype);
		break;
		
	case PRES_C_MAPPING_MESSAGE_ATTRIBUTE: {
		pres_c_mapping_message_attribute *msg_attr =
			&(map->pres_c_mapping_u_u.message_attribute);
		assert((msg_attr->kind == PRES_C_MESSAGE_ATTRIBUTE_FLAGS)
		       || (msg_attr->kind == PRES_C_MESSAGE_ATTRIBUTE_TIMEOUT)
		       || (msg_attr->kind
			   == PRES_C_MESSAGE_ATTRIBUTE_SEQUENCE_RECEIVED)
		       || (msg_attr->kind
			   == PRES_C_MESSAGE_ATTRIBUTE_CLIENT_REFERENCE)
		       || (msg_attr->kind
			   == PRES_C_MESSAGE_ATTRIBUTE_SERVERCOPY));
		/*
		 * The `ctype' corresponding to an attribute could be
		 * anything, so we can't really check anything about it
		 * here.
		 *
		 * attributes are not part of an IDL-defined message and
		 * therefore don't have MINT representations.  So we can't
		 * say anything about the `itype' either.
		 */
		break;
	}
	
	case PRES_C_MAPPING_INITIALIZE:
		assert(map->pres_c_mapping_u_u.initialize.value);
		break;
		
	case PRES_C_MAPPING_ILLEGAL:
		/*
		 * This node indicates that this mapping (most likely a case
		 * of some union) should not occur -- i.e., it is an illegal
		 * mapping at this point.
		 * Any associated MINT and CAST are ignored.
		 */
		break;
		
	case PRES_C_MAPPING_PARAM_ROOT: {
		pres_c_mapping_param_root *root_map =
			&(map->pres_c_mapping_u_u.param_root);
		
		/* `root_map' may or may not specify an internal `ctype'. */
		check_mapping(pres,
			      root_map->map,
			      (root_map->ctype ? root_map->ctype : ctype),
			      itype);
		if (root_map->init)
			cast_check_init(root_map->init);
		break;
	}
	
	case PRES_C_MAPPING_ELSEWHERE:
		/*
		 * This is an assertion that the mapping has been or will be
		 * performed elsewhere, or by some other means than a regular
		 * PRES_C_MAPPING.  Thus, ignore any associated CAST/MINT.
		 */
		break;
		
	case PRES_C_MAPPING_SELECTOR: {
		/*
		 * Assert we have a CAST_AGGREGATE and a valid index range.
		 * See comment in mu_mapping_selector.cc.
		 */
		pres_c_mapping_selector *smap
			= &(map->pres_c_mapping_u_u.selector);
		cast_aggregate_type *at;
		int slot = smap->index;
		
		ctype = cast_find_typedef_type(&(pres->cast), ctype);
		assert(ctype);
		assert(ctype->kind == CAST_TYPE_AGGREGATE);
		at = &ctype->cast_type_u_u.agg_type;
		assert(slot >= 0);
		assert(slot < (signed int)at->scope.cast_scope_len);
		assert(at->scope.cast_scope_val[slot].u.kind == CAST_VAR_DEF);
		
		check_mapping(pres,
			      smap->mapping,
			      (at->scope.cast_scope_val[slot].u.
			       cast_def_u_u.var_def.type),
			      itype);
		
		break;
	}
	
	case PRES_C_MAPPING_TEMPORARY: {
		pres_c_temporary *temp = &map->pres_c_mapping_u_u.temp;
		
		CHECK_OPT_STRING(temp->name);
		assert(temp->ctype);
		assert(temp->is_const == 0 || temp->is_const == 1);
		CHECK_OPT_STRING(temp->prehandler);
		CHECK_OPT_STRING(temp->posthandler);
		check_mapping(pres, temp->map, temp->ctype, itype);
		
		break;
	}

	case PRES_C_MAPPING_SINGLETON: {
		pres_c_mapping_singleton *s
			= &map->pres_c_mapping_u_u.singleton;
		struct inline_state is;
		
		is.which = SINGLE;
		is.u.ttype = &ctype;
		
		check_inline(pres, s->inl, &is, itype);
		
		break;
	}

	default:
		panic("check_mapping: unknown pres_c_mapping_kind %d",
		      map->kind);
	}
}

static void check_func(pres_c_1 *pres, pres_c_func *func,
		       pres_c_skel *skel)
{
	cast_def *cdef = NULL;
	
	assert(func);
	assert(skel);
	switch (func->kind) {
	case PRES_C_SERVER_FUNC: {
		struct pres_c_server_func *sfunc = &func->pres_c_func_u.sfunc;
		struct inline_state is;
		
		CHECK_STUB_CAST_REF(sfunc->c_func);
		cdef = &pres->stubs_cast.cast_scope_val[sfunc->c_func];
		assert(cdef->u.kind == CAST_FUNC_DECL);
		is.which = FUNC;
		is.u.ftype = &cdef->u.cast_def_u_u.func_type;
		
		check_stub_op_flags(sfunc->op_flags);
		
		check_inline(pres, sfunc->request_i, &is,
			     skel->request_itype);
		if (sfunc->reply_i) {
			check_inline(pres, sfunc->reply_i, &is,
				     skel->reply_itype);
		} else {
			assert(sfunc->op_flags & PRES_C_STUB_OP_FLAG_ONEWAY);
		}
		
		check_inline(pres, sfunc->target_i, &is, sfunc->target_itype);
		
		if (sfunc->client_i) {
			check_inline(pres, sfunc->client_i, &is,
				     sfunc->client_itype);
		} else {
			assert(sfunc->client_itype == mint_ref_null);
		}
		break;
	}
	
	case PRES_C_RECEIVE_FUNC: {
		struct pres_c_receive_func *rfunc = &func->pres_c_func_u.rfunc;
		struct inline_state is;
		mint_ref toss_ref;
		pres_c_inline msg_inl;
		
		CHECK_STUB_CAST_REF(rfunc->c_func);
		cdef = &pres->stubs_cast.cast_scope_val[rfunc->c_func];
		assert(cdef->u.kind == CAST_FUNC_DECL);
		is.which = FUNC;
		is.u.ftype = &cdef->u.cast_def_u_u.func_type;
		
		check_stub_op_flags(rfunc->op_flags);
		
		toss_ref = skel->request_itype;
		msg_inl = rfunc->msg_i;
		descend_collapsed_union(pres, &toss_ref, &msg_inl);
		descend_collapsed_union(pres, &toss_ref, &msg_inl);
		descend_collapsed_union(pres, &toss_ref, &msg_inl);

		check_inline(pres, msg_inl, &is, rfunc->simple_msg_itype);
		
		check_inline(pres, rfunc->target_i, &is, rfunc->target_itype);
		
		check_inline(pres, rfunc->client_i, &is, rfunc->client_itype);
		break;
	}
	
	default:
		panic("check_func: unknown pres_c_func_kind %d",
		      func->kind);
	} /* end switch */
}

static void check_stub(pres_c_1 *pres, int n)
{
	pres_c_stub *stub = &(pres->stubs.stubs_val[n]);
	cast_def *cdef = NULL;
	
	assert(n >= 0);
	assert(n < (signed int)pres->stubs.stubs_len);
	
	switch (stub->kind)
	{
		case PRES_C_MARSHAL_STUB:
		{
			pres_c_marshal_stub *mstub
				= &stub->pres_c_stub_u.mstub;
			struct inline_state is;
			cdef = &pres->stubs_cast.cast_scope_val[mstub->c_func];

			CHECK_STUB_CAST_REF(mstub->c_func);
			assert(cdef->u.kind == CAST_FUNC_DECL);
			is.which = FUNC;
			is.u.ftype = &cdef->u.cast_def_u_u.func_type;

			CHECK_MINT_REF(mstub->itype);

			check_inline(pres, mstub->i, &is, mstub->itype);
			/*
			 * XXX --- We can't check the target inline because we
			 * don't have any MINT representation of the target
			 * object.  (m/u stubs don't *have* target objects!)
			 *
			 * check_inline(pres, mstub->target_i, &is, XXX);
			 */
			break;
		}
		case PRES_C_UNMARSHAL_STUB:
		{
			pres_c_marshal_stub *ustub
				= &stub->pres_c_stub_u.ustub;
			struct inline_state is;
			cdef = &pres->stubs_cast.cast_scope_val[ustub->c_func];

			CHECK_STUB_CAST_REF(ustub->c_func);
			assert(cdef->u.kind == CAST_FUNC_DECL);
			is.which = FUNC;
			is.u.ftype = &cdef->u.cast_def_u_u.func_type;

			CHECK_MINT_REF(ustub->itype);

			check_inline(pres, ustub->i, &is, ustub->itype);
			/*
			 * XXX --- We can't check the target inline because we
			 * don't have any MINT representation of the target
			 * object.  (m/u stubs don't *have* target objects!)
			 *
			 * check_inline(pres, ustub->target_i, &is, XXX);
			 */
			break;
		}
		case PRES_C_CLIENT_STUB:
		{
			pres_c_client_stub *cstub = &stub->pres_c_stub_u.cstub;
			struct inline_state is;
			cdef = &pres->stubs_cast.cast_scope_val[cstub->c_func];

			CHECK_STUB_CAST_REF(cstub->c_func);
			assert(cdef->u.kind == CAST_FUNC_DECL);
			is.which = FUNC;
			is.u.ftype = &cdef->u.cast_def_u_u.func_type;
			
			check_stub_op_flags(cstub->op_flags);
			
			CHECK_MINT_REF(cstub->request_itype);
			CHECK_MINT_REF(cstub->reply_itype);

			check_inline(pres, cstub->request_i, &is,
				     cstub->request_itype);
			check_inline(pres, cstub->reply_i, &is,
				     cstub->reply_itype);

			check_inline(pres, cstub->target_i, &is,
				     cstub->target_itype);
			if (cstub->client_i) {
				check_inline(pres, cstub->client_i, &is,
					     cstub->client_itype);
			} else {
				assert(cstub->client_itype == mint_ref_null);
			}
			break;
		}
		case PRES_C_CLIENT_SKEL:
		case PRES_C_SERVER_SKEL:
		{
			/* Client and Server skels use the same underlying
                           structure */
			pres_c_skel *skel;
			unsigned i;

			if (stub->kind == PRES_C_SERVER_SKEL)
				skel = &stub->pres_c_stub_u.sskel;
			else
				skel = &stub->pres_c_stub_u.cskel;
			
			cdef = &pres->stubs_cast.cast_scope_val[skel->c_def];

			CHECK_STUB_CAST_REF(skel->c_def);
			assert((cdef->u.kind == CAST_VAR_DECL)
			       || (cdef->u.kind == CAST_FUNC_DECL));
			
			CHECK_MINT_REF(skel->request_itype);
			CHECK_MINT_REF(skel->reply_itype);

			if (skel->funcs.funcs_len > 0)
				assert(skel->funcs.funcs_val);
			
			for (i = 0; i < skel->funcs.funcs_len; i++)
			{
				check_func(pres, &skel->funcs.funcs_val[i],
					   skel);
			}
			break;
		}
		case PRES_C_SEND_STUB:
		{
			pres_c_msg_stub *send_stub
				= &stub->pres_c_stub_u.send_stub;
			struct inline_state is;
			cdef = &pres->stubs_cast.cast_scope_val[send_stub->c_func];

			CHECK_STUB_CAST_REF(send_stub->c_func);
			assert(cdef->u.kind == CAST_FUNC_DECL);
			is.which = FUNC;
			is.u.ftype = &cdef->u.cast_def_u_u.func_type;

			CHECK_MINT_REF(send_stub->msg_itype);

			check_inline(pres, send_stub->msg_i, &is,
				     send_stub->msg_itype);
			check_inline(pres, send_stub->target_i, &is,
				     send_stub->target_itype);
			break;
		}
		case PRES_C_RECV_STUB:
		{
			pres_c_msg_stub *recv_stub
				= &stub->pres_c_stub_u.recv_stub;
			struct inline_state is;
			cdef = &pres->stubs_cast.cast_scope_val[recv_stub->c_func];

			CHECK_STUB_CAST_REF(recv_stub->c_func);
			assert(cdef->u.kind == CAST_FUNC_DECL);
			is.which = FUNC;
			is.u.ftype = &cdef->u.cast_def_u_u.func_type;

			CHECK_MINT_REF(recv_stub->msg_itype);

			check_inline(pres, recv_stub->msg_i, &is,
				     recv_stub->msg_itype);
			check_inline(pres, recv_stub->target_i, &is,
				     recv_stub->target_itype);
			break;
		}
		case PRES_C_MESSAGE_MARSHAL_STUB:
		{
			pres_c_msg_marshal_stub *mmstub
				= &stub->pres_c_stub_u.mmstub;
			struct inline_state is;
			cdef = &pres->stubs_cast.cast_scope_val[mmstub->c_func];

			CHECK_STUB_CAST_REF(mmstub->c_func);
			assert(cdef->u.kind == CAST_FUNC_DECL);
			is.which = FUNC;
			is.u.ftype = &cdef->u.cast_def_u_u.func_type;

			check_stub_op_flags(mmstub->op_flags);
			
			CHECK_MINT_REF(mmstub->itype);

			check_inline(pres, mmstub->i, &is, mmstub->itype);
			
			break;
		}
		case PRES_C_MESSAGE_UNMARSHAL_STUB:
		{
			pres_c_msg_marshal_stub *mustub
				= &stub->pres_c_stub_u.mustub;
			struct inline_state is;
			cdef = &pres->stubs_cast.cast_scope_val[mustub->c_func];

			CHECK_STUB_CAST_REF(mustub->c_func);
			assert(cdef->u.kind == CAST_FUNC_DECL);
			is.which = FUNC;
			is.u.ftype = &cdef->u.cast_def_u_u.func_type;

			check_stub_op_flags(mustub->op_flags);
			
			CHECK_MINT_REF(mustub->itype);

			check_inline(pres, mustub->i, &is, mustub->itype);
			
			break;
		}
		case PRES_C_CONTINUE_STUB:
		{
			pres_c_continue_stub *cstub
				= &stub->pres_c_stub_u.continue_stub;
			struct inline_state is;
			cdef = &pres->stubs_cast.cast_scope_val[cstub->c_func];

			CHECK_STUB_CAST_REF(cstub->c_func);
			assert(cdef->u.kind == CAST_FUNC_DECL);
			is.which = FUNC;
			is.u.ftype = &cdef->u.cast_def_u_u.func_type;

			CHECK_MINT_REF(cstub->itype);

			check_inline(pres, cstub->i, &is, cstub->itype);
			
			break;
		}
		default:
			panic("check_stub: unknown pres_c_stub_kind %d",
			      stub->kind);
	}
	
	assert(cdef &&
	       (cdef->channel >= 0) &&
	       (cdef->channel < pres->meta_data.channels.channels_len));
}

/* Check a tag list to make sure it is all valid */
void check_tag_list(tag_list *tl)
{
	unsigned int lpc;
	tag_item *ti;
	
	assert(!tl->parent);
	for( lpc = 0; lpc < tl->items.items_len; lpc++ ) {
		ti = &tl->items.items_val[lpc];
		CHECK_STRING(ti->tag);
		switch( ti->data.kind ) {
		case TAG_TAG_LIST:
			check_tag_list(ti->data.tag_data_u.tl);
			break;
		case TAG_STRING:
			CHECK_STRING(ti->data.tag_data_u.str);
			break;
		case TAG_REF:
			/* Don't allow ptr ref's to pass through */
			assert(cmp_tag_ref_class("ptr:",
						 ti->data.tag_data_u.ref));
			break;
		case TAG_REF_ARRAY: {
			unsigned int lpc2;
			
			for( lpc2 = 0;
			     lpc2 < ti->data.tag_data_u.ref_a.ref_a_len;
			     lpc2++ ) {
				/* Don't allow ptr ref's to pass through */
				assert(cmp_tag_ref_class("ptr:",
							 ti->data.
							 tag_data_u.ref));
			}
			break;
		}
		case TAG_STRING_ARRAY: {
			unsigned int lpc2;
			
			for( lpc2 = 0;
			     lpc2 < ti->data.tag_data_u.str_a.str_a_len;
			     lpc2++ ) {
				CHECK_STRING(ti->data.tag_data_u.str_a.
					     str_a_val[lpc2]);
			}
			break;
		}
		case TAG_TAG_LIST_ARRAY: {
			unsigned int lpc2;
			
			for( lpc2 = 0;
			     lpc2 < ti->data.tag_data_u.tl_a.tl_a_len;
			     lpc2++ ) {
				check_tag_list(ti->data.tag_data_u.tl_a.
					       tl_a_val[lpc2]);
			}
			break;
		}
		case TAG_CAST_SCOPED_NAME:
			cast_check_scoped_name(&ti->data.tag_data_u.scname);
			break;
		case TAG_CAST_SCOPED_NAME_ARRAY: {
			unsigned int lpc2;
			
			for( lpc2 = 0;
			     lpc2 < ti->data.tag_data_u.tl_a.tl_a_len;
			     lpc2++ ) {
				cast_check_scoped_name(&ti->data.tag_data_u.
						       scname_a.
						       scname_a_val[lpc2]);
			}
			break;
		}
		case TAG_CAST_DEF:
			cast_check_def(ti->data.tag_data_u.cdef);
			break;
		case TAG_CAST_DEF_ARRAY: {
			unsigned int lpc2;
			
			for( lpc2 = 0;
			     lpc2 < ti->data.tag_data_u.tl_a.tl_a_len;
			     lpc2++ ) {
				cast_check_def(ti->data.tag_data_u.cdef_a.
					       cdef_a_val[lpc2]);
			}
			break;
		}
		case TAG_CAST_TYPE:
			cast_check_type(ti->data.tag_data_u.ctype);
			break;
		case TAG_CAST_TYPE_ARRAY: {
			unsigned int lpc2;
			
			for( lpc2 = 0;
			     lpc2 < ti->data.tag_data_u.tl_a.tl_a_len;
			     lpc2++ ) {
				cast_check_type(ti->data.tag_data_u.ctype_a.
						ctype_a_val[lpc2]);
			}
			break;
		}
		case TAG_CAST_EXPR:
			cast_check_expr(ti->data.tag_data_u.cexpr);
			break;
		case TAG_CAST_EXPR_ARRAY: {
			unsigned int lpc2;
			
			for( lpc2 = 0;
			     lpc2 < ti->data.tag_data_u.tl_a.tl_a_len;
			     lpc2++ ) {
				cast_check_expr(ti->data.tag_data_u.cexpr_a.
						cexpr_a_val[lpc2]);
			}
			break;
		}
		case TAG_CAST_STMT:
			cast_check_stmt(ti->data.tag_data_u.cstmt);
			break;
		case TAG_CAST_STMT_ARRAY: {
			unsigned int lpc2;
			
			for( lpc2 = 0;
			     lpc2 < ti->data.tag_data_u.tl_a.tl_a_len;
			     lpc2++ ) {
				cast_check_stmt(ti->data.tag_data_u.cstmt_a.
						cstmt_a_val[lpc2]);
			}
			break;
		}
		case TAG_CAST_INIT:
			cast_check_init(ti->data.tag_data_u.cinit);
			break;
		case TAG_CAST_INIT_ARRAY: {
			unsigned int lpc2;
			
			for( lpc2 = 0;
			     lpc2 < ti->data.tag_data_u.tl_a.tl_a_len;
			     lpc2++ ) {
				cast_check_init(ti->data.tag_data_u.cinit_a.
						cinit_a_val[lpc2]);
			}
			break;
		}
		default:
			break;
		}
	}
}

void pres_c_1_check(pres_c_1 *pres)
{
	int i;
	
	/* Check the integrity of the embedded MINT and CAST individually. */
	mint_1_check(&pres->mint);
	cast_check(&pres->cast);
	cast_check(&pres->stubs_cast);
	
	/* Now check the integrity of the stubs
	   and their relationships to the MINT and CAST.  */
	for (i = 0; i < (signed int) pres->stubs.stubs_len; i++) {
		assert(pres->stubs.stubs_val);
		check_stub(pres, i);
	}
	if( pres->pres_attrs )
		check_tag_list(pres->pres_attrs);
	check_meta(&pres->meta_data);
}

/* End of file. */

