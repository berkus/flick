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

#include <string.h>

#include <mom/compiler.h>
#include <mom/c/pfe.hh>
#include <mom/c/libcast.h>

pg_state::pg_state()
{
	int lpc;
	
	/*
	 * Initialize our AOI, MINT, and PRES_C data members.
	 */
	in_aoi = 0;
	out_pres = 0;
	top_union = mint_ref_null;
	aoi_to_mint_association = 0;
	
	/*
	 * Initailize the switches that control the presentation.
	 */
	gen_client = 0;
	gen_server = 0;
	
	client_stubs_for_inherited_operations = 1;
	server_funcs_for_inherited_operations = 1;
	async_stubs = 0;
	
	/*********************************************************************/
	
	/*
	 * Initialize the strings that are used to construct names.  Most
	 * presentation generators will override at least some of these values.
	 */
#define NAME_FORMAT(type)      names.formats[ type##_fmt ]
#define NAME_LITERAL_STR(type) names.literals[ type##_lit ].str
#define NAME_LITERAL_LEN(type) names.literals[ type##_lit ].len
	
	/* The `null' format should never ever be used. */
	NAME_FORMAT(null) =			"";
	
	NAME_FORMAT(client_stub) =		"%S";
	NAME_FORMAT(server_skel) =		"%I%_server";
	NAME_FORMAT(client_skel) =		"%I%_client";
	NAME_FORMAT(server_func) =		"%I%_server%_%s";
	NAME_FORMAT(receive_request_func) =	"%I%_%s%_do%_request";
	NAME_FORMAT(receive_reply_func) =	"%I%_%s%_do%_reply";
	NAME_FORMAT(marshal_stub) =		"flick%_marshal%_stub%_%S";
	NAME_FORMAT(unmarshal_stub) =		"flick%_unmarshal%_stub%_%S";

	NAME_FORMAT(client_stub_scoped) =		"";
	NAME_FORMAT(server_skel_scoped) =		"";
	NAME_FORMAT(client_skel_scoped) =		"";
	NAME_FORMAT(server_func_scoped) =		"";
	NAME_FORMAT(receive_request_func_scoped) =	"";
	NAME_FORMAT(receive_reply_func_scoped) =	"";
	NAME_FORMAT(marshal_stub_scoped) =		"";
	NAME_FORMAT(unmarshal_stub_scoped) =		"";

	NAME_FORMAT(message_marshal_request_stub) =	"%S%_encode%_request";
	NAME_FORMAT(message_unmarshal_request_stub) =	"%S%_decode%_request";
	NAME_FORMAT(message_marshal_reply_stub) =	"%S%_encode%_reply";
	NAME_FORMAT(message_marshal_except_stub) =	"%S%_encode%_exception";
	NAME_FORMAT(message_unmarshal_reply_stub) =	"%S%_decode%_reply";
	
	NAME_FORMAT(message_marshal_request_stub_scoped) =	"";
	NAME_FORMAT(message_unmarshal_request_stub_scoped) =	"";
	NAME_FORMAT(message_marshal_reply_stub_scoped) =	"";
	NAME_FORMAT(message_marshal_except_stub_scoped) =	"";
	NAME_FORMAT(message_unmarshal_reply_stub_scoped) =	"";
	
	
	NAME_FORMAT(message_request_type) =		"%I%_Request";
	NAME_FORMAT(message_reply_type) =		"%I%_Reply";
	NAME_FORMAT(interface_message_request_type) =	"%S%_Request";
	NAME_FORMAT(interface_message_reply_type) =	"%S%_Reply";
	
	NAME_FORMAT(message_request_type_scoped) =		"";
	NAME_FORMAT(message_reply_type_scoped) =		"";
	NAME_FORMAT(interface_message_request_type_scoped) =	"";
	NAME_FORMAT(interface_message_reply_type_scoped) =	"";
	
	NAME_FORMAT(send_request_stub) =	"%I%_send%_request";
	NAME_FORMAT(send_reply_stub) =		"%I%_send%_reply";
	NAME_FORMAT(recv_request_stub) =	"%I%_recv%_request";
	NAME_FORMAT(recv_reply_stub) =		"%I%_recv%_reply";
	
	NAME_FORMAT(send_request_stub_scoped) =	"";
	NAME_FORMAT(send_reply_stub_scoped) =		"";
	NAME_FORMAT(recv_request_stub_scoped) =	"";
	NAME_FORMAT(recv_reply_stub_scoped) =		"";
	
	NAME_FORMAT(continue_request_stub) =	"%S%_continue%_request";
	NAME_FORMAT(continue_reply_stub) =	"%S%_continue%_reply";
	NAME_FORMAT(request_continuer_func_type) = "%I%_Request%_Continuer";
	NAME_FORMAT(reply_continuer_func_type) =   "%I%_Reply%_Continuer";
	
	NAME_FORMAT(continue_request_stub_scoped) =	"";
	NAME_FORMAT(continue_reply_stub_scoped) =	"";
	NAME_FORMAT(request_continuer_func_type_scoped) = "";
	NAME_FORMAT(reply_continuer_func_type_scoped) =   "";
	
	NAME_FORMAT(const) =				"%S";
	NAME_FORMAT(type) =				"%S";
	
	NAME_FORMAT(const_scoped) =				"";
	NAME_FORMAT(type_scoped) =				"";
	
	NAME_FORMAT(enum_tag) =				"%S";
	NAME_FORMAT(enum_member) =			"%S";
	
	NAME_FORMAT(enum_tag_scoped) =				"";
	
	NAME_FORMAT(struct_slot) =			"%s";
	NAME_FORMAT(struct_union_tag) =			"";
	/*
	 * XXX --- Do we need access to the structure type name?
	 * XXX --- Do we need multiple format strings for different classes of
	 *         structure slots (e.g., discriminators)?
	 */
	
	// NAME_FORMAT(struct_union_discrim_slot) = ;
	
	NAME_FORMAT(basic_message_type) =		"flick_msg_t";
	NAME_FORMAT(client_basic_object_type) =		"flick_ref_t";
	NAME_FORMAT(server_basic_object_type) =		"flick_ref_t";
	
	NAME_FORMAT(basic_message_type_scoped) =		"";
	NAME_FORMAT(client_basic_object_type_scoped) =		"";
	NAME_FORMAT(server_basic_object_type_scoped) =		"";
	
	NAME_FORMAT(client_interface_object_type) =	"%S";
	NAME_FORMAT(server_interface_object_type) =	"%S";
	
	NAME_FORMAT(client_interface_object_type_scoped) =	"";
	NAME_FORMAT(server_interface_object_type_scoped) =	"";
	
	NAME_FORMAT(client_stub_object_type) =		"%I";
	NAME_FORMAT(server_func_object_type) =		"%I";
	
	NAME_FORMAT(client_stub_object_type_scoped) =		"";
	NAME_FORMAT(server_func_object_type_scoped) =		"";
	
	NAME_FORMAT(client_stub_environment_type) =	"flick_env_t";
	NAME_FORMAT(server_func_environment_type) =	"flick_env_t";
	
	NAME_FORMAT(client_stub_environment_type_scoped) =	"";
	NAME_FORMAT(server_func_environment_type_scoped) =	"";
	
	NAME_FORMAT(stub_invocation_id_type) =	"flick_invocation_id_t";
	NAME_FORMAT(stub_client_ref_type) =	"flick_client_t";
	
	NAME_FORMAT(stub_invocation_id_type_scoped) =	"";
	NAME_FORMAT(stub_client_ref_type_scoped) =	"";
	
	NAME_FORMAT(client_stub_client_sid_type) =	"flick_sid_t";
	NAME_FORMAT(server_func_client_sid_type) =	"flick_sid_t";
	
	NAME_FORMAT(client_stub_client_sid_type_scoped) =	"";
	NAME_FORMAT(server_func_client_sid_type_scoped) =	"";
	
	NAME_FORMAT(client_stub_server_sid_type) =	"flick_sid_t";
	NAME_FORMAT(server_func_server_sid_type) =	"flick_sid_t";
	
	NAME_FORMAT(client_stub_server_sid_type_scoped) =	"";
	NAME_FORMAT(server_func_server_sid_type_scoped) =	"";
	
	NAME_FORMAT(stub_param) =			"%s";
	
	/*
	 * XXX --- Change the names of the special stub parameters at your own
	 * peril!  For example, the runtime header files have certain parameter
	 * names hardwired.
	 *
	 * XXX --- It is important for the name of each special parameter to
	 * begin with an underscore, so that the back end won't munge the names
	 * when creating the corresponding local variable within a server
	 * dispatch function (and thus induce the runtime compatibility problem
	 * described above).
	 */
	NAME_FORMAT(client_stub_object_param) =			"_obj";
	NAME_FORMAT(server_func_object_param) =			"_obj";
	
	NAME_FORMAT(client_stub_environment_param) =		"_env";
	NAME_FORMAT(server_func_environment_param) =		"_env";
	
	NAME_FORMAT(client_stub_client_sid_param) =		"_csid";
	NAME_FORMAT(server_func_client_sid_param) =		"_csid";
	
	NAME_FORMAT(client_stub_required_server_sid_param) =	"_ssid";
	NAME_FORMAT(server_func_required_server_sid_param) =	"_ssid";
	
	NAME_FORMAT(client_stub_actual_server_sid_param) =	"_out_ssid";
	NAME_FORMAT(server_func_actual_server_sid_param) =	"_out_ssid";
	/*
	 * XXX --- Do we need access to the name of the operation/stub?
	 */
	
	NAME_FORMAT(operation_request_code) =		"%S";
	NAME_FORMAT(operation_reply_code) =		"%S";
	
	NAME_FORMAT(exception_type) =			"%S";
	NAME_FORMAT(exception_code) =			"%S";
	
	NAME_FORMAT(exception_type_scoped) =			"";
	
	NAME_FORMAT(allocator_function) =		"malloc";
	NAME_FORMAT(deallocator_function) =		"free";
	
	NAME_FORMAT(allocator_function_scoped) =		"";
	NAME_FORMAT(deallocator_function_scoped) =		"";
	
	NAME_FORMAT(presentation_include_file) =	"flick%/pres%/%g.h";
	NAME_FORMAT(interface_include_file) =		"";
	NAME_FORMAT(interface_default_include_file) =	"";
	
	/*********************************************************************/
	
	/* The `null' literal should never ever be used. */
	NAME_LITERAL_STR(null) =			"";
	NAME_LITERAL_LEN(null) =			0;
	
	NAME_LITERAL_STR(separator) =			"_";
	NAME_LITERAL_LEN(separator) =			sizeof("_") - 1;
	
	NAME_LITERAL_STR(presentation_style) =		"generic";
	NAME_LITERAL_LEN(presentation_style) =		sizeof("generic") - 1;
	
	NAME_LITERAL_STR(filename_component_separator) = "/";
	NAME_LITERAL_LEN(filename_component_separator) = sizeof("/") - 1;
	
	/*********************************************************************/
	
	/*
	 * Initialize stateful data members.
	 */
	parent_interface_ref = aoi_ref_null;
	derived_interface_ref = aoi_ref_null;
	
	for( lpc = 0; lpc < PG_CHANNEL_MAX; lpc++ ) {
		pg_channel_maps[lpc] = 0;
	}
	
	name = 0;
	scope_stack = create_ptr_stack();
	current_scope_name = null_scope_name;
	cur_aoi_idx = 0;
	
	calc_name_data.count = 0;
	calc_name_data.size = 16;
	calc_name_data.components =
		(calc_name_component *)
		mustmalloc(sizeof(calc_name_component) * calc_name_data.size);
	
	union_aggregate_type = CAST_AGGREGATE_UNION;
	struct_aggregate_type = CAST_AGGREGATE_STRUCT;
	struct_union_aggregate_type = CAST_AGGREGATE_STRUCT;
	except_aggregate_type = CAST_AGGREGATE_STRUCT;
	
	current_protection = CAST_PROT_NONE;
	
	new_list( &type_collections );
	current_scope_name = null_scope_name;
	
	server_func_spec = 0;
	client_func_spec = 0;
	mu_storage_class = CAST_SC_NONE;
	
	struct_type_node_flags = 0;
	union_type_node_flags = 0;
	enum_type_node_flags = 0;
}

/* End of file. */

