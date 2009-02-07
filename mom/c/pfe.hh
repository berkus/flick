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

#ifndef _mom_c_pfe_hh_
#define _mom_c_pfe_hh_

/* #define ATTRIBUTE_STUBS */
#include <stdio.h>

#include <mom/compiler.h>

extern "C" {
#include <mom/aoi.h>
#include <mom/pres_c.h>
}

#include <mom/c/p_type_collection.hh>

/*****************************************************************************/

struct pg_flags {
	const char *input;
	const char *output;
	int client;
	int use_sids;
	int client_stubs_for_inherited_operations;
	int server_funcs_for_inherited_operations;
	int async_stubs;
};


/*****************************************************************************/

/*
 * The following structures are used by `calc_name' to construct name strings.
 */

struct calc_name_component {
	/*
	 * We keep both a pointer and a length so that we can point to strings
	 * that are not NUL-terminated (i.e., pieces of the format string given
	 * to `calc_name').
	 */
	const char *str;
	int len;
};

struct calc_name_struct {
	/*
	 * Our list of name components.  These are concatenated to construct
	 * the final result.
	 */
	calc_name_component *components;
	
	/*
	 * `size' is the size of the array pointed to by `components', and
	 * `count' is the number of actual components in the array (i.e., the
	 * number of slots being used).
	 */
	int size;
	int count;
};

/*
 * These constants control the operation of `calc_name_module'.  These should
 * probably become enumerations instead.
 */
#define CALC_NAME_NOT_QUALIFIED		(0)
#define CALC_NAME_QUALIFIED		(1)
#define CALC_NAME_NO_SEPARATOR		(0)
#define CALC_NAME_SEPARATOR		(1)

/*
 * `DECLARE_CALC_NAME_FUNCTION' is used to declare `pg_state' member functions
 * for the different kinds of names that may be generated.
 */
#define DECLARE_CALC_NAME_FUNCTION(type) \
  virtual char * calc_##type##_name(const char *basic_name)

#define DECLARE_CALC_SCOPED_NAME_FUNCTION(type) \
  virtual cast_scoped_name calc_##type##_scoped_name(aoi_ref parent_ref, \
						     const char *basic_name)

/*
 * A `name_strings' structure contains the format strings and literal strings
 * that are used by the `calc_*_name' methods to build the names of various
 * presentation elements: stub names, type names, and so on.  Every `pg_state'
 * contains a `names_strings' structure.
 */

	typedef enum 
	{
		/*
		 * Enumerate the different ``classes'' or ``types'' of elements
		 * for which a presentation generator must construct names.
		 * These enumeration names are used to index into an array of
		 * format strings, one for each kind of presentation element.
		 * See `p_calc_name.cc' for more discussion about how name
		 * format strings are used.
		 *
		 * The list of presentation elements is certainly incomplete.
		 * If you create a new kind of name string format, you must
		 * also:
		 *
		 * + In this file, use `DECLARE_CALC_NAME_FUNCTION' to create a
		 *   `pg_state' method to format strings with your new spec.
		 *
		 * + In `p_calc_name.cc', use `DEFINE_CALC_NAME_FUNCTION' to
		 *   define your new `pg_state' method.
		 +
		 + + In `p_calc_name.cc', use `DEFINE_CALC_NAME_FMT_OPTION' to
		 *   make a command line option corresponding to your new
		 *   name string format (so that users can change the format).
		 *
		 * + In `pg_state.cc', give your new name string format a
		 *   default value.
		 *
		 * + In the constructors for specific presentation generators,
		 *   override the default value of your new name format string
		 *   as necessary.
		 *
		 * + Finally, override your new name-calculating method as
		 *   necessary for specific presentation generators.
		 */
		null_fmt,
		
		/*
		 * Stub names.
		 */
		client_stub_fmt,
		
		client_stub_scoped_fmt,
		
		server_skel_fmt,
		client_skel_fmt,
		server_func_fmt,
		receive_request_func_fmt,
		receive_reply_func_fmt,
		
		server_skel_scoped_fmt,
		client_skel_scoped_fmt,
		server_func_scoped_fmt,
		receive_request_func_scoped_fmt,
		receive_reply_func_scoped_fmt,
		
		marshal_stub_fmt,
		unmarshal_stub_fmt,

		marshal_stub_scoped_fmt,
		unmarshal_stub_scoped_fmt,

		message_marshal_request_stub_fmt,
		message_unmarshal_request_stub_fmt,
		message_marshal_reply_stub_fmt,
		message_marshal_except_stub_fmt,
		message_unmarshal_reply_stub_fmt,
		
		message_marshal_request_stub_scoped_fmt,
		message_unmarshal_request_stub_scoped_fmt,
		message_marshal_reply_stub_scoped_fmt,
		message_marshal_except_stub_scoped_fmt,
		message_unmarshal_reply_stub_scoped_fmt,
		
		message_request_type_fmt,
		message_reply_type_fmt,
		interface_message_request_type_fmt,
		interface_message_reply_type_fmt,
		
		message_request_type_scoped_fmt,
		message_reply_type_scoped_fmt,
		interface_message_request_type_scoped_fmt,
		interface_message_reply_type_scoped_fmt,
		
		send_request_stub_fmt,
		send_reply_stub_fmt,
		recv_request_stub_fmt,
		recv_reply_stub_fmt,
		
		send_request_stub_scoped_fmt,
		send_reply_stub_scoped_fmt,
		recv_request_stub_scoped_fmt,
		recv_reply_stub_scoped_fmt,
		
		continue_request_stub_fmt,
		continue_reply_stub_fmt,
		request_continuer_func_type_fmt,
		reply_continuer_func_type_fmt,
		
		continue_request_stub_scoped_fmt,
		continue_reply_stub_scoped_fmt,
		request_continuer_func_type_scoped_fmt,
		reply_continuer_func_type_scoped_fmt,
		
		/*
		 * Basic names: constants, types, ...
		 */
		const_fmt,
		type_fmt,
		
		const_scoped_fmt,
		type_scoped_fmt,
		
		enum_tag_fmt,
		enum_member_fmt,
		
		enum_tag_scoped_fmt,
		
		struct_slot_fmt,
		struct_union_tag_fmt,
		
		/*
		 * Names used to create object types from interfaces.
		 *
		 * A ``basic object type'' is a generic type used to define an
		 * interface-specific object type.  Example basic object types
		 * would be such things as `CORBA_Object', `mom_ref_t', etc.
		 *
		 * The other object types are the interface-specific objects.
		 */
		basic_message_type_fmt,
		client_basic_object_type_fmt,
		server_basic_object_type_fmt,
		
		basic_message_type_scoped_fmt,
		client_basic_object_type_scoped_fmt,
		server_basic_object_type_scoped_fmt,
		
		client_interface_object_type_fmt,
		server_interface_object_type_fmt,
		
		client_interface_object_type_scoped_fmt,
		server_interface_object_type_scoped_fmt,
		
		/*
		 * Stub component names: types, parameters, and operation
		 * codes.
		 *
		 * Notice that the ``client_stub/server_func'' object type
		 * formats are used to create the types of the target object as
		 * presented in the stub/func parameter lists.  These are
		 * separate from the two sets of object type formats previously
		 * listed so that (1) stubs can refer to object types other
		 * than the type for the interface in which the stubs are
		 * defined/inherited, and (2) we can work around a current
		 * behavior of `calc_name'.  The `stub/func_object_type's are
		 * scoped *within* the interface in which the corresponding
		 * operation appears, but the `basic_object_type's and
		 * `interface_object_type's are scoped *outside* of the
		 * interfaces to which they refer.
		 */
		client_stub_object_type_fmt,
		server_func_object_type_fmt,
		
		client_stub_object_type_scoped_fmt,
		server_func_object_type_scoped_fmt,
		
		client_stub_environment_type_fmt,
		server_func_environment_type_fmt,
		
		client_stub_environment_type_scoped_fmt,
		server_func_environment_type_scoped_fmt,
		
		stub_invocation_id_type_fmt,
		stub_client_ref_type_fmt,
		
		stub_invocation_id_type_scoped_fmt,
		stub_client_ref_type_scoped_fmt,
		
		client_stub_client_sid_type_fmt,
		server_func_client_sid_type_fmt,
		
		client_stub_client_sid_type_scoped_fmt,
		server_func_client_sid_type_scoped_fmt,
		
		client_stub_server_sid_type_fmt,
		server_func_server_sid_type_fmt,
		
		client_stub_server_sid_type_scoped_fmt,
		server_func_server_sid_type_scoped_fmt,
		
		stub_param_fmt,
		
		// Two format strings for every kind of stub special parameter:
		// one for client stub and one for server function.  See the
		// definition of `stub_special_param_kind' below.
		client_stub_object_param_fmt,
		server_func_object_param_fmt,
		
		client_stub_environment_param_fmt,
		server_func_environment_param_fmt,
		
		client_stub_client_sid_param_fmt,
		server_func_client_sid_param_fmt,
		
		client_stub_required_server_sid_param_fmt,
		server_func_required_server_sid_param_fmt,
		
		client_stub_actual_server_sid_param_fmt,
		server_func_actual_server_sid_param_fmt,
		
		//
		operation_request_code_fmt,
		operation_reply_code_fmt,
		
		/*
		 * Exceptions.
		 */
		exception_type_fmt,
		exception_code_fmt,
		// exception_slot_fmt,
		
		exception_type_scoped_fmt,
		
		/*
		 * Memory allocator functions.
		 */
		allocator_function_fmt,
		deallocator_function_fmt,
		
		allocator_function_scoped_fmt,
		deallocator_function_scoped_fmt,
		
		/*
		 * `#include' file names.
		 */
		presentation_include_file_fmt,
		interface_include_file_fmt,
		interface_default_include_file_fmt,
		
		number_of_name_fmt_kinds
	} name_fmt_kind;
	
	typedef enum
	{
		/*
		 * Enumerate the different literal strings that are used to
		 * construct names.  These enumeration names are used to index
		 * into an array of literal strings (`calc_name_component'
		 * strucutres).
		 *
		 * If you create a new literal string, you must also:
		 *
		 + + In `p_calc_name.cc', use `DEFINE_CALC_NAME_LIT_OPTION' to
		 *   make a command line option corresponding to your new
		 *   string (so that users can change its value).
		 *
		 * + In `pg_state.cc', give your new string a default value.
		 *
		 * + In the constructors for specific presentation generators,
		 *   override the default value of your string as necessary.
		 *
		 * Presumably, you will also have to modify the `calc_name'
		 * method to create a new `%'-escape to access your new literal
		 * string, too.
		 */
		null_lit,
		
		separator_lit,
		
		presentation_style_lit,
		
		filename_component_separator_lit,
		
		number_of_name_lit_kinds
	} name_lit_kind;
	
struct name_strings {
	/*
	 * `formats' is the array of `printf'-style format strings used to
	 * construct the names of various presentation elements.  This array is
	 * indexed by the enumeration type defined above.
	 *
	 * Later on, we use `DECLARE_CALC_NAME_FUNCTION' to declare `pg_state'
	 * member functions that correspond to each kind of name that can be
	 * built --- these are in one-to-one correspondence with the format
	 * strings below.
	 *
	 * XXX --- Idea: Turn this `name_strings' structure into a real name
	 * generator class, and make the `calc_name' methods be methods of this
	 * class.  Need access to certain `pg_state' member data, however.
	 */
	const char *formats[number_of_name_fmt_kinds];
	
	/*
	 * These are literal strings used to construct names: separators,
	 * prefixes, suffixes, etc.
	 */
	calc_name_component literals[number_of_name_lit_kinds];
};

/*
 * `p_calc_name.cc' defines an array of `name_fmt_option_struct's, one for each
 * format string, and an array of `name_lit_option_struct's, one for each
 * literal string.  These arrays are interpreted by `pg_state::build_flags' in
 * order to create real command line options which allow the user to change the
 * format and literal strings used to compute various names.
 */
struct name_fmt_option_struct {
	const char			*name;
	name_fmt_kind			index;
	const char			*explain;
};

struct name_lit_option_struct {
	const char			*name;
	name_lit_kind			index;
	const char			*explain;
};


/*****************************************************************************/

/*
 * A `stub_special_params' structure describes the ``special'' parameters that
 * are part of a client stub or server work function.  These special parameters
 * include the target object reference, the environment, and other elements
 * that aren't specified directly by the IDL source.
 *
 * We need to know four things about each special parameter: the role it plays,
 * its index in the parameter list, its C type (described in CAST), and its
 * name (determined by an appropriate `calc_*_name' method).
 */
struct stub_special_params {
	
	typedef enum 
	{
		object_ref,
		environment_ref,
		return_ref,
		message_ref,
		invocation_ref,
		client_ref,
		continue_func_ref,
		continue_data_ref,
		client_sid,		/* client-specified effective SID */
		required_server_sid,	/* client-specified required SID  */
		actual_server_sid,	/* server-supplied actual SID     */
		
		/* THIS MUST BE THE LAST ELEMENT OF THE ENUMERATION. */
		number_of_stub_special_param_kinds
	} stub_special_param_kind;
	
	struct stub_param_info {
		int index;
		cast_param_spec spec;
		cast_type ctype;
		const char *name;
	};
	
	stub_param_info params[number_of_stub_special_param_kinds];
};


/*****************************************************************************/

/*
 * These bits are used to mark (interface, operation/attribute/interface) pairs
 * once they have been processed by `p_client_stubs' and `p_server_skel'.
 */
#define P_CLIENT_STUB_OP_MARK		(0x00000001)
#define P_CLIENT_STUB_PARENT_MARK	(0x00000002)
#define P_SERVER_SKEL_OP_MARK		(0x00000004)
#define P_SERVER_SKEL_PARENT_MARK	(0x00000008)

class pg_state {
protected:
	/*
	 * Constructors.
	 */
	pg_state();
	
	/* Data structures */
	
	aoi *in_aoi;		// The aoi file we're translating
	meta *meta_data;
	pres_c_1 *out_pres;	// The presentation we're generating
	mint_ref top_union;	// Points to top level union of all operations
	mint_ref *aoi_to_mint_association;	// The link between aoi & mint
	int gen_client;		// 0 == do not generate client stubs
	int gen_server;		// 0 == do not generate server stubs
	int gen_sids;		// 0 == do not include SIDs in param lists
	
	// The builtin file, used for definitions the pg's create automatically
	io_file_index builtin_file;
	
	// The current set of channels for each file
	enum {
		PG_CHANNEL_CLIENT_DECL,
		PG_CHANNEL_CLIENT_IMPL,
		PG_CHANNEL_SERVER_DECL,
		PG_CHANNEL_SERVER_IMPL,
		PG_CHANNEL_MAX
	};
	
	// The string names for each channel
	static const char *pg_channel_names[PG_CHANNEL_MAX];
	/*
	 * This is a mapping used to get a specific channel index for a file,
	 * PG_CHANNEL_* pair.  We do this since there isn't a strong
	 * relationship between the two in the XDR structures.
	 */
	data_channel_index *pg_channel_maps[PG_CHANNEL_MAX];
	
	/*
	 * The `printf'-style format strings that define the names of (many)
	 * presentation components: stub functions, ...
	 */
	name_strings names;
	
	/*
	 * These flags control `p_client_stubs' and `p_server_skel': do we make
	 * client stubs and/or server functions for operations that are
	 * inherited by an interface?
	 */
	int client_stubs_for_inherited_operations;
	int server_funcs_for_inherited_operations;
	int async_stubs;
	
	/*********************************************************************/
	
	/*
	 * `cur_aoi_idx' is the index of the top-level AOI definition that is
	 * currently being processed.
	 * `gen_aoi_idx' is the index of the next AOI definition that will
	 * be processed.
	 */
	aoi_ref cur_aoi_idx;
	aoi_ref gen_aoi_idx;
	
	/*
	 * AOI references to the interfaces that are currently being processed.
	 */
	aoi_ref parent_interface_ref;
	aoi_ref derived_interface_ref;
	
	/*
	 * As we descend through the AOI tree, `name' keeps track of the name
	 * of the current ``thing'' being defined.  It is used to generate
	 * names of C types and structure members and such.
	 *
	 * XXX --- The `name' slot is antiquated but is still required in
	 * certain cases --- most notably, when the name that must be generated
	 * depends on the name of a non-top-level AOI construct (e.g., a slot
	 * name).  The ONC RPC mapping of sequences within structures is an
	 * example of a mapping that requires names to be derived from the name
	 * of a slot (i.e., `<slot>_len' and `<slot>_val' from `<slot>').
	 *
	 * When the ``thing'' being processed is a top-level AOI construct, we
	 * can get that thing's name through `a(cur_aoi_idx.).name'.
	 *
	 * XXX --- Really, we should just add a `name' argument to the PG
	 * methods that need to build CAST and PRES_C.
	 */
	/*
	 * XXX --- `name' should be `const char *', but that poisons more than
	 * I want to fix right now.  For now, sites at which we store a const
	 * string into name are marked with `pg_state_name_strlit'; compare
	 * this with the `ir_strlit' macro defined in `compiler.h'.
	 *
	 * If/when the `name' slot becomes `const char *', we should get rid of
	 * the `pg_state_name_strlit' macro.
	 */
	/* const */ char *name;
	
#define pg_state_name_strlit(str)	((char *) (str))
	
	cast_scope *root_scope;
	struct ptr_stack *scope_stack;
	cast_scoped_name current_scope_name;
	cast_func_spec server_func_spec;
	cast_func_spec client_func_spec;
	cast_storage_class mu_storage_class;
	
	/* Stupid data accessor functions (used to be macros) */
	virtual aoi_def &a(int i);         // in_aoi->aoi_val[i]
	virtual mint_def &m(int i);        // out_pres->mint.mint_1_val[i]
	virtual cast_def &c(int i);        // out_pres->cast.cast_scope_val[i]
	virtual pres_c_stub &s(int i);     // out_pres->stubs.stubs_val[i]
	virtual cast_func_type &cf(int n); // out_pres->cast.cast_scope_val[n].
					   // u.cast_def_u_u.func_type
	virtual data_channel_index &ch(aoi_ref ar, int channel_type);
	
	/*
	 * Functions for generating names.
	 */
	calc_name_struct calc_name_data;
	
	/* The `null' format should never be used. */
	/* DECLARE_CALC_NAME_FUNCTION(null);       */
	
	DECLARE_CALC_NAME_FUNCTION(client_stub);
	DECLARE_CALC_NAME_FUNCTION(server_skel);
	DECLARE_CALC_NAME_FUNCTION(client_skel);
	DECLARE_CALC_NAME_FUNCTION(server_func);
	DECLARE_CALC_NAME_FUNCTION(receive_request_func);
	DECLARE_CALC_NAME_FUNCTION(receive_reply_func);
	DECLARE_CALC_NAME_FUNCTION(marshal_stub);
	DECLARE_CALC_NAME_FUNCTION(unmarshal_stub);
	DECLARE_CALC_NAME_FUNCTION(message_marshal_request_stub);
	DECLARE_CALC_NAME_FUNCTION(message_unmarshal_request_stub);
	DECLARE_CALC_NAME_FUNCTION(message_marshal_reply_stub);
	DECLARE_CALC_NAME_FUNCTION(message_marshal_except_stub);
	DECLARE_CALC_NAME_FUNCTION(message_unmarshal_reply_stub);
	DECLARE_CALC_NAME_FUNCTION(message_request_type);
	DECLARE_CALC_NAME_FUNCTION(message_reply_type);
	DECLARE_CALC_NAME_FUNCTION(interface_message_request_type);
	DECLARE_CALC_NAME_FUNCTION(interface_message_reply_type);
	DECLARE_CALC_NAME_FUNCTION(send_request_stub);
	DECLARE_CALC_NAME_FUNCTION(send_reply_stub);
	DECLARE_CALC_NAME_FUNCTION(recv_request_stub);
	DECLARE_CALC_NAME_FUNCTION(recv_reply_stub);
	DECLARE_CALC_NAME_FUNCTION(continue_request_stub);
	DECLARE_CALC_NAME_FUNCTION(continue_reply_stub);
	DECLARE_CALC_NAME_FUNCTION(request_continuer_func_type);
	DECLARE_CALC_NAME_FUNCTION(reply_continuer_func_type);
	DECLARE_CALC_NAME_FUNCTION(const);
	DECLARE_CALC_NAME_FUNCTION(type);
	DECLARE_CALC_NAME_FUNCTION(enum_tag);
	DECLARE_CALC_NAME_FUNCTION(enum_member);
	DECLARE_CALC_NAME_FUNCTION(struct_slot);
	DECLARE_CALC_NAME_FUNCTION(struct_union_tag);
	DECLARE_CALC_NAME_FUNCTION(basic_message_type);
	DECLARE_CALC_NAME_FUNCTION(client_basic_object_type);
	DECLARE_CALC_NAME_FUNCTION(server_basic_object_type);
	DECLARE_CALC_NAME_FUNCTION(client_interface_object_type);
	DECLARE_CALC_NAME_FUNCTION(server_interface_object_type);
	DECLARE_CALC_NAME_FUNCTION(client_stub_object_type);
	DECLARE_CALC_NAME_FUNCTION(server_func_object_type);
	DECLARE_CALC_NAME_FUNCTION(client_stub_environment_type);
	DECLARE_CALC_NAME_FUNCTION(server_func_environment_type);
	DECLARE_CALC_NAME_FUNCTION(stub_invocation_id_type);
	DECLARE_CALC_NAME_FUNCTION(stub_client_ref_type);
	DECLARE_CALC_NAME_FUNCTION(client_stub_client_sid_type);
	DECLARE_CALC_NAME_FUNCTION(server_func_client_sid_type);
	DECLARE_CALC_NAME_FUNCTION(client_stub_server_sid_type);
	DECLARE_CALC_NAME_FUNCTION(server_func_server_sid_type);
	DECLARE_CALC_NAME_FUNCTION(stub_param);
	DECLARE_CALC_NAME_FUNCTION(client_stub_object_param);
	DECLARE_CALC_NAME_FUNCTION(server_func_object_param);
	DECLARE_CALC_NAME_FUNCTION(client_stub_environment_param);
	DECLARE_CALC_NAME_FUNCTION(server_func_environment_param);
	DECLARE_CALC_NAME_FUNCTION(client_stub_client_sid_param);
	DECLARE_CALC_NAME_FUNCTION(server_func_client_sid_param);
	DECLARE_CALC_NAME_FUNCTION(client_stub_required_server_sid_param);
	DECLARE_CALC_NAME_FUNCTION(server_func_required_server_sid_param);
	DECLARE_CALC_NAME_FUNCTION(client_stub_actual_server_sid_param);
	DECLARE_CALC_NAME_FUNCTION(server_func_actual_server_sid_param);
	DECLARE_CALC_NAME_FUNCTION(operation_request_code);
	DECLARE_CALC_NAME_FUNCTION(operation_reply_code);
	DECLARE_CALC_NAME_FUNCTION(exception_type);
	DECLARE_CALC_NAME_FUNCTION(exception_code);
	DECLARE_CALC_NAME_FUNCTION(allocator_function);
	DECLARE_CALC_NAME_FUNCTION(deallocator_function);
	DECLARE_CALC_NAME_FUNCTION(presentation_include_file);
	DECLARE_CALC_NAME_FUNCTION(interface_include_file);
	DECLARE_CALC_NAME_FUNCTION(interface_default_include_file);
	
	DECLARE_CALC_SCOPED_NAME_FUNCTION(client_stub);
	DECLARE_CALC_SCOPED_NAME_FUNCTION(server_skel);
	DECLARE_CALC_SCOPED_NAME_FUNCTION(client_skel);
	DECLARE_CALC_SCOPED_NAME_FUNCTION(server_func);
	DECLARE_CALC_SCOPED_NAME_FUNCTION(receive_request_func);
	DECLARE_CALC_SCOPED_NAME_FUNCTION(receive_reply_func);
	DECLARE_CALC_SCOPED_NAME_FUNCTION(marshal_stub);
	DECLARE_CALC_SCOPED_NAME_FUNCTION(unmarshal_stub);
	DECLARE_CALC_SCOPED_NAME_FUNCTION(message_marshal_request_stub);
	DECLARE_CALC_SCOPED_NAME_FUNCTION(message_unmarshal_request_stub);
	DECLARE_CALC_SCOPED_NAME_FUNCTION(message_marshal_reply_stub);
	DECLARE_CALC_SCOPED_NAME_FUNCTION(message_marshal_except_stub);
	DECLARE_CALC_SCOPED_NAME_FUNCTION(message_unmarshal_reply_stub);
	DECLARE_CALC_SCOPED_NAME_FUNCTION(message_request_type);
	DECLARE_CALC_SCOPED_NAME_FUNCTION(message_reply_type);
	DECLARE_CALC_SCOPED_NAME_FUNCTION(interface_message_request_type);
	DECLARE_CALC_SCOPED_NAME_FUNCTION(interface_message_reply_type);
	DECLARE_CALC_SCOPED_NAME_FUNCTION(send_request_stub);
	DECLARE_CALC_SCOPED_NAME_FUNCTION(send_reply_stub);
	DECLARE_CALC_SCOPED_NAME_FUNCTION(recv_request_stub);
	DECLARE_CALC_SCOPED_NAME_FUNCTION(recv_reply_stub);
	DECLARE_CALC_SCOPED_NAME_FUNCTION(continue_request_stub);
	DECLARE_CALC_SCOPED_NAME_FUNCTION(continue_reply_stub);
	DECLARE_CALC_SCOPED_NAME_FUNCTION(request_continuer_func_type);
	DECLARE_CALC_SCOPED_NAME_FUNCTION(reply_continuer_func_type);
	DECLARE_CALC_SCOPED_NAME_FUNCTION(const);
	DECLARE_CALC_SCOPED_NAME_FUNCTION(type);
	DECLARE_CALC_SCOPED_NAME_FUNCTION(enum_tag);
	DECLARE_CALC_SCOPED_NAME_FUNCTION(basic_message_type);
	DECLARE_CALC_SCOPED_NAME_FUNCTION(client_basic_object_type);
	DECLARE_CALC_SCOPED_NAME_FUNCTION(server_basic_object_type);
	DECLARE_CALC_SCOPED_NAME_FUNCTION(client_interface_object_type);
	DECLARE_CALC_SCOPED_NAME_FUNCTION(server_interface_object_type);
	DECLARE_CALC_SCOPED_NAME_FUNCTION(client_stub_object_type);
	DECLARE_CALC_SCOPED_NAME_FUNCTION(server_func_object_type);
	DECLARE_CALC_SCOPED_NAME_FUNCTION(client_stub_environment_type);
	DECLARE_CALC_SCOPED_NAME_FUNCTION(server_func_environment_type);
	DECLARE_CALC_SCOPED_NAME_FUNCTION(stub_invocation_id_type);
	DECLARE_CALC_SCOPED_NAME_FUNCTION(stub_client_ref_type);
	DECLARE_CALC_SCOPED_NAME_FUNCTION(client_stub_client_sid_type);
	DECLARE_CALC_SCOPED_NAME_FUNCTION(server_func_client_sid_type);
	DECLARE_CALC_SCOPED_NAME_FUNCTION(client_stub_server_sid_type);
	DECLARE_CALC_SCOPED_NAME_FUNCTION(server_func_server_sid_type);
	DECLARE_CALC_SCOPED_NAME_FUNCTION(exception_type);
	DECLARE_CALC_SCOPED_NAME_FUNCTION(allocator_function);
	DECLARE_CALC_SCOPED_NAME_FUNCTION(deallocator_function);
	
	virtual char *calc_name(const char *format, const char *object_name);
	virtual void calc_scoped_name(cast_scoped_name *name,
				      aoi_ref ref,
				      const char *format);
	
	virtual char *calc_name_from_ref(aoi_ref ref);
	virtual cast_scoped_name calc_scoped_name_from_ref(aoi_ref ref);
	
	virtual void calc_name_module(aoi_ref last_ref,
				      int qualified_p,
				      int separator_p);
	
	/* creates the mappings between files and channels */
	virtual void map_file_channels(io_file_index file);
	
	virtual int add_function(tag_list *tl,
				 cast_scoped_name fname,
				 int tag, ...);
	
	/* Functions Added by KBF */
	/* This is for correct nested scoped names (primarily corba) */
	virtual char *getscopedname(int aoi_idx);
	
	/* Internal functions. */
	virtual void p_interface_def_include(aoi_interface *ai);
	virtual void p_interface_def_typedef(aoi_interface *ai);
	virtual void p_interface_async_msg_types(aoi_interface *ai);
	
	virtual void p_interface_table_clear();
	virtual int  p_interface_table_get_value(aoi_interface *ai,
						 void *object);
	virtual void p_interface_table_set_value(aoi_interface *ai,
						 void *object,
						 int value);
	
	virtual void      p_client_stubs(aoi_interface *a);
	virtual void      p_client_stubs_internal(aoi_ref this_ref,
						  aoi_ref derived_ref);
	
	virtual void	  p_client_stub_special_params(aoi_operation *ao,
						       stub_special_params *s);
	virtual int	p_msg_send_stub_special_params(aoi_interface *ai,
						       stub_special_params *s,
						       int request);
	
	virtual int	  p_msg_marshal_stub_special_params(
		aoi_operation *ao,
		stub_special_params *s,
		int client,
		int request);
	
	virtual void      p_client_stub_return_type(
		aoi_operation *ao,
		mint_ref mr,
		cast_type *out_ctype,
		pres_c_mapping *out_mapping);
	virtual void      p_client_stub_find_refs(
		aoi_interface *a,
		aoi_operation *ao,
		mint_const oper_request_discrim,
		mint_const oper_reply_discrim,
		/* OUT */ mint_ref *request_ref,
		/* OUT */ mint_ref *reply_ref);
	
	
	virtual void      p_skel_internal(aoi_ref this_ref,
					  aoi_ref derived_ref,
					  int skel_index);
	
	virtual void	  p_server_func_special_params(aoi_operation *ao,
						       stub_special_params *s);
	
	virtual int	  p_receive_func_special_params(
		aoi_operation *ao,
		stub_special_params *s,
		int request);
	
	virtual void      p_server_func_return_type(
		aoi_operation *ao,
		mint_ref mr,
		cast_type *out_ctype,
		pres_c_mapping *out_mapping);
	virtual void      p_server_func_alloc_return(
		cast_type return_ctype,
		pres_c_mapping *out_mapping);
	virtual void      p_server_func_find_refs(
		aoi_interface *a,
		aoi_operation *ao,
		mint_const oper_request_discrim,
		mint_const oper_reply_discrim,
		/* OUT */ mint_ref *request_ref,
		/* OUT */ mint_ref *reply_ref);
	virtual void p_server_func_make_decl(aoi_interface *ai,
					     aoi_operation *ao,
					     char *opname,
					     cast_func_type *cfunc);
	virtual void      p_receive_func_make_refs(
		aoi_interface *a,
		aoi_operation *ao,
		mint_const oper_discrim,
		int request,
		/* OUT */ mint_ref *m_ref);
	
	
	virtual void p_param_type(aoi_type at, mint_ref mr, aoi_direction dir,
				  cast_type *out_ctype,
				  pres_c_mapping *out_mapping);
	
	virtual void p_async_param_type(aoi_type at, mint_ref mr,
					aoi_direction dir,
					cast_type *out_ctype,
					pres_c_mapping *out_mapping,
					int encode);
	
	virtual void p_async_param_return_type(aoi_type at, mint_ref mr,
					       cast_type *out_ctype,
					       pres_c_mapping *out_mapping,
					       int encode);
	
	virtual void p_param_server_alloc_out(cast_type param_ctype,
					      mint_ref param_itype,
					      pres_c_mapping param_mapping,
					      pres_c_mapping *alloc_mapping);
	
	// This is called before we start generating the pres_c
	virtual void build_init_cast(void);
	
	virtual void p_emit_include_stmt(const char *filename,
					 int system_only);
	
	/* This stuff is for argument handling */
	virtual FILE **cmdline(int argc, char **argv);
	virtual pg_flags args(int argc, char **argv, char *info = 0);
	virtual int build_flags(flags_in **);
	virtual pg_flags handler(flags_out out, flags_in *in,
				 char *info, int count);
	virtual void p_namespace_def();
public:
	// KBF - Added this, so we can override EVERYTHING
	virtual int main(int argc, char **argv);
	
	virtual void init_name_context();
	
	/* This is for any preprocessing of ANYTHING */
	virtual void preprocess();
	
	/*
	 * Routines to inline AOI type nodes into the elements of a C structure
	 * or the parameters of a C function declaration.
	 */
	virtual pres_c_inline p_inline_add_atom(cast_type inl_ctype, 
						char *atom_name,
						cast_type atom_ctype,
						pres_c_mapping atom_mapping);
	virtual pres_c_inline p_inline_struct(aoi_struct *as,
					      p_type_collection *inl_ptc,
					      cast_type inl_ctype);
	virtual pres_c_inline p_inline_exception(aoi_exception *as,
						 p_type_collection *inl_ptc,
						 cast_type inl_ctype);
	virtual pres_c_inline p_inline_struct_union(aoi_union *au,
						    p_type_collection *inl_ptc,
						    cast_type inl_ctype);
	virtual pres_c_inline p_inline_type(aoi_type at,
					    char *name,
					    p_type_collection *inl_ptc,
					    cast_type inl_ctype);
	
	/* Routines to translate AOI type nodes
	   into C type definitions and matching pres_c mapping nodes.
	   p_type() is the top-level switch that dispatches to the others.  */
	virtual void p_indirect_type(aoi_ref ref, p_type_collection **out_ptc);
	virtual void p_integer_type(aoi_integer *ai,
				    p_type_collection **out_ptc);
	virtual void p_scalar_type(aoi_scalar *ai,
				   p_type_collection **out_ptc);
	virtual void p_scalar(int bits, int is_signed,
			      p_type_collection **out_ptc);
	virtual void p_float_type(aoi_float *ai, p_type_collection **out_ptc);
	virtual void p_char_type(aoi_char *ac, p_type_collection **out_ptc);
	virtual void p_struct_type(aoi_struct *as,
				   p_type_collection **out_ptc);
	virtual void p_except_type(aoi_exception *as,
				   p_type_collection **out_ptc);
	virtual void p_fixed_array_type(aoi_array *aa,
					p_type_collection **out_ptc);
	virtual void p_variable_array_type(aoi_array *aa,
					   p_type_collection **out_ptc)
		= 0;
	virtual void p_variable_array_length_type(aoi_array *aa,
						  p_type_collection **out_ptc,
						  pres_c_mapping *out_map,
						  char *arglist_name);
	virtual void p_variable_array_maximum_type(aoi_array *aa,
						   p_type_collection **out_ptc,
						   pres_c_mapping *out_map,
						   char *arglist_name);
	virtual void p_variable_array_release_type(aoi_array *aa,
						   p_type_collection **out_ptc,
						   pres_c_mapping *out_map,
						   char *arglist_name);
	virtual void p_array_type(aoi_array *aa, p_type_collection **out_ptc);
	virtual void p_union_type(aoi_union *au, p_type_collection **out_ptc);
	virtual void p_interface_type(aoi_interface *ai,
				      p_type_collection **out_ptc);
	virtual void p_forward_type(p_type_collection **out_ptc);
	virtual void p_enum_type(aoi_enum *ae,
				 p_type_collection **out_ptc);
	virtual void p_void_type(p_type_collection **out_ptc);
	virtual void p_optional_type(aoi_optional *ao,
				     p_type_collection **out_ptc);
	virtual void p_any_type(p_type_collection **out_ptc)
		= 0;
	virtual void p_type_tag_type(p_type_collection **out_ptc)
		= 0;
	virtual void p_typed_type(aoi_typed *at, p_type_collection **out_ptc)
		= 0;
	
	virtual void p_type(aoi_type at, p_type_collection **out_ptc);
	
	/* Create and clear a new stub entry in 'pres' and return its index. */
	virtual int p_add_stub(pres_c_1 *pres);
	
	/* Routines to create stub descriptions.  */
	virtual void p_marshal_stub_conn(pres_c_marshal_stub *mstub,
					 int func_ref);
	virtual void p_marshal_stub_data(cast_type ctype, pres_c_mapping map,
					 pres_c_stub_kind kind,
					 pres_c_marshal_stub *mstub,
					 int func_ref);
	virtual void p_mu_stub(cast_type ctype_name, pres_c_mapping map,
			       mint_ref itype,
			       pres_c_stub_kind kind, char *func_name,
			       pres_c_mapping seethru_map);
	
	virtual void p_marshal_stub(aoi_type at,
				    cast_type ctype_name,
				    pres_c_mapping map,
				    pres_c_mapping seethru_map);
	virtual void p_unmarshal_stub(aoi_type at,
				      cast_type ctype_name,
				      pres_c_mapping map,
				      pres_c_mapping seethru_map);
	
	/* Routines to translate definitions in the topmost AOI scope
	   to C definitions and stub descriptions.
	   On entry, the `name' variable above is the name being defined.
	   p_def() is the top-level switch that dispatches to the others.  */
	virtual void p_typedef_def(aoi_type at);
	virtual void p_const_def(aoi_const_def *ac);
	virtual cast_type p_const_type(aoi_const_def *ac);
	virtual cast_expr p_const_translate(aoi_const_def *ac);
	virtual cast_type p_make_ctypename(aoi_ref ref);
	
	/* Routines for interface presentation generation. */
	virtual void p_interface_def(aoi_interface *ai);

	virtual void p_message_marshal_stub(aoi_interface *ai,
					    aoi_operation *ao,
					    int client, int request);
	virtual void p_message_marshal_exception_stub(aoi_interface *ai,
						      aoi_operation *ao);
	virtual void p_client_stub(aoi_interface *ai, aoi_operation *ao);
	virtual void p_send_stub(aoi_interface *ai, int request);
#ifdef ATTRIBUTE_STUBS
	virtual void p_client_attrib_stub(aoi_interface *a, aoi_attribute *aa);
	virtual void p_server_attrib_func(pres_c_skel *skel, int *i,
					  aoi_interface *a, aoi_attribute *aa);
#endif // ATTRIBUTE_STUBS
	
	virtual pres_c_func p_server_func(aoi_interface *ai,
					  aoi_operation *ao);
	
	virtual pres_c_func p_receive_func(aoi_interface *ai,
					   aoi_operation *ao,
					   int request);
	
	virtual int p_skel(aoi_interface *a);
	virtual cast_ref p_skel_cdef(const char *skel_name,
				     const char *skel_type_name);
	
	virtual int  p_msg_cont_stub_special_params(aoi_interface *ai,
						       stub_special_params *s,
						       int request);
	virtual void p_continue_stub(aoi_interface *ai,
				     aoi_operation *ao,
				     int request);
	virtual void p_continue_func_types(aoi_interface *ai);
	
	virtual cast_func_param p_param(
		aoi_parameter *ap,
		int cast_param_index,
		mint_ref mr,
		pres_c_inline request_i,
		pres_c_inline reply_i,
		int request_mint_struct_slot_index,
		int reply_mint_struct_slot_index);
	virtual cast_func_param p_async_param(
		aoi_parameter *ap,
		int cast_param_index,
		mint_ref mr,
		pres_c_inline inl,
		int mint_struct_slot_index,
		int encode,
		int request);
	virtual cast_func_param p_async_param_return(
		const char *param_name,
		aoi_type param_aoi_type,
		int cast_param_index,
		mint_ref mr,
		pres_c_inline inl,
		int mint_struct_slot_index,
		int encode,
		int request);
	
	virtual void p_param_client_sid(int param_index,
					pres_c_inline request_inline);
	virtual void p_param_required_server_sid(int param_index,
						 pres_c_inline request_inline);
	virtual void p_param_actual_server_sid(int param_index,
					       pres_c_inline reply_inline);
	
	virtual void p_def(aoi_type at);
	
	virtual void p_shuffle_types(int org, int next);

	/* Top-level generator routine, which calls everything else.  */
	virtual void gen();
	virtual void gen_scope(int scope);
	
	virtual void process_client_params(
		cast_func_type *cp,
		stub_special_params *specials,
		mint_ref request_ref, mint_ref reply_ref,
		aoi_operation *ao,
		pres_c_inline request_l4_inl, pres_c_inline reply_l4_inl,
		pres_c_inline target_inl, pres_c_inline client_inl);
	
	virtual void process_server_params(
		cast_func_type *cp,
		stub_special_params *specials,
		mint_ref request_ref, mint_ref reply_ref,
		aoi_operation *ao,
		pres_c_inline request_l4_inl, pres_c_inline reply_l4_inl,
		pres_c_inline target_inl, pres_c_inline client_inl);
	
	virtual void process_async_params(
		cast_func_type *cp,
		stub_special_params *specials,
		mint_ref m_ref,
		aoi_operation *ao,
		pres_c_inline l4_inl,
		pres_c_inline target_inl, pres_c_inline client_inl,
		int encode, int request);
	
	virtual pres_c_mapping p_make_exception_discrim_map(
		char *arglist_name);
	virtual void p_do_return_union(aoi_operation *ao,
				       pres_c_inline *reply_l4_inl,
				       mint_ref reply_ref,
				       cast_ref cfunc,
				       pres_c_inline_index discrim_idx);
	
	virtual pres_c_allocator p_get_allocator(void);
	virtual pres_c_allocation p_get_allocation(void);
	virtual int p_is_normal_return_case(mint_union_def *mu, int i);
	virtual void p_do_exceptional_case(
		pres_c_inline_virtual_union_case *vucase,
		mint_union_case *ucase,
		int loc,
		pres_c_inline_index discrim_idx) = 0;
	
        /* Given a MINT constant, this function creates and returns an
	   equivalent CAST expression. */
	virtual cast_expr p_mint_const_to_cast(mint_const mint_literal);
	virtual cast_expr p_mint_exception_id_const_to_cast(
		mint_const mint_literal);
	
	/* Allows the pg's to add their own builtin functions to the
	   internal representations.  p_add_builtin_server_func is called
	   by p_server_skel when adding the other functions, and
	   p_add_builtin_client_func is called at the end of gen */
        virtual void p_add_builtin_server_func(aoi_interface *ai,
					       char *name,
					       pres_c_skel *skel);
	virtual void p_add_builtin_client_func();
	
	cast_aggregate_kind union_aggregate_type;
	cast_aggregate_kind struct_aggregate_type;
	cast_aggregate_kind struct_union_aggregate_type;
	cast_aggregate_kind except_aggregate_type;
	
	cast_def_protection current_protection;
	
	/* Generates mappings from internal error values
	   generated by stubs to presentation specific values */
	virtual void gen_error_mappings();
	
	virtual p_type_collection *p_new_type_collection(
		const char *base_name);
	
	unsigned int struct_type_node_flags;
	unsigned int union_type_node_flags;
	unsigned int enum_type_node_flags;
	struct dl_list type_collections;
	
	virtual void make_prim_collections();
	
	enum {
		PRIM_COLLECTION_VOID,
		PRIM_COLLECTION_BOOLEAN,
		PRIM_COLLECTION_CHAR,
		PRIM_COLLECTION_SCHAR,
		PRIM_COLLECTION_UCHAR,
		PRIM_COLLECTION_OCTET,
		PRIM_COLLECTION_SHORT,
		PRIM_COLLECTION_USHORT,
		PRIM_COLLECTION_LONG,
		PRIM_COLLECTION_ULONG,
		PRIM_COLLECTION_LONGLONG,
		PRIM_COLLECTION_ULONGLONG,
		PRIM_COLLECTION_FLOAT,
		PRIM_COLLECTION_DOUBLE,
		
		/*
		 * Things that aren't ``primitive'' per se, but which are
		 * nevertheless built-in.
		 */
		PRIM_COLLECTION_ENUM,
		PRIM_COLLECTION_STRING,
		PRIM_COLLECTION_TYPE_TAG,
		PRIM_COLLECTION_TYPED_ANY,
		
		/*
		 * This must be the last memeber of the enumeration.
		 */
		PRIM_COLLECTION_MAX
	};
	p_type_collection *prim_collections[PRIM_COLLECTION_MAX];
};

#endif /* _mom_c_pfe_hh_ */

/* End of file. */

