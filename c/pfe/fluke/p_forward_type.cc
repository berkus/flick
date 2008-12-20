/*
 * Copyright (c) 1997, 1998, 1999 The University of Utah and
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
#include <mom/libaoi.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include <string.h>

#include "pg_fluke.hh"

/*
 * In Fluke, server-side target objects are represented by application-defined
 * structures.  (A target object is an object upon which an operation is being
 * invoked.)  Server work functions receive pointers to these structures.
 * Object references passed as parameters are presented as `mom_ref_t's, as
 * determined by `pg_fluke::p_indirect_type'.
 *
 * On the client side, all object references are presented as `mom_ref_t's.
 * When we are generating a client presentation, we never get to the method
 * below because `pg_fluke::p_typedef_def' screens out client-side interface
 * typedefs.
 */

void pg_fluke::p_forward_type(p_type_collection **out_ptc)
{
	p_type_collection *ptc;
	p_type_node *ptn;
	char *int_name;

	int_name = calc_server_basic_object_type_name(a(cur_aoi_idx).name);
	ptc = p_new_type_collection(int_name);
	
	cast_type  struct_type = cast_new_type(CAST_TYPE_STRUCT_NAME);
	
	struct_type->cast_type_u_u.struct_name =
		cast_new_scoped_name(int_name, NULL);

	ptn = new p_type_node;
	ptn->set_name("definition");
	ptn->set_format("%s");
	ptn->set_type(struct_type);
	ptn->set_mapping(pres_c_new_mapping(PRES_C_MAPPING_IGNORE));
	ptc->add_type("default", ptn);
	
	if( *out_ptc )
		(*out_ptc)->set_collection_ref(ptc);
	else
		*out_ptc = ptc;
	/*
	 * `PRES_C_MAPPING_IGNORE' is okay for now because the Fluke back end
	 * never tries to marshal or unmarshal a target object reference.
	 * However, the mapping style is bad news for any back end that *does*
	 * need to marshal/unmarshal target object references.
	 */
}

/* End of file. */

