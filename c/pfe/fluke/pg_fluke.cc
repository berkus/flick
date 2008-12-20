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

#include "pg_fluke.hh"

/*
 * Initialize the aggregrate static constant members of class `pg_fluke'.
 * ANSI C++ doesn't let us initialize these in the class declaration --- blech!
 */
const char * const pg_fluke::fdev_seq_type_name    = "fluke_fdev_seq_octet";
const char * const pg_fluke::fdev_seq_alloc_name   = "fdev";
const char * const pg_fluke::fdev_seq_alloc_header = "fluke/sac/fdev_flick.h";

/*****/

pg_fluke::pg_fluke()
{
#define NAME_FORMAT(type)      names.formats[name_strings::##type##_fmt]
#define NAME_LITERAL_STR(type) names.literals[name_strings::##type##_lit].str
#define NAME_LITERAL_LEN(type) names.literals[name_strings::##type##_lit].len
	
	/* Should define them all... */
	NAME_FORMAT(server_func) =		"%I%_server%_%s";

	NAME_FORMAT(client_basic_object_type) =	"mom_ref_t"; /* Not used. */
	NAME_FORMAT(server_basic_object_type) =	"%S_struct";
	
	NAME_FORMAT(client_interface_object_type) =	"%S"; /* Not used. */
	NAME_FORMAT(server_interface_object_type) =	"%S";
	/*
	 * `%I' for server object type won't work, because `%I' gives you the
	 * interfaces you are *contained in*, which in this case would not
	 * include the interface itself (because the object we're defining
	 * isn't *inside* the interface)!
	 */
	
	NAME_FORMAT(client_stub_object_type) =		"mom_ref_t";
	NAME_FORMAT(server_func_object_type) =		"%I";
	
	NAME_FORMAT(client_stub_environment_type) =	"mom_exc_t";
	NAME_FORMAT(server_func_environment_type) =	"mom_exc_t";
	
	NAME_FORMAT(client_stub_client_sid_type) =	"mom_ref_t";
	NAME_FORMAT(server_func_client_sid_type) =	"mom_ref_t";
	NAME_FORMAT(client_stub_server_sid_type) =	"mom_ref_t";
	NAME_FORMAT(server_func_server_sid_type) =	"mom_ref_t";
	
	/* Not wired up yet. */
	NAME_FORMAT(operation_request_code) =		"req%_%P%_%s";
	NAME_FORMAT(operation_reply_code) =		"repl%_%P%_%s";
	
	/* Not wired up yet. */
	NAME_FORMAT(exception_code) =			"exc%_%P%_%s";
	
	// NAME_FORMAT(allocator_function) =		"%g_alloc";
	// NAME_FORMAT(deallocator_function) =		"%g_free";
	//
	// Until I update the BE:
	NAME_FORMAT(allocator_function) =		"%g";
	NAME_FORMAT(deallocator_function) =		"%g";
	
	NAME_FORMAT(interface_include_file) =		"fluke/%M.h";
	NAME_FORMAT(interface_default_include_file) =	"fluke/flick.h";
	
	/*********************************************************************/
	
	NAME_LITERAL_STR(separator) =			"_";
	NAME_LITERAL_LEN(separator) =			sizeof("_") - 1;
	
	NAME_LITERAL_STR(presentation_style) =		"mom";
	NAME_LITERAL_LEN(presentation_style) =		sizeof("mom") - 1;

	struct_type_node_flags = PTF_NAME_REF;
	union_type_node_flags = PTF_NAME_REF;
	enum_type_node_flags = PTF_NAME_REF;
}

/* End of file. */

