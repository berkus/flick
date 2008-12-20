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

#include <mom/compiler.h>
#include <mom/libaoi.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include <mom/c/pg_corbaxx.hh>

/*****************************************************************************/

void pg_corbaxx::p_any_type(p_type_collection **/*out_ptc*/)
{
	/*
	 * We should only get here if we have an `AOI_ANY' that is not in the
	 * `type' field of a containing `AOI_TYPED'.  (See `p_typed_type'.)
	 *
	 * In other words, if we get here, we have some sort of standalone,
	 * *untyped* `any' value, and there's no standard CORBA presentation
	 * for that.
	 */
	panic("In `pg_corbaxx::p_any_type', "
	      "there is no standard CORBA presentation for untyped `any' "
	      "values.");
}

/*****************************************************************************/

/*
 * Make the presentation functions for inserting and extracting values of the
 * `ptc'-described type into and from CORBA `any's.
 */
void pg_corbaxx::p_any_funcs(p_type_collection *ptc, aoi_type at)
{
	cast_type bool_ctype, any_ctype, obj_ctype, ctype1, ctype2;
	p_type_collection *any_ptc;
	unsigned amin = 0, amax = 0;
	cast_scope *scope;
	int is_variable;
	tag_list *tl;
	tag_item *ti;
	
	scope = root_scope;
	if( !(ti = find_tag(ptc->get_tag_list(), "main")) )
		ti = find_tag(ptc->get_collection_ref()->get_tag_list(),
			      "main");
	if( !ti )
		return;
	tl = ti->data.tag_data_u.tl;
	is_variable = isVariable(at);
	bool_ctype = prim_collections[PRIM_COLLECTION_BOOLEAN]->
		find_type("definition")->get_type();
	any_ptc = prim_collections[PRIM_COLLECTION_TYPED_ANY];
	any_ctype = any_ptc->find_type("definition")->get_type();
	while(at->kind == AOI_INDIRECT)
		at = in_aoi->defs.
			defs_val[at->aoi_type_u_u.indirect_ref].binding;
	ctype1 = cast_new_reference_type(any_ctype);
	switch(at->kind) {
	case AOI_ARRAY:
		aoi_get_array_len(in_aoi,
				  &at->aoi_type_u_u.array_def,
				  &amin,
				  &amax);
		if (!(at->aoi_type_u_u.array_def.flgs &
		      AOI_ARRAY_FLAG_NULL_TERMINATED_STRING)) {
			if( amin == amax ) {
				obj_ctype = ptc->find_type("any_type")->
					get_type();
				ctype2 = cast_new_qualified_type(
					obj_ctype,
					CAST_TQ_CONST);
				ctype2 = cast_new_reference_type(ctype2);
				break;
			}
		} else {
			obj_ctype = 0;
			ctype2 = 0;
			break;
		}
	default:
		obj_ctype = ptc->find_type("definition")->get_type();
		ctype2 = cast_new_qualified_type(obj_ctype, CAST_TQ_CONST);
		ctype2 = cast_new_reference_type(ctype2);
		break;
	case AOI_INTERFACE:
	case AOI_FWD_INTRFC:
		obj_ctype = ptc->find_type("pointer")->get_type();
		ctype2 = obj_ctype;
		break;
	}
	pres_function(out_pres,
		      tl,
		      null_scope_name,
		      cast_new_scoped_name("operator<<=", NULL),
		      PFA_Protection, CAST_PROT_PUBLIC,
		      PFA_FunctionKind, "operator<<=(copy)",
		      PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		      PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		      PFA_Scope, scope,
		      PFA_ReturnType, cast_new_type(CAST_TYPE_VOID),
		      PFA_Parameter, ctype1, "a", NULL,
		      PFA_Parameter, ctype2, "o", NULL,
		      PFA_TAG_DONE);
	switch( at->kind ) {
	case AOI_INTERFACE:
	case AOI_FWD_INTRFC:
		break;
	default:
		if( (at->kind == AOI_ARRAY) && (amin == amax) )
			break;
		ctype2 = cast_new_pointer_type(obj_ctype);
		pres_function(out_pres,
			      tl,
			      null_scope_name,
			      cast_new_scoped_name("operator<<=", NULL),
			      PFA_Protection, CAST_PROT_PUBLIC,
			      PFA_FunctionKind, "operator<<=(no-copy)",
			      PFA_DeclChannel, ch(cur_aoi_idx,
						  PG_CHANNEL_CLIENT_DECL),
			      PFA_ImplChannel, ch(cur_aoi_idx,
						  PG_CHANNEL_CLIENT_IMPL),
			      PFA_Scope, scope,
			      PFA_ReturnType, cast_new_type(CAST_TYPE_VOID),
			      PFA_Parameter, ctype1, "a", NULL,
			      PFA_Parameter, ctype2, "o", NULL,
			      PFA_TAG_DONE);
		break;
	}
	ctype1 = cast_new_qualified_type(any_ctype, CAST_TQ_CONST);
	ctype1 = cast_new_reference_type(ctype1);
	switch( at->kind ) {
	case AOI_INTERFACE:
	case AOI_FWD_INTRFC:
		ctype2 = cast_new_reference_type(ctype2);
		break;
	default:
		if( (at->kind == AOI_ARRAY) && (amin == amax) ) {
			ctype2 = cast_new_reference_type(obj_ctype);
			break;
		} else {
			ctype2 = cast_new_pointer_type(obj_ctype);
			ctype2 = cast_new_reference_type(ctype2);
		}
		break;
	}
	pres_function(out_pres,
		      tl,
		      null_scope_name,
		      cast_new_scoped_name("operator>>=", NULL),
		      PFA_Protection, CAST_PROT_PUBLIC,
		      PFA_FunctionKind, "bool operator>>=(any &, ptr &)",
		      PFA_DeclChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		      PFA_ImplChannel, ch(cur_aoi_idx, PG_CHANNEL_CLIENT_IMPL),
		      PFA_Scope, scope,
		      PFA_ReturnType, bool_ctype,
		      PFA_Parameter, ctype1, "a", NULL,
		      PFA_Parameter, ctype2, "o", NULL,
		      PFA_TAG_DONE);
}

/* End of file. */

