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

#include <string.h>

#include <mom/libaoi.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include <mom/c/pg_corbaxx.hh>

cast_ref pg_corbaxx::p_skel_cdef(const char *skel_name,
				 const char */*skel_type_name*/)
{
	cast_func_type cfunc_decl, cfunc_def;
	p_type_collection *ptc;
	cast_scope *scope;
	cast_type type;
	cast_scoped_name scname;
	cast_expr expr;
	cast_init init;
	int param_idx;
	int cdef;
	
	/* Locate our type collection */
	ptc = p_type_collection::find_collection(&type_collections,
						 cur_aoi_idx);
	/* Grab the poa definition so we can add stuff to id */
	type = ptc->get_collection_ref()->
		find_type("poa_definition")->get_type();
	scope = &type->cast_type_u_u.agg_type.scope;
	/* Add the skeleton function to the poa. */
	cast_init_function_type(&cfunc_decl, 0);
	cast_init_function_type(&cfunc_def, 0);
	cdef = cast_add_def(scope,
			    cast_new_scoped_name(skel_name, NULL),
			    CAST_SC_NONE,
			    CAST_FUNC_DECL,
			    ch(cur_aoi_idx, PG_CHANNEL_SERVER_DECL),
			    CAST_PROT_PUBLIC);
	cfunc_decl.spec = CAST_FUNC_VIRTUAL;
	cfunc_decl.return_type = cast_new_type(CAST_TYPE_VOID);
	cfunc_def.return_type = cast_new_type(CAST_TYPE_VOID);
	param_idx = cast_func_add_param(&cfunc_decl);
	type = cast_new_type_scoped_name(cast_new_scoped_name("CORBA",
							       "ServerRequest",
							      NULL));
	type = cast_new_reference_type(type);
	cfunc_decl.params.params_val[param_idx].type
		= type;
	cfunc_decl.params.params_val[param_idx].name
		= ir_strlit("_tao_req");
	param_idx = cast_func_add_param(&cfunc_def);
	cfunc_def.params.params_val[param_idx].type
		= type;
	cfunc_def.params.params_val[param_idx].name
		= ir_strlit("_tao_req");
	param_idx = cast_func_add_param(&cfunc_decl);
	type = cast_new_type(CAST_TYPE_VOID);
	type = cast_new_pointer_type(type);
	cfunc_decl.params.params_val[param_idx].type
		= type;
	cfunc_decl.params.params_val[param_idx].name
		= ir_strlit("_tao_context");
	param_idx = cast_func_add_param(&cfunc_def);
	cfunc_def.params.params_val[param_idx].type
		= type;
	cfunc_def.params.params_val[param_idx].name
		= ir_strlit("_tao_context");
	
	scope->cast_scope_val[cdef].u.cast_def_u_u.func_type = cfunc_decl;
	param_idx = cast_func_add_param(&cfunc_decl);
	type = cast_new_type_scoped_name(cast_new_scoped_name("CORBA",
							      "Environment",
							      NULL));
	type = cast_new_reference_type(type);
	scname = cast_new_scoped_name("CORBA",
				      "Environment",
				      "default_environment",
				      NULL);
	expr = cast_new_expr_scoped_name(scname);
	expr = cast_new_expr_call_0(expr);
	init = cast_new_init_expr(expr);
	cfunc_decl.params.params_val[param_idx].type = type;
	cfunc_decl.params.params_val[param_idx].name = ir_strlit("_tao_env");
	cfunc_decl.params.params_val[param_idx].default_value = init;
	param_idx = cast_func_add_param(&cfunc_def);
	cfunc_def.params.params_val[param_idx].type = type;
	cfunc_def.params.params_val[param_idx].name = ir_strlit("_tao_env");
	scope->cast_scope_val[cdef].u.cast_def_u_u.func_type = cfunc_decl;
	scname = cast_copy_scoped_name(&current_poa_scope_name);
	cast_add_scope_name(&scname,
			    skel_name,
			    null_template_arg_array);
	if( cast_scoped_name_is_empty( &current_poa_scope_name ) ) {
		scname = cast_new_scoped_name(
			flick_asprintf("POA_%s",
				       a(cur_aoi_idx).name),
			NULL);
	}
	else {
		scname = cast_copy_scoped_name(&current_poa_scope_name);
		cast_add_scope_name(&scname,
				    a(cur_aoi_idx).name,
				    null_template_arg_array);
	}
	cast_add_scope_name(&scname,
			    skel_name,
			    null_template_arg_array);
	cdef = cast_add_def(&out_pres->stubs_cast,
			    scname,
			    CAST_SC_NONE,
			    CAST_FUNC_DECL,
			    ch(cur_aoi_idx, PG_CHANNEL_SERVER_IMPL),
			    CAST_PROT_PUBLIC);
	out_pres->stubs_cast.cast_scope_val[cdef].u.cast_def_u_u.func_type =
		cfunc_def;
	return cdef;
}

void pg_corbaxx::p_server_func_make_decl(aoi_interface *ai,
					 aoi_operation *ao,
					 char *opname,
					 cast_func_type *cfunc)
{
	union tag_data_u data;
	cast_ref cr;
	cast_scope *scope;
	cast_type ctype;
	cast_scope *deep_scope;
	p_type_collection *ptc;
	cast_scoped_name scn = cast_new_scoped_name(opname, NULL);
	tag_list *pt_tl, *tl;
	unsigned int lpc;
	tag_item *ti;
	tag_data td, td2;
	
	pg_state::p_server_func_make_decl(ai, ao, opname, cfunc);
	
	ptc = p_type_collection::find_collection(&type_collections,
						 cur_aoi_idx);
	/* For every operation in the server we need to
	   add use the tags to describe it. */
	pt_tl = ptc->get_tag_list();
	tl = create_tag_list(0);
	if( !(ti = find_tag(pt_tl, "operation")) )
		ti = add_tag(pt_tl, "operation", TAG_TAG_LIST_ARRAY, 0);
	data.tl = tl;
	append_tag_data(&ti->data, data);
	add_tag(tl, "name", TAG_STRING, opname);
	add_tag(tl, "return_type", TAG_CAST_TYPE, cfunc->return_type);
	td = create_tag_data(TAG_STRING_ARRAY, 0);
	td2 = create_tag_data(TAG_CAST_TYPE_ARRAY, 0);
	for( lpc = 0; lpc < cfunc->params.params_len; lpc++ ) {
		if( !(cfunc->params.params_val[lpc].spec &
		      CAST_PARAM_IMPLICIT) ) {
			data.str = cfunc->params.params_val[lpc].name;
			append_tag_data(&td, data);
			data.ctype = cfunc->params.params_val[lpc].type;
			append_tag_data(&td2, data);
		}
	}
	add_tag(tl, "parameter", TAG_STRING_ARRAY, 0)->data = td;
	add_tag(tl, "parameter_type", TAG_CAST_TYPE_ARRAY, 0)->data = td2;
	
	/* pg_state::p_server_func_make_decl added it to the interface
	   class but we also need to add it to the poa. */
	ctype = ptc->get_collection_ref()->
		find_type("poa_definition")->get_type();
	scope = &ctype->cast_type_u_u.agg_type.scope;
	deep_scope = scope;
	if( (ao->flags & (AOI_OP_FLAG_GETTER|AOI_OP_FLAG_SETTER)) ||
	    (cr = cast_find_def(&deep_scope,
				scn,
				CAST_FUNC_DECL)) == -1 ) {
		cr = cast_add_def(scope,
				  scn,
				  CAST_SC_NONE,
				  CAST_FUNC_DECL,
				  ch(cur_aoi_idx, PG_CHANNEL_SERVER_DECL),
				  current_protection);
		scope->cast_scope_val[cr].u.cast_def_u_u.func_type = *cfunc;
		scope->cast_scope_val[cr].u.cast_def_u_u.func_type.spec =
			CAST_FUNC_PURE|CAST_FUNC_VIRTUAL;
	}
	
	/* We also need to add it to the poa_tie... */
	ctype = ptc->get_collection_ref()->find_type("poa_tie_definition")->
		get_type();
	scope = &ctype->cast_type_u_u.template_type.def->
		cast_type_u_u.agg_type.scope;
	deep_scope = scope;
	pt_tl = find_tag(pt_tl, "poa_tie")->data.tag_data_u.tl;
	if( (ao->flags & (AOI_OP_FLAG_GETTER|AOI_OP_FLAG_SETTER)) ||
	    ((cr = cast_find_def(&deep_scope,
				 scn,
				 CAST_FUNC_DECL)) == -1) &&
	    strcmp(opname, "_is_a") ) {
		cast_type ttype, ftype;
		
		cr = cast_add_def(scope,
				  scn,
				  CAST_SC_NONE,
				  CAST_FUNC_DECL,
				  ch(cur_aoi_idx, PG_CHANNEL_SERVER_DECL),
				  current_protection);
		scope->cast_scope_val[cr].u.cast_def_u_u.func_type = *cfunc;
		scope->cast_scope_val[cr].u.cast_def_u_u.func_type.spec = 0;
		
		scope = &out_pres->stubs_cast;
		scn = cast_copy_scoped_name(&find_tag(pt_tl, "name")->
					    data.tag_data_u.scname);
		cast_add_template_arg_array_value(
			&scn.cast_scoped_name_val[scn.cast_scoped_name_len
						 - 1].args,
			cast_new_template_arg_name(
				cast_new_scoped_name("T", NULL)));
		cast_add_scope_name(&scn,
				    opname,
				    null_template_arg_array);
		ftype = cast_new_function_type(0, 0);
		ttype = cast_new_template_type(ftype);
		cast_template_add_param( &ttype->cast_type_u_u.
					 template_type,
					 CAST_TEMP_PARAM_CLASS,
					 "T",
					 0 );
		cr = cast_add_def(scope,
				  scn,
				  CAST_SC_NONE,
				  CAST_TYPE,
				  ch(cur_aoi_idx, PG_CHANNEL_SERVER_IMPL),
				  CAST_PROT_PUBLIC);
		scope->cast_scope_val[cr].u.cast_def_u_u.type = ttype;
		ftype->cast_type_u_u.func_type = *cfunc;
		ftype->cast_type_u_u.func_type.spec = 0;
		
		/* Create the tags to describe this function.  We add
		   it to the model functions since it has no known
		   name, but the body is known. */
		tl = create_tag_list(0);
		if( !(ti = find_tag(pt_tl, "model_func")) ) {
			ti = add_tag(pt_tl, "model_func",
				     TAG_TAG_LIST_ARRAY, 0);
		}
		data.tl = tl;
		append_tag_data(&ti->data, data);
		add_tag(tl, "kind", TAG_STRING, "tie_func");
		add_tag(tl, "name", TAG_STRING, opname);
		add_tag(tl, "c_func", TAG_INTEGER, cr);
		add_tag(tl, "return", TAG_INTEGER,
			(cfunc->return_type->kind != CAST_TYPE_VOID));
		td = create_tag_data(TAG_STRING_ARRAY, 0);
		for( lpc = 0; lpc < cfunc->params.params_len; lpc++ ) {
			if( !(cfunc->params.params_val[lpc].spec &
			      CAST_PARAM_IMPLICIT) ) {
				data.str = cfunc->params.params_val[lpc].name;
				append_tag_data(&td, data);
			}
		}
		add_tag(tl, "parameter", TAG_STRING_ARRAY, 0)->data = td;
	}
}

void pg_corbaxx::p_server_func_special_params(aoi_operation *ao,
					      stub_special_params *specials)
{
	p_type_collection *ptc;
	stub_special_params::stub_param_info *this_param;
	
	ptc = p_type_collection::find_collection(&type_collections,
						 cur_aoi_idx);
	/* Do the library thing... */
	pg_state::p_server_func_special_params(ao, specials);
	
	/*
	 * Set the object reference type and index.  In CORBA, the object
	 * reference is the first parameter to the server work function.  (This
	 * is what the PG library does anyway; we redo it just to be explicit!)
	 */
	this_param = &(specials->params[stub_special_params::object_ref]);
	
	this_param->spec = CAST_PARAM_IMPLICIT;
	this_param->ctype
		= cast_new_pointer_type(ptc->find_type("poa_definition")->
					get_type());
	this_param->index = 0;
	
	/*
	 * Set the environment reference type and index.  A CORBA environment
	 * reference has type `CORBA_Environment *' and appears after all of
	 * the normal parameters.  We place it after any SIDs as well.
	 */
	this_param = &(specials->params[stub_special_params::environment_ref]);
	
	this_param->ctype =
		cast_new_reference_type(
			cast_new_type_name(
				/*
				 * XXX --- Don't use `ao->name' until
				 * `pg_corbaxx::p_get_env_struct_type' has
				 * access to the operation name, too.
				 */
				calc_server_func_environment_type_name("")
				));
	this_param->index = (ao->params.params_len + 1
			     + (gen_sids ?
				2 /* after two SID arguments */ :
				0 /* no SID arguments */ ));
	
	/*
	 * Set the (effective) client SID index.  We do not set the type; CORBA
	 * has no standard for SID types so we accept whatever type the PG
	 * library has provided.
	 */
	this_param = &(specials->params[stub_special_params::client_sid]);
	
	if (gen_sids)
		this_param->index = (ao->params.params_len + 1);
	else
		this_param->index = -1;
	
	/*
	 * Set the required server SID index.  Again, we do not set the type.
	 */
	this_param = &(specials->params[stub_special_params::
				       required_server_sid]);
	
	if (gen_sids)
		this_param->index = (ao->params.params_len + 2);
	else
		this_param->index = -1;
	
	/*
	 * Set the actual server SID index.  Again, we do not set the type.
	 */
	this_param = &(specials->params[stub_special_params::
				       actual_server_sid]);
	
	this_param->index = -1;
	
	/* Finally, we're done! */
}

/*
 * This method determines the return type of a server work function.
 */
void pg_corbaxx::p_server_func_return_type(aoi_operation *ao, int mr,
					   cast_type *out_ctype,
					   pres_c_mapping *out_mapping)
{
	/* CORBA-ize the return value --- se `pg_corbaxx::p_param_type()'. */
	p_param_type(ao->return_type, mr, AOI_DIR_RET, out_ctype, out_mapping);
}

/* End of file. */

