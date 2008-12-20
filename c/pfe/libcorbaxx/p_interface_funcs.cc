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

#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include <mom/c/pg_corbaxx.hh>

/* This will construct the functions that are a
   part of the C++ interface objects */
void pg_corbaxx::p_interface_funcs(p_type_collection *ptc, cast_scope *scope)
{
	cast_scoped_name scn;
	cast_type iptr_type;
	p_type_node *ptn;
	cast_def_protection last_prot = current_protection;
	cast_type type, type2, env_type;
	tag_list *tl, *main_tl;
	cast_init def_env;
	cast_expr expr;
	
	tl = ptc->get_tag_list();
	main_tl = create_tag_list(0);
	add_tag(tl, "main", TAG_TAG_LIST, main_tl);
	add_tag(main_tl, "form", TAG_STRING, "class");
	ptn = ptc->find_type("pointer");
	add_tag(main_tl, "name", TAG_CAST_SCOPED_NAME,
		cast_copy_scoped_name(&current_scope_name));
	if( ptn )
		iptr_type = ptn->get_type();
	else
		iptr_type = cast_new_pointer_type(ptc->
						  find_type("definition")->
						  get_type());
	current_protection = CAST_PROT_PUBLIC;
	scn = cast_new_scoped_name("CORBA", "Environment", NULL);
	env_type = cast_new_type_scoped_name(scn);
	env_type = cast_new_reference_type(env_type);
	scn = cast_new_scoped_name("CORBA",
				   "Environment",
				   "default_environment",
				   NULL);
	expr = cast_new_expr_scoped_name(scn);
	expr = cast_new_expr_call_0(expr);
	def_env = cast_new_init_expr(expr);
	add_function(main_tl,
		     cast_new_scoped_name("_duplicate", NULL),
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "_duplicate()",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_Scope, scope,
		     PFA_StorageClass, CAST_SC_STATIC,
		     PFA_ReturnType, iptr_type,
		     PFA_Parameter, iptr_type, "obj", NULL,
		     PFA_TAG_DONE);
	
	type = cast_new_type_scoped_name(cast_new_scoped_name("CORBA",
							      "Object_ptr",
							      NULL));
	add_function(main_tl,
		     cast_new_scoped_name("_narrow", NULL),
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "_narrow()",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_Scope, scope,
		     PFA_StorageClass, CAST_SC_STATIC,
		     PFA_ReturnType, iptr_type,
		     PFA_Parameter, type, "obj", NULL,
		     PFA_Parameter, env_type, "_env", def_env,
		     PFA_TAG_DONE);
	add_function(main_tl,
		     cast_new_scoped_name("_unchecked_narrow", NULL),
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "_unchecked_narrow()",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_Scope, scope,
		     PFA_StorageClass, CAST_SC_STATIC,
		     PFA_ReturnType, iptr_type,
		     PFA_Parameter, type, "_obj", NULL,
		     PFA_Parameter, env_type, "_env", def_env,
		     PFA_TAG_DONE);
	
	add_function(main_tl,
		     cast_new_scoped_name("_nil", NULL),
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "_nil()",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_Scope, scope,
		     PFA_StorageClass, CAST_SC_STATIC,
		     PFA_ReturnType, iptr_type,
		     PFA_TAG_DONE);
	
	type = prim_collections[PRIM_COLLECTION_BOOLEAN]->
		find_type("definition")->get_type();
	type2 = prim_collections[PRIM_COLLECTION_CHAR]->
		find_type("definition")->get_type();
	type2 = cast_new_qualified_type(type2, CAST_TQ_CONST);
	type2 = cast_new_pointer_type(type2);
	add_function(main_tl,
		     cast_new_scoped_name("_is_a", NULL),
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "_is_a()",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_Scope, scope,
		     PFA_Spec, CAST_FUNC_VIRTUAL,
		     PFA_ReturnType, type,
		     PFA_Parameter, type2, "_type_id", NULL,
		     PFA_Parameter, env_type, "_env", def_env,
		     PFA_TAG_DONE);
	
	type = cast_new_prim_type(CAST_PRIM_CHAR, 0);
	type = cast_new_qualified_type(type, CAST_TQ_CONST);
	type = cast_new_pointer_type(type);
	add_function(main_tl,
		     cast_new_scoped_name("_interface_repository_id", NULL),
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "_interface_repository_id()",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_Scope, scope,
		     PFA_Spec, CAST_FUNC_CONST|CAST_FUNC_VIRTUAL,
		     PFA_ReturnType, type,
		     PFA_TAG_DONE);
	
	current_protection = CAST_PROT_PROTECTED;
	add_function(main_tl,
		     cast_new_scoped_name(name, NULL),
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "T()",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_Constructor,
		     PFA_Scope, scope,
		     PFA_ReturnType, cast_new_type(CAST_TYPE_NULL),
		     PFA_TAG_DONE);
	
	add_function(main_tl,
		     cast_new_scoped_name(flick_asprintf("~%s", name),
					  NULL),
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "~T()",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_Scope, scope,
		     PFA_ReturnType, cast_new_type(CAST_TYPE_NULL),
		     PFA_Spec, CAST_FUNC_VIRTUAL,
		     PFA_TAG_DONE);
	
	current_protection = CAST_PROT_PRIVATE;
	type = cast_new_type_name(name);
	type = cast_new_qualified_type(type, CAST_TQ_CONST);
	type = cast_new_reference_type(type);
	add_function(main_tl,
		     cast_new_scoped_name(name, NULL),
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "T(T &)",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_Constructor,
		     PFA_Scope, scope,
		     PFA_ReturnType, cast_new_type(CAST_TYPE_NULL),
		     PFA_Parameter, type, "v", NULL,
		     PFA_TAG_DONE);
	
	add_function(main_tl,
		     cast_new_scoped_name("operator=", NULL),
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "void operator=(const T &)",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		     PFA_Scope, scope,
		     PFA_ReturnType, cast_new_type(CAST_TYPE_VOID),
		     PFA_Parameter, type, "v", NULL,
		     PFA_TAG_DONE);
	current_protection = last_prot;
}

/* This will create the poa and its functions */
void pg_corbaxx::p_poa_class(p_type_collection *ptc)
{
	cast_type serv_type, ctype;
	cast_scoped_name save_name;
	cast_scoped_name sc_name, scn;
	cast_def_protection last_prot = current_protection;
	cast_scope *scope;
	p_type_node *ptn;
	tag_list *tl, *pt_tl;
	cast_type type, type2, env_type;
	char *poa_name;
	
	scn = cast_new_scoped_name("CORBA", "Environment", NULL);
	env_type = cast_new_type_scoped_name(scn);
	env_type = cast_new_reference_type(env_type);
	pt_tl = ptc->get_tag_list();
	tl = create_tag_list(0);
	add_tag(pt_tl, "poa", TAG_TAG_LIST, tl);
	add_tag(tl, "form", TAG_STRING, "class");
	serv_type = cast_new_class_type(0);
	poa_name = add_poa_scope(calc_name_from_ref(cur_aoi_idx));
	cast_add_scope_name(&current_poa_scope_name,
			    poa_name,
			    null_template_arg_array);
	push_ptr(poa_scope_stack, &serv_type->cast_type_u_u.agg_type.scope);
	save_name = current_scope_name;
	current_scope_name = current_poa_scope_name;
	sc_name = cast_new_scoped_name(poa_name, NULL);
	add_tag(tl, "name", TAG_CAST_SCOPED_NAME,
		cast_copy_scoped_name(&current_scope_name));
	
	ptn = new p_type_node;
	ptn->set_name("poa_forward_declaration");
	ptn->set_format("%s");
	ctype = cast_new_type(CAST_TYPE_CLASS_NAME);
	ctype->cast_type_u_u.class_name = cast_new_scoped_name(poa_name, NULL);
	ptn->set_type(ctype);
	ptn = ptc->add_type("poa", ptn);
	ptn->set_channel(ch(cur_aoi_idx, PG_CHANNEL_SERVER_DECL));
	
	ptn = new p_type_node;
	ptn->set_name("poa_pointer");
	ptn->set_format("%s_ptr");
	ctype = cast_new_type_name(poa_name);
	ptn->set_type(cast_new_pointer_type(ctype));
	ptn = ptc->add_type("poa", ptn);
	ptn->set_channel(ch(cur_aoi_idx, PG_CHANNEL_SERVER_DECL));
	
	serv_type->cast_type_u_u.agg_type.name = sc_name;
	cast_class_add_parent(serv_type,
			      CAST_PARENT_VIRTUAL|CAST_PARENT_PUBLIC,
			      cast_new_scoped_name("PortableServer",
						   "ServantBase",
						   NULL));
	
	scope = &serv_type->cast_type_u_u.agg_type.scope;
	
	current_protection = CAST_PROT_PROTECTED;
	
	/* <sc_name> () */
	add_function(tl,
		     sc_name,
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "POA_T()",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_SERVER_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_SERVER_IMPL),
		     PFA_Constructor,
		     PFA_Scope, scope,
		     PFA_ReturnType, cast_new_type(CAST_TYPE_NULL),
		     PFA_TAG_DONE);
	current_protection = CAST_PROT_PUBLIC;
	
	/* <sc_name> (<sc_name> &rhs) */
	type = cast_new_type_scoped_name(sc_name);
	type = cast_new_qualified_type(type, CAST_TQ_CONST);
	type = cast_new_reference_type(type);
	add_function(tl,
		     sc_name,
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "POA_T(POA_T &)",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_SERVER_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_SERVER_IMPL),
		     PFA_Constructor,
		     PFA_Scope, scope,
		     PFA_ReturnType, cast_new_type(CAST_TYPE_NULL),
		     PFA_Parameter, type, "rhs", NULL,
		     PFA_TAG_DONE);
	
	/* ~<sc_name> () */
	add_function(tl,
		     cast_new_scoped_name(flick_asprintf("~%s", poa_name),
					  NULL),
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "~POA_T()",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_SERVER_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_SERVER_IMPL),
		     PFA_Spec, CAST_FUNC_VIRTUAL,
		     PFA_Scope, scope,
		     PFA_ReturnType, cast_new_type(CAST_TYPE_NULL),
		     PFA_TAG_DONE);
	
	/* _this () */
	add_function(tl,
		     cast_new_scoped_name("_this", NULL),
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "_this()",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_SERVER_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_SERVER_IMPL),
		     PFA_Scope, scope,
		     PFA_ReturnType, cast_new_pointer_type(ptc->find_type("definition")->get_type()),
		     PFA_Parameter, cast_new_reference_type(cast_new_type_scoped_name(cast_new_scoped_name("CORBA", "Environment", NULL))), "_env", NULL,
		     PFA_TAG_DONE);
	
	type = prim_collections[PRIM_COLLECTION_BOOLEAN]->
		find_type("definition")->get_type();
	type2 = prim_collections[PRIM_COLLECTION_CHAR]->
		find_type("definition")->get_type();
	type2 = cast_new_qualified_type(type2, CAST_TQ_CONST);
	type2 = cast_new_pointer_type(type2);
	add_function(tl,
		     cast_new_scoped_name("_is_a", NULL),
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "_is_a()",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_SERVER_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_SERVER_IMPL),
		     PFA_Scope, scope,
		     PFA_Spec, CAST_FUNC_VIRTUAL,
		     PFA_ReturnType, type,
		     PFA_Parameter, type2, "type_id", NULL,
		     PFA_Parameter, env_type, "_env", NULL,
		     PFA_TAG_DONE);
	
	/* void *_downcast (const char *) */
	type = cast_new_type(CAST_TYPE_VOID);
	type = cast_new_pointer_type(type);
	type2 = cast_new_prim_type(CAST_PRIM_CHAR,0);
	type2 = cast_new_qualified_type(type2, CAST_TQ_CONST);
	type2 = cast_new_pointer_type(type2);
	add_function(tl,
		     cast_new_scoped_name("_downcast", NULL),
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "_downcast(const char *)",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_SERVER_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_SERVER_IMPL),
		     PFA_Scope, scope,
		     PFA_Spec, CAST_FUNC_VIRTUAL,
		     PFA_ReturnType, type,
		     PFA_Parameter, type2, "logical_type_id", NULL,
		     PFA_TAG_DONE);
	
	/* virtual const char *_interface_repository_id() const */
	ctype = cast_new_prim_type(CAST_PRIM_CHAR, 0);
	ctype = cast_new_qualified_type(ctype, CAST_TQ_CONST);
	ctype = cast_new_pointer_type(ctype);
	add_function(tl,
		     cast_new_scoped_name("_interface_repository_id", NULL),
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "_interface_repository_id()",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_SERVER_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_SERVER_IMPL),
		     PFA_Scope, scope,
		     PFA_Spec, CAST_FUNC_CONST|CAST_FUNC_VIRTUAL,
		     PFA_ReturnType, ctype,
		     PFA_TAG_DONE);
	
	ptn = new p_type_node;
	ptn->set_name("poa_definition");
	ptn->set_format("%s");
	ptn->set_type(serv_type);
	ptn = ptc->add_type("poa", ptn);
	ptn->set_channel(ch(cur_aoi_idx, PG_CHANNEL_SERVER_DECL));
	
	cast_del_scope_name(&current_poa_scope_name);
	pop_ptr(poa_scope_stack);
	current_scope_name = save_name;
	current_protection = last_prot;
}

/* This will create the poa tie and its functions */
void pg_corbaxx::p_poa_tie(p_type_collection *ptc)
{
	cast_def_protection last_prot = current_protection;
	cast_type tie_class, tie_type;
	cast_scoped_name sc_tie_name;
	cast_scoped_name save_name;
	cast_scope *tie_scope;
	p_type_node *ptn;
	cast_type type;
	char *tie_name;
	tag_list *tl, *pt_tl;
	
	pt_tl = ptc->get_tag_list();
	tl = create_tag_list(0);
	add_tag(pt_tl, "poa_tie", TAG_TAG_LIST, tl);
	add_tag(tl, "form", TAG_STRING, "class");
	tie_name = flick_asprintf("%s_tie", calc_name_from_ref(cur_aoi_idx));
	tie_class = cast_new_class_type(0);
	tie_name = add_poa_scope(tie_name);
	cast_add_scope_name(&current_poa_scope_name,
			    tie_name,
			    null_template_arg_array);
	push_ptr(poa_scope_stack, &tie_class->cast_type_u_u.
		 agg_type.scope);
	save_name = current_scope_name;
	current_scope_name = current_poa_scope_name;
	add_tag(tl, "name", TAG_CAST_SCOPED_NAME,
		cast_copy_scoped_name(&current_scope_name));
	sc_tie_name = cast_new_scoped_name(tie_name, NULL);
	tie_class->cast_type_u_u.agg_type.name =
		cast_new_scoped_name(tie_name, NULL);
	ptn = ptc->find_type("poa_definition");
	cast_class_add_parent(tie_class,
			      CAST_PARENT_PUBLIC,
			      ptc->get_collection_ref()->
			      find_type("poa_definition")->get_type()->
			      cast_type_u_u.agg_type.name);
	
	tie_scope = &tie_class->cast_type_u_u.agg_type.scope;
	
	current_protection = CAST_PROT_PUBLIC;
	
	/* <tie_name> (T &t) */
	add_function(tl,
		     sc_tie_name,
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "T_tie(T &)",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_SERVER_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_SERVER_IMPL),
		     PFA_Constructor,
		     PFA_Scope, tie_scope,
		     PFA_ReturnType, cast_new_type(CAST_TYPE_NULL),
		     PFA_Parameter, cast_new_reference_type(cast_new_type_name("T")), "t", NULL,
		     PFA_Template,
		     PFA_TemplateParameter, CAST_TEMP_PARAM_CLASS, "T", NULL,
		     PFA_TAG_DONE);
	/* <tie_name> (T &t, PortableServer::POA_ptr poa) */
	add_function(tl,
		     sc_tie_name,
		     PFA_Protection, current_protection,
		     PFA_Constructor,
		     PFA_FunctionKind, "T_tie(T &, poa)",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_SERVER_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_SERVER_IMPL),
		     PFA_Scope, tie_scope,
		     PFA_ReturnType, cast_new_type(CAST_TYPE_NULL),
		     PFA_Parameter, cast_new_reference_type(cast_new_type_name("T")), "t", NULL,
		     PFA_Parameter, cast_new_type_scoped_name(cast_new_scoped_name("PortableServer", "POA_ptr", NULL)), "poa", NULL,
		     PFA_Template,
		     PFA_TemplateParameter, CAST_TEMP_PARAM_CLASS, "T", NULL,
		     PFA_TAG_DONE);
	/* <tie_name> (T *t, CORBA::Boolean release = 1) */
	add_function(tl,
		     sc_tie_name,
		     PFA_Protection, current_protection,
		     PFA_Constructor,
		     PFA_FunctionKind, "T_tie(T *, rel)",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_SERVER_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_SERVER_IMPL),
		     PFA_Scope, tie_scope,
		     PFA_ReturnType, cast_new_type(CAST_TYPE_NULL),
		     PFA_Parameter, cast_new_pointer_type(cast_new_type_name("T")), "t", NULL,
		     PFA_Parameter, cast_new_type_scoped_name(cast_new_scoped_name("CORBA", "Boolean", NULL)), "release", cast_new_init_expr(cast_new_expr_lit_int(1, 0)),
		     PFA_Template,
		     PFA_TemplateParameter, CAST_TEMP_PARAM_CLASS, "T", NULL,
		     PFA_TAG_DONE);
	/* <tie_name> (T *t,
	   PortableServer::POA_ptr poa,
	   CORBA::Boolean release = 1) */
	add_function(tl,
		     sc_tie_name,
		     PFA_Protection, current_protection,
		     PFA_Constructor,
		     PFA_FunctionKind, "T_tie(T *, poa, rel)",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_SERVER_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_SERVER_IMPL),
		     PFA_Scope, tie_scope,
		     PFA_ReturnType, cast_new_type(CAST_TYPE_NULL),
		     PFA_Parameter, cast_new_pointer_type(cast_new_type_name("T")), "t", NULL,
		     PFA_Parameter, cast_new_type_scoped_name(cast_new_scoped_name("PortableServer", "POA_ptr", NULL)), "poa", NULL,
		     PFA_Parameter, cast_new_type_scoped_name(cast_new_scoped_name("CORBA", "Boolean", NULL)), "release", cast_new_init_expr(cast_new_expr_lit_int(1, 0)),
		     PFA_Template,
		     PFA_TemplateParameter, CAST_TEMP_PARAM_CLASS, "T", NULL,
		     PFA_TAG_DONE);
	/* ~<tie_name> () */
	add_function(tl,
		     cast_new_scoped_name(flick_asprintf("~%s", tie_name),
					  NULL),
		     PFA_Protection, current_protection,
		     PFA_Scope, tie_scope,
		     PFA_FunctionKind, "~T_tie()",
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_SERVER_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_SERVER_IMPL),
		     PFA_ReturnType, cast_new_type(CAST_TYPE_NULL),
		     PFA_Template,
		     PFA_TemplateParameter, CAST_TEMP_PARAM_CLASS, "T", NULL,
		     PFA_TAG_DONE);
	/* T *_tied_object() */
	add_function(tl,
		     cast_new_scoped_name("_tied_object", NULL),
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "T *_tied_object()",
		     PFA_Scope, tie_scope,
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_SERVER_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_SERVER_IMPL),
		     PFA_ReturnType, cast_new_pointer_type(cast_new_type_name("T")),
		     PFA_Template,
		     PFA_TemplateParameter, CAST_TEMP_PARAM_CLASS, "T", NULL,
		     PFA_TAG_DONE);
	/* void _tied_object(T &obj) */
	add_function(tl,
		     cast_new_scoped_name("_tied_object", NULL),
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "void _tied_object(T &obj)",
		     PFA_Scope, tie_scope,
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_SERVER_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_SERVER_IMPL),
		     PFA_ReturnType, cast_new_type(CAST_TYPE_VOID),
		     PFA_Parameter, cast_new_reference_type(cast_new_type_name("T")), "obj", NULL,
		     PFA_Template,
		     PFA_TemplateParameter, CAST_TEMP_PARAM_CLASS, "T", NULL,
		     PFA_TAG_DONE);
	/* void _tied_object(T &obj, CORBA::Boolean release = 1) */
	add_function(tl,
		     cast_new_scoped_name("_tied_object", NULL),
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "void _tied_object(T *obj, rel)",
		     PFA_Scope, tie_scope,
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_SERVER_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_SERVER_IMPL),
		     PFA_ReturnType, cast_new_type(CAST_TYPE_VOID),
		     PFA_Parameter, cast_new_reference_type(cast_new_type_name("T")), "obj", NULL,
		     PFA_Parameter, cast_new_type_scoped_name(cast_new_scoped_name("CORBA", "Boolean", NULL)), "release", cast_new_init_expr(cast_new_expr_lit_int(1, 0)),
		     PFA_Template,
		     PFA_TemplateParameter, CAST_TEMP_PARAM_CLASS, "T", NULL,
		     PFA_TAG_DONE);
	/* CORBA::Boolean _is_owner() */
	add_function(tl,
		     cast_new_scoped_name("_is_owner", NULL),
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "bool _is_owner()",
		     PFA_Scope, tie_scope,
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_SERVER_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_SERVER_IMPL),
		     PFA_ReturnType, cast_new_type_scoped_name(cast_new_scoped_name("CORBA", "Boolean", NULL)),
		     PFA_Template,
		     PFA_TemplateParameter, CAST_TEMP_PARAM_CLASS, "T", NULL,
		     PFA_TAG_DONE);
	/* void _is_owner(CORBA::Boolean b) */
	add_function(tl,
		     cast_new_scoped_name("_is_owner", NULL),
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "_is_owner(bool)",
		     PFA_Scope, tie_scope,
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_SERVER_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_SERVER_IMPL),
		     PFA_ReturnType, cast_new_type(CAST_TYPE_VOID),
		     PFA_Parameter, cast_new_type_scoped_name(cast_new_scoped_name("CORBA", "Boolean", NULL)), "b", NULL,
		     PFA_Template,
		     PFA_TemplateParameter, CAST_TEMP_PARAM_CLASS, "T", NULL,
		     PFA_TAG_DONE);
	/* PortableServer::POA_ptr _default_POA() */
	add_function(tl,
		     cast_new_scoped_name("_default_POA", NULL),
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "poa _default_POA()",
		     PFA_Scope, tie_scope,
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_SERVER_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_SERVER_IMPL),
		     PFA_ReturnType, cast_new_type_scoped_name(cast_new_scoped_name("PortableServer", "POA_ptr", NULL)),
		     PFA_Template,
		     PFA_TemplateParameter, CAST_TEMP_PARAM_CLASS, "T", NULL,
		     PFA_TAG_DONE);
	tie_type = cast_new_template_type(tie_class);
	cast_template_add_param(&tie_type->cast_type_u_u.template_type,
				CAST_TEMP_PARAM_CLASS,
				"T",
				0);
	/* <tie_name> (const <tie_name> &); */
	current_protection = CAST_PROT_PRIVATE;
	type = cast_new_type_name(tie_name);
	type = cast_new_qualified_type(type, CAST_TQ_CONST);
	type = cast_new_reference_type(type);
	add_function(tl,
		     sc_tie_name,
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "T_tie(const T &)",
		     PFA_Scope, tie_scope,
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_SERVER_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_SERVER_IMPL),
		     PFA_ReturnType, cast_new_type(CAST_TYPE_NULL),
		     PFA_Parameter, type, "c", NULL,
		     PFA_Template,
		     PFA_TemplateParameter, CAST_TEMP_PARAM_CLASS, "T", NULL,
		     PFA_TAG_DONE);
	add_function(tl,
		     cast_new_scoped_name("operator=", NULL),
		     PFA_Protection, current_protection,
		     PFA_FunctionKind, "void operator=(const T_tie &)",
		     PFA_Scope, tie_scope,
		     PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_SERVER_DECL),
		     PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_SERVER_IMPL),
		     PFA_ReturnType, cast_new_type(CAST_TYPE_VOID),
		     PFA_Parameter, type, "c", NULL,
		     PFA_Template,
		     PFA_TemplateParameter, CAST_TEMP_PARAM_CLASS, "T", NULL,
		     PFA_TAG_DONE);
	
	ptn = new p_type_node;
	ptn->set_name("poa_tie_definition");
	ptn->set_format("%s_tie");
	ptn->set_type(tie_type);
	ptn->set_flags(PTF_NO_REF);
	ptn = ptc->add_type("poa", ptn);
	ptn->set_channel(ch(cur_aoi_idx, PG_CHANNEL_SERVER_DECL));
	pop_ptr(poa_scope_stack);
	cast_del_scope_name(&current_poa_scope_name);
	current_scope_name = save_name;
	current_protection = last_prot;
}
