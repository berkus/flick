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

#include <mom/libaoi.h>

#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include <mom/c/pg_corbaxx.hh>

pres_c_inline pg_corbaxx::p_inline_exception(aoi_exception *ae,
					     p_type_collection *inl_ptc,
					     cast_type inl_ctype)
{
	pres_c_inline		inl   = pres_c_new_inline_struct(ae->slots.
								 slots_len);
	pres_c_inline_struct	*sinl = &(inl->pres_c_inline_u_u.struct_i);
	
	aoi_exception_slot	*slot;
	cast_func_type cfunc;
	p_type_collection *ptc;
	cast_type type;
	unsigned int i;
	int param_idx;
	aoi_type t;
	int cdef;
	
	/*****/
	
	/*
	 * XXX --- Need to save/restore `name' for now, for the benefit of
	 * `p_variable_array_type' at least.  Grrr.
	 */
	char *old_name = name;
	
	/* The function will be used to hold an
	   initializing constructor */
	cast_init_function_type(&cfunc, 0);
	cfunc.return_type = cast_new_type(CAST_TYPE_NULL);
	for ( i = 0; i < ae->slots.slots_len; i++) {
		slot = &(ae->slots.slots_val[i]);
		name = calc_struct_slot_name(slot->name);
		
		sinl->slots.slots_val[i].mint_struct_slot_index
			= i;
		sinl->slots.slots_val[i].inl
			= p_inline_type(slot->type,
					// calc_struct_slot_name(slot->name)
					name,
					inl_ptc,
					inl_ctype);
		
		/* The constructor parameters don't exactly match the
		   inline types so we need to handle them specially */
		ptc = 0;
		p_type(slot->type, &ptc);
		t = slot->type;
		while( t->kind == AOI_INDIRECT ) {
			t = in_aoi->defs.defs_val[t->aoi_type_u_u.
						 indirect_ref].binding;
		}
		switch( t->kind ) {
		case AOI_UNION:
		case AOI_STRUCT:
		case AOI_TYPED:
			type = ptc->find_type("definition")->get_type();
			type = cast_new_qualified_type(type, CAST_TQ_CONST);
			type = cast_new_reference_type(type);
			break;
		case AOI_FWD_INTRFC:
		case AOI_INTERFACE:
		case AOI_TYPE_TAG:
			type = ptc->find_type("pointer")->get_type();
			type = cast_new_qualified_type(type, CAST_TQ_CONST);
			break;
		case AOI_ARRAY: {
			unsigned amin, amax;
			
			aoi_get_array_len(in_aoi,
					  &t->aoi_type_u_u.array_def,
					  &amin,
					  &amax);
			if( (t->aoi_type_u_u.array_def.flgs
			     == AOI_ARRAY_FLAG_NULL_TERMINATED_STRING) ) {
				type = cast_new_prim_type(CAST_PRIM_CHAR, 0);
				type = cast_new_qualified_type(type,
							       CAST_TQ_CONST);
				type = cast_new_pointer_type(type);
			} else if( amin != amax ) {
				type = ptc->find_type("definition")->
					get_type();
				type = cast_new_qualified_type(type,
							       CAST_TQ_CONST);
				type = cast_new_reference_type(type);
			} else {
				type = ptc->find_type("definition")->
					get_type();
				type = cast_new_qualified_type(type,
							       CAST_TQ_CONST);
			}
			break;
		}
		default:
			type = ptc->find_type("definition")->get_type();
			break;
		}
		/* Add the type to the parameters */
		param_idx = cast_func_add_param(&cfunc);
		cfunc.params.params_val[param_idx].type = type;
		cfunc.params.params_val[param_idx].name =
			flick_asprintf("_%s_", name);
	}
	name = old_name;
	
	/* If there are slots than we need to make the special
	   constructor that has the same form of arguments as members */
	if( ae->slots.slots_len ) {
		union tag_data_u data;
		cast_scoped_name scn;
		tag_list *main_tl, *tl;
		cast_scope *scope;
		tag_item *ti;
		tag_data td;
		
		/* Add the constructor to the exception class */
		scope = &inl_ctype->cast_type_u_u.agg_type.scope;
		cdef = cast_add_def(scope,
				    cast_new_scoped_name(name, NULL),
				    CAST_SC_NONE,
				    CAST_FUNC_DECL,
				    ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
				    current_protection);
		scope->cast_scope_val[cdef].u.cast_def_u_u.func_type = cfunc;
		scope = &out_pres->stubs_cast;
		scn = cast_copy_scoped_name(&current_scope_name);
		cast_add_scope_name(&scn,
				    name,
				    null_template_arg_array);
		cast_add_scope_name(&scn,
				    name,
				    null_template_arg_array);
		cdef = cast_add_def(scope,
				    scn,
				    CAST_SC_NONE,
				    CAST_FUNC_DECL,
				    ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
				    current_protection);
		scope->cast_scope_val[cdef].u.cast_def_u_u.func_type = cfunc;
		
		/* Add the tags describing the constructor */
		main_tl = inl_ptc->get_tag_list();
		main_tl = find_tag(main_tl, "main")->data.tag_data_u.tl;
		tl = create_tag_list(0);
		add_tag(main_tl, "init_ctor", TAG_TAG_LIST, tl);
		if( !(ti = find_tag(main_tl, "pres_func")) )
			ti = add_tag(main_tl, "pres_func",
				     TAG_STRING_ARRAY, 0);
		data.str = "init_ctor";
		append_tag_data(&ti->data, data);
		add_tag(tl, "name", TAG_CAST_SCOPED_NAME,
			cast_new_scoped_name(name, NULL));
		add_tag(tl, "c_func", TAG_INTEGER, cdef);
		add_tag(tl, "kind", TAG_STRING, "init_ctor");
		td = create_tag_data(TAG_STRING_ARRAY, 0);
		for( i = 0; i < cfunc.params.params_len; i++ ) {
			data.str = cfunc.params.params_val[i].name;
			append_tag_data(&td, data);
		}
		add_tag(tl, "parameter", TAG_STRING_ARRAY, 0)->data = td;
	}
	
	return inl;
}

/* End of file. */

