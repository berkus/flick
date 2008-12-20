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
#include <mom/c/pfe.hh>
#include <mom/libaoi.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include <mom/c/pg_corbaxx.hh>

/* This takes an aoi_type and returns the 'usable' type, mapping
   and the type collection of the type. Usable simply means that
   the returned type is the proper one taken from the type
   collection and not just the "definition" type.
*/
void pg_corbaxx::p_usable_type(aoi_type at,
			       p_type_collection **out_ptc,
			       cast_type *out_ctype,
			       pres_c_mapping *out_map)
{
	p_type_collection *ptc = 0;
	pres_c_mapping map;
	p_type_node *ptn;
	cast_type ctype;
	aoi_type t;
	
	p_type(at, &ptc);
	ptn = ptc->find_type("definition");
	map = ptn->get_mapping();
	ctype = ptn->get_type();
	
	t = at;
	while( t->kind == AOI_INDIRECT ) {
		t = in_aoi->defs.defs_val[t->aoi_type_u_u.
					 indirect_ref].binding;
	}
	switch( t->kind ) {
	case AOI_FWD_INTRFC:
	case AOI_INTERFACE:
		ptn = ptc->find_type("managed_object");
		ctype = ptn->get_type();
		map = ptn->get_mapping();
		break;
	case AOI_ARRAY:
		if (t->aoi_type_u_u.array_def.flgs
		    == AOI_ARRAY_FLAG_NULL_TERMINATED_STRING) {
			ptn = ptc->find_type("managed_string");
			ctype = ptn->get_type();
			map = ptn->get_mapping();
		}
		break;
	case AOI_TYPE_TAG:
		ctype = ptc->find_type("smart_pointer")->get_type();
		break;
	default:
		break;
	}
	
	*out_ctype = ctype;
	*out_map = map;
	*out_ptc = ptc;
}
