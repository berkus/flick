/*
 * Copyright (c) 1995, 1996, 1998, 1999 The University of Utah and
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

void pg_state::p_except_type(aoi_exception *ae,
			     p_type_collection **out_ptc)
{
	cast_type ctype = cast_new_aggregate_type(except_aggregate_type, 0);
	ctype->cast_type_u_u.agg_type.name = cast_new_scoped_name(name, NULL);
	p_type_collection *ptc;
	p_type_node *ptn;
	
	ptc = p_new_type_collection(calc_name_from_ref(cur_aoi_idx));
	ptn = new p_type_node;
	ptn->set_flags(PTF_NAME_REF);
	ptn->set_name("definition");
	ptn->set_format("%s");
	ptn->set_type(ctype);
	
	push_ptr(scope_stack, &ctype->cast_type_u_u.agg_type.scope);
	
	pres_c_mapping map = pres_c_new_mapping(PRES_C_MAPPING_STRUCT);
	ptn->set_mapping(map);
	ptc->add_type("default", ptn);
	
	if( *out_ptc )
		(*out_ptc)->set_collection_ref(ptc);
	else
		*out_ptc = ptc;
	map->pres_c_mapping_u_u.struct_i = p_inline_exception(ae,
							      *out_ptc,
							      ctype);

	pop_ptr(scope_stack);
}

/* Given a MINT constant that refers to an exception id, this function
   creates and returns a corresponding CAST expression.  The default is
   to convert it's value directly. */
cast_expr pg_state::p_mint_exception_id_const_to_cast(mint_const mint_literal)
{
	return p_mint_const_to_cast(mint_literal);
}

