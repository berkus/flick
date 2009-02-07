/*
 * Copyright (c) 1995, 1996, 1997, 1999 The University of Utah and
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
#include <stdlib.h>
#include <ctype.h>

#include <mom/compiler.h>
#include <mom/c/pfe.hh>

#include "pg_sun.hh"

/*****************************************************************************/

/*
 * These functions override methods of `pg_state'.
 */

char *pg_sun::calc_client_stub_name(const char *basic_name)
{
	char *basic_name_and_version_id, *client_stub_name;
	
	basic_name_and_version_id =
		calc_operation_and_version_id(basic_name);
	client_stub_name =
		pg_state::calc_client_stub_name(basic_name_and_version_id);
	
	free(basic_name_and_version_id);
	
	return client_stub_name;
}

char *pg_sun::calc_server_skel_name(const char * /*basic_name*/)
{
	char *program_name_and_version_id, *server_skel_name;
	
	program_name_and_version_id =
		calc_program_and_version_id();
	server_skel_name =
		pg_state::calc_server_skel_name(program_name_and_version_id);
	
	free(program_name_and_version_id);
	
	return server_skel_name;
}

char *pg_sun::calc_server_func_name(const char *basic_name)
{
	char *basic_name_and_version_id, *server_func_name;
	
	basic_name_and_version_id =
		calc_operation_and_version_id(basic_name);
	server_func_name =
		pg_state::calc_server_func_name(basic_name_and_version_id);
	
	free(basic_name_and_version_id);
	
	return server_func_name;
}


/*****************************************************************************/

/*
 * These functions are auxiliaries for the override methods above.
 */

#define NAME_LITERAL_STR(type) names.literals[ type##_lit].str

char *pg_sun::calc_operation_and_version_id(const char *basic_name)
{
	/*
	 * In Sun RPC, an operation name is constructed according to the rule:
	 * <downcased_operation_name> + "_" + <version_number>
	 *
	 * The operation name is determined by `basic_name'.  We must dig the
	 * version number out of the `pg_state::derived_interface_ref' member
	 * slot.
	 */
	aoi_interface *interface;
	
	aoi_const_int version;
	char *result;
	
	/*
	 * First, find the AOI interface that represents the Sun RPC `version'
	 * that is currently being processed.
	 */
	assert((derived_interface_ref >= 0)
	       && (derived_interface_ref < ((aoi_ref) in_aoi->defs.defs_len)));
	assert(a(derived_interface_ref).binding
	       && (a(derived_interface_ref).binding->kind == AOI_INTERFACE));
	
	interface = &(a(derived_interface_ref).binding->aoi_type_u_u.
		      interface_def);
	
	/*
	 * Dig the version number out of the discriminator code for the
	 * interface that represents this version.
	 */
	assert(interface->code);
	assert(interface->code->kind == AOI_CONST_STRUCT);
	assert(interface->code->
	       aoi_const_u_u.const_struct.aoi_const_struct_len == 2);
	assert(interface->code->
	       aoi_const_u_u.const_struct.aoi_const_struct_val[1]->kind ==
	       AOI_CONST_INT);
	
	version = interface->code->
		  aoi_const_u_u.const_struct.aoi_const_struct_val[1]->
		  aoi_const_u_u.const_int;
	
	if (version < 0)
		panic("In `pg_sun::calc_operation_and_version_id', "
		      "negative version number found.");
	
	/* Construct the new name. */
	result = flick_asprintf("%s%s%d",
				basic_name,
				NAME_LITERAL_STR(separator),
				version);
	for (char *c = result; *c; ++c)
		*c = tolower(*c);
	
	return result;
}

char *pg_sun::calc_program_and_version_id()
{
	/*
	 * In Sun RPC, a server skeleton name is constructed according to the
	 * rule: <downcased_program_name> + "_" + <version_number>
	 *
	 * Both the program name and version number must be dug out of the
	 * `pg_state::derived_interface_name' meber slot.
	 */
	aoi_interface *interface;
	
	char *program_name;
	aoi_const_int version;
	char *result;
	
	/*
	 * First, find the AOI interface that represents the Sun RPC `version'
	 * that is currently being processed.
	 */
	assert((derived_interface_ref >= 0)
	       && (derived_interface_ref < ((aoi_ref) in_aoi->defs.defs_len)));
	assert(a(derived_interface_ref).binding
	       && (a(derived_interface_ref).binding->kind == AOI_INTERFACE));
	
	interface = &(a(derived_interface_ref).binding->aoi_type_u_u.
		      interface_def);
	
	/*
	 * Find the program name by looking up the parent of the current
	 * interface.
	 */
	assert(interface);
	assert(interface->parents.parents_len == 1);
	assert(interface->parents.parents_val[0]);
	assert(interface->parents.parents_val[0]->kind == AOI_INDIRECT);
	
	program_name = a(interface->
			 parents.parents_val[0]->
			 aoi_type_u_u.indirect_ref).name;
	
	/*
	 * Dig the version number out of the discriminator code for the
	 * interface that represents this version.  We could dig it out of the
	 * parent if we wanted to do so.
	 */
	assert(interface->code);
	assert(interface->code->kind == AOI_CONST_STRUCT);
	assert(interface->code->
	       aoi_const_u_u.const_struct.aoi_const_struct_len == 2);
	assert(interface->code->
	       aoi_const_u_u.const_struct.aoi_const_struct_val[1]->kind ==
	       AOI_CONST_INT);
	
	version = interface->code->
		  aoi_const_u_u.const_struct.aoi_const_struct_val[1]->
		  aoi_const_u_u.const_int;
	
	if (version < 0)
		panic("In `pg_sun::calc_program_and_version_id', "
		      "negative version number found.");
	
	/* Construct the new operation name. */
	result = flick_asprintf("%s%s%d",
				program_name,
				NAME_LITERAL_STR(separator),
				version);
	for (char *c = result; *c; ++c)
		*c = tolower(*c);
	
	return result;
}

/*****************************************************************************/

/* End of file. */


