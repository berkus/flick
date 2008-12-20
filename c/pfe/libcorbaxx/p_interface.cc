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
#include <assert.h>

#include <mom/libaoi.h>
#include <mom/libmint.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include <mom/c/pg_corbaxx.hh>

void pg_corbaxx::p_interface_def(aoi_interface *ai)
{
	cast_def_protection last_prot = current_protection;
	p_type_collection *ptc, *parent_ptc;
	union tag_data_u data;
	pres_c_mapping map;
	p_type_node *ptn;
	cast_type class_type;
	cast_scope *scope;
	unsigned int lpc;
	tag_list *tl;
	tag_data td;
	
	p_interface_def_include(ai);
	
	/* Setup the type collection for this interface, we leech off
	   of p_forward_type for now... */
	current_protection = CAST_PROT_NONE;
	scope = (cast_scope *)top_ptr(scope_stack);
	ptc = p_new_type_collection(calc_name_from_ref(cur_aoi_idx));
	add_tail( &type_collections, &ptc->link );
	ptc->set_ref(cur_aoi_idx);
	p_forward_type(&ptc);
	
	/* Add the list of parents. */
	tl = ptc->get_tag_list();
	td = create_tag_data(TAG_INTEGER_ARRAY, 0);
	for( lpc = 0; lpc < ai->parents.parents_len; lpc++ ) {
		parent_ptc = p_type_collection::
			find_collection(&type_collections,
					ai->parents.parents_val[lpc]->
					aoi_type_u_u.indirect_ref);
		data.i = parent_ptc->get_attr_index();
		append_tag_data(&td, data);
	}
	add_tag(tl, "parent_pres_index", TAG_INTEGER_ARRAY, 0)->data = td;
	
	/* Create the interface class */
	class_type = cast_new_class_type(0);
	
	/* We need to remove the definitions added by
	   p_forward_type and add our new class */
	ptn = ptc->find_type("definition");
	ptn->remove(ptc->find_scope("default"));
	ptn = ptc->get_collection_ref()->find_type("definition");
	ptn->remove(ptc->find_scope("default"));
	map = ptn->get_mapping();
	cast_add_scope_name(&current_scope_name,
			    name,
			    null_template_arg_array);
	
	ptn = new p_type_node;
	ptn->set_name("definition");
	ptn->set_format("%s");
	ptn->set_type(class_type);
	ptn->set_mapping(map);
	ptn->set_channel(ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL));
	ptc->add_type("default", ptn);
	
	/* Suppress the interface functions for the server side */
	p_interface_funcs(ptc, &class_type->cast_type_u_u.agg_type.scope);
	cast_del_scope_name(&current_scope_name);
	
	/* Make marshal/unmarshal stubs for the interface itself. */
	cast_type ctype_name;
	aoi_type_u at;
	
	at.kind = AOI_INTERFACE;
	at.aoi_type_u_u.interface_def = *ai;
	
	ctype_name = ptc->find_type("pointer")->get_type();
	map = ptc->get_collection_ref()->find_type("definition")->
		get_mapping();
	
	/* Create a PRES_C_MAPPING_STUB for the stubs. */
	pres_c_mapping stub_map = pres_c_new_mapping(PRES_C_MAPPING_STUB);
	
	p_marshal_stub(&at, ctype_name, stub_map, map);
	p_unmarshal_stub(&at, ctype_name, stub_map, map);
	
	ptc->define_types();
	
	/* Generate client/server code for this interface. */
	p_any_funcs(ptc, a(cur_aoi_idx).binding);
	
	push_ptr(scope_stack, &class_type->cast_type_u_u.agg_type.scope);
	cast_add_scope_name(&current_scope_name,
			    name,
			    null_template_arg_array);
	
	current_protection = CAST_PROT_PUBLIC;
	/* Generate any defs in our body */
	gen_scope(a(cur_aoi_idx).scope + 1);
	if (gen_client)	{
		/* Generate client stubs. */
		p_client_stubs(ai);
	}
	if (gen_server)	{
		/* Generate the server skeleton stub. */
		p_skel(ai);
	}
	pop_ptr(scope_stack);
	cast_del_scope_name(&current_scope_name);
	current_protection = last_prot;
}

void pg_corbaxx::p_interface_def_typedef(aoi_interface *ai)
{
	aoi_type_u at;
	
	at.kind                       = AOI_INTERFACE;
	at.aoi_type_u_u.interface_def = *ai;
	
	/* Define the interface type as a data object with real marshal and
	   unmarshal functions. */
	p_typedef_def(&at);
	
	if (async_stubs)
		p_interface_async_msg_types(ai);
}

void pg_corbaxx::p_interface_type(aoi_interface * /*ai*/,
				  p_type_collection **out_ptc)
{
	p_type_collection *ptc;
	pres_c_mapping map;
	p_type_node *ptn;
	
	ptc = p_new_type_collection(a(cur_aoi_idx).name);
	ptn = new p_type_node;
	ptn->set_name("definition");
	ptn->set_format("%s");
	map = pres_c_new_mapping(PRES_C_MAPPING_REFERENCE);
	map->pres_c_mapping_u_u.ref.kind = PRES_C_REFERENCE_COPY;
	map->pres_c_mapping_u_u.ref.ref_count = 1;
	map->pres_c_mapping_u_u.ref.arglist_name = ir_strlit("");
	ptn->set_mapping(map);
	ptc->add_type("default", ptn);
	
	if( *out_ptc )
		(*out_ptc)->set_collection_ref(ptc);
	else
		*out_ptc = ptc;
}

/* End of file. */
