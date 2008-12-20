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

#include <assert.h>

#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>
#include "private.hh"

void pg_state::p_typedef_def(aoi_type at)
{
	/* Create the mapping and the C type. */
	p_type_collection *ptc;
	cast_type ctype_name;
	pres_c_mapping map;
	
	/* Define the C type with a typedef. */
	ptc = p_new_type_collection(calc_name_from_ref(cur_aoi_idx));
	add_tail( &type_collections, &ptc->link );
	ptc->set_ref(cur_aoi_idx);
	
	p_type(at, &ptc);
	
	/* We need to catch AOI_FWD_INTRFC and see if
	   a stub will have to be generated or not.  If
	   the reference value is not -1 then it has
	   a real interface definition so there is no
	   reason to make a stub.  However, if the
	   interface is implied or has no real interface
	   then we need to make m/u stubs. */
	if( (at->kind == AOI_FWD_INTRFC) &&
	    at->aoi_type_u_u.fwd_intrfc_def != -1 )
		return;
	/* Create a CAST type name for the stubs to refer to. */
	ctype_name = ptc->find_type("definition")->get_type();
	map = ptc->get_collection_ref()->
		find_type("definition")->get_mapping();
	
	/* Create a PRES_C_MAPPING_STUB for the stubs. */
	pres_c_mapping stub_map = pres_c_new_mapping(PRES_C_MAPPING_STUB);
	
	/* Create the marshal/unmarshal stubs. */
	/*
	 * A little history... The code here used to be:
	 *     p_marshal_stub(at, ctype_name, stub_map);
	 *     p_unmarshal_stub(at, ctype_name, stub_map);
	 *
	 * But `map' tells us how to really marshal the parameter.  `stub_map'
	 * tells us to call the marshaling function --- duh!  We can't lose
	 * `map' if we want to be able to inline the body of the marshaling
	 * function!  So then I tried:
	 *
	 *     p_marshal_stub(at, ctype_name, map);
	 *     p_unmarshal_stub(at, ctype_name, map);
	 *
	 * But this produces a PRES_C structure that doesn't get past
	 * `pres_c_check' because we associate a type name (`ctype_name') with
	 * a mapping other than a stub mapping.  In other words, although we
	 * preserve the "structural" mapping, we've lost the stub mapping!
	 *
	 * So now, we give both maps to the function-producing functions.
	 */
	p_marshal_stub(at, ctype_name, stub_map, map);
	p_unmarshal_stub(at, ctype_name, stub_map, map);
	
	/* XXX - does "map" need to be assigned to anything else? */
	ptc->define_types();
}

/* End of file. */

