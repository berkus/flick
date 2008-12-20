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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Flick; see the file COPYING.  If not, write to
 * the Free Software Foundation, 59 Temple Place #330, Boston, MA 02111, USA.
 */

#include <assert.h>
#include <string.h>

#include <mom/libaoi.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include <mom/c/pg_corbaxx.hh>

/* This will create the out class for the given aoi_type */
void pg_corbaxx::p_out_class(p_type_collection *ptc, aoi_type at)
{
	cast_type raw_type, ptr_type, var_type, out_type, slice_type = 0, type;
	cast_scoped_name sc_out_name;
	cast_def_protection last_prot = current_protection;
	cast_type out_class;
	cast_scope *scope;
	p_type_node *ptn;
	int is_variable = 0, is_sequence = 0, string_elem = 0;
	unsigned amax, amin;
	char *out_name;
	const char *pres_type;
	tag_list *tl, *pt_tl;
	int slice_pres_index;
	tag_item *ti;
	
	pt_tl = ptc->get_tag_list();
	tl = create_tag_list(0);
	is_variable = isVariable(at);
	switch( at->kind ) {
	case AOI_FWD_INTRFC:
	case AOI_INTERFACE:
		pres_type = "interface";
		break;
	case AOI_ARRAY: {
		slice_pres_index = find_tag(ptc->get_collection_ref()->
					    get_tag_list(),
					    "slice_pres_index")->data.
			tag_data_u.i;
		ti = find_tag(out_pres->pres_attrs, "pres_type");
		ti = find_tag(get_tag_data(&ti->data, slice_pres_index).tl,
			      "idl_type");
		if( !strcmp(ti->data.tag_data_u.str, "string") ) {
			string_elem = 1;
		}
		aoi_get_array_len(in_aoi,
				  &at->aoi_type_u_u.array_def,
				  &amin,
				  &amax);
		if( amin == amax )
			pres_type = "array";
		else
			pres_type = "sequence";
		break;
	}
	case AOI_STRUCT:
		pres_type = "struct";
		break;
	default:
		pres_type = "";
		break;
	}
	add_tag(pt_tl, "out", TAG_TAG_LIST, tl);
	add_tag(tl, "form", TAG_STRING, "class");
	out_name = flick_asprintf("%s_out", ptc->get_name());
	sc_out_name = cast_new_scoped_name(out_name, NULL);
	cast_add_scope_name(&current_scope_name,
			    out_name,
			    null_template_arg_array);
	add_tag(tl, "name", TAG_CAST_SCOPED_NAME,
		cast_copy_scoped_name(&current_scope_name));
	
	out_class = cast_new_class_type(0);
	scope = &out_class->cast_type_u_u.agg_type.scope;
	
	ptn = new p_type_node;
	ptn->set_name("out_pointer");
	ptn->set_format("%s_out");
	ptn->set_type(out_class);
	ptc->add_type("default", ptn);
	switch( at->kind ) {
	case AOI_INTERFACE:
	case AOI_FWD_INTRFC:
		ptn = ptc->find_type("pointer");
		ptr_type = ptn->get_type();
		raw_type = ptr_type;
		break;
	case AOI_ARRAY:
		aoi_get_array_len(in_aoi,
				  &at->aoi_type_u_u.array_def,
				  &amin,
				  &amax);
		ptn = ptc->find_type("array_slice");
		slice_type = ptn->get_type();
		if( amin == amax ) {
			raw_type = cast_new_pointer_type( slice_type );
			ptr_type = raw_type;
			break;
		}
		else
			is_sequence = 1;
	default:
		raw_type = ptc->find_type("definition")->get_type();
		ptr_type = cast_new_pointer_type(raw_type);
		break;
	}
	var_type = ptc->find_type("smart_pointer")->get_type();
	out_type = ptc->find_type("out_pointer")->get_type();
	
	current_protection = CAST_PROT_PUBLIC;
	type = cast_new_reference_type(ptr_type);
	add_function(tl,
		     sc_out_name,
		     PFA_Protection, current_protection,
		     PFA_Constructor,
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_FunctionKind, "T_out(T_ptr &)",
		     PFA_Scope, scope,
		     PFA_ReturnType, cast_new_type(CAST_TYPE_NULL),
		     PFA_Parameter, type, "r", NULL,
		     PFA_TAG_DONE);
	
	type = cast_new_reference_type(var_type);
	add_function(tl,
		     sc_out_name,
		     PFA_Protection, current_protection,
		     PFA_Constructor,
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_FunctionKind, "T_out(T_var &)",
		     PFA_Scope, scope,
		     PFA_ReturnType, cast_new_type(CAST_TYPE_NULL),
		     PFA_Parameter, type, "r", NULL,
		     PFA_TAG_DONE);
	
	type = cast_new_qualified_type(out_type, CAST_TQ_CONST);
	type = cast_new_reference_type(type);
	add_function(tl,
		     sc_out_name,
		     PFA_Protection, current_protection,
		     PFA_Constructor,
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_FunctionKind, "T_out(T_out &)",
		     PFA_Scope, scope,
		     PFA_ReturnType, cast_new_type(CAST_TYPE_NULL),
		     PFA_Parameter, type, "r", NULL,
		     PFA_TAG_DONE);
	
	type = cast_new_qualified_type(out_type, CAST_TQ_CONST);
	type = cast_new_reference_type(type);
	add_function(tl,
		     cast_new_scoped_name("operator=", NULL),
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "T_out &operator=(const T_out &)",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_Scope, scope,
		     PFA_ReturnType, cast_new_reference_type(out_type),
		     PFA_Parameter, type, "r", NULL,
		     PFA_TAG_DONE);
	
	switch( at->kind ) {
	case AOI_ARRAY:
	case AOI_STRUCT:
	case AOI_UNION:
		current_protection = CAST_PROT_PRIVATE;
	case AOI_INTERFACE:
	case AOI_FWD_INTRFC:
		type = cast_new_qualified_type(var_type, CAST_TQ_CONST);
		type = cast_new_reference_type(type);
		add_function(tl,
			     cast_new_scoped_name("operator=", NULL),
			     PFA_Protection, current_protection,
			     PFA_DeclChannel, ch(cur_aoi_idx,
						 PG_CHANNEL_CLIENT_DECL),
			     PFA_ImplChannel, ch(cur_aoi_idx,
						 PG_CHANNEL_CLIENT_IMPL),
			     PFA_FunctionKind, "T_out &operator=(const T_var &)",
			     PFA_Scope, scope,
			     PFA_ReturnType,
			     cast_new_reference_type(out_type),
			     PFA_Parameter, type, "r", NULL,
			     PFA_TAG_DONE);
		break;
	default:
		break;
	}
	current_protection = CAST_PROT_PUBLIC;
	
	add_function(tl,
		     cast_new_scoped_name("operator=", NULL),
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "T_out &operator=(T *)",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_Scope, scope,
		     PFA_ReturnType, cast_new_reference_type(out_type),
		     PFA_Parameter, ptr_type, "r", NULL,
		     PFA_TAG_DONE);
	
	add_function(tl,
		     cast_new_scoped_name("operator", NULL),
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "operator T *&()",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_Scope, scope,
		     PFA_ReturnType, cast_new_reference_type(ptr_type),
		     PFA_Spec, CAST_FUNC_OPERATOR,
		     PFA_TAG_DONE);
	
	add_function(tl,
		     cast_new_scoped_name("ptr", NULL),
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "ptr()",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_Scope, scope,
		     PFA_ReturnType, cast_new_reference_type(ptr_type),
		     PFA_TAG_DONE);
	
	switch( at->kind ) {
	case AOI_ARRAY:
		cast_scoped_name scn;
		cast_type type2;
		
		if( !is_sequence || !string_elem ) {
			type = cast_new_reference_type(slice_type);
			scn = cast_new_scoped_name("CORBA", "ULong", NULL);
			type2 = cast_new_type_scoped_name(scn);
			add_function(tl,
				     cast_new_scoped_name("operator[]", NULL),
				     PFA_Protection, current_protection,
				     PFA_FunctionKind,
				     "T_slice operator[](ulong)",
				     PFA_DeclChannel,
				     ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
				     PFA_ImplChannel,
				     ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
				     PFA_Scope, scope,
				     PFA_ReturnType, type,
				     PFA_Parameter, type2, "index", NULL,
				     PFA_TAG_DONE);
			type = cast_new_qualified_type(slice_type,
						       CAST_TQ_CONST);
			type = cast_new_reference_type(type);
			add_function(tl,
				     cast_new_scoped_name("operator[]",
							  NULL),
				     PFA_Protection,
				     current_protection,
				     PFA_FunctionKind, "const T_slice "
				     "operator[](ulong) const",
				     PFA_DeclChannel,
				     ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
				     PFA_ImplChannel,
				     ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
				     PFA_Scope, scope,
				     PFA_Spec, CAST_FUNC_CONST,
				     PFA_ReturnType, type,
				     PFA_Parameter, type2, "index", NULL,
				     PFA_TAG_DONE);
		}
		if( !is_sequence )
			break;
	default:
		add_function(tl,
			     cast_new_scoped_name("operator->", NULL),
			     PFA_Protection, current_protection,
			     PFA_FunctionKind, "T *operator->()",
			     PFA_DeclChannel, ch(cur_aoi_idx,
						 PG_CHANNEL_CLIENT_DECL),
			     PFA_ImplChannel, ch(cur_aoi_idx,
						 PG_CHANNEL_CLIENT_IMPL),
			     PFA_Scope, scope,
			     PFA_ReturnType, ptr_type,
			     PFA_TAG_DONE);
		break;
	}
	
	cast_del_scope_name(&current_scope_name);
	current_protection = last_prot;
}

/* This will create the _var class for the given aoi_type */
void pg_corbaxx::p_type_var(p_type_collection *ptc, aoi_type at)
{
	p_type_node *ptn;
	cast_type class_var_type;
	cast_aggregate_type *class_var;
	char *name_var = flick_asprintf("%s_var", ptc->get_name());
	cast_scoped_name sc_name_var = cast_new_scoped_name(name_var, NULL);
	cast_def_protection last_prot = current_protection;
	cast_scope *scope;
	cast_type ptr_type, raw_type, slice_type = 0, var_type;
	pres_c_mapping map = 0;
	cast_type type, type2;
	unsigned amin, amax;
	int is_sequence = 0, is_variable = 0, string_elem = 0;
	tag_list *tl, *pt_tl;
	int slice_pres_index;
	tag_item *ti;
	const char *pres_type;
	int cdef;
	
	pt_tl = ptc->get_tag_list();
	tl = create_tag_list(0);
	while(at->kind == AOI_INDIRECT)
		at = in_aoi->defs.
			defs_val[at->aoi_type_u_u.indirect_ref].binding;
	is_variable = isVariable(at);
	switch( at->kind ) {
	case AOI_UNION:
	case AOI_STRUCT:
		if( is_variable )
			pres_type = "variable_struct";
		else
			pres_type = "fixed_struct";
		break;
	case AOI_ARRAY: {
		slice_pres_index = find_tag(ptc->get_collection_ref()->
					    get_tag_list(),
					    "slice_pres_index")->
			data.tag_data_u.i;
		ti = find_tag(out_pres->pres_attrs, "pres_type");
		ti = find_tag(get_tag_data(&ti->data, slice_pres_index).tl,
			      "idl_type");
		if( !strcmp(ti->data.tag_data_u.str, "string") ) {
			string_elem = 1;
		}
		aoi_get_array_len(in_aoi,
				  &at->aoi_type_u_u.array_def,
				  &amin,
				  &amax);
		if( amin == amax ) {
			if( is_variable )
				pres_type = "variable_array";
			else
				pres_type = "fixed_array";
		}
		else
			pres_type = "sequence";
		break;
	}
	case AOI_FWD_INTRFC:
	case AOI_INTERFACE:
		pres_type = "interface";
		break;
	default:
		pres_type = flick_asprintf("aoi_kind == %d", at->kind);
		break;
	}
	add_tag(pt_tl, "var", TAG_TAG_LIST, tl);
	add_tag(tl, "form", TAG_STRING, "class");
	cast_add_scope_name(&current_scope_name,
			    name_var,
			    null_template_arg_array);
	add_tag(tl, "name", TAG_CAST_SCOPED_NAME,
		cast_copy_scoped_name(&current_scope_name));
	/* Setup the "T_var" class type */
	switch( at->kind ) {
	case AOI_INTERFACE:
	case AOI_FWD_INTRFC:
		ptn = ptc->find_type("pointer");
		ptr_type = ptn->get_type();
		raw_type = ptr_type;
		map = PRES_C_M_XLATE,
			PMA_InternalCType, ptr_type,
			PMA_InternalMapping, ptn->get_mapping(),
			END_PRES_C;
		break;
	case AOI_ARRAY:
		ptn = ptc->find_type("array_slice");
		slice_type = ptn->get_type();
		if( amax == amin ) {
			raw_type = cast_new_pointer_type( slice_type );
			ptr_type = raw_type;
			break;
		}
		else
			is_sequence = 1;
	default:
		raw_type = ptc->find_type("definition")->get_type();
		ptr_type = cast_new_pointer_type( raw_type );
		break;
	}
	assert(ptr_type);
	class_var_type = cast_new_class_type(0);
	
	ptn = new p_type_node;
	ptn->set_name("smart_pointer");
	ptn->set_format("%s_var");
	ptn->set_type(class_var_type);
	ptc->add_type("default", ptn);
	var_type = ptc->find_type("smart_pointer")->get_type();
	if( map )
		ptc->find_type("smart_pointer")->set_mapping(map);
	class_var = &class_var_type->cast_type_u_u.agg_type;
	scope = &class_var->scope;
	current_protection = CAST_PROT_PUBLIC;
	
	/* T_var() : ptr_(T::_nil()) {} */
	add_function(tl,
		     sc_name_var,
		     PFA_Protection, current_protection,
		     PFA_Scope, scope,
		     PFA_Constructor,
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_FunctionKind, "T_var()",
		     PFA_ReturnType, cast_new_type(CAST_TYPE_NULL),
		     PFA_TAG_DONE);
	
	/* T_var(T_ptr p) : ptr_(p) {} */
	add_function(tl,
		     sc_name_var,
		     PFA_Protection, current_protection,
		     PFA_Scope, scope,
		     PFA_Constructor,
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_FunctionKind, "T_var(T_ptr)",
		     PFA_ReturnType, cast_new_type(CAST_TYPE_NULL),
		     PFA_Parameter, ptr_type, "p", NULL,
		     PFA_TAG_DONE);
	
	/* T_var(const T_var &a) : ptr_(T::_duplicate(T_ptr(a))) {} */
	type = cast_new_qualified_type(var_type, CAST_TQ_CONST);
	type = cast_new_reference_type(type);
	add_function(tl,
		     sc_name_var,
		     PFA_Protection, current_protection,
		     PFA_Scope, scope,
		     PFA_Constructor,
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_FunctionKind, "T_var(const T_var &)",
		     PFA_ReturnType, cast_new_type(CAST_TYPE_NULL),
		     PFA_Parameter, type, "a", NULL,
		     PFA_TAG_DONE);
	
	/* ~T_var() */
	add_function(tl,
		     cast_new_scoped_name(flick_asprintf("~%s",
							 name_var),
					  NULL),
		     PFA_Scope, scope,
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "~T_var()",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_ReturnType, cast_new_type(CAST_TYPE_NULL),
		     PFA_TAG_DONE);
	
	/* T_var &operator=(A_ptr p) */
	type = cast_new_reference_type(var_type);
	add_function(tl,
		     cast_new_scoped_name("operator=", NULL),
		     PFA_Protection, current_protection,
		     PFA_Scope, scope,
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_FunctionKind, "T_var &operator=(T_ptr)",
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
		     PFA_FunctionKind, "T_var &operator=(T_var &)",
		     PFA_ReturnType, type,
		     PFA_Parameter, type2, "p", NULL,
		     PFA_TAG_DONE);
	
	switch( at->kind ) {
	case AOI_ARRAY:
		cast_scoped_name scn;
		
		if( !is_sequence || !string_elem) {
			type = cast_new_reference_type(slice_type);
			scn = cast_new_scoped_name("CORBA", "ULong", NULL);
			type2 = cast_new_type_scoped_name(scn);
			add_function(tl,
				     cast_new_scoped_name("operator[]", NULL),
				     PFA_Protection, current_protection,
				     PFA_Scope, scope,
				     PFA_FunctionKind,
				     "T_slice operator[](ulong)",
				     PFA_DeclChannel,
				     ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
				     PFA_ImplChannel,
				     ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
				     PFA_ReturnType, type,
				     PFA_Parameter, type2, "index", NULL,
				     PFA_TAG_DONE);
			type = cast_new_qualified_type(slice_type,
						       CAST_TQ_CONST);
			type = cast_new_reference_type(type);
			add_function(tl,
				     cast_new_scoped_name("operator[]",
							  NULL),
				     PFA_Protection, current_protection,
				     PFA_Scope, scope,
				     PFA_DeclChannel,
				     ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
				     PFA_ImplChannel,
				     ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
				     PFA_FunctionKind, "const T_slice "
				     "operator[](ulong) const",
				     PFA_Spec, CAST_FUNC_CONST,
				     PFA_ReturnType, type,
				     PFA_Parameter, type2, "index", NULL,
				     PFA_TAG_DONE);
		}
		if( !is_sequence )
			break;
	default:
		/* T_ptr operator->() */
		add_function(tl,
			     cast_new_scoped_name("operator->", NULL),
			     PFA_Protection, current_protection,
			     PFA_Scope, scope,
			     PFA_DeclChannel,
			     ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
			     PFA_ImplChannel,
			     ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
			     PFA_FunctionKind, "T_ptr operator->()",
			     PFA_ReturnType, ptr_type,
			     PFA_Spec, ((at->kind == AOI_INTERFACE) ||
					(at->kind == AOI_FWD_INTRFC)) ?
			     CAST_FUNC_CONST : 0,
			     PFA_TAG_DONE);
		
		/* const T_ptr operator->() const */
		if( !((at->kind == AOI_INTERFACE) ||
		      (at->kind == AOI_FWD_INTRFC)) ) {
			type = cast_new_qualified_type(raw_type,
						       CAST_TQ_CONST);
			type = cast_new_pointer_type(type);
			add_function(tl,
				     cast_new_scoped_name("operator->", NULL),
				     PFA_Protection, current_protection,
				     PFA_Scope, scope,
				     PFA_DeclChannel,
				     ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
				     PFA_ImplChannel,
				     ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
				     PFA_FunctionKind, "const T_ptr operator->() const",
				     PFA_ReturnType, type,
				     PFA_Spec, CAST_FUNC_CONST,
				     PFA_TAG_DONE);
		}
		break;
	}
	
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
	
	switch( at->kind ) {
	case AOI_FWD_INTRFC:
	case AOI_INTERFACE:
	case AOI_ARRAY:
		if( !is_sequence )
			break;
	default:
		/* operator T_ptr &() const */
		type = cast_new_reference_type(raw_type);
		add_function(tl,
			     cast_new_scoped_name("operator", NULL),
			     PFA_Protection, current_protection,
			     PFA_Scope, scope,
			     PFA_DeclChannel, ch(cur_aoi_idx,
						 PG_CHANNEL_CLIENT_DECL),
			     PFA_ImplChannel, ch(cur_aoi_idx,
						 PG_CHANNEL_CLIENT_IMPL),
			     PFA_FunctionKind, "operator T_ptr &() const",
			     PFA_ReturnType, type,
			     PFA_Spec, CAST_FUNC_OPERATOR|CAST_FUNC_CONST,
			     PFA_TAG_DONE);
		break;
	}
	
	/* */
	switch( at->kind ) {
	case AOI_INTERFACE:
	case AOI_FWD_INTRFC:
		type = ptr_type;
		break;
	case AOI_ARRAY:
		if( !is_sequence ) {
			type = cast_new_qualified_type(slice_type,
						       CAST_TQ_CONST);
			type = cast_new_pointer_type(type);
			break;
		}
	case AOI_STRUCT:
	case AOI_UNION:
		type = cast_new_qualified_type(raw_type, CAST_TQ_CONST);
		type = cast_new_reference_type(type);
		break;
	default:
		type = raw_type;
		break;
	}
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
	
	/* */
	switch( at->kind ) {
	case AOI_ARRAY:
		if( !is_sequence ) {
			type = ptr_type;
			break;
		}
	default:
		type = cast_new_reference_type(raw_type);
		break;
	}
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
	
	/* */
	switch( at->kind ) {
	case AOI_INTERFACE:
	case AOI_FWD_INTRFC:
		type = cast_new_reference_type(ptr_type);
		break;
	case AOI_ARRAY:
		if( !is_sequence ) {
			if( is_variable ) {
				type = cast_new_pointer_type(slice_type);
				type = cast_new_reference_type(type);
				break;
			}
			else
				type = ptr_type;
		}
	default:
		if( is_variable )
			type = cast_new_pointer_type(raw_type);
		else
			type = raw_type;
		type = cast_new_reference_type(type);
		break;
	}
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
	
	/* */
	switch( at->kind ) {
	case AOI_INTERFACE:
	case AOI_FWD_INTRFC:
		type = ptr_type;
		break;
	case AOI_ARRAY:
		if( !is_sequence ) {
			type = raw_type;
			break;
		}
	default:
		if( is_variable )
			type = cast_new_pointer_type(raw_type);
		else
			type = raw_type;
		break;
	}
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
	
	cast_del_scope_name(&current_scope_name);
	current_protection = last_prot;
}
