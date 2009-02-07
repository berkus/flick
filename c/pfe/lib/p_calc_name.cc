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
#include <string.h>

#include <mom/compiler.h>
#include <mom/libaoi.h>
#include <mom/c/libcast.h>
#include <mom/c/pfe.hh>

/*
 * This file contains `calc_name': the basic `pg_state' function that is used
 * to determine the names of most (someday, all) of the components that make up
 * a presentation.  These presentation components include client stubs, server
 * stubs, marshal/unmarshal stubs, data types, parameters, ... --- in short,
 * every piece of a presentation that has a name!
 *
 * `calc_name' is driven by `printf'-like format strings that define how the
 * names of various presentation elements should be constructed.  Each
 * `pg_state' has a set of name format strings, one for each class or kind of
 * presentation element.  The set of presentation element types is listed in
 * `mom/c/pfe.hh'.  The set of `%'-escapes interpreted by `calc_name' is listed
 * in the code below.
 *
 * `calc_name' is almost never called directly.  Rather, it is called through
 * helper methods that are each specific to one kind of element.  For example,
 * to compute the name of a client stub, a presentation generator should call
 * `calc_client_stub_name' rather than `calc_name'.  `calc_client_stub_name'
 * locates the format string for client stub names and passes it to `calc_name'
 * along with the AOI-defined (unscoped) name of the operation that will be
 * implemented.
 *
 * Because all of the name format strings are collected in one place within the
 * `pg_state' class, it is easy for individual presentation generators to
 * change the ``rules'' for building presented names.  It is also possible to
 * make a command-line option corresponding to each format string, so that
 * users can customize presentations to some degree.
 *
 * Because there is a method for each kind of presentation element, it is
 * possible for a presentation generator to customize name generation by
 * overriding specific `calc_*_name' methods.  This technique should be used
 * ONLY when the `calc_name' method cannot implement the required name
 * construction rules.  (Method overrides reduce users' ability to customize
 * name generation through the command line options describe above.)
 */


/*****************************************************************************/

/* Old cruft that still lingers... */
void pg_state::init_name_context()
{
	name = pg_state_name_strlit("");
}


/*****************************************************************************/

/*
 * We allow users to change the `pg_state' name format strings and literal
 * strings through command line options.  This facility will become obsolete
 * and go away when we implement a ``real'' presentation modification facility.
 *
 * The macro `DEFINE_CALC_NAME_FMT_OPTION' creates a `name_fmt_option_struct'
 * corresponding to a format string within a `pg_state'.  Similarly, the macro
 * `DEFINE_CALC_NAME_LIT_OPTION' creates a `name_lit_option_struct'
 * corresponding to a literal string.  The function `pg_state::build_flags' in
 * `pg_args.cc' scans the arrays of name format and literal options defined
 * below in order to create and execute the ``real'' command line option
 * structures.
 *
 * For both macros, `type' is the name of a format/literal string kind, minus
 * the `_fmt'/`_lit' extension, as defined in `mom/c/pfe.hh'.  `explain' is a
 * human-readable description of the type of elements to which the format is
 * applied (e.g., client stubs, object types, etc.), or a description of the
 * purpose of the literal string.  Let the examples below guide you.
 */

#define DEFINE_CALC_NAME_FMT_OPTION(type, explain)	\
{							\
	#type "_fmt",					\
	type##_fmt,			\
	"Specify the format of " explain		\
}

#define DEFINE_CALC_NAME_LIT_OPTION(type, explain)	\
{							\
	#type "_lit",					\
	type##_lit,			\
	"Specify the string for " explain		\
}

name_fmt_option_struct name_fmt_options[] =
{
	/* The `null' format should never be used.       */
	/* DEFINE_CALC_NAME_FMT_OPTION(null, "nothing"), */
	
	DEFINE_CALC_NAME_FMT_OPTION(client_stub,
				    "client stub names"),
	
	DEFINE_CALC_NAME_FMT_OPTION(server_skel,
				    "server skeleton names"),
	DEFINE_CALC_NAME_FMT_OPTION(client_skel,
				    "client skeleton names"),
	DEFINE_CALC_NAME_FMT_OPTION(server_func,
				    "server function names"),
	DEFINE_CALC_NAME_FMT_OPTION(receive_request_func,
				    "request receive function names"),
	DEFINE_CALC_NAME_FMT_OPTION(receive_reply_func,
				    "reply receive function names"),
	
	DEFINE_CALC_NAME_FMT_OPTION(marshal_stub,
				    "marshal stub names"),
	DEFINE_CALC_NAME_FMT_OPTION(unmarshal_stub,
				    "unmarshal stub names"),
	
	DEFINE_CALC_NAME_FMT_OPTION(message_marshal_request_stub,
				    "message marshal request stub names"),
	DEFINE_CALC_NAME_FMT_OPTION(message_unmarshal_request_stub,
				    "message unmarshal request stub names"),
	DEFINE_CALC_NAME_FMT_OPTION(message_marshal_reply_stub,
				    "message marshal reply stub names"),
	DEFINE_CALC_NAME_FMT_OPTION(message_marshal_except_stub,
				    "message marshal exception stub names"),
	DEFINE_CALC_NAME_FMT_OPTION(message_unmarshal_reply_stub,
				    "message unmarshal reply stub names"),
	
	DEFINE_CALC_NAME_FMT_OPTION(message_request_type,
				    "message request type names"),
	DEFINE_CALC_NAME_FMT_OPTION(message_reply_type,
				    "message reply type names"),
	DEFINE_CALC_NAME_FMT_OPTION(interface_message_request_type,
				    "message request type names"),
	DEFINE_CALC_NAME_FMT_OPTION(interface_message_reply_type,
				    "message reply type names"),
	
	DEFINE_CALC_NAME_FMT_OPTION(send_request_stub,
				    "send request stub names"),
	DEFINE_CALC_NAME_FMT_OPTION(send_reply_stub,
				    "send reply stub names"),
	DEFINE_CALC_NAME_FMT_OPTION(recv_request_stub,
				    "receive request stub names"),
	DEFINE_CALC_NAME_FMT_OPTION(recv_reply_stub,
				    "receive reply stub names"),
	
	DEFINE_CALC_NAME_FMT_OPTION(continue_request_stub,
				    "request continuation stub names"),
	DEFINE_CALC_NAME_FMT_OPTION(continue_reply_stub,
				    "reply continuation stub names"),
	DEFINE_CALC_NAME_FMT_OPTION(request_continuer_func_type,
				    "request continuation func type names"),
	DEFINE_CALC_NAME_FMT_OPTION(reply_continuer_func_type,
				    "reply continuation func type names"),
	
	DEFINE_CALC_NAME_FMT_OPTION(const,
				    "constant names"),
	DEFINE_CALC_NAME_FMT_OPTION(type,
				    "ordinary data type names"),
	
	DEFINE_CALC_NAME_FMT_OPTION(enum_tag,
				    "enumeration type tags/labels"),
	DEFINE_CALC_NAME_FMT_OPTION(enum_member,
				    "enumeration member names"),
	
	DEFINE_CALC_NAME_FMT_OPTION(struct_slot,
				    "structure slot names"),
	DEFINE_CALC_NAME_FMT_OPTION(struct_union_tag,
				    "tags of unions within discriminated "
				    "union types"),
	
	DEFINE_CALC_NAME_FMT_OPTION(basic_message_type,
				    "``generic'' (asynchronous) "
				    "message object types"),
	DEFINE_CALC_NAME_FMT_OPTION(client_basic_object_type,
				    "``generic'' client object types"),
	DEFINE_CALC_NAME_FMT_OPTION(server_basic_object_type,
				    "``generic'' server object types"),
	
	DEFINE_CALC_NAME_FMT_OPTION(client_interface_object_type,
				    "interface-specific client object types"),
	DEFINE_CALC_NAME_FMT_OPTION(server_interface_object_type,
				    "interface-specific server object types"),
	
	DEFINE_CALC_NAME_FMT_OPTION(client_stub_object_type,
				    "client stub target object types"),
	DEFINE_CALC_NAME_FMT_OPTION(server_func_object_type,
				    "server function target object types"),
	
	DEFINE_CALC_NAME_FMT_OPTION(client_stub_environment_type,
				    "client stub environment types"),
	DEFINE_CALC_NAME_FMT_OPTION(server_func_environment_type,
				    "server function environment types"),
	
	DEFINE_CALC_NAME_FMT_OPTION(stub_invocation_id_type,
				    "send/receive stub invocation id type"),
	DEFINE_CALC_NAME_FMT_OPTION(stub_client_ref_type,
				    "send/receive stub client reference type"),
	
	DEFINE_CALC_NAME_FMT_OPTION(client_stub_client_sid_type,
				    "client stub client SID types"),
	DEFINE_CALC_NAME_FMT_OPTION(server_func_client_sid_type,
				    "server function client SID types"),
	
	DEFINE_CALC_NAME_FMT_OPTION(client_stub_server_sid_type,
				    "client stub server SID types"),
	DEFINE_CALC_NAME_FMT_OPTION(server_func_server_sid_type,
				    "server function server SID types"),
	
	DEFINE_CALC_NAME_FMT_OPTION(stub_param,
				    "stub data parameter names"),
	
	DEFINE_CALC_NAME_FMT_OPTION(client_stub_object_param,
				    "client stub target object parameter "
				    "names"),
	DEFINE_CALC_NAME_FMT_OPTION(server_func_object_param,
				    "server function target object parameter "
				    "names"),
	
	DEFINE_CALC_NAME_FMT_OPTION(client_stub_environment_param,
				    "client stub environment parameter names"),
	DEFINE_CALC_NAME_FMT_OPTION(server_func_environment_param,
				    "server function environment parameter "
				    "names"),
	
	DEFINE_CALC_NAME_FMT_OPTION(client_stub_client_sid_param,
				    "client stub client SID parameter names"),
	DEFINE_CALC_NAME_FMT_OPTION(server_func_client_sid_param,
				    "server function client SID parameter "
				    "names"),
	
	DEFINE_CALC_NAME_FMT_OPTION(client_stub_required_server_sid_param,
				    "client stub required server SID "
				    "parameter names"),
	DEFINE_CALC_NAME_FMT_OPTION(server_func_required_server_sid_param,
				    "server function required server SID "
				    "parameter names"),
	
	DEFINE_CALC_NAME_FMT_OPTION(client_stub_actual_server_sid_param,
				    "client stub actual server SID parameter "
				    "names"),
	DEFINE_CALC_NAME_FMT_OPTION(server_func_actual_server_sid_param,
				    "server function actual server SID "
				    "parameter names"),
	
	DEFINE_CALC_NAME_FMT_OPTION(operation_request_code,
				    "operation request codes"),
	DEFINE_CALC_NAME_FMT_OPTION(operation_reply_code,
				    "operation reply codes"),
	
	DEFINE_CALC_NAME_FMT_OPTION(exception_type,
				    "exception type names"),
	DEFINE_CALC_NAME_FMT_OPTION(exception_code,
				    "exception codes"),
	
	DEFINE_CALC_NAME_FMT_OPTION(allocator_function,
				    "memory allocation functions"),
	DEFINE_CALC_NAME_FMT_OPTION(deallocator_function,
				    "memory deallocation functions"),
	
	DEFINE_CALC_NAME_FMT_OPTION(presentation_include_file,
				    "the presentation style `#include' file "
				    "name"),
	DEFINE_CALC_NAME_FMT_OPTION(interface_include_file,
				    "per-interface `#include' file names"),
	DEFINE_CALC_NAME_FMT_OPTION(interface_default_include_file,
				    "default per-interface `#include' file "
				    "names"),
	
	/* `pg_state::build_flags' looks for a terminating null entry. */
	{ 0, null_fmt, 0}
};

name_lit_option_struct name_lit_options[] =
{
	/* The `null' literal should never be used.      */
	/* DEFINE_CALC_NAME_LIT_OPTION(null, "nothing"), */
	
	DEFINE_CALC_NAME_LIT_OPTION(separator,
				    "name component separators (`%_')"),
	
	DEFINE_CALC_NAME_LIT_OPTION(presentation_style,
				    "the presentation style (`%g')"),
	
	DEFINE_CALC_NAME_LIT_OPTION(filename_component_separator,
				    "filename component separators (`%/')"),
	
	/* `pg_state::build_flags' looks for a terminating null entry. */
	{ 0, null_lit, 0}
};


/*****************************************************************************/

/*
 * In order to allow fine-grained control over name generation in different
 * presentation generators, we define a `pg_state' member function for every
 * kind of name that we generate.  That is, we define one member function for
 * each kind of name format string.  The basic `pg_state' implementation of
 * each member function is simple: just call `calc_name' with the format string
 * for the given presentation element type.
 *
 * Individual presentation generators then have two ways to change how the
 * names for specific presentation elements are built.  The first (preferred)
 * method is to change the default value of the format string corresponding to
 * the presentation element type.  The second technique is to override the
 * method that is called to generate names for that presentation element type.
 * The latter method may be used when the existing set of `calc_name' format
 * escapes is insufficient to describe the names that must be created.
 *
 * The macro `DEFINE_CALC_NAME_FUNCTION' is used to define the base `pg_state'
 * methods for each type of presentation element.
 */

#define DEFINE_CALC_NAME_FUNCTION(type)					\
  char *pg_state::calc_##type##_name(const char *basic_name)		\
  {									\
	return calc_name(names.formats[ type##_fmt ],	\
			 basic_name);					\
  }

#define DEFINE_CALC_SCOPED_NAME_FUNCTION(type)				    \
  cast_scoped_name pg_state::calc_##type##_scoped_name(			    \
	aoi_ref parent_ref,						    \
	const char *base_name)						    \
  {									    \
	cast_scoped_name scname = null_scope_name;			    \
									    \
	calc_scoped_name(&scname,					    \
			 parent_ref,					    \
			 names.formats[ type##_scoped_fmt ]); \
	cast_add_scope_name(&scname,					    \
			    base_name,					    \
			    null_template_arg_array);			    \
	return scname;							    \
  }

/* The `null' format should never be used. */
/* DEFINE_CALC_NAME_FUNCTION(null)         */

DEFINE_CALC_NAME_FUNCTION(client_stub)

DEFINE_CALC_SCOPED_NAME_FUNCTION(client_stub)

DEFINE_CALC_NAME_FUNCTION(server_skel)
DEFINE_CALC_NAME_FUNCTION(client_skel)
DEFINE_CALC_NAME_FUNCTION(server_func)
DEFINE_CALC_NAME_FUNCTION(receive_request_func)
DEFINE_CALC_NAME_FUNCTION(receive_reply_func)

DEFINE_CALC_SCOPED_NAME_FUNCTION(server_skel)
DEFINE_CALC_SCOPED_NAME_FUNCTION(client_skel)
DEFINE_CALC_SCOPED_NAME_FUNCTION(server_func)
DEFINE_CALC_SCOPED_NAME_FUNCTION(receive_request_func)
DEFINE_CALC_SCOPED_NAME_FUNCTION(receive_reply_func)

DEFINE_CALC_NAME_FUNCTION(marshal_stub)
DEFINE_CALC_NAME_FUNCTION(unmarshal_stub)

DEFINE_CALC_SCOPED_NAME_FUNCTION(marshal_stub)
DEFINE_CALC_SCOPED_NAME_FUNCTION(unmarshal_stub)

DEFINE_CALC_NAME_FUNCTION(message_marshal_request_stub)
DEFINE_CALC_NAME_FUNCTION(message_unmarshal_request_stub)
DEFINE_CALC_NAME_FUNCTION(message_marshal_reply_stub)
DEFINE_CALC_NAME_FUNCTION(message_marshal_except_stub)
DEFINE_CALC_NAME_FUNCTION(message_unmarshal_reply_stub)

DEFINE_CALC_SCOPED_NAME_FUNCTION(message_marshal_request_stub)
DEFINE_CALC_SCOPED_NAME_FUNCTION(message_unmarshal_request_stub)
DEFINE_CALC_SCOPED_NAME_FUNCTION(message_marshal_reply_stub)
DEFINE_CALC_SCOPED_NAME_FUNCTION(message_marshal_except_stub)
DEFINE_CALC_SCOPED_NAME_FUNCTION(message_unmarshal_reply_stub)

DEFINE_CALC_NAME_FUNCTION(message_request_type)
DEFINE_CALC_NAME_FUNCTION(message_reply_type)
DEFINE_CALC_NAME_FUNCTION(interface_message_request_type)
DEFINE_CALC_NAME_FUNCTION(interface_message_reply_type)

DEFINE_CALC_SCOPED_NAME_FUNCTION(message_request_type)
DEFINE_CALC_SCOPED_NAME_FUNCTION(message_reply_type)
DEFINE_CALC_SCOPED_NAME_FUNCTION(interface_message_request_type)
DEFINE_CALC_SCOPED_NAME_FUNCTION(interface_message_reply_type)

DEFINE_CALC_NAME_FUNCTION(send_request_stub)
DEFINE_CALC_NAME_FUNCTION(send_reply_stub)
DEFINE_CALC_NAME_FUNCTION(recv_request_stub)
DEFINE_CALC_NAME_FUNCTION(recv_reply_stub)

DEFINE_CALC_SCOPED_NAME_FUNCTION(send_request_stub)
DEFINE_CALC_SCOPED_NAME_FUNCTION(send_reply_stub)
DEFINE_CALC_SCOPED_NAME_FUNCTION(recv_request_stub)
DEFINE_CALC_SCOPED_NAME_FUNCTION(recv_reply_stub)

DEFINE_CALC_NAME_FUNCTION(continue_request_stub)
DEFINE_CALC_NAME_FUNCTION(continue_reply_stub)
DEFINE_CALC_NAME_FUNCTION(request_continuer_func_type)
DEFINE_CALC_NAME_FUNCTION(reply_continuer_func_type)

DEFINE_CALC_SCOPED_NAME_FUNCTION(continue_request_stub)
DEFINE_CALC_SCOPED_NAME_FUNCTION(continue_reply_stub)
DEFINE_CALC_SCOPED_NAME_FUNCTION(request_continuer_func_type)
DEFINE_CALC_SCOPED_NAME_FUNCTION(reply_continuer_func_type)

DEFINE_CALC_NAME_FUNCTION(const)
DEFINE_CALC_NAME_FUNCTION(type)

DEFINE_CALC_SCOPED_NAME_FUNCTION(const)
DEFINE_CALC_SCOPED_NAME_FUNCTION(type)

DEFINE_CALC_NAME_FUNCTION(enum_tag)
DEFINE_CALC_NAME_FUNCTION(enum_member)

DEFINE_CALC_SCOPED_NAME_FUNCTION(enum_tag)

DEFINE_CALC_NAME_FUNCTION(struct_slot)
DEFINE_CALC_NAME_FUNCTION(struct_union_tag)

DEFINE_CALC_NAME_FUNCTION(basic_message_type)
DEFINE_CALC_NAME_FUNCTION(client_basic_object_type)
DEFINE_CALC_NAME_FUNCTION(server_basic_object_type)

DEFINE_CALC_SCOPED_NAME_FUNCTION(basic_message_type)
DEFINE_CALC_SCOPED_NAME_FUNCTION(client_basic_object_type)
DEFINE_CALC_SCOPED_NAME_FUNCTION(server_basic_object_type)

DEFINE_CALC_NAME_FUNCTION(client_interface_object_type)
DEFINE_CALC_NAME_FUNCTION(server_interface_object_type)

DEFINE_CALC_SCOPED_NAME_FUNCTION(client_interface_object_type)
DEFINE_CALC_SCOPED_NAME_FUNCTION(server_interface_object_type)

DEFINE_CALC_NAME_FUNCTION(client_stub_object_type)
DEFINE_CALC_NAME_FUNCTION(server_func_object_type)

DEFINE_CALC_SCOPED_NAME_FUNCTION(client_stub_object_type)
DEFINE_CALC_SCOPED_NAME_FUNCTION(server_func_object_type)

DEFINE_CALC_NAME_FUNCTION(client_stub_environment_type)
DEFINE_CALC_NAME_FUNCTION(server_func_environment_type)

DEFINE_CALC_SCOPED_NAME_FUNCTION(client_stub_environment_type)
DEFINE_CALC_SCOPED_NAME_FUNCTION(server_func_environment_type)

DEFINE_CALC_NAME_FUNCTION(stub_invocation_id_type)
DEFINE_CALC_NAME_FUNCTION(stub_client_ref_type)

DEFINE_CALC_SCOPED_NAME_FUNCTION(stub_invocation_id_type)
DEFINE_CALC_SCOPED_NAME_FUNCTION(stub_client_ref_type)

DEFINE_CALC_NAME_FUNCTION(client_stub_client_sid_type)
DEFINE_CALC_NAME_FUNCTION(server_func_client_sid_type)

DEFINE_CALC_SCOPED_NAME_FUNCTION(client_stub_client_sid_type)
DEFINE_CALC_SCOPED_NAME_FUNCTION(server_func_client_sid_type)

DEFINE_CALC_NAME_FUNCTION(client_stub_server_sid_type)
DEFINE_CALC_NAME_FUNCTION(server_func_server_sid_type)

DEFINE_CALC_SCOPED_NAME_FUNCTION(client_stub_server_sid_type)
DEFINE_CALC_SCOPED_NAME_FUNCTION(server_func_server_sid_type)

DEFINE_CALC_NAME_FUNCTION(stub_param)

DEFINE_CALC_NAME_FUNCTION(client_stub_object_param)
DEFINE_CALC_NAME_FUNCTION(server_func_object_param)

DEFINE_CALC_NAME_FUNCTION(client_stub_environment_param)
DEFINE_CALC_NAME_FUNCTION(server_func_environment_param)

DEFINE_CALC_NAME_FUNCTION(client_stub_client_sid_param)
DEFINE_CALC_NAME_FUNCTION(server_func_client_sid_param)

DEFINE_CALC_NAME_FUNCTION(client_stub_required_server_sid_param)
DEFINE_CALC_NAME_FUNCTION(server_func_required_server_sid_param)

DEFINE_CALC_NAME_FUNCTION(client_stub_actual_server_sid_param)
DEFINE_CALC_NAME_FUNCTION(server_func_actual_server_sid_param)

DEFINE_CALC_NAME_FUNCTION(operation_request_code)
DEFINE_CALC_NAME_FUNCTION(operation_reply_code)

DEFINE_CALC_NAME_FUNCTION(exception_type)
DEFINE_CALC_NAME_FUNCTION(exception_code)

DEFINE_CALC_SCOPED_NAME_FUNCTION(exception_type)

DEFINE_CALC_NAME_FUNCTION(allocator_function)
DEFINE_CALC_NAME_FUNCTION(deallocator_function)

DEFINE_CALC_SCOPED_NAME_FUNCTION(allocator_function)
DEFINE_CALC_SCOPED_NAME_FUNCTION(deallocator_function)

DEFINE_CALC_NAME_FUNCTION(presentation_include_file)
DEFINE_CALC_NAME_FUNCTION(interface_include_file)
DEFINE_CALC_NAME_FUNCTION(interface_default_include_file)

/*
 * This function is another useful interface to `calc_name': compute the name
 * of an presentation element from an AOI reference (i.e., an index to an AOI
 * definition).  Of course, this function is only useful for objects that get
 * their names from `aoi_def's: generally, types and interface references.
 *
 * See how this function is used by `p_typedef_def' and `p_make_ctypename'.
 */
char *pg_state::calc_name_from_ref(aoi_ref ref)
{
	aoi_ref saved_cur_aoi_idx;
	aoi_ref saved_derived_interface_ref;
	aoi_ref saved_parent_interface_ref;
	
	aoi_kind ref_kind;
	char *ref_name;
	
	/*****/
	
	assert((ref >= 0) && (ref < ((aoi_ref) in_aoi->defs.defs_len)));
	
	ref_kind = a(ref).binding->kind;
	ref_name = a(ref).name;
	
	/*
	 * Set `cur_aoi_idx' to `ref' so that `calc_name' can find the right
	 * scope information.
	 *
	 * Moreover, set our interface references to `aoi_ref_null' so that we
	 * don't inadvertently generate names as if they were defined as part
	 * of the interfaces we might be processing at the moment.  `ref' *may*
	 * refer to something that is within the current interface, but any
	 * scope components in the name generated for `ref' will have to be
	 * derived from `ref' itself, not from the fact that we happen to be
	 * processing any particluar interface at the moment.  In short, we
	 * need to generate a globally appropriate name for `ref', not a name
	 * driven by the PG's current context.
	 */
	saved_cur_aoi_idx = cur_aoi_idx;
	saved_derived_interface_ref = derived_interface_ref;
	saved_parent_interface_ref = parent_interface_ref;
	
	cur_aoi_idx = ref;
	derived_interface_ref = aoi_ref_null;
	parent_interface_ref = aoi_ref_null;
	
	/*
	 * Generate the name by invoking the `calc_*_name' function that is
	 * appropriate for `ref_kind'.
	 */
	switch (ref_kind) {
	default:
		ref_name = calc_type_name(ref_name);
		break;
		
	case AOI_INTERFACE:
	case AOI_FWD_INTRFC:
		if (gen_client)
			ref_name = calc_client_interface_object_type_name(
				ref_name);
		else if (gen_server)
			ref_name = calc_server_interface_object_type_name(
				ref_name);
		else
			panic("In `pg_state::calc_name_from_ref', "
			      "generating neither client nor server.");
		break;
		
	case AOI_EXCEPTION:
		ref_name = calc_exception_type_name(ref_name);
		break;
	}
	
	/* Restore our AOI references before returning. */
	cur_aoi_idx = saved_cur_aoi_idx;
	derived_interface_ref = saved_derived_interface_ref;
	parent_interface_ref = saved_parent_interface_ref;

	return ref_name;
}

cast_scoped_name pg_state::calc_scoped_name_from_ref(aoi_ref ref)
{
	aoi_ref saved_cur_aoi_idx;
	aoi_ref saved_derived_interface_ref;
	aoi_ref saved_parent_interface_ref;
	
	aoi_kind ref_kind;
	char *ref_name;
	cast_scoped_name scoped_name = null_scope_name;
	
	/*****/
	
	assert((ref >= 0) && (ref < ((aoi_ref) in_aoi->defs.defs_len)));
	
	ref_kind = a(ref).binding->kind;
	ref_name = a(ref).name;
	
	/*
	 * Set `cur_aoi_idx' to `ref' so that `calc_name' can find the right
	 * scope information.
	 *
	 * Moreover, set our interface references to `aoi_ref_null' so that we
	 * don't inadvertently generate names as if they were defined as part
	 * of the interfaces we might be processing at the moment.  `ref' *may*
	 * refer to something that is within the current interface, but any
	 * scope components in the name generated for `ref' will have to be
	 * derived from `ref' itself, not from the fact that we happen to be
	 * processing any particluar interface at the moment.  In short, we
	 * need to generate a globally appropriate name for `ref', not a name
	 * driven by the PG's current context.
	 */
	saved_cur_aoi_idx = cur_aoi_idx;
	saved_derived_interface_ref = derived_interface_ref;
	saved_parent_interface_ref = parent_interface_ref;
	
	cur_aoi_idx = ref;
	derived_interface_ref = aoi_ref_null;
	parent_interface_ref = aoi_ref_null;

	ref = aoi_get_parent_scope(in_aoi, ref);
	/*
	 * Generate the name by invoking the `calc_*_name' function that is
	 * appropriate for `ref_kind'.
	 */
	switch (ref_kind) {
	default:
		scoped_name = calc_type_scoped_name(
			ref,
			calc_type_name(ref_name)
			);
		break;
		
	case AOI_INTERFACE:
	case AOI_FWD_INTRFC:
		if (gen_client)
			scoped_name =
				calc_client_interface_object_type_scoped_name(
					ref,
					calc_client_interface_object_type_name(
						ref_name)
					);
		else if (gen_server)
			scoped_name =
				calc_server_interface_object_type_scoped_name(
					ref,
					calc_server_interface_object_type_name(
						ref_name)
					);
		else
			panic("In `pg_state::calc_name_from_ref', "
			      "generating neither client nor server.");
		break;
		
	case AOI_EXCEPTION:
		scoped_name = calc_exception_type_scoped_name(
			ref,
			calc_exception_type_name(ref_name)
			);
		break;
	}
	
	/* Restore our AOI references before returning. */
	cur_aoi_idx = saved_cur_aoi_idx;
	derived_interface_ref = saved_derived_interface_ref;
	parent_interface_ref = saved_parent_interface_ref;

	return scoped_name;
}


/*****************************************************************************/

/*
 * Here is the implementation of the core name-calcuation engine.
 *
 * First, define a macro for adding a name compoenent to our internal list
 * (array) of components, reallocating the array as necessary.
 */

#define CALC_NAME_COMPONENTS_INCREMENT (16)

#define CALC_NAME_ADD_COMPONENT(data, data_len)     		 \
	do {								 \
		if (calc_name_data.count >= calc_name_data.size) {	 \
			calc_name_data.size +=				 \
				CALC_NAME_COMPONENTS_INCREMENT;		 \
			calc_name_data.components =			 \
				(calc_name_component *)			 \
				mustrealloc(calc_name_data.components,	 \
					    (sizeof(calc_name_component) \
					     * calc_name_data.size));	 \
		}							 \
									 \
		calc_name_data.components[calc_name_data.count].str	 \
			= (data); \
		calc_name_data.components[calc_name_data.count].len \
			= (data_len); \
									 \
		++calc_name_data.count;					 \
	} while (0)

#define NAME_LITERAL_STR(type) names.literals[ type##_lit ].str
#define NAME_LITERAL_LEN(type) names.literals[ type##_lit ].len

/*
 * An internal auxiliary.  `calc_name_module' locates the names of the AOI
 * objects (modules, interfaces, or types) that contain the AOI object
 * referenced by `last_ref'.  These names, along with appropriate separators,
 * are added to our list of name pieces.
 *
 * `last_ref' is the initial AOI reference.  This function does *not* add the
 * name associated with `last_ref' to our list of name components.
 *
 * If `qualified_p' is true, `calc_name_module' traverses the AOI scopes all
 * the way back to the root scope.  The name of every encompassing object is
 * added to our name component list.  If `qualified_p' is false, then only the
 * name of the directly encompassing object is added to our name list.
 *
 * If `separator_p' is true, then we output a separator immediately after the
 * output name component.  This flag is set for all recursive calls.
 */

void pg_state::calc_name_module(aoi_ref last_ref,
				int qualified_p,
				int separator_p)
{
	aoi_ref ref;
	
	if( a(last_ref).scope <= 0 )
		return;
	
	ref = aoi_get_parent_scope(in_aoi, last_ref);
	
	assert(a(ref).binding);
	/*
	 * XXX --- Relax this assertion because now we call this function with
	 * things that aren't interfaces, but which may be conatined in
	 * interfaces (e.g., type names).
	 *
	 * assert(a(ref).binding->kind == AOI_NAMESPACE);
	 *
	 * XXX --- Relax it a lot.  Structs define scopes, too!
	 */
#if 0
	assert((a(ref).binding->kind == AOI_NAMESPACE)
	       || (a(ref).binding->kind == AOI_INTERFACE));
#endif
	
	/* Output the names of the scopes conatining `ref'. */
	if (qualified_p == CALC_NAME_QUALIFIED)
		calc_name_module(ref,
				 CALC_NAME_QUALIFIED, CALC_NAME_SEPARATOR);
	
	/* Now output the name of this scope. */
	CALC_NAME_ADD_COMPONENT(a(ref).name, strlen(a(ref).name));
	if (separator_p == CALC_NAME_SEPARATOR)
		CALC_NAME_ADD_COMPONENT(NAME_LITERAL_STR(separator),
					NAME_LITERAL_LEN(separator));
}

/*
 * Finally, the core of the name generation engine.  `calc_name' interprets a
 * `printf'-like format string in order to construct the name of a presentation
 * element.  The current set of `%'-escapes is this:
 *
 * `%_'		A name component separator (as determined by `names.literals').
 * `%-'		Same as `%_'.
 * `%/'		A filename component separator.
 *
 * `%g'		The name of the presentation style (from `names.literals').
 *
 * `%s'		The unscoped name of the element, generally taken from the IDL
 *		input.  This string is found in the `object_name' parameter.
 *
 * `%S'		The scoped name of the element, with separators between the
 *		components.
 *
 * `%m'		The unscoped name of the module/interface/... element in which
 *		the current-being-defined element is contained (i.e., its
 *		``lexical parent'').
 *
 * `%M'		The scoped name of the element in which the current element is
 *		contained (i.e., its ``lexical lineage'').
 *
 * `%i'		The unscoped name of the interface in which the current element
 *		is being defined.  If the current element is not a component of
 *		an interface, `%i' produces nothing.
 *
 * `%I'		The scoped name of the interface in which the current element
 *		is being defined.  As with `%i', this produces nothing if the
 *		current element is not part of an interface.
 *
 * `%p'		The unscoped name of the interface from which the current
 *		interface element is being inherited (`p' for `parent').  If
 *		the element is being defined by the current interface, `%p' is
 *		the same as `%i'.  If the current element is not part of an
 *		interface, `%p' produces nothing.
 *
 * `%P'		The scoped name of the interface from which the current
 *		interface element is being inherited.
 *
 * `%n'		The unscoped name of the module/interface/... that contains the
 *		interface from which the current interface element in being
 *		derived.  (`%n' is `%m' applied to the ``parent interface.'')
 *		I haven't figured out why this would be useful, and `n' is a
 *		terrible mnemonic for this anyway.
 *
 * `%N'		The scoped version of `%n'.
 *
 * To interpret these escapes, `calc_name' refers to three AOI references kept
 * in the invoking `pg_state':
 *
 * `cur_aoi_idx'		The current top-level AOI definition.
 * `derived_interface_ref'	The AOI interface that is currently being
 *				processed (by `p_client_stubs/p_server_skel').
 * `parent_interface_ref'	The AOI interface from which the current
 *				interface (`derived_interface_ref') is
 *				inheriting a component operation, type, ...
 *
 * XXX --- I am not sure that `calc_name' properly handles all of the ways in
 * which these AOI references may interact with one another.  `cur_aoi_idx' was
 * sort of bashed in at the last moment.  Look for `was derived_ref' and XXX in
 * the code below.
 *
 * XXX --- Ideas for new escapes:
 *
 * `%c'		The ``code'' of this element (e.g., an interface) as specified
 *		in the AOI file.  This would perhaps require a PG-specific code
 *		extraction method.  Adding `%c' would help eliminate the Sun
 *		PG overrides of certain `calc_*_name' methods.
 *
 * `%('		Groups of alternatives.  Process the alternatives until one
 * `%)'		results in a non-empty string.  This would help eliminate the
 * `%|'		`interface_default_include_file' format: the Fluke PG could set
 *		the `interface_include_file' format string to something like
 *		`fluke%/%(%M%|flick%).h'.
 */

char *pg_state::calc_name(const char *format, const char *object_name)
{
	aoi_ref parent_ref = parent_interface_ref;
	aoi_ref derived_ref = derived_interface_ref;
	aoi_ref current_ref = cur_aoi_idx;
	
	char *result_str;
	int result_len;
	
	const char *src;
	char *dst;
	int len;
	
	int i;
	
	/*
	 * Parse the format string into a sequence of name components.
	 */
	calc_name_data.count = 0;
	
	for (src = format; *src; ) {
		if (*src != '%') {
			/*
			 * Simple case: Copy characters from input to output.
			 */
			for (len = 0; (src[len] && (src[len] != '%')); ++len)
				/* Do nothing. */ ;
			CALC_NAME_ADD_COMPONENT(src, len);
			src += len;
			
			/*
			 * We could `continue' at this point, but let's be a
			 * little more clever instead.  `*src' is either a `%'
			 * or a NUL: if it's `%' we can fall through to the
			 * `switch' below.
			 */
			if (!(*src))
				break;
		}
		
		/*
		 * Hard cases: Expand a `%' macro in the format string.
		 */
		switch (*(++src)) {
		default:
			panic("In `pg_state::calc_name', "
			      "unknown dispatch macro `%%%c'.", *src);
			break;
			
			/*
			 * Literal strings: separators, ...
			 */
		case '-':
		case '_':
			CALC_NAME_ADD_COMPONENT(
				NAME_LITERAL_STR(separator),
				NAME_LITERAL_LEN(separator)
				);
			break;
		case '/':
			CALC_NAME_ADD_COMPONENT(
				NAME_LITERAL_STR(filename_component_separator),
				NAME_LITERAL_LEN(filename_component_separator)
				);
			break;
		case 'g':
			CALC_NAME_ADD_COMPONENT(
				NAME_LITERAL_STR(presentation_style),
				NAME_LITERAL_LEN(presentation_style));
			break;
			
			/*
			 * Module names.
			 */
		case 'M':
			calc_name_module(current_ref /* was `derived_ref' */,
					 CALC_NAME_QUALIFIED,
					 CALC_NAME_NO_SEPARATOR);
			break;
			
		case 'm':
			calc_name_module(current_ref /* was `derived_ref' */,
					 CALC_NAME_QUALIFIED,
					 CALC_NAME_NO_SEPARATOR);
			break;
			
		case 'N':
			calc_name_module(parent_ref,
					 CALC_NAME_QUALIFIED,
					 CALC_NAME_NO_SEPARATOR);
			break;
			
		case 'n':
			calc_name_module(parent_ref,
					 CALC_NAME_QUALIFIED,
					 CALC_NAME_NO_SEPARATOR);
			break;
			
			/*
			 * Parent interface names.
			 */
		case 'P':
			calc_name_module(parent_ref,
					 CALC_NAME_QUALIFIED,
					 CALC_NAME_SEPARATOR);
			/* FALLTHROUGH */
		case 'p':
			if (parent_ref != aoi_ref_null)
				CALC_NAME_ADD_COMPONENT(a(parent_ref).name,
							strlen(a(parent_ref).
							       name));
			break;
			
			/*
			 * Derived interface names.
			 */
		case 'I':
			calc_name_module(current_ref /* was `derived_ref' */,
					 CALC_NAME_QUALIFIED,
					 CALC_NAME_SEPARATOR);
			/* XXX? --- Is this next `if' right? */
			if (current_ref != derived_ref)
				break;
			/* FALLTHROUGH */
		case 'i':
			if (derived_ref != aoi_ref_null)
				CALC_NAME_ADD_COMPONENT(a(derived_ref).name,
							strlen(a(derived_ref).
							       name));
			break;
			
			/*
			 * The name of the current interface component:
			 * operation, attribute, type, whatever.
			 */
		case 'S':
			calc_name_module(current_ref /* was `derived_ref' */,
					 CALC_NAME_QUALIFIED,
					 CALC_NAME_SEPARATOR);
			/* XXX? --- Is this next `if' right? */
			if (current_ref != derived_ref)
				;
			else if (derived_ref != aoi_ref_null) {
				CALC_NAME_ADD_COMPONENT(a(derived_ref).name,
							strlen(a(derived_ref).
							       name));
				CALC_NAME_ADD_COMPONENT(
					NAME_LITERAL_STR(separator),
					NAME_LITERAL_LEN(separator));
			}
			/* FALLTHROUGH. */
		case 's':
			CALC_NAME_ADD_COMPONENT(object_name,
						strlen(object_name));
			break;
		}
		++src;
	}
	
	/*
	 * Concatenate all of the components into the final name string.
	 */
	result_len = 1; /* 1 for terminating NUL. */
	for (i = 0; i < calc_name_data.count; ++i)
		result_len += calc_name_data.components[i].len;
	
	result_str = (char *) mustmalloc(sizeof(char) * result_len);
	
	for (i = 0, dst = result_str; i < calc_name_data.count; ++i) {
		strncpy(dst,
			calc_name_data.components[i].str,
			calc_name_data.components[i].len);
		dst += calc_name_data.components[i].len;
	}
	*dst = 0;
	
	// fprintf(stderr, "  In:    `%s' `%s'\n", format, object_name);
	// fprintf(stderr, "    Out: `%s'\n", result_str);
	return result_str;
}

void pg_state::calc_scoped_name(cast_scoped_name *scoped_name,
				aoi_ref ref,
				const char *format)
{
	char *result_str;
	int result_len;
	
	const char *src;
	char *dst;
	int len;
	
	int i;

	if( !format || !(*format) || (ref == aoi_ref_null) ) {
		if( scoped_name->cast_scoped_name_len > 1 )
			cast_prepend_scope_name(scoped_name, "",
						null_template_arg_array);
		return;
	}
	calc_name_data.count = 0;
	
	for (src = format; *src; ) {
		if (*src != '%') {
			for (len = 0; (src[len] && (src[len] != '%')); ++len)
				/* Do nothing. */ ;
			CALC_NAME_ADD_COMPONENT(src, len);
			src += len;
			
			if (!(*src))
				break;
		}
		switch (*(++src)) {
		default:
			panic("In `pg_state::calc_scoped_name', "
			      "unknown dispatch macro `%%%c'.", *src);
			break;
		case 'n':
			CALC_NAME_ADD_COMPONENT(a(ref).name,
						strlen(a(ref).name));
			break;
		}
		++src;
	}
	
	/*
	 * Concatenate all of the components into the final name string.
	 */
	result_len = 1; /* 1 for terminating NUL. */
	for (i = 0; i < calc_name_data.count; ++i)
		result_len += calc_name_data.components[i].len;
	
	result_str = (char *) mustmalloc(sizeof(char) * result_len);
	
	for (i = 0, dst = result_str; i < calc_name_data.count; ++i) {
		strncpy(dst,
			calc_name_data.components[i].str,
			calc_name_data.components[i].len);
		dst += calc_name_data.components[i].len;
	}
	*dst = 0;

	// fprintf(stderr, "  In:    `%s' `%s'\n", format, object_name);
	// fprintf(stderr, "    Out: `%s'\n", result_str);
	calc_scoped_name(scoped_name,
			 aoi_get_parent_scope(in_aoi, ref),
			 format);
	cast_add_scope_name(scoped_name, result_str, null_template_arg_array);
}

/*****************************************************************************/

/* End of file. */

