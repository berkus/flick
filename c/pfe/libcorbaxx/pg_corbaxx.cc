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

#include <string.h>

#include <mom/c/pg_corbaxx.hh>
#include <mom/c/libpres_c.h>
#include <mom/c/libcast.h>

pg_corbaxx::pg_corbaxx()
{
#define NAME_FORMAT(type)      names.formats[ type##_fmt]
#define NAME_LITERAL_STR(type) names.literals[ type##_lit].str
#define NAME_LITERAL_LEN(type) names.literals[ type##_lit].len
	
	/*
	 * CORBA does not specify the names of server skeletons nor the names
	 * of marshal/unmarshal stubs.
	 */
	NAME_FORMAT(client_stub) =		"%s";
	NAME_FORMAT(server_skel) =		"_dispatch";
	NAME_FORMAT(server_func) =		"%s";
	NAME_FORMAT(marshal_stub) =		"flick%_marshal%_stub%_%S";
	NAME_FORMAT(unmarshal_stub) =		"flick%_unmarshal%_stub%_%S";
	
	NAME_FORMAT(struct_slot) =			"%s";
	NAME_FORMAT(struct_union_tag) =			"";
	
	NAME_FORMAT(client_basic_object_type) =		"CORBA::Object";
	NAME_FORMAT(server_basic_object_type) =		"CORBA::Object";
	
	NAME_FORMAT(client_interface_object_type) =	"%s";
	NAME_FORMAT(server_interface_object_type) =	"%s";
	
	NAME_FORMAT(client_stub_object_type) =		"%I";
	NAME_FORMAT(server_func_object_type) =		"%I";
	
	NAME_FORMAT(client_stub_environment_type) =	"CORBA::Environment";
	NAME_FORMAT(server_func_environment_type) =	"CORBA::Environment";
	
	NAME_FORMAT(stub_invocation_id_type) =	"CORBA::Invocation_id";
	NAME_FORMAT(stub_client_ref_type) =	"CORBA::Client";
	
	NAME_FORMAT(stub_param) =			"%s";
	
	NAME_FORMAT(client_stub_object_param) =			"this";
	NAME_FORMAT(server_func_object_param) =			"this";
	
	NAME_FORMAT(const) =				"%s";
	NAME_FORMAT(type) =				"%s";
	
	NAME_FORMAT(enum_tag) =				"%s";
	NAME_FORMAT(enum_member) =			"%s";
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
	
	NAME_FORMAT(exception_type) =			"%s";
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
	
	NAME_LITERAL_STR(presentation_style) =		"corbaxx";
	NAME_LITERAL_LEN(presentation_style) =		sizeof("corbaxx") - 1;

	NAME_FORMAT(client_stub_scoped) =		"%n";
	NAME_FORMAT(server_skel_scoped) =		"%n";
	NAME_FORMAT(client_skel_scoped) =		"%n";
	NAME_FORMAT(server_func_scoped) =		"%n";
	NAME_FORMAT(receive_request_func_scoped) =	"%n";
	NAME_FORMAT(receive_reply_func_scoped) =	"%n";
	NAME_FORMAT(marshal_stub_scoped) =		"%n";
	NAME_FORMAT(unmarshal_stub_scoped) =		"%n";
	
	NAME_FORMAT(message_marshal_request_stub_scoped) =	"%n";
	NAME_FORMAT(message_unmarshal_request_stub_scoped) =	"%n";
	NAME_FORMAT(message_marshal_reply_stub_scoped) =	"%n";
	NAME_FORMAT(message_marshal_except_stub_scoped) =	"%n";
	NAME_FORMAT(message_unmarshal_reply_stub_scoped) =	"%n";

	NAME_FORMAT(message_request_type_scoped) =		"%n";
	NAME_FORMAT(message_reply_type_scoped) =		"%n";
	NAME_FORMAT(interface_message_request_type_scoped) =	"%n";
	NAME_FORMAT(interface_message_reply_type_scoped) =	"%n";
	
	NAME_FORMAT(send_request_stub_scoped) =	"%n";
	NAME_FORMAT(send_reply_stub_scoped) =		"%n";
	NAME_FORMAT(recv_request_stub_scoped) =	"%n";
	NAME_FORMAT(recv_reply_stub_scoped) =		"%n";
	
	NAME_FORMAT(continue_request_stub_scoped) =	"%n";
	NAME_FORMAT(continue_reply_stub_scoped) =	"%n";
	NAME_FORMAT(request_continuer_func_type_scoped) = "%n";
	NAME_FORMAT(reply_continuer_func_type_scoped) =   "%n";
	
	NAME_FORMAT(const_scoped) =				"%n";
	NAME_FORMAT(type_scoped) =				"%n";
	
	NAME_FORMAT(enum_tag_scoped) =				"%n";
	
	NAME_FORMAT(basic_message_type_scoped) =		"%n";
	NAME_FORMAT(client_basic_object_type_scoped) =		"%n";
	NAME_FORMAT(server_basic_object_type_scoped) =		"%n";
	
	NAME_FORMAT(client_interface_object_type_scoped) =	"%n";
	NAME_FORMAT(server_interface_object_type_scoped) =	"%n";
	
	NAME_FORMAT(client_stub_object_type_scoped) =		"%n";
	NAME_FORMAT(server_func_object_type_scoped) =		"%n";
	
	NAME_FORMAT(client_stub_environment_type_scoped) =	"%n";
	NAME_FORMAT(server_func_environment_type_scoped) =	"%n";
	
	NAME_FORMAT(stub_invocation_id_type_scoped) =	"%n";
	NAME_FORMAT(stub_client_ref_type_scoped) =	"%n";
	
	NAME_FORMAT(client_stub_client_sid_type_scoped) =	"%n";
	NAME_FORMAT(server_func_client_sid_type_scoped) =	"%n";
	
	NAME_FORMAT(client_stub_server_sid_type_scoped) =	"%n";
	NAME_FORMAT(server_func_server_sid_type_scoped) =	"%n";
	
	NAME_FORMAT(exception_type_scoped) =			"%n";
	
	NAME_FORMAT(allocator_function_scoped) =		"%n";
	NAME_FORMAT(deallocator_function_scoped) =		"%n";
	
	server_func_spec = CAST_FUNC_PURE|CAST_FUNC_VIRTUAL;
	client_func_spec = CAST_FUNC_VIRTUAL;
	
	cast_language = CAST_CXX;
	
	poa_scope_stack = create_ptr_stack();
	current_poa_scope_name = null_scope_name;
	except_aggregate_type = CAST_AGGREGATE_CLASS;
	struct_union_aggregate_type = CAST_AGGREGATE_CLASS;
}

/* End of file. */

