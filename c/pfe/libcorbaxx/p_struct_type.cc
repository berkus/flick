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

#include <stdlib.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include <mom/c/pg_corbaxx.hh>

void pg_corbaxx::p_struct_type(aoi_struct *as,
			       p_type_collection **out_ptc)
{
	p_type_collection *ptc;
	p_type_node *ptn;
	char *struct_name;
	union tag_data_u data;
	tag_data *pt_td;
	tag_list *tl, *main_tl;
	cast_scoped_name scn;
	
	/* Make the CAST structures */
	struct_name = calc_type_name(a(cur_aoi_idx).name);
	cast_type ctype = cast_new_aggregate_type(struct_aggregate_type, 0);
	
	ctype->cast_type_u_u.agg_type.name =
		cast_new_scoped_name(struct_name, NULL);
	
	/* Make the type collection for the struct */
	ptc = p_new_type_collection(struct_name);
	
	if( (*out_ptc) ) {
		tl = (*out_ptc)->get_tag_list();
	} else {
		pt_td = &(find_tag(out_pres->pres_attrs, "pres_type")->data);
		tl = create_tag_list(0);
		data.tl = tl;
		ptc->set_attr_index(append_tag_data(pt_td, data));
		ptc->set_tag_list(tl);
	}
	
	/* Add some tags to describe the struct */
	add_tag(tl, "idl_type", TAG_STRING, "struct");
	add_tag(tl, "size", TAG_STRING,
		isVariable(a(cur_aoi_idx).binding) ? "variable" : "fixed");
	main_tl = create_tag_list(0);
	add_tag(tl, "main", TAG_TAG_LIST, main_tl);
	add_tag(main_tl, "form", TAG_STRING, "struct");
	
	ptn = new p_type_node;
	ptn->set_name("definition");
	ptn->set_format("%s");
	ptn->set_type(ctype);
	
	pres_c_mapping map = pres_c_new_mapping(PRES_C_MAPPING_STRUCT);
	ptn->set_mapping(map);
	ptc->add_type("default", ptn);
	
	if( *out_ptc )
		(*out_ptc)->set_collection_ref(ptc);
	else
		*out_ptc = ptc;
	
	ptc = *out_ptc;
	
	/* Add a _var and _out types */
	p_type_var(ptc, a(cur_aoi_idx).binding);
	
	if( isVariable(a(cur_aoi_idx).binding) ) {
		p_out_class(ptc, a(cur_aoi_idx).binding);
	} else {
		ptn = new p_type_node;
		ptn->set_name("out_pointer");
		ptn->set_format("%s_out");
		ptn->set_type(cast_new_reference_type(ptc->
						      find_type("definition")->
						      get_type()));
		ptc->add_type("default", ptn);
	}
	
	ptc->define_types();
	
	/* Setup some state, recurse through the aoi and then
	   do the inline_struct... */
	push_ptr(scope_stack, &ctype->cast_type_u_u.agg_type.scope);
	cast_add_scope_name(&current_scope_name,
			    struct_name,
			    null_template_arg_array);
	scn = cast_copy_scoped_name(&current_scope_name);
	add_tag(main_tl, "name", TAG_CAST_SCOPED_NAME, scn);
	
	gen_scope(a(cur_aoi_idx).scope + 1);
	
	map->pres_c_mapping_u_u.struct_i = p_inline_struct(as,
							   *out_ptc,
							   ctype);
	
	pop_ptr(scope_stack);
	cast_del_scope_name(&current_scope_name);
}

/* End of file. */

