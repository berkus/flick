/*
 * Copyright (c) 1997, 1998 The University of Utah and
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

#include <string.h>

#include <mom/c/pg_corba.hh>

pg_corba::pg_corba()
{
#define NAME_FORMAT(type)      names.formats[ type##_fmt ]
#define NAME_LITERAL_STR(type) names.literals[ type##_lit ].str
#define NAME_LITERAL_LEN(type) names.literals[ type##_lit ].len
	
	/*
	 * CORBA does not specify the names of server skeletons nor the names
	 * of marshal/unmarshal stubs.
	 */
	NAME_FORMAT(client_stub) =		"%S";
	NAME_FORMAT(server_skel) =		"%I%_server";
	NAME_FORMAT(server_func) =		"%S";
	NAME_FORMAT(marshal_stub) =		"flick%_marshal%_stub%_%S";
	NAME_FORMAT(unmarshal_stub) =		"flick%_unmarshal%_stub%_%S";
	
	NAME_FORMAT(struct_slot) =			"%s";
	NAME_FORMAT(struct_union_tag) =			"";
	
	NAME_FORMAT(client_basic_object_type) =		"CORBA_Object";
	NAME_FORMAT(server_basic_object_type) =		"CORBA_Object";
	
	NAME_FORMAT(client_interface_object_type) =	"%S";
	NAME_FORMAT(server_interface_object_type) =	"%S";
	
	NAME_FORMAT(client_stub_object_type) =		"%I";
	NAME_FORMAT(server_func_object_type) =		"%I";
	
	NAME_FORMAT(client_stub_environment_type) =	"CORBA_Environment";
	NAME_FORMAT(server_func_environment_type) =	"CORBA_Environment";
	
	NAME_FORMAT(stub_invocation_id_type) =	"CORBA_Invocation_id";
	NAME_FORMAT(stub_client_ref_type) =	"CORBA_Client";
	
	NAME_FORMAT(stub_param) =			"%s";
	
	NAME_FORMAT(client_stub_object_param) =			"_obj";
	NAME_FORMAT(server_func_object_param) =			"_obj";
	
	/*
	 * XXX --- For historical reasons, the name of the environment param
	 * in the CORBA PG is different that the default set by the `pg_state'
	 * constructor.
	 */
	NAME_FORMAT(client_stub_environment_param) =		"_ev";
	NAME_FORMAT(server_func_environment_param) =		"_ev";
	
	NAME_FORMAT(client_stub_client_sid_param) =		"_csid";
	NAME_FORMAT(server_func_client_sid_param) =		"_csid";
	
	NAME_FORMAT(client_stub_required_server_sid_param) =	"_ssid";
	NAME_FORMAT(server_func_required_server_sid_param) =	"_ssid";
	
	NAME_FORMAT(client_stub_actual_server_sid_param) =	"_out_ssid";
	NAME_FORMAT(server_func_actual_server_sid_param) =	"_out_ssid";
	
	NAME_FORMAT(operation_request_code) =		"%S";
	NAME_FORMAT(operation_reply_code) =		"%S";
	
	NAME_FORMAT(exception_type) =			"%S";
	NAME_FORMAT(exception_code) =			"ex%_%S";
	
	// XXX --- Need to override `p_get_allocator/calc_*_name' methods
	// in order to get correct names for strings, user exceptions, etc.
	//
	// NAME_FORMAT(allocator_function) =		"CORBA_%S_alloc";
	// NAME_FORMAT(deallocator_function) =		"CORBA_free";
	//
	// Until I update the BE:
	NAME_FORMAT(allocator_function) =		"CORBA";
	NAME_FORMAT(deallocator_function) =		"CORBA";
	
	/*********************************************************************/
	
	NAME_LITERAL_STR(separator) =			"_";
	NAME_LITERAL_LEN(separator) =			sizeof("_") - 1;
	
	NAME_LITERAL_STR(presentation_style) =		"corba";
	NAME_LITERAL_LEN(presentation_style) =		sizeof("corba") - 1;

	make_prim_collections();
}

/* End of file. */

