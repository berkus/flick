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

#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include <mom/c/pg_corbaxx.hh>

void pg_corbaxx::p_union_type(aoi_union *au,
			      p_type_collection **out_ptc)
{
	p_type_collection *ptc;
	union tag_data_u data;
	tag_list *tl;
	tag_data *pt_td;
	p_type_node *ptn;
	char *union_name;
	cast_def_protection last_prot = current_protection;
	aoi_type at = a(cur_aoi_idx).binding;
	
	/*
	 * Create an aggregate type of whatever type the pg specified.
	 * The C presentation for an AOI union is not just a C union, but
	 * rather, a C struct containing a discriminator field and a C union.
	 */
	union_name = calc_type_name(a(cur_aoi_idx).name);
	cast_type ctype = cast_new_aggregate_type(struct_union_aggregate_type,
						  0);
	
	ctype->cast_type_u_u.agg_type.name =
		cast_new_scoped_name(union_name, NULL);
	
	ptc = p_new_type_collection(union_name);
	
	if( (*out_ptc) ) {
		tl = (*out_ptc)->get_tag_list();
	} else {
		pt_td = &(find_tag(out_pres->pres_attrs, "pres_type")->data);
		tl = create_tag_list(0);
		data.tl = tl;
		ptc->set_attr_index(append_tag_data(pt_td, data));
		ptc->set_tag_list(tl);
	}
	
	add_tag(tl, "size", TAG_STRING,
		isVariable(a(cur_aoi_idx).binding) ? "variable" : "fixed");
	add_tag(tl, "idl_type", TAG_STRING, "union");
	
	ptn = new p_type_node;
	ptn->set_flags(struct_type_node_flags);
	ptn->set_name("definition");
	ptn->set_format("%s");
	ptn->set_type(ctype);
	
	/* This PRES_C_MAPPING_STRUCT ``bumps'' us into inline mode. */
	pres_c_mapping map = pres_c_new_mapping(PRES_C_MAPPING_STRUCT);
	ptn->set_mapping(map);
	ptc->add_type("default", ptn);
	
	if( *out_ptc )
		(*out_ptc)->set_collection_ref(ptc);
	else
		*out_ptc = ptc;
	
	p_union_class(*out_ptc);
	
	ptc = *out_ptc;
	
	/* Add a _var and _out types */
	p_type_var(ptc, at);
	
	if( isVariable(at) ) {
		p_out_class(ptc, at);
	}
	else {
		ptn = new p_type_node;
		ptn->set_name("out_pointer");
		ptn->set_format("%s_out");
		ptn->set_type(cast_new_reference_type(ptc->
						      find_type("definition")->
						      get_type()));
		ptc->add_type("default", ptn);
	}
	
	ptc->define_types();
	
	push_ptr(scope_stack, &ctype->cast_type_u_u.agg_type.scope);
	cast_add_scope_name(&current_scope_name,
			    name,
			    null_template_arg_array);
	
	current_protection = CAST_PROT_PUBLIC;
	gen_scope(a(cur_aoi_idx).scope + 1);
	last_prot = current_protection;
	
	/* For an AOI union type, we will use a PRES_C inline struct_union. */
	map->pres_c_mapping_u_u.struct_i = p_inline_struct_union(au,
								 *out_ptc,
								 ctype);
	
	pop_ptr(scope_stack);
	cast_del_scope_name(&current_scope_name);
}

/* This will create the functions associated with a union class */
void pg_corbaxx::p_union_class(p_type_collection *ptc)
{
	cast_def_protection last_prot = current_protection;
	cast_type u_type, raw_type, type, type2;
	cast_scoped_name scn;
	cast_scope *scope;
	tag_list *tl, *pt_tl;
	const char *u_name;
	
	pt_tl = ptc->get_tag_list();
	tl = create_tag_list(0);
	add_tag(pt_tl, "main", TAG_TAG_LIST, tl);
	add_tag(tl, "form", TAG_STRING, "class");
	u_name = ptc->get_name();
	cast_add_scope_name(&current_scope_name,
			    u_name,
			    null_template_arg_array);
	scn = cast_new_scoped_name(u_name, NULL);
	u_type = ptc->get_collection_ref()->
		find_type("definition")->get_type();
	raw_type = ptc->find_type("definition")->get_type();
	add_tag(pt_tl, "type", TAG_CAST_TYPE, raw_type);
	add_tag(tl, "name", TAG_CAST_SCOPED_NAME,
		cast_copy_scoped_name(&current_scope_name));
	scope = &u_type->cast_type_u_u.agg_type.scope;
	current_protection = CAST_PROT_PUBLIC;
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
		     cast_new_scoped_name(flick_asprintf("~%s", u_name), NULL),
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
		     PFA_FunctionKind, "T &operator=(const T &)",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_Scope, scope,
		     PFA_ReturnType, cast_new_reference_type(raw_type),
		     PFA_Parameter, type, "o", NULL,
		     PFA_TAG_DONE);
	current_protection = CAST_PROT_PRIVATE;
	type = prim_collections[PRIM_COLLECTION_LONG]->
		find_type("definition")->get_type();
	type2 = prim_collections[PRIM_COLLECTION_BOOLEAN]->
		find_type("definition")->get_type();
	add_function(tl,
		     cast_new_scoped_name("_reset", NULL),
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "_reset(long, bool)",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_Scope, scope,
		     PFA_ReturnType, cast_new_type(CAST_TYPE_VOID),
		     PFA_Parameter, type, "_new_disc_", NULL,
		     PFA_Parameter, type2, "_finalize_", NULL,
		     PFA_TAG_DONE);
	type = cast_new_pointer_type(cast_new_type(CAST_TYPE_VOID));
	add_function(tl,
		     cast_new_scoped_name("_discriminant", NULL),
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "_discriminant()",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_Scope, scope,
		     PFA_Spec, CAST_FUNC_VIRTUAL,
		     PFA_ReturnType, type,
		     PFA_TAG_DONE);
	add_function(tl,
		     cast_new_scoped_name("_reset", NULL),
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "_reset()",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_Scope, scope,
		     PFA_Spec, CAST_FUNC_VIRTUAL,
		     PFA_ReturnType, cast_new_type(CAST_TYPE_VOID),
		     PFA_TAG_DONE);
	add_function(tl,
		     cast_new_scoped_name("_access", NULL),
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "_access()",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_Scope, scope,
		     PFA_Spec, CAST_FUNC_VIRTUAL,
		     PFA_ReturnType, type,
		     PFA_Parameter, type2, "flag", NULL,
		     PFA_TAG_DONE);
	current_protection = last_prot;
	cast_del_scope_name(&current_scope_name);
}

/* End of file. */

