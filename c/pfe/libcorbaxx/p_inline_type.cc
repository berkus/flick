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

#include <mom/compiler.h>
#include <mom/c/pfe.hh>
#include <mom/libaoi.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include <mom/c/pg_corbaxx.hh>

/*
 * By default, this routine doesn't do any further inlining; it just creates an
 * atom and switches back into 1-1 mapping mode.  Subclasses can override this
 * to do more elaborate inlining.
 */
pres_c_inline pg_corbaxx::p_inline_type(aoi_type at,
					char *slot_name,
					p_type_collection *inl_ptc,
					cast_type inl_ctype)
{
	p_type_collection *ptc;
	cast_type ctype;
	pres_c_mapping map;
	int is_string = 0, is_object = 0;
	aoi_type t;
	
	p_usable_type(at, &ptc, &ctype, &map);
	
	if( at->kind == AOI_ARRAY ) {
		unsigned amin, amax;
		
		aoi_get_array_len(in_aoi,
				  &at->aoi_type_u_u.array_def,
				  &amin,
				  &amax);
		if( amin == amax ) {
			/* Its an anonymous array so we need to construct
			   C++ infrastructure for it */
			ptc = p_anon_array(flick_asprintf("_%s", slot_name),
					   ptc, at);
			ctype = ptc->find_type("definition")->get_type();
		}
	}
	t = at;
	while( t->kind == AOI_INDIRECT )
		t = in_aoi->defs.
			defs_val[t->aoi_type_u_u.indirect_ref].binding;
	switch( t->kind ) {
	case AOI_ARRAY:
		if (t->aoi_type_u_u.array_def.flgs &
		    AOI_ARRAY_FLAG_NULL_TERMINATED_STRING)
			is_string = 1;
		break;
	case AOI_FWD_INTRFC:
	case AOI_INTERFACE:
		is_object = 1;
		break;
	default:
		break;
	}
	if( inl_ptc ) {
		union tag_data_u data;
		tag_list *main_tl, *tl;
		tag_item *ti;
		int slot_num;
		
		/* Add tags describing this member */
		main_tl = inl_ptc->get_tag_list();
		tl = create_tag_list(0);
		if( !(ti = find_tag(main_tl, "member")) )
			ti = add_tag(main_tl, "member", TAG_TAG_LIST_ARRAY, 0);
		data.tl = tl;
		slot_num = append_tag_data(&ti->data, data);
		add_tag(tl, "name", TAG_STRING, name);
		add_tag(tl, "pres_index", TAG_INTEGER, ptc->get_attr_index());
		add_tag(tl, "type", TAG_CAST_TYPE, ctype);
		if( is_string ) {
			if( !(ti = find_tag(main_tl, "managed-strings")) )
				ti = add_tag(main_tl, "managed-strings",
					     TAG_INTEGER_ARRAY, 0);
			data.i = slot_num;
			append_tag_data(&ti->data, data);
		}
		if( is_object ) {
			if( !(ti = find_tag(main_tl, "managed-objects")) )
				ti = add_tag(main_tl, "managed-objects",
					     TAG_INTEGER_ARRAY, 0);
			data.i = slot_num;
			append_tag_data(&ti->data, data);
		}
	}
	
	return p_inline_add_atom(inl_ctype, slot_name, ctype, map);
}

/* End of file. */

