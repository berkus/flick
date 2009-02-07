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

#include <mom/c/libpres_c.h>
#include <mom/c/libcast.h>

#include "pg_sun.hh"

void pg_sun::make_prim_collections()
{
	p_type_collection *ptc;
	p_scope_node *psn;
	p_type_node *ptn;
	
	pg_state::make_prim_collections();

	delete prim_collections[PRIM_COLLECTION_BOOLEAN];
	
	ptc = new p_type_collection;
	psn = new p_scope_node;
	psn->set_name("default");
	ptc->add_scope(psn);
	ptn = new p_type_node;
	ptc->set_name("boolean");
	ptn->set_name("definition");
	ptn->set_format("%s");
	ptn->set_type(cast_new_prim_type(CAST_PRIM_INT, 0));
	ptn->set_mapping(pres_c_new_mapping(PRES_C_MAPPING_DIRECT));
	ptc->add_type("default", ptn);
	prim_collections[PRIM_COLLECTION_BOOLEAN] = ptc;
}

pg_sun::pg_sun()
{
#define NAME_FORMAT(type)      names.formats[ type##_fmt]
#define NAME_LITERAL_STR(type) names.literals[ type##_lit].str
#define NAME_LITERAL_LEN(type) names.literals[ type##_lit].len
	
	/*
	 * Override some of the format strings for Sun RPC-style presentations.
	 *
	 * XXX --- should do them all just to be safe.
	 */
	NAME_FORMAT(client_stub) =			"%s";
	NAME_FORMAT(server_skel) =			"%s";
	NAME_FORMAT(server_func) =			"%s";
	
	/*
	 * Note that `pg_sun' overrides the methods that determine the
	 * presented object types.  Object reference types are pointers to
	 * names types, not just named types.
	 */
	NAME_FORMAT(client_interface_object_type) =	"%S";
	NAME_FORMAT(server_interface_object_type) =	"%S";
	
	NAME_FORMAT(client_stub_object_type) =		"CLIENT";
	NAME_FORMAT(server_func_object_type) =		"svc_req";
	
	// NAME_FORMAT(allocator_function) =		"malloc";
	// NAME_FORMAT(deallocator_function) =		"free";
	//
	// Until I update the BE:
	NAME_FORMAT(allocator_function) =		"%g";
	NAME_FORMAT(deallocator_function) =		"%g";
	
	NAME_FORMAT(client_stub_environment_param) =		"flick_errno";
	NAME_FORMAT(server_func_environment_param) =		"flick_errno";
	
	/*********************************************************************/
	
	NAME_LITERAL_STR(presentation_style) =		"sun";
	NAME_LITERAL_LEN(presentation_style) =		sizeof("sun") - 1;
}

/* End of file. */

