/*
 * Copyright (c) 1995, 1996, 1997, 1998, 1999 The University of Utah and
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

#include "private.hh"

void pg_state::p_union_type(aoi_union *au,
			    p_type_collection **out_ptc)
{
	p_type_collection *ptc;
	p_type_node *ptn;
	char *union_name;
	
	/*
	 * Create an aggregate type of whatever type the pg specified.
	 * The C presentation for an AOI union is not just a C union, but
	 * rather, a C struct containing a discriminator field and a C union.
	 */
	union_name = calc_type_name(a(cur_aoi_idx).name);
	cast_type ctype = cast_new_aggregate_type(struct_union_aggregate_type,
						  0);
	
	ctype->cast_type_u_u.agg_type.name =
		cast_new_scoped_name(union_name, NULL);
	
	ptc = p_new_type_collection(union_name);
	ptn = new p_type_node;
	ptn->set_flags(struct_type_node_flags);
	ptn->set_name("definition");
	ptn->set_format("%s");
	ptn->set_type(ctype);
	
	push_ptr(scope_stack, &ctype->cast_type_u_u.agg_type.scope);
	
	/* This PRES_C_MAPPING_STRUCT ``bumps'' us into inline mode. */
	pres_c_mapping map = pres_c_new_mapping(PRES_C_MAPPING_STRUCT);
	ptn->set_mapping(map);
	
	ptc->add_type("default", ptn);
	
	if( *out_ptc )
		(*out_ptc)->set_collection_ref(ptc);
	else
		*out_ptc = ptc;
	
	/* For an AOI union type, we will use a PRES_C inline struct_union. */
	map->pres_c_mapping_u_u.struct_i = p_inline_struct_union(au,
								 *out_ptc,
								 ctype);
	pop_ptr(scope_stack);
	
	/* Generate the contents of this union. */
	gen_scope(a(cur_aoi_idx).scope + 1);
}

/* End of file. */

