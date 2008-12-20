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
#include <string.h>

#include <mom/compiler.h>
#include <mom/c/libpres_c.h>
#include <mom/c/libcast.h>
#include <mom/libaoi.h>

#include <mom/c/pg_corbaxx.hh>

void pg_corbaxx::p_union_case(p_type_collection *inl_ptc,
			      pres_c_inline_struct_union *suinl,
			      cast_scope *class_scope,
			      cast_scope *scope,
			      tag_list *main_tl,
			      aoi_const val,
			      aoi_field *var,
			      int index)
{
	int is_string = 0, is_variable = 0, is_sequence = 0;
	p_type_collection *ptc = 0;
	pres_c_mapping variant_map;
	cast_type variant_ctype;
	cast_scoped_name scn;
	union tag_data_u data;
	unsigned amin, amax;
	p_type_node *ptn;
	tag_list *tl;
	tag_item *ti;
	cast_type type;
	aoi_type at, orig_at;
	int cdef;
	
	/* Get the element type and play around with it for a bit...
	   Similar stuff is done in p_usable_type, but unions have
	   special cases for some elements so we do everything here. */
	p_type(var->type, &ptc);
	
	orig_at = var->type;
	at = orig_at;
	ptn = ptc->find_type("definition");
	variant_ctype = ptn->get_type();
	variant_map = ptn->get_mapping();
	while( at->kind == AOI_INDIRECT )
		at = in_aoi->defs.
			defs_val[at->aoi_type_u_u.indirect_ref].binding;
	is_variable = isVariable(at);
	switch( at->kind ) {
	case AOI_FWD_INTRFC:
	case AOI_INTERFACE: {
		pres_c_allocation_u dfault;
		pres_c_allocation alloc;
		
		ptn = ptc->find_type("managed_object");
		variant_map = ptn->get_mapping();
		/* Variable structs are pointers because they
		   have a member with a constructor and
		   constructors aren't allowed in unions. */
		dfault.allow = PRES_C_ALLOCATION_ALLOW;
		if( gen_server ) {
			dfault.pres_c_allocation_u_u.val.flags =
				PRES_C_ALLOC_ALWAYS |
				PRES_C_RUN_CTOR;
		}
		if( gen_client ) {
			dfault.pres_c_allocation_u_u.val.flags =
				PRES_C_ALLOC_NEVER |
				PRES_C_DEALLOC_NEVER;
		}
		dfault.pres_c_allocation_u_u.val.allocator =
			p_get_allocator();
		dfault.pres_c_allocation_u_u.val.alloc_init = 0;
		alloc = p_get_allocation();
		alloc.cases[PRES_C_DIRECTION_IN] = dfault;
		dfault.allow = PRES_C_ALLOCATION_ALLOW;
		dfault.pres_c_allocation_u_u.val.flags =
			PRES_C_ALLOC_ALWAYS | PRES_C_RUN_CTOR;
		dfault.pres_c_allocation_u_u.val.allocator =
			p_get_allocator();
		alloc.cases[PRES_C_DIRECTION_OUT] = dfault;
		alloc.cases[PRES_C_DIRECTION_RETURN] = dfault;
		pres_c_interpose_indirection_pointer(&variant_ctype,
						     &variant_map,
						     alloc);
		break;
	}
	case AOI_TYPE_TAG:
	case AOI_TYPED:
	case AOI_STRUCT:
	case AOI_UNION:
		if( is_variable || (at->kind == AOI_UNION) ) {
			pres_c_allocation_u dfault;
			pres_c_allocation alloc;
			
			/* Variable structs are pointers because they
			   have a member with a constructor and
			   constructors aren't allowed in unions. */
			dfault.allow = PRES_C_ALLOCATION_ALLOW;
			if( gen_server ) {
				dfault.pres_c_allocation_u_u.val.flags =
					PRES_C_ALLOC_ALWAYS |
					PRES_C_RUN_CTOR;
			}
			if( gen_client ) {
				dfault.pres_c_allocation_u_u.val.flags =
					PRES_C_ALLOC_NEVER |
					PRES_C_DEALLOC_NEVER;
			}
			dfault.pres_c_allocation_u_u.val.allocator =
				p_get_allocator();
			dfault.pres_c_allocation_u_u.val.alloc_init = 0;
			alloc = p_get_allocation();
			alloc.cases[PRES_C_DIRECTION_IN] = dfault;
			dfault.allow = PRES_C_ALLOCATION_ALLOW;
			dfault.pres_c_allocation_u_u.val.flags =
				PRES_C_ALLOC_ALWAYS | PRES_C_RUN_CTOR;
			dfault.pres_c_allocation_u_u.val.allocator =
				p_get_allocator();
			alloc.cases[PRES_C_DIRECTION_OUT] = dfault;
			alloc.cases[PRES_C_DIRECTION_RETURN] = dfault;
			pres_c_interpose_indirection_pointer(&variant_ctype,
							     &variant_map,
							     alloc);
		}
		break;
	case AOI_ARRAY: {
		if( at->aoi_type_u_u.array_def.flgs ==
		    AOI_ARRAY_FLAG_NULL_TERMINATED_STRING )
			is_string = 1;
		aoi_get_array_len(in_aoi,
				  &at->aoi_type_u_u.array_def,
				  &amin,
				  &amax);
		if( amin != amax )
			is_sequence = 1;
		if( !is_string && (orig_at == at) && !is_sequence ) {
			/* If this array exists only in the union
			   than we need to construct it and all of
			   its C++ infrastructure. */
			push_ptr(scope_stack, class_scope);
			ptc = p_anon_array(flick_asprintf("_%s", var->name),
					   ptc, at);
			pop_ptr(scope_stack);
		}
		if( !is_string && !is_sequence ) {
			pres_c_inline_allocation_context *ac;
			pres_c_inline alloc_inl;
			pres_c_mapping curr_map;
			p_type_collection *curr;
			p_type_node *ptn;
			
			variant_ctype = ptc->find_type("array_slice")->
				get_type();
			variant_ctype = cast_new_pointer_type(variant_ctype);
			curr = ptc;
			while( curr->get_collection_ref() )
				curr = curr->get_collection_ref();
			ptn = curr->find_type("definition");
			curr_map = ptn->get_mapping();
			alloc_inl = curr_map->pres_c_mapping_u_u.singleton.inl;
			ac = &alloc_inl->pres_c_inline_u_u.acontext;
			if( gen_client ) {
				ac->alloc.cases[PRES_C_DIRECTION_OUT].
					pres_c_allocation_u_u.val.flags |=
					PRES_C_ALLOC_ALWAYS |
					PRES_C_RUN_CTOR;
			}
		}
		if( !is_string && is_sequence ) {
			pres_c_allocation_u dfault;
			pres_c_allocation alloc;
			
			/* Sequences will have constructors so we need
			   to make them pointers, similar to var structs */
			dfault.allow = PRES_C_ALLOCATION_ALLOW;
			if( gen_server ) {
				dfault.pres_c_allocation_u_u.val.flags =
					PRES_C_ALLOC_ALWAYS |
					PRES_C_RUN_CTOR;
			}
			if( gen_client ) {
				dfault.pres_c_allocation_u_u.val.flags =
					PRES_C_ALLOC_NEVER |
					PRES_C_DEALLOC_NEVER;
			}
			dfault.pres_c_allocation_u_u.val.allocator =
				p_get_allocator();
			dfault.pres_c_allocation_u_u.val.alloc_init = 0;
			alloc = p_get_allocation();
			alloc.cases[PRES_C_DIRECTION_IN] = dfault;
			dfault.allow = PRES_C_ALLOCATION_ALLOW;
			dfault.pres_c_allocation_u_u.val.flags =
				PRES_C_ALLOC_ALWAYS | PRES_C_RUN_CTOR;
			dfault.pres_c_allocation_u_u.val.allocator =
				p_get_allocator();
			alloc.cases[PRES_C_DIRECTION_OUT] = dfault;
			alloc.cases[PRES_C_DIRECTION_RETURN] = dfault;
			pres_c_interpose_indirection_pointer(&variant_ctype,
							     &variant_map,
							     alloc);
		}
		if( gen_server && is_string ) {
			p_type_node *ptn;
			
			ptn = ptc->find_type("peeled_managed_string");
			variant_ctype = ptn->get_type();
			variant_map = ptn->get_mapping();
		}
		break;
	}
	default:
		break;
	}
	/* Add the element to the union */
	scn = cast_new_scoped_name(calc_struct_slot_name(var->name), NULL);
	cdef = cast_add_def(scope,
			    scn,
			    CAST_SC_NONE,
			    CAST_VAR_DEF,
			    ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
			    current_protection);
	scope->cast_scope_val[cdef].u.cast_def_u_u.var_def.
		type = variant_ctype;
	if( val )
		suinl->cases.cases_val[index].mapping = variant_map;
	else
		suinl->dfault->mapping = variant_map;
	if( ((at->kind == AOI_STRUCT) && is_variable) ||
	    (at->kind == AOI_TYPED) || (at->kind == AOI_UNION) ||
	    ((at->kind == AOI_ARRAY) && (amin != amax)) )
		variant_ctype = variant_ctype->cast_type_u_u.
			pointer_type.target;
	
	/* Construct the tags to describe this member */
	tl = create_tag_list(0);
	if( !(ti = find_tag(inl_ptc->get_tag_list(), "member")) )
		ti = add_tag(inl_ptc->get_tag_list(), "member",
			     TAG_TAG_LIST_ARRAY, 0);
	data.tl = tl;
	append_tag_data(&ti->data, data);
	if( val ) {
		add_tag(tl, "disc", TAG_CAST_EXPR,
			aoi_const_to_cast_expr(val));
		add_tag(tl, "the_default", TAG_BOOL, 0);
	}
	else {
		add_tag(tl, "disc", TAG_CAST_EXPR,
			cast_new_expr_lit_int(0,0));
		add_tag(tl, "the_default", TAG_BOOL, 1);
	}
	
	add_tag(tl, "name", TAG_STRING, calc_struct_slot_name(var->name));
	add_tag(tl, "pres_index", TAG_INTEGER, ptc->get_attr_index());
	add_tag(tl, "type", TAG_CAST_TYPE, variant_ctype);
	switch( at->kind ) {
	case AOI_INTERFACE:
	case AOI_FWD_INTRFC:
		add_tag(tl, "managed", TAG_STRING, "object");
		break;
	default:
		break;
	}
	
	/* Add the setter/getter functions for this member */
	switch( at->kind ) {
	case AOI_TYPE_TAG:
	case AOI_INTERFACE:
	case AOI_FWD_INTRFC:
		variant_ctype = ptc->find_type("pointer")->get_type();
		/* FALLTHROUGH */
	case AOI_INTEGER:
	case AOI_FLOAT:
	case AOI_CHAR:
	case AOI_ENUM:
	case AOI_SCALAR:
		data.i = index;
		add_function(main_tl,
			     scn,
			     PFA_FunctionKind, "atomic_set",
			     PFA_DeclChannel, ch(cur_aoi_idx,
						 PG_CHANNEL_CLIENT_DECL),
			     PFA_ImplChannel, ch(cur_aoi_idx,
						 PG_CHANNEL_CLIENT_IMPL),
			     PFA_Scope, class_scope,
			     PFA_ReturnType, cast_new_type(CAST_TYPE_VOID),
			     PFA_Parameter, variant_ctype, "v", NULL,
			     PFA_Tag, "index", TAG_INTEGER, data,
			     PFA_Model,
			     PFA_TAG_DONE);
		add_function(main_tl,
			     scn,
			     PFA_FunctionKind, "atomic_get",
			     PFA_DeclChannel, ch(cur_aoi_idx,
						 PG_CHANNEL_CLIENT_DECL),
			     PFA_ImplChannel, ch(cur_aoi_idx,
						 PG_CHANNEL_CLIENT_IMPL),
			     PFA_Scope, class_scope,
			     PFA_Spec, CAST_FUNC_CONST,
			     PFA_ReturnType, variant_ctype,
			     PFA_Tag, "index", TAG_INTEGER, data,
			     PFA_Model,
			     PFA_TAG_DONE);
		break;
	case AOI_ARRAY:
		if( amin == amax ) {
			data.i = index;
			type = ptc->find_type("definition")->get_type();
			add_function(main_tl,
				     scn,
				     PFA_FunctionKind, "set",
				     PFA_DeclChannel,
				     ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
				     PFA_ImplChannel,
				     ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
				     PFA_Scope, class_scope,
				     PFA_ReturnType,
				     cast_new_type(CAST_TYPE_VOID),
				     PFA_Parameter, type, "v", NULL,
				     PFA_Tag, "index", TAG_INTEGER, data,
				     PFA_Model,
				     PFA_TAG_DONE);
			type = ptc->find_type("array_slice")->get_type();
			type = cast_new_pointer_type(type);
			add_function(main_tl,
				     scn,
				     PFA_FunctionKind, "rw_get",
				     PFA_DeclChannel,
				     ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
				     PFA_ImplChannel,
				     ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
				     PFA_Scope, class_scope,
				     PFA_ReturnType, type,
				     PFA_Tag, "index", TAG_INTEGER, data,
				     PFA_Model,
				     PFA_Spec, CAST_FUNC_CONST,
				     PFA_TAG_DONE);
			break;
		}
		if( is_string ) {
			data.i = index;
			type = cast_new_prim_type(CAST_PRIM_CHAR, 0);
			type = cast_new_pointer_type(type);
			add_function(main_tl,
				     scn,
				     PFA_FunctionKind, "char *set",
				     PFA_DeclChannel,
				     ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
				     PFA_ImplChannel,
				     ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
				     PFA_Scope, class_scope,
				     PFA_ReturnType,
				     cast_new_type(CAST_TYPE_VOID),
				     PFA_Parameter, type, "v", NULL,
				     PFA_Tag, "index", TAG_INTEGER, data,
				     PFA_Model,
				     PFA_TAG_DONE);
			type = cast_new_prim_type(CAST_PRIM_CHAR, 0);
			type = cast_new_qualified_type(type, CAST_TQ_CONST);
			type = cast_new_pointer_type(type);
			add_function(main_tl,
				     scn,
				     PFA_FunctionKind, "const char *set",
				     PFA_DeclChannel,
				     ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
				     PFA_ImplChannel,
				     ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
				     PFA_Scope, class_scope,
				     PFA_ReturnType,
				     cast_new_type(CAST_TYPE_VOID),
				     PFA_Parameter, type, "v", NULL,
				     PFA_Tag, "index", TAG_INTEGER, data,
				     PFA_Model,
				     PFA_TAG_DONE);
			type = ptc->find_type("smart_pointer")->get_type();
			type = cast_new_qualified_type(type, CAST_TQ_CONST);
			type = cast_new_reference_type(type);
			add_function(main_tl,
				     scn,
				     PFA_FunctionKind, "_var_set",
				     PFA_DeclChannel,
				     ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
				     PFA_ImplChannel,
				     ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
				     PFA_Scope, class_scope,
				     PFA_ReturnType,
				     cast_new_type(CAST_TYPE_VOID),
				     PFA_Parameter, type, "v", NULL,
				     PFA_Tag, "index", TAG_INTEGER, data,
				     PFA_Model,
				     PFA_TAG_DONE);
			type = cast_new_prim_type(CAST_PRIM_CHAR, 0);
			type = cast_new_qualified_type(type, CAST_TQ_CONST);
			type = cast_new_pointer_type(type);
			add_function(main_tl,
				     scn,
				     PFA_FunctionKind, "string_get",
				     PFA_DeclChannel,
				     ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
				     PFA_ImplChannel,
				     ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
				     PFA_Scope, class_scope,
				     PFA_Spec, CAST_FUNC_CONST,
				     PFA_ReturnType, type,
				     PFA_Tag, "index", TAG_INTEGER, data,
				     PFA_Model,
				     PFA_TAG_DONE);
			break;
		}
	case AOI_TYPED:
	case AOI_STRUCT:
	case AOI_UNION:
		if( at->kind == AOI_TYPED )
			type = variant_ctype;
		else {
			type = cast_new_qualified_type(variant_ctype,
						       CAST_TQ_CONST);
			type = cast_new_reference_type(type);
		}
		data.i = index;
		add_function(main_tl,
			     scn,
			     PFA_FunctionKind, "set",
			     PFA_DeclChannel, ch(cur_aoi_idx,
						 PG_CHANNEL_CLIENT_DECL),
			     PFA_ImplChannel, ch(cur_aoi_idx,
						 PG_CHANNEL_CLIENT_IMPL),
			     PFA_Scope, class_scope,
			     PFA_ReturnType, cast_new_type(CAST_TYPE_VOID),
			     PFA_Parameter, type, "v", NULL,
			     PFA_Tag, "index", TAG_INTEGER, data,
			     PFA_Model,
			     PFA_TAG_DONE);
		type = cast_new_qualified_type(variant_ctype, CAST_TQ_CONST);
		if( at->kind != AOI_TYPED )
			type = cast_new_reference_type(type);
		add_function(main_tl,
			     scn,
			     PFA_FunctionKind, "ro_get",
			     PFA_DeclChannel, ch(cur_aoi_idx,
						 PG_CHANNEL_CLIENT_DECL),
			     PFA_ImplChannel, ch(cur_aoi_idx,
						 PG_CHANNEL_CLIENT_IMPL),
			     PFA_Scope, class_scope,
			     PFA_Spec, CAST_FUNC_CONST,
			     PFA_ReturnType, type,
			     PFA_Tag, "index", TAG_INTEGER, data,
			     PFA_Model,
			     PFA_TAG_DONE);
		if( at->kind == AOI_TYPED )
			type = variant_ctype;
		else
			type = cast_new_reference_type(variant_ctype);
		add_function(main_tl,
			     scn,
			     PFA_FunctionKind, "rw_get",
			     PFA_DeclChannel, ch(cur_aoi_idx,
						 PG_CHANNEL_CLIENT_DECL),
			     PFA_ImplChannel, ch(cur_aoi_idx,
						 PG_CHANNEL_CLIENT_IMPL),
			     PFA_Scope, class_scope,
			     PFA_ReturnType, type,
			     PFA_Tag, "index", TAG_INTEGER, data,
			     PFA_Model,
			     PFA_TAG_DONE);
		break;
	default:
		panic("Not handling member in union");
		break;
	}
}

/* presentation generation routine for AOI_UNION type */
pres_c_inline pg_corbaxx::p_inline_struct_union(aoi_union *au,
						p_type_collection *inl_ptc,
						cast_type inl_ctype)
{
	char *old_name;
	cast_scoped_name scn;
	
	pres_c_inline inl;
	pres_c_inline_struct_union *suinl;
	
	p_type_collection *ptc;
	p_type_node *ptn;
	cast_type discrim_ctype, union_ctype, variant_ctype;
	pres_c_mapping discrim_map, union_map, variant_map;
	cast_scope *class_scope = (cast_scope *)top_ptr(scope_stack);
	cast_def_protection last_prot = current_protection;
	
	int union_ctype_cases_len, union_ctype_cases_idx;
	unsigned int i;

	cast_scope *scope;
	
	/* Save the original name context for when we return. */
	old_name = name;
	
	current_protection = CAST_PROT_PUBLIC;
	
	/* Count the number of non-void union variants in our AOI_UNION. */
	union_ctype_cases_len = 0;
	for (i = 0; i < au->cases.cases_len; ++i)
		if (au->cases.cases_val[i].var.type->kind != AOI_VOID)
			++union_ctype_cases_len;
	if (au->dfault && (au->dfault->type->kind != AOI_VOID))
		++union_ctype_cases_len;
	
	/* Create the pres_c_inline_struct_union which will be the return
	   value of this function.  This object will describe how the pieces of
	   our discriminated union (AOI_UNION) match up with the pieces within
	   the C structure definition we are creating (`inl_ctype'). */
	inl = pres_c_new_inline_struct_union(union_ctype_cases_len);
	suinl = &(inl->pres_c_inline_u_u.struct_union);
	
	/* `cases' is the array that describe how variants of our discriminated
	   union match up with the variants within the C presentation of that
	   union.  These correspondences are established later in this function
	   (in the `for' loop below). */
	suinl->cases.cases_len = au->cases.cases_len;
	suinl->cases.cases_val =
		(pres_c_inline_struct_union_case *)
		mustmalloc(sizeof(pres_c_inline_struct_union_case) *
			   au->cases.cases_len);
	
	/* Prepare the discriminator mapping, save it in `suinl', and add the
	   slot for the discriminator to our structure definition `inl_ctype'.
	*/
	if( !strcmp( "_d", au->discriminator.name ) )
		au->discriminator.name = ir_strlit("disc_");
	name = calc_struct_slot_name(au->discriminator.name);
	ptc = 0;
	p_type(au->discriminator.type, &ptc);
	ptn = ptc->find_type("definition");
	discrim_ctype = ptn->get_type();
	discrim_map = ptn->get_mapping();
	suinl->discrim.mapping = discrim_map;
	
	/* The following invocation of `p_inline_add_atom' has the side effect
	   of adding the discriminator slot to the structure definition. */
	pres_c_inline t_inl;
	char *discrim_name = calc_struct_slot_name(au->discriminator.name);
	
	t_inl = p_inline_add_atom(inl_ctype,
				  discrim_name,
				  discrim_ctype,
				  discrim_map);
	suinl->discrim.index = t_inl->pres_c_inline_u_u.atom.index;
	tag_list *tl = inl_ptc->get_tag_list();
	
	add_tag(tl, "discrim", TAG_STRING, discrim_name);
	add_tag(tl, "discrim_pres_index", TAG_INTEGER, ptc->get_attr_index());
	
	tl = find_tag(tl, "main")->data.tag_data_u.tl;
	
	scn = cast_new_scoped_name("_d", NULL);
	add_function(tl,
		     scn,
		     PFA_Scope, class_scope,
		     PFA_FunctionKind, "_d(T)",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_ReturnType, cast_new_type(CAST_TYPE_VOID),
		     PFA_Parameter, discrim_ctype, "_d_", NULL,
		     PFA_TAG_DONE);
	add_function(tl,
		     scn,
		     PFA_Scope, class_scope,
		     PFA_FunctionKind, "T _d()",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_Spec, CAST_FUNC_CONST,
		     PFA_ReturnType, discrim_ctype,
		     PFA_TAG_DONE);
	
	/* Create a cast_union_type to contain the slots that correspond to the
	   variants of our discriminated union. */
	union_ctype = cast_new_aggregate_type(union_aggregate_type, 0);
	union_ctype->cast_type_u_u.agg_type.name =
		cast_new_scoped_name(
			calc_struct_union_tag_name(au->union_label),
			NULL);
	
	scope = &union_ctype->cast_type_u_u.agg_type.scope;
	
	/* Compute the name of the union member; this will be used by
	   `p_inline_add_atom' below. */
	name = calc_struct_slot_name(au->union_label);
	
	/* Now process the variants of our discriminated union. */
	for (i = 0, union_ctype_cases_idx = 0; i < au->cases.cases_len; i++) {
		/*
		 * XXX --- We must set `name' here for the benefit of `pg_sun::
		 * p_variable_array_type'.  Grrr.
		 */
		name = calc_struct_slot_name(au->cases.cases_val[i].var.name);
		ptc = 0;
		p_type(au->cases.cases_val[i].var.type,
		       &ptc);
		ptn = ptc->find_type("definition");
		variant_ctype = ptn->get_type();
		variant_map = ptn->get_mapping();
		
		if (au->cases.cases_val[i].var.type->kind == AOI_VOID) {
			/* In the PRES_C description, we must indicate that
			   this variant of the union doesn't correspond to
			   any member of the `union_ctype'.  (There are no
			   void members!) */
			suinl->cases.cases_val[i].mapping = variant_map;
			suinl->cases.cases_val[i].index = -1;
		} else {
			suinl->cases.cases_val[i].index =
				union_ctype_cases_idx;
			p_union_case(inl_ptc,
				     suinl,
				     class_scope,
				     scope,
				     tl,
				     au->cases.cases_val[i].val,
				     &au->cases.cases_val[i].var,
				     i);
			++union_ctype_cases_idx;
		}
	}
	if (au->dfault) {
		suinl->dfault = (pres_c_inline_struct_union_case *)
			mustmalloc(
				   sizeof(pres_c_inline_struct_union_case)
				   );
		
		/*
		 * XXX --- We must set `name' here for the benefit of `pg_sun::
		 * p_variable_array_type'.  Grrr.
		 */
		name = calc_struct_slot_name(au->dfault->name);
		ptc = 0;
		p_type(au->dfault->type, &ptc);
		ptn = ptc->find_type("definition");
		variant_ctype = ptn->get_type();
		variant_map = ptn->get_mapping();
		
		if (au->dfault->type->kind == AOI_VOID) {
			/* As described above, remember that there is no
			   member of the `union_ctype' that corresponds to this
			   case. */
			suinl->dfault->mapping = variant_map;
			suinl->dfault->index = -1;
		} else {
			/* Remember which member of the `union_ctype'
			   corresponds to this AOI union case. */
			suinl->dfault->index = union_ctype_cases_len - 1;
			p_union_case(inl_ptc,
				     suinl,
				     class_scope,
				     scope,
				     tl,
				     0,
				     au->dfault,
				     i);
		}
	} else
		/* Remember that there is no default case. */
		suinl->dfault = 0;
	
	/* Create a direct pres_c_mapping for the C union, and add the union
	   slot to the structure definition we are creating (`inl_ctype'). */
	union_map = (pres_c_mapping)
		mustcalloc(sizeof(struct pres_c_mapping_u));
	union_map->kind = PRES_C_MAPPING_DIRECT;
	
	/* The following invocation of `p_inline_add_atom' has the side effect
	   of adding the union slot to the structure definition. */
	t_inl = p_inline_add_atom(inl_ctype,
				  calc_struct_slot_name(au->union_label),
				  union_ctype,
				  union_map);
	suinl->union_index = t_inl->pres_c_inline_u_u.atom.index;
	
	add_tag(inl_ptc->get_tag_list(), "union_name", TAG_STRING,
		calc_struct_slot_name(au->union_label));
	
	current_protection = last_prot;
	/* Restore the name context. */
	name = old_name;
	
	return inl;
}

/* End of file. */

