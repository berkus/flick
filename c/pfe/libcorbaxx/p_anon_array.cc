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
#include <mom/c/libpres_c.h>
#include <mom/c/libcast.h>
#include <mom/libaoi.h>

#include <mom/c/pg_corbaxx.hh>

/* p_array_dims will add an integer array tag called 'len'
   corresponding to the lengths of each dimension in the array */
void pg_corbaxx::p_array_dims(tag_list *tl, aoi_array *aa)
{
	union tag_data_u data;
	unsigned amin, amax;
	aoi_type elem_type;
	tag_data td;
	
	aoi_get_array_len(in_aoi,
			  aa,
			  &amin,
			  &amax);
	td = create_tag_data(TAG_INTEGER_ARRAY, 1);
	data.i = amax;
	set_tag_data(&td, 0, data);
	elem_type = aa->element_type;
	while( elem_type ) {
		switch( elem_type->kind ) {
		case AOI_INDIRECT:
			elem_type = in_aoi->defs.
				defs_val[elem_type->aoi_type_u_u.indirect_ref].
				binding;
			break;
		case AOI_ARRAY:
			if( !(elem_type->aoi_type_u_u.array_def.flgs &
			      AOI_ARRAY_FLAG_NULL_TERMINATED_STRING) ) {
				aoi_get_array_len(in_aoi,
						  &elem_type->aoi_type_u_u.
						  array_def,
						  &amin,
						  &amax);
				if( amin == amax ) {
					data.i = amax;
					append_tag_data(&td, data);
					elem_type = elem_type->aoi_type_u_u.
						array_def.element_type;
				}
				else
					elem_type = 0;
			}
			else
				elem_type = 0;
			break;
		default:
			elem_type = 0;
			break;
		}
	}
	add_tag(tl, "len", TAG_INTEGER_ARRAY, 0)->data = td;
}

/* p_anon_array is used to give a non-typedef'd array, an anonymous array,
   a name and the corresponding C++ infrastructure.  It is mainly used by
   union's when they have a member that is specified array. */
p_type_collection *pg_corbaxx::p_anon_array(char *arr_name,
					    p_type_collection *ptc_ref,
					    aoi_type at)
{
	p_type_collection *ptc, *elem_ptc = 0;
	union tag_data_u data;
	p_type_node *ptn;
	tag_data *pt_td;
	tag_list *tl;
	
	/* Make a new type collection to give everything a name, and
	   then just do the normal typedef style thing. */
	ptc = p_new_type_collection(arr_name);
	pt_td = &(find_tag(out_pres->pres_attrs, "pres_type")->data);
	tl = create_tag_list(0);
	data.tl = tl;
	ptc->set_tag_list(tl);
	ptc->set_collection_ref(ptc_ref);
	ptc->set_attr_index(append_tag_data(pt_td, data));
	p_type(at->aoi_type_u_u.array_def.element_type, &elem_ptc);
	p_type_var(ptc, at);
	p_array_forany(ptc, at);
	if( isVariable(at) ) {
		p_out_class(ptc, at);
	}
	else {
		ptn = new p_type_node;
		ptn->set_name("out_pointer");
		ptn->set_format("%s_out");
		ptn->set_type(ptc->find_type("definition")->get_type());
		ptc->add_type("default", ptn);
	}
	add_tag(tl, "anonymous", TAG_NONE);
	ptc->define_types();
	p_array_funcs(ptc);
	return(ptc);
}
