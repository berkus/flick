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

#include <mom/compiler.h>
#include <mom/c/pfe.hh>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>
#include <mom/libaoi.h>

#include "private.hh"

/*
 * If you override this method, you should also override `p_interface_type',
 * which handles normal interface declarations.
 */
void pg_state::p_forward_type(p_type_collection **out_ptc)
{
	p_type_collection *ptc;
	p_type_node *ptn;
	cast_scoped_name fwd_scn;
	pres_c_mapping map;
	char *fwd_name;
	
	/*
	 * Here we are handling *basic* interface types, not *typedef'ed*
	 * interface types.  This method is called by `p_type' when we have a
	 * direct reference to an AOI forward interface.  References to
	 * *specific* interface types always occur through AOI_INDIRECT nodes,
	 * and so we handle those cases in `p_indirect_type'.
	 */
	ptc = p_new_type_collection(a(cur_aoi_idx).name);
	ptn = new p_type_node;
	ptn->set_name("definition");
	ptn->set_format("%s");
	if (gen_client) {
		fwd_name = calc_client_basic_object_type_name(a(cur_aoi_idx).
							      name);
		fwd_scn = calc_client_basic_object_type_scoped_name(
			aoi_get_parent_scope(in_aoi, cur_aoi_idx), fwd_name);
	}
	else if (gen_server) {
		fwd_name = calc_server_basic_object_type_name(a(cur_aoi_idx).
							      name);
		fwd_scn = calc_server_basic_object_type_scoped_name(
			aoi_get_parent_scope(in_aoi, cur_aoi_idx), fwd_name);
	}
	else
		panic("In `pg_state::p_forward_type', "
		      "generating neither client nor server.");

	ptn->set_type(cast_new_type_scoped_name(fwd_scn));
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

