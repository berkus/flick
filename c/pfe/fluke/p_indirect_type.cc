/*
 * Copyright (c) 1997, 1999 The University of Utah and
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

#include "pg_fluke.hh"

void pg_fluke::p_indirect_type(aoi_ref ref,
			       p_type_collection **out_ptc)
{
	p_type_collection *ptc;
	pres_c_mapping map;
	p_type_node *ptn;
	cast_type ctype;
	char *int_name;

	/*
	 * In the Fluke presentation, all object references (except possibly
	 * for the target object reference) passed as parameters are presented
	 * as `mom_ref_t' (controlled by `calc_client_basic_object_type_name').
	 *
	 * The target object is the object upon which an operation is being
	 * invoked.  The type of the target object reference is determined by
	 * `p_client_stub_object_type' and `p_server_func_object_type'.  This
	 * method is invoked to determine the presented type and mapping for
	 * other object references that are passed as operation parameters.
	 *
	 * Note that `mom_ref_t' is the type of the reference itself.  The
	 * presentation generator may add another level of indirection in order
	 * to present `inout', `out', and return parameters.
	 */
	switch (a(ref).binding->kind) {
	case AOI_INTERFACE:
	case AOI_FWD_INTRFC:
		int_name = calc_client_basic_object_type_name(a(ref).name);
		ptc = p_new_type_collection(int_name);
		/*
		 * In Fluke, the object reference type is `mom_ref_t', and
		 * this is how non-target object references are presented to
		 * BOTH clients and servers.  No need to check whether we are
		 * generating the client or the server.
		 */
		ctype = cast_new_type_name(int_name);
		/*
		 * This is mapped as an object reference, with a reference
		 * count of 1.
		 */
		map = pres_c_new_mapping(PRES_C_MAPPING_REFERENCE);
		map->pres_c_mapping_u_u.ref.kind
			= PRES_C_REFERENCE_COPY;
		map->pres_c_mapping_u_u.ref.ref_count = 1;
		map->pres_c_mapping_u_u.ref.arglist_name = ir_strlit("");
		
		ptn = new p_type_node;
		ptn->set_name("definition");
		ptn->set_format("%s");
		ptn->set_type(ctype);
		ptn->set_mapping(map);
		ptc->add_type("default", ptn);
		if( *out_ptc )
			(*out_ptc)->set_collection_ref(ptc);
		else
			*out_ptc = ptc;
		break;
		
	default:
		/* For all other types, just do the CORBA thing. */
		pg_corba::p_indirect_type(ref, out_ptc);
		break;
	}
}

/* End of file. */

