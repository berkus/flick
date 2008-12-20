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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Flick; see the file COPYING.  If not, write to
 * the Free Software Foundation, 59 Temple Place #330, Boston, MA 02111, USA.
 */

#include <assert.h>

#include <mom/libaoi.h>
#include <mom/compiler.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

/* A var args version that passes down to the real one below */
int pres_function(pres_c_1 *out_pres,
		  tag_list *parent_tl,
		  cast_scoped_name current_scope_name,
		  cast_scoped_name fname,
		  int tag, ...)
{
	va_list arg_addr;
	int retval;
	
	va_start( arg_addr, tag );
	retval = vpres_function(out_pres, parent_tl, current_scope_name,
				fname, tag, arg_addr);
	va_end( arg_addr );
	return( retval );
}

int vpres_function(pres_c_1 *out_pres,
		   tag_list *parent_tl,
		   cast_scoped_name current_scope_name,
		   cast_scoped_name fname,
		   int tag,
		   va_list arg_addr)
{
	cast_scope *scope = 0;
	cast_func_type cfunc_decl, cfunc_def;
	cast_storage_class sc = CAST_SC_NONE;
	cast_type ttype = 0, ftype = 0;
	union tag_data_u data;
	cast_scoped_name scname;
	tag_list *tl = 0;
	tag_item *ti;
	char *func_kind = "";
	data_channel_index decl_channel = PASSTHRU_DATA_CHANNEL;
	data_channel_index impl_channel = PASSTHRU_DATA_CHANNEL;
	cast_def_protection current_protection = CAST_PROT_NONE;
	tag_data td;
	int is_global = 0;
	int model = 0;
	int name_idx;
	int cdef;
	
	if( parent_tl )
		tl = create_tag_list(0);
	
	td = create_tag_data(TAG_STRING_ARRAY, 0);
	
	scname = cast_copy_scoped_name(&current_scope_name);
	name_idx = scname.cast_scoped_name_len - 1;
	cast_init_function_type(&cfunc_decl, 0);
	cast_init_function_type(&cfunc_def, 0);
	/* Walk through the list of tags and get anything interesting */
	while( tag != PFA_TAG_DONE ) {
		switch( tag ) {
		case PFA_DeclChannel:
			decl_channel = va_arg( arg_addr, data_channel_index );
			break;
		case PFA_ImplChannel:
			impl_channel = va_arg( arg_addr, data_channel_index );
			break;
		case PFA_StorageClass:
			sc = va_arg( arg_addr, cast_storage_class );
			break;
		case PFA_Scope:
			scope = va_arg( arg_addr, cast_scope * );
			break;
		case PFA_ReturnType:
			cfunc_decl.return_type = va_arg( arg_addr, cast_type );
			cfunc_def.return_type = cfunc_decl.return_type;
			break;
		case PFA_Parameter: {
			int param_idx;
			
			param_idx = cast_func_add_param(&cfunc_decl);
			cfunc_decl.params.params_val[param_idx].type =
				va_arg( arg_addr, cast_type );
			data.str = va_arg( arg_addr, char * );
			cfunc_decl.params.params_val[param_idx].name =
				ir_strlit(data.str);
			cfunc_decl.params.params_val[param_idx].default_value =
				va_arg( arg_addr, cast_init );
			param_idx = cast_func_add_param(&cfunc_def);
			cfunc_def.params.params_val[param_idx] =
				cfunc_decl.params.params_val[param_idx];
			cfunc_def.params.params_val[param_idx].default_value =
				0;
			append_tag_data(&td, data);
			break;
		}
		case PFA_Spec:
			cfunc_def.spec = va_arg(arg_addr, cast_func_spec);
			cfunc_decl.spec = cfunc_def.spec;
			break;
		case PFA_Template:
			ftype = cast_new_function_type(0, 0);
			ttype = cast_new_template_type(ftype);
			break;
		case PFA_TemplateParameter: {
			cast_template_param_kind tpkind;
			cast_template_arg default_val;
			char *name;
			
			tpkind = va_arg( arg_addr, cast_template_param_kind );
			name = va_arg( arg_addr, char * );
			default_val =  va_arg( arg_addr, cast_template_arg );
			cast_template_add_param( &ttype->cast_type_u_u.
						 template_type,
						 tpkind,
						 name,
						 default_val );
			cast_add_template_arg_array_value(
				&scname.cast_scoped_name_val[name_idx].args,
				cast_new_template_arg_name(
					cast_new_scoped_name(name, NULL)));
			break;
		}
		case PFA_FunctionKind:
			func_kind = va_arg(arg_addr, char *);
			break;
		case PFA_Tag: {
			tag_data_kind kind;
			char *tag_name;
			tag_item *ti;
			
			tag_name = va_arg(arg_addr, char *);
			kind = va_arg(arg_addr, tag_data_kind);
			data = va_arg(arg_addr, union tag_data_u);
			if( tl ) {
				ti = add_tag(tl, tag_name, TAG_NONE);
				ti->data.kind = kind;
				set_tag_data(&ti->data, 0, data);
			}
			break;
		}
		case PFA_Constructor:
			if( tl )
				add_tag(tl, "constructor", TAG_INTEGER, 1);
			break;
		case PFA_Model:
			model = 1;
			break;
		case PFA_Protection:
			current_protection = va_arg(arg_addr,
						    cast_def_protection);
			break;
		case PFA_GlobalFunction:
			is_global = 1;
			break;
		default:
			panic( "PFA_ Tag %d not understood", tag );
			break;
		}
		tag = va_arg( arg_addr, int );
	}
	if( tl ) {
		/* Add the list of parameters to the tag list */
		add_tag(tl, "parameter", TAG_STRING_ARRAY, 0)->data = td;
	}
	
	/* Add the function to the scope */
	cdef = cast_add_def(scope,
			    fname,
			    sc,
			    CAST_FUNC_DECL,
			    decl_channel,
			    current_protection);
	scope->cast_scope_val[cdef].u.cast_def_u_u.func_type = cfunc_decl;
	
	/* Add it to the stubs_cast */
	cast_add_scope_name(&scname,
			    fname.cast_scoped_name_val[0].name,
			    null_template_arg_array);
	if( ttype ) {
		/* If its a template than we need to add the template type */
		cdef = cast_add_def(&out_pres->stubs_cast,
				    scname,
				    sc,
				    CAST_TYPE,
				    impl_channel,
				    current_protection);
		out_pres->stubs_cast.cast_scope_val[cdef].u.
			cast_def_u_u.type = ttype;
		ftype->cast_type_u_u.func_type = cfunc_def;
		ftype->cast_type_u_u.func_type.spec &= ~CAST_FUNC_VIRTUAL;
	} else {
		cdef = cast_add_def(&out_pres->stubs_cast,
				    scname,
				    sc,
				    CAST_FUNC_DECL,
				    impl_channel,
				    current_protection);
		out_pres->stubs_cast.cast_scope_val[cdef].u.cast_def_u_u.
			func_type = cfunc_def;
		out_pres->stubs_cast.cast_scope_val[cdef].u.cast_def_u_u.
			func_type.spec &= ~CAST_FUNC_VIRTUAL;
	}
	
	if( tl ) {
		/* Record the function index in the stubs_cast */
		add_tag(tl, "c_func", TAG_INTEGER, cdef);
		/* Record the name used to call the function, if its static
		   then we need a fully scoped name */
		if( (sc == CAST_SC_STATIC) || is_global )
			data.scname = scname;
		else
			data.scname = fname;
		add_tag(tl, "name", TAG_CAST_SCOPED_NAME, data.scname);
		add_tag(tl, "kind", TAG_STRING, func_kind);
		if( model ) {
			/* The function has no unique name so we add the
			   tag_list to the list of "model" functions.  Model
			   functions have similar bodies but dissimilar
			   names. */
			if( !(ti = find_tag(parent_tl, "model_func")) ) {
				ti = add_tag(parent_tl, "model_func",
					     TAG_TAG_LIST_ARRAY, 0);
			}
			data.tl = tl;
			append_tag_data(&ti->data, data);
		} else {
			/* The function has a unique name, func_kind, so we add
			   the tag_list with that name and record the name in
			   the pres_func tag_list */
			add_tag(parent_tl, func_kind, TAG_TAG_LIST, tl);
			if( !(ti = find_tag(parent_tl, "pres_func")) ) {
				ti = add_tag(parent_tl, "pres_func",
					     TAG_STRING_ARRAY, 0);
			}
			data.str = func_kind;
			append_tag_data(&ti->data, data);
		}
	}
	return( cdef );
}
