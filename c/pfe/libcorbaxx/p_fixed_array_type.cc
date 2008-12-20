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
#include <mom/libaoi.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include <mom/c/pg_corbaxx.hh>

/* This will produce the declarations for the
   functions associated with arrays */
void pg_corbaxx::p_array_funcs(p_type_collection *ptc)
{
	cast_type slice_type, ptr_type, const_ptr_type;
	cast_storage_class sc;
	cast_scoped_name scn;
	tag_list *pt_tl, *tl;
	cast_scope *scope;
	const char *arr_name;
	int cdef;
	
	scope = &out_pres->cast;
	if( cast_scoped_name_is_empty(&current_scope_name) )
		sc = CAST_SC_NONE;
	else {
		cdef = cast_find_def(&scope, current_scope_name,
				     CAST_NAMESPACE|CAST_TYPE|CAST_TYPEDEF);
		if( scope->cast_scope_val[cdef].u.kind == CAST_NAMESPACE )
			sc = CAST_SC_NONE;
		else
			sc = CAST_SC_STATIC;
	}
	pt_tl = ptc->get_tag_list();
	tl = create_tag_list(0);
	add_tag(pt_tl, "main", TAG_TAG_LIST, tl);
	scope = (cast_scope *)top_ptr(scope_stack);
	slice_type = ptc->find_type("array_slice")->get_type();
	ptr_type = cast_new_pointer_type(slice_type);
	const_ptr_type = cast_new_qualified_type(slice_type, CAST_TQ_CONST);
	const_ptr_type = cast_new_pointer_type(const_ptr_type);
	arr_name = ptc->get_name();
	scn = cast_copy_scoped_name(&current_scope_name);
	cast_add_scope_name(&scn,
			    arr_name,
			    null_template_arg_array);
	add_tag(tl, "form", TAG_STRING, "array");
	add_tag(tl, "name", TAG_CAST_SCOPED_NAME, scn);
	add_function(tl,
		     cast_new_scoped_name(flick_asprintf("%s_alloc",
							 arr_name),
					  NULL),
		     PFA_FunctionKind, "T_alloc()",
		     PFA_GlobalFunction,
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_Scope, scope,
		     PFA_ReturnType, ptr_type,
		     PFA_StorageClass, sc,
		     PFA_TAG_DONE);
	add_function(tl,
		     cast_new_scoped_name(flick_asprintf("%s_dup",
							 arr_name),
					  NULL),
		     PFA_FunctionKind, "T_dup()",
		     PFA_GlobalFunction,
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_Scope, scope,
		     PFA_ReturnType, ptr_type,
		     PFA_Parameter, const_ptr_type, "_slice", NULL,
		     PFA_StorageClass, sc,
		     PFA_TAG_DONE);
	add_function(tl,
		     cast_new_scoped_name(flick_asprintf("%s_copy",
							 arr_name),
					  NULL),
		     PFA_FunctionKind, "T_copy()",
		     PFA_GlobalFunction,
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_Scope, scope,
		     PFA_ReturnType, cast_new_type(CAST_TYPE_VOID),
		     PFA_Parameter, ptr_type, "_to_slice", NULL,
		     PFA_Parameter, const_ptr_type, "_from_slice", NULL,
		     PFA_StorageClass, sc,
		     PFA_TAG_DONE);
	add_function(tl,
		     cast_new_scoped_name(flick_asprintf("%s_free",
							 arr_name),
					  NULL),
		     PFA_FunctionKind, "T_free()",
		     PFA_GlobalFunction,
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_Scope, scope,
		     PFA_ReturnType, cast_new_type(CAST_TYPE_VOID),
		     PFA_Parameter, ptr_type, "_slice", NULL,
		     PFA_StorageClass, sc,
		     PFA_TAG_DONE);
}

/* This will create the _forany class for the given aoi_type */
void pg_corbaxx::p_array_forany(p_type_collection *ptc, aoi_type at)
{
	p_type_node *ptn;
	cast_type class_forany_type;
	cast_aggregate_type *class_forany;
	char *name_forany = flick_asprintf("%s_forany", ptc->get_name());
	cast_scoped_name sc_name_forany = cast_new_scoped_name(name_forany,
							       NULL);
	cast_def_protection last_prot = current_protection;
	cast_scope *scope;
	cast_type ptr_type, raw_type, slice_type = 0, var_type, bool_ctype;
	cast_type type, type2;
	cast_init cinit;
	unsigned amin, amax;
	int is_variable = 0;
	tag_list *tl, *pt_tl;
	int cdef;
	
	pt_tl = ptc->get_tag_list();
	tl = create_tag_list(0);
	while(at->kind == AOI_INDIRECT)
		at = in_aoi->defs.defs_val[at->aoi_type_u_u.indirect_ref].
			binding;
	is_variable = isVariable(at);
	aoi_get_array_len(in_aoi,
			  &at->aoi_type_u_u.array_def,
			  &amin,
			  &amax);
	add_tag(pt_tl, "forany", TAG_TAG_LIST, tl);
	add_tag(tl, "form", TAG_STRING, "class");
	cast_add_scope_name(&current_scope_name,
			    name_forany,
			    null_template_arg_array);
	add_tag(tl, "name", TAG_CAST_SCOPED_NAME,
		cast_copy_scoped_name(&current_scope_name));
	ptn = ptc->find_type("array_slice");
	slice_type = ptn->get_type();
	raw_type = cast_new_pointer_type( slice_type );
	ptr_type = raw_type;
	bool_ctype = prim_collections[PRIM_COLLECTION_BOOLEAN]->
		find_type("definition")->get_type();
	class_forany_type = cast_new_class_type(0);
	ptn = new p_type_node;
	ptn->set_name("any_type");
	ptn->set_format("%s_forany");
	ptn->set_type(class_forany_type);
	ptc->add_type("default", ptn);
	var_type = ptc->find_type("any_type")->get_type();
	class_forany = &class_forany_type->cast_type_u_u.agg_type;
	scope = &class_forany->scope;
	current_protection = CAST_PROT_PUBLIC;
	
	/* T_forany() : ptr_(T::_nil()) {} */
	add_function(tl,
		     sc_name_forany,
		     PFA_Protection, current_protection,
		     PFA_Scope, scope,
		     PFA_Constructor,
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_FunctionKind, "T_forany()",
		     PFA_ReturnType, cast_new_type(CAST_TYPE_NULL),
		     PFA_TAG_DONE);
	
	/* T_forany(T_ptr p) : ptr_(p) {} */
	cinit = cast_new_init_expr(cast_new_expr_lit_int(0,0));
	add_function(tl,
		     sc_name_forany,
		     PFA_Protection, current_protection,
		     PFA_Scope, scope,
		     PFA_Constructor,
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_FunctionKind, "T_forany(T_ptr, bool)",
		     PFA_ReturnType, cast_new_type(CAST_TYPE_NULL),
		     PFA_Parameter, ptr_type, "p", NULL,
		     PFA_Parameter, bool_ctype, "nocopy", cinit,
		     PFA_TAG_DONE);
	
	/* T_forany(const T_forany &a) : ptr_(T::_duplicate(T_ptr(a))) {} */
	type = cast_new_qualified_type(var_type, CAST_TQ_CONST);
	type = cast_new_reference_type(type);
	add_function(tl,
		     sc_name_forany,
		     PFA_Protection, current_protection,
		     PFA_Scope, scope,
		     PFA_Constructor,
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_FunctionKind, "T_forany(const T_forany &)",
		     PFA_ReturnType, cast_new_type(CAST_TYPE_NULL),
		     PFA_Parameter, type, "a", NULL,
		     PFA_TAG_DONE);
	
	/* ~T_forany() */
	add_function(tl,
		     cast_new_scoped_name(flick_asprintf("~%s",
							 name_forany),
					  NULL),
		     PFA_Scope, scope,
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "~T_forany()",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_ReturnType, cast_new_type(CAST_TYPE_NULL),
		     PFA_TAG_DONE);
	
	/* T_forany &operator=(A_ptr p) */
	type = cast_new_reference_type(var_type);
	add_function(tl,
		     cast_new_scoped_name("operator=", NULL),
		     PFA_Protection, current_protection,
		     PFA_Scope, scope,
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_FunctionKind, "T_forany &operator=(T_ptr)",
		     PFA_ReturnType, type,
		     PFA_Parameter, ptr_type, "p", NULL,
		     PFA_TAG_DONE);
	type2 = cast_new_qualified_type(var_type, CAST_TQ_CONST);
	type = cast_new_reference_type(var_type);
	type2 = cast_new_reference_type(type2);
	add_function(tl,
		     cast_new_scoped_name("operator=", NULL),
		     PFA_Protection, current_protection,
		     PFA_Scope, scope,
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_FunctionKind, "T_forany &operator=(T_forany &)",
		     PFA_ReturnType, type,
		     PFA_Parameter, type2, "p", NULL,
		     PFA_TAG_DONE);
	
	cast_scoped_name scn;
	
	type = cast_new_reference_type(slice_type);
	scn = cast_new_scoped_name("CORBA", "ULong", NULL);
	type2 = cast_new_type_scoped_name(scn);
	add_function(tl,
		     cast_new_scoped_name("operator[]", NULL),
		     PFA_Protection, current_protection,
		     PFA_Scope, scope,
		     PFA_FunctionKind, "T_slice operator[](ulong)",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_ReturnType, type,
		     PFA_Parameter, type2, "index", NULL,
		     PFA_TAG_DONE);
	type = cast_new_qualified_type(slice_type, CAST_TQ_CONST);
	type = cast_new_reference_type(type);
	add_function(tl,
		     cast_new_scoped_name("operator[]", NULL),
		     PFA_Protection, current_protection,
		     PFA_Scope, scope,
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_FunctionKind, "const T_slice operator[](ulong) const",
		     PFA_Spec, CAST_FUNC_CONST,
		     PFA_ReturnType, type,
		     PFA_Parameter, type2, "index", NULL,
		     PFA_TAG_DONE);
	
	/* operator const T_ptr &() const */
	type = cast_new_qualified_type(raw_type, CAST_TQ_CONST);
	type = cast_new_reference_type(type);
	add_function(tl,
		     cast_new_scoped_name("operator", NULL),
		     PFA_Protection, current_protection,
		     PFA_Scope, scope,
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_FunctionKind, "operator const T_ptr &() const",
		     PFA_ReturnType, type,
		     PFA_Spec, CAST_FUNC_OPERATOR|CAST_FUNC_CONST,
		     PFA_TAG_DONE);
	
	/* operator T_ptr &() */
	add_function(tl,
		     cast_new_scoped_name("operator", NULL),
		     PFA_Protection, current_protection,
		     PFA_Scope, scope,
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_FunctionKind, "operator T_ptr &()",
		     PFA_ReturnType, cast_new_reference_type(raw_type),
		     PFA_Spec, CAST_FUNC_OPERATOR,
		     PFA_TAG_DONE);
	
	type = cast_new_qualified_type(slice_type,
				       CAST_TQ_CONST);
	type = cast_new_pointer_type(type);
	cdef = add_function(tl,
			    cast_new_scoped_name("in", NULL),
			    PFA_Protection, current_protection,
			    PFA_Scope, scope,
			    PFA_DeclChannel, ch(cur_aoi_idx,
						PG_CHANNEL_CLIENT_DECL),
			    PFA_ImplChannel, ch(cur_aoi_idx,
						PG_CHANNEL_CLIENT_IMPL),
			    PFA_FunctionKind, "in()",
			    PFA_ReturnType, type,
			    PFA_Spec, CAST_FUNC_CONST,
			    PFA_TAG_DONE);
	
	type = ptr_type;
	cdef = add_function(tl,
			    cast_new_scoped_name("inout", NULL),
			    PFA_Protection, current_protection,
			    PFA_Scope, scope,
			    PFA_DeclChannel, ch(cur_aoi_idx,
						PG_CHANNEL_CLIENT_DECL),
			    PFA_ImplChannel, ch(cur_aoi_idx,
						PG_CHANNEL_CLIENT_IMPL),
			    PFA_FunctionKind, "inout()",
			    PFA_ReturnType, type,
			    PFA_TAG_DONE);
	
	type = cast_new_pointer_type(slice_type);
	type = cast_new_reference_type(type);
	cdef = add_function(tl,
			    cast_new_scoped_name("out", NULL),
			    PFA_Protection, current_protection,
			    PFA_Scope, scope,
			    PFA_DeclChannel, ch(cur_aoi_idx,
						PG_CHANNEL_CLIENT_DECL),
			    PFA_ImplChannel, ch(cur_aoi_idx,
						PG_CHANNEL_CLIENT_IMPL),
			    PFA_FunctionKind, "out()",
			    PFA_ReturnType, type,
			    PFA_TAG_DONE);
	
	type = raw_type;
	cdef = add_function(tl,
			    cast_new_scoped_name("_retn", NULL),
			    PFA_Protection, current_protection,
			    PFA_Scope, scope,
			    PFA_DeclChannel, ch(cur_aoi_idx,
						PG_CHANNEL_CLIENT_DECL),
			    PFA_ImplChannel, ch(cur_aoi_idx,
						PG_CHANNEL_CLIENT_IMPL),
			    PFA_FunctionKind, "_retn()",
			    PFA_ReturnType, type,
			    PFA_TAG_DONE);
	
	/* */
	cdef = add_function(tl,
			    cast_new_scoped_name("ptr", NULL),
			    PFA_Protection, current_protection,
			    PFA_FunctionKind, "ptr()",
			    PFA_Scope, scope,
			    PFA_DeclChannel, ch(cur_aoi_idx,
						PG_CHANNEL_CLIENT_DECL),
			    PFA_ImplChannel, ch(cur_aoi_idx,
						PG_CHANNEL_CLIENT_IMPL),
			    PFA_ReturnType, ptr_type,
			    PFA_Spec, CAST_FUNC_CONST,
			    PFA_TAG_DONE);
	/* */
	cdef = add_function(tl,
			    cast_new_scoped_name("nocopy", NULL),
			    PFA_Protection, current_protection,
			    PFA_FunctionKind, "nocopy()",
			    PFA_Scope, scope,
			    PFA_DeclChannel, ch(cur_aoi_idx,
						PG_CHANNEL_CLIENT_DECL),
			    PFA_ImplChannel, ch(cur_aoi_idx,
						PG_CHANNEL_CLIENT_IMPL),
			    PFA_ReturnType, bool_ctype,
			    PFA_Spec, CAST_FUNC_CONST,
			    PFA_TAG_DONE);
	
	cast_del_scope_name(&current_scope_name);
	current_protection = last_prot;
}

void pg_corbaxx::p_fixed_array_type(aoi_array *aa,
				    p_type_collection **out_ptc)
{
	/* Create the ctype and mapping for the array's target. */
	p_type_node *new_ptn;
	cast_type ctype;
	p_type_collection *ptc, *elem_ptc = 0;
	p_type_node *ptn;
	cast_type to_ctype;
	pres_c_mapping to_map;
	union tag_data_u data;
	tag_data *pt_td;
	tag_list *tl;
	aoi_type at;
	
	cast_type lctype = mint_to_ctype(
		&out_pres->mint,
		out_pres->mint.standard_refs.unsigned32_ref);
	
	p_usable_type(aa->element_type, &elem_ptc, &to_ctype, &to_map);
	
	/* Find the length of the array. */
	unsigned length, max;
	aoi_get_array_len(in_aoi, aa, &length, &max);
	assert(length == max);
	
	/* Create the array ctype. */
	ctype = cast_new_type(CAST_TYPE_ARRAY);
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
			      PIA_Name, "array_len",
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
		if( isVariable(aa->element_type) ) {
			ac->alloc.cases[PRES_C_DIRECTION_OUT].
				pres_c_allocation_u_u.val.flags
				= PRES_C_ALLOC_ALWAYS | PRES_C_DEALLOC_NEVER |
				PRES_C_RUN_CTOR | PRES_C_RUN_DTOR;
		} else {
			ac->alloc.cases[PRES_C_DIRECTION_OUT].
				pres_c_allocation_u_u.val.flags
				= PRES_C_ALLOC_NEVER | PRES_C_DEALLOC_NEVER;
		}
		
	} else if (gen_server) {
		
		ac->alloc.cases[PRES_C_DIRECTION_IN].
			pres_c_allocation_u_u.val.flags
			= PRES_C_ALLOC_ALWAYS | PRES_C_DEALLOC_ALWAYS |
			PRES_C_RUN_CTOR | PRES_C_RUN_DTOR;
		ac->alloc.cases[PRES_C_DIRECTION_INOUT].
			pres_c_allocation_u_u.val.flags
			= PRES_C_ALLOC_ALWAYS | PRES_C_DEALLOC_ALWAYS |
			PRES_C_RUN_CTOR | PRES_C_RUN_DTOR;
		ac->alloc.cases[PRES_C_DIRECTION_OUT].
			pres_c_allocation_u_u.val.flags
			= PRES_C_ALLOC_ALWAYS | PRES_C_DEALLOC_ALWAYS |
			PRES_C_RUN_CTOR | PRES_C_RUN_DTOR;
		/*
		 * Further, fixed arrays are not reallocated by the callee
		 * (client stub or server work function).  Thus, we don't
		 * have to use the default presentation-supplied allocator.
		 */
		ac->alloc.cases[PRES_C_DIRECTION_IN].
			pres_c_allocation_u_u.val.allocator.kind
			= PRES_C_ALLOCATOR_NAME;
		ac->alloc.cases[PRES_C_DIRECTION_INOUT].
			pres_c_allocation_u_u.val.allocator.kind
			= PRES_C_ALLOCATOR_NAME;
		ac->alloc.cases[PRES_C_DIRECTION_OUT].
			pres_c_allocation_u_u.val.allocator.kind
			= PRES_C_ALLOCATOR_NAME;
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
	
	ptn = new p_type_node;
	ptn->set_name("element_type");
	ptn->set_format("");
	ptn->set_type(to_ctype);
	ptc->add_type("default", ptn);
	
	/*
	 * Emit the `typedef' for the ``array slice'' type corresponding to
	 * the presented array type.  See Section 14.13 of the CORBA 2.0 spec
	 * for inforamtion about array slices.
	 */
	new_ptn = new p_type_node;
	new_ptn->set_name("array_slice");
	new_ptn->set_format("%s_slice");
	new_ptn->set_type(to_ctype);
	ptc->add_type("default", new_ptn);
	
	new_ptn = new p_type_node;
	new_ptn->set_name("pointer");
	new_ptn->set_format("");
	new_ptn->set_type(cast_new_pointer_type(to_ctype));
	ptc->add_type("default", new_ptn);
	
	/* Add some tags to describe the array */
	pt_td = &(find_tag(out_pres->pres_attrs, "pres_type")->data);
	tl = create_tag_list(0);
	data.tl = tl;
	ptc->set_attr_index(append_tag_data(pt_td, data));
	ptc->set_tag_list(tl);
	add_tag(tl, "slice_pres_index",
		TAG_INTEGER, elem_ptc->get_attr_index());
	add_tag(tl, "size", TAG_STRING,
		isVariable(a(cur_aoi_idx).binding) ? "variable" : "fixed");
	add_tag(tl, "idl_type", TAG_STRING, "array");
	p_array_dims(tl, aa);
	at = aa->element_type;
	while(at->kind == AOI_INDIRECT)
		at = in_aoi->defs.defs_val[at->aoi_type_u_u.indirect_ref].
			binding;
	switch(at->kind) {
	case AOI_ARRAY:
		if(at->aoi_type_u_u.array_def.flgs &
		   AOI_ARRAY_FLAG_NULL_TERMINATED_STRING)
			add_tag(tl, "managed", TAG_STRING, "string");
		break;
	case AOI_FWD_INTRFC:
	case AOI_INTERFACE:
		add_tag(tl, "managed", TAG_STRING, "object");
		break;
	default:
		break;
	}
	if( *out_ptc )
		(*out_ptc)->set_collection_ref(ptc);
	else
		*out_ptc = ptc;
}

/* End of file. */

