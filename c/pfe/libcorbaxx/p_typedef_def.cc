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

#include <assert.h>

#include <mom/libaoi.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include <mom/c/pg_corbaxx.hh>

void pg_corbaxx::p_typedef_def(aoi_type at)
{
	p_type_collection *ptc;
	p_type_node *ptn;
	cast_type ctype;
	pres_c_mapping map, stub_map;
	union tag_data_u data;
	tag_data *pt_td;
	tag_list *tl;
	tag_data td;
	
	pt_td = &(find_tag(out_pres->pres_attrs, "pres_type")->data);
	switch(at->kind) {
	case AOI_EXCEPTION:
		/* Setup a new named type collection and add
		   it to the list of collections */
		ptc = p_new_type_collection(calc_name_from_ref(cur_aoi_idx));
		add_tail( &type_collections, &ptc->link );
		tl = create_tag_list(0);
		data.tl = tl;
		ptc->set_attr_index(append_tag_data(pt_td, data));
		ptc->set_tag_list(tl);
		ptc->set_ref(cur_aoi_idx);
		ptc->set_id(get_repository_id(cur_aoi_idx));
		
		/* Get the type */
		p_type(at, &ptc);
		
		add_tag(tl, "name", TAG_STRING, ptc->get_name());
		
		ctype = ptc->find_type("definition")->get_type();
		
		ptn = new p_type_node;
		ptn->set_name("pointer");
		ptn->set_format("%s_ptr");
		ptn->set_type(cast_new_pointer_type(ctype));
		ptc->add_type("default", ptn);
		
		map = ptc->get_collection_ref()->
			find_type("definition")->get_mapping();
		
		stub_map = pres_c_new_mapping(PRES_C_MAPPING_STUB);
		
		p_marshal_stub(at, ctype, stub_map, map);
		p_unmarshal_stub(at, ctype, stub_map, map);
		
		ptc->define_types();
		p_any_funcs(ptc, at);
		break;
	case AOI_UNION:
	case AOI_STRUCT:
		/* Setup a new named type collection and add
		   it to the list of collections */
		ptc = p_new_type_collection(calc_name_from_ref(cur_aoi_idx));
		add_tail( &type_collections, &ptc->link );
		tl = create_tag_list(0);
		data.tl = tl;
		ptc->set_attr_index(append_tag_data(pt_td, data));
		ptc->set_tag_list(tl);
		ptc->set_ref(cur_aoi_idx);
		ptc->set_id(get_repository_id(cur_aoi_idx));
		
		/* Get the type */
		p_type(at, &ptc);
		
		add_tag(tl, "name", TAG_STRING, ptc->get_name());
		
		ctype = ptc->find_type("definition")->get_type();
		
		ptn = new p_type_node;
		ptn->set_name("pointer");
		ptn->set_format("");
		ptn->set_type(cast_new_pointer_type(ctype));
		ptc->add_type("default", ptn);
		
		map = ptc->get_collection_ref()->
			find_type("definition")->get_mapping();
		
		stub_map = pres_c_new_mapping(PRES_C_MAPPING_STUB);
		
		p_marshal_stub(at, ctype, stub_map, map);
		p_unmarshal_stub(at, ctype, stub_map, map);
		
		p_any_funcs(ptc, at);
		break;
	case AOI_ARRAY:
		unsigned amin, amax;
		
		/* Setup a new named type collection and add
		   it to the list of collections */
		ptc = p_new_type_collection(calc_name_from_ref(cur_aoi_idx));
		add_tail( &type_collections, &ptc->link );
		tl = create_tag_list(0);
		data.tl = tl;
		ptc->set_attr_index(append_tag_data(pt_td, data));
		ptc->set_tag_list(tl);
		ptc->set_ref(cur_aoi_idx);
		ptc->set_id(get_repository_id(cur_aoi_idx));
		
		p_type(at, &ptc);
		
		add_tag(tl, "name", TAG_STRING, ptc->get_name());
		
		aoi_get_array_len(in_aoi,
				  &at->aoi_type_u_u.array_def,
				  &amin,
				  &amax);
		if (!(at->aoi_type_u_u.array_def.flgs &
		      AOI_ARRAY_FLAG_NULL_TERMINATED_STRING)) {
			/* Add _var and _out types */
			if( amin == amax ) {
				p_type_var(ptc, at);
				p_array_forany(ptc, at);
			}
			if( isVariable(at) ) {
				if( amin == amax )
					p_out_class(ptc, at);
			}
			else {
				ptn = new p_type_node;
				ptn->set_name("out_pointer");
				ptn->set_format("%s_out");
				ptn->set_type(ptc->find_type("definition")->
					      get_type());
				ptc->add_type("default", ptn);
			}
		}
		else {
			/* If its a string then we need to change the mapping
			   to the raw mapping since all strings in C++ are
			   just char *'s, even if they are typedef'd */
			ptn = ptc->find_type("definition");
			ptn->set_mapping(ptc->get_collection_ref()->
					 find_type("definition")->
					 get_mapping());
			ptn = ptc->find_type("managed_string");
			ptn->set_mapping(ptc->get_collection_ref()->
					 find_type("managed_string")->
					 get_mapping());
		}
		/* Because the m/u stubs work on overloading we need to
		   suppress them from being output for strings since they
		   are all char *'s in C++. */
		if( !(at->aoi_type_u_u.array_def.flgs &
		      AOI_ARRAY_FLAG_NULL_TERMINATED_STRING) ) {
			ctype = ptc->find_type("definition")->get_type();
			map = ptc->get_collection_ref()->
				find_type("definition")->get_mapping();
			
			stub_map = pres_c_new_mapping(PRES_C_MAPPING_STUB);
			
			p_marshal_stub(at, ctype, stub_map, map);
			p_unmarshal_stub(at, ctype, stub_map, map);
		}
		ptc->define_types();
		if( amin == amax ) {
			p_array_funcs(ptc);
		}
		if( !(at->aoi_type_u_u.array_def.flgs &
		      AOI_ARRAY_FLAG_NULL_TERMINATED_STRING) ) {
			p_any_funcs(ptc, at);
		}
		break;
	case AOI_ENUM:
		/* Setup a new named type collection and add
		   it to the list of collections */
		ptc = p_new_type_collection(calc_name_from_ref(cur_aoi_idx));
		add_tail( &type_collections, &ptc->link );
		tl = create_tag_list(0);
		data.tl = tl;
		ptc->set_attr_index(append_tag_data(pt_td, data));
		ptc->set_tag_list(tl);
		ptc->set_ref(cur_aoi_idx);
		ptc->set_id(get_repository_id(cur_aoi_idx));
		
		p_type(at, &ptc);
		
		unsigned int lpc;
		
		/* Add the list of enum members to the tags */
		td = create_tag_data(TAG_STRING_ARRAY, 0);
		for( lpc = 0;
		     lpc < at->aoi_type_u_u.enum_def.defs.defs_len;
		     lpc++ ) {
			data.str = at->aoi_type_u_u.enum_def.
				defs.defs_val[lpc].name;
			append_tag_data(&td, data);
		}
		add_tag(tl, "member", TAG_STRING_ARRAY, 0)->data = td;
		add_tag(tl, "name", TAG_STRING, ptc->get_name());
		add_tag(tl, "size", TAG_STRING, "fixed");
		
		ptn = new p_type_node;
		ptn->set_name("out_pointer");
		ptn->set_format("%s_out");
		ptn->set_type(cast_new_reference_type(p_make_ctypename(cur_aoi_idx)));
		ptc->add_type("default", ptn);
		
		ctype = ptc->find_type("definition")->get_type();
		map = ptc->get_collection_ref()->
			find_type("definition")->get_mapping();
		
		add_tag(tl, "idl_type", TAG_STRING, "enum");
		
		stub_map = pres_c_new_mapping(PRES_C_MAPPING_STUB);
		
		p_marshal_stub(at, ctype, stub_map, map);
		p_unmarshal_stub(at, ctype, stub_map, map);
		ptc->define_types();
		break;
	case AOI_INDIRECT:
	case AOI_INTEGER:
	case AOI_FLOAT:
	case AOI_CHAR:
	case AOI_SCALAR:
		/* Setup a new named type collection and add
		   it to the list of collections */
		ptc = p_new_type_collection(calc_name_from_ref(cur_aoi_idx));
		add_tail( &type_collections, &ptc->link );
		tl = create_tag_list(0);
		data.tl = tl;
		ptc->set_attr_index(append_tag_data(pt_td, data));
		ptc->set_tag_list(tl);
		ptc->set_ref(cur_aoi_idx);
		ptc->set_id(get_repository_id(cur_aoi_idx));
		
		p_type(at, &ptc);
		
		aoi_type t;
		
		add_tag(tl, "name", TAG_STRING, ptc->get_name());
		
		ctype = ptc->find_type("definition")->get_type();
		/* For indirect types 'typedef's of named types we need
		   to play around with the type for some things */
		t = at;
		while( t->kind == AOI_INDIRECT ) {
			t = in_aoi->defs.defs_val[t->aoi_type_u_u.
						 indirect_ref].binding;
		}
		switch( t->kind ) {
		case AOI_FWD_INTRFC:
		case AOI_INTERFACE:
			ctype = ptc->find_type("pointer")->get_type();
			break;
		case AOI_ARRAY:
			if (t->aoi_type_u_u.array_def.flgs
			    == AOI_ARRAY_FLAG_NULL_TERMINATED_STRING) {
				ctype = 0;
			}
			break;
		default:
			break;
		}
		if( ctype ) {
			map = ptc->get_collection_ref()->
				find_type("definition")->get_mapping();
			
			stub_map = pres_c_new_mapping(PRES_C_MAPPING_STUB);
			
			p_marshal_stub(at, ctype, stub_map, map);
			p_unmarshal_stub(at, ctype, stub_map, map);
		}
		ptc->define_types();
		break;
	case AOI_FWD_INTRFC:
		if( at->aoi_type_u_u.fwd_intrfc_def != -1 )
			return;
		ptc = p_new_type_collection(calc_name_from_ref(cur_aoi_idx));
		add_tail( &type_collections, &ptc->link );
		ptc->set_ref(cur_aoi_idx);
		
		p_type(at, &ptc);
		
		/* Create a CAST type name for the stubs to refer to. */
		ctype = ptc->find_type("pointer")->get_type();
		map = ptc->get_collection_ref()->
			find_type("definition")->get_mapping();
		
		/* Create a PRES_C_MAPPING_STUB for the stubs. */
		stub_map = pres_c_new_mapping(PRES_C_MAPPING_STUB);
		
		p_marshal_stub(at, ctype, stub_map, map);
		p_unmarshal_stub(at, ctype, stub_map, map);
		
		ptc->define_types();
		break;
	default:
		pg_state::p_typedef_def(at);
		break;
	}
}

/* End of file. */

