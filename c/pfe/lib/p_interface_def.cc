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

#include <stdio.h>
/*
  #include <string.h>
  We declare these individually rather than use the header because
  the HPUX and SunOS4 systems we tried compiling on have funky g++
  installations that make strrchr get munged _strrchr__FPci instead
  of _strrchr
  */
extern "C" char *strrchr(const char *s, int c);
extern "C" int strcmp(const char *s, const char *s2);

#include <mom/compiler.h>
#include <mom/libaoi.h>
#include <mom/c/pfe.hh>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include "private.hh"

/* Generate presentation for an AOI interface definition. */
void pg_state::p_interface_def(aoi_interface *ai)
{
	/*
	 * This function generates `pres_c_stub' entries for the given AOI
	 * interface `a'.  These `pres_c_stub' entries will be elements in the
	 * `out_pres->stubs' member variable in the `pg_state' structure.  For
	 * each AOI interface there will be one server stub (i.e., the server
	 * dispatch function) but multiple client stubs (generally, one client
	 * stub for each function).
	 *
	 * Filling in the `pres_c_stub' entry will involve allocation and
	 * assignment of other CAST and PRES_C data structures.
	 */
	
	/* Generate an `#include' for this interface if necessary. */
	p_interface_def_include(ai);
	
	/* Generate a typedef for the object reference (interface name). */
	p_interface_def_typedef(ai);
	
	/* Generate the contents of this interface. */
	gen_scope(a(cur_aoi_idx).scope + 1);
	
	if (gen_client)	{
		/* Generate client stubs. */
		p_client_stubs(ai);
	}
	if (gen_server)	{
		/* Generate the server skeleton stub. */
		p_skel(ai);
	}
}


/*****************************************************************************/

/***** Auxiliary functions. *****/

/*
 * Generate an `#include' statement to get user-supplied goo for the current
 * interface.  Most presentations don't require this.  This method is
 * sufficiently generic (customizable though the `calc_name' methods and format
 * strings) that few if any PGs should need to override this.
 *
 * XXX --- This code is similar to that in `pg_state::build_init_cast'.  Maybe
 * we should write a generic `#include' statement verifier.
 */
void pg_state::p_interface_def_include(aoi_interface * /*interface*/)
{
	char *include_file_name;
	char *include_file_name_nondir;
	
	calc_name_component *separator_component;
	char separator_char;
	
	/*
	 * Compute the `#include' file name to get interface-specific goo.
	 */
	include_file_name =
		calc_interface_include_file_name(a(cur_aoi_idx).name);
	
	/*
	 * Now we have to make sure that the file name is valid.  Dig the
	 * file name component separator character out of our `names.literals'.
	 */
	separator_component =
		&(names.literals[name_strings::
				filename_component_separator_lit]);
	
	switch (separator_component->len) {
	case 1:
		separator_char = *(separator_component->str);
		break;
		
	case 0:
		warn("Can't really cope with a zero-character filename "
		     "component separator.");
		separator_char = '/';
		break;
		
	default:
		warn("Can't really cope with a multicharacter filename "
		     "component separator.");
		separator_char = separator_component->
				 str[separator_component->len - 1];
		break;
	}
	
	/*
	 * Now find the final file name component.
	 */
	include_file_name_nondir = strrchr(include_file_name, separator_char);
	if (include_file_name_nondir) {
		/*
		 * Don't increment in the bizarro case that `separator_char'
		 * is NUL.
		 */
		if (*include_file_name_nondir)
			++include_file_name_nondir;
	} else
		include_file_name_nondir = include_file_name;
	
	/*
	 * If the `#include' file name is bogus, try the default instead.
	 */
	if ((strcmp("", include_file_name_nondir) == 0)
	    || (strcmp(".h", include_file_name_nondir) == 0)) {
		/* Redo `include_file_name' and `include_file_name_nondir'. */
		include_file_name =
			calc_interface_default_include_file_name(
				a(cur_aoi_idx).name);
		
		include_file_name_nondir = strrchr(include_file_name,
						   separator_char);
		if (include_file_name_nondir) {
			if (*include_file_name_nondir)
				++include_file_name_nondir;
		} else
			include_file_name_nondir = include_file_name;
	}
	
	/*
	 * If the `#include' file name is good, emit the `#include' statement.
	 */
	if ((strcmp("", include_file_name_nondir) != 0)
	    && (strcmp(".h", include_file_name_nondir) != 0))
		p_emit_include_stmt(include_file_name,
				    1 /* 1 == system header */);
}

/*
 * Generate a `typedef' for the interface-specific object reference (derived
 * from the interface name).  Some presentation generators may override this
 * method, but again, the preferred specialization technique is through the
 * `calc_name' facilities (i.e., changing the format strings).
 */
void pg_state::p_interface_def_typedef(aoi_interface *ai)
{
	char *interface_name = a(cur_aoi_idx).name;
	cast_scoped_name basic_object_type_name;
	char *object_type_name;
	aoi_ref parent_ref;
	int cdef;
	
	/*
	 * Compute the name of the ``basic'' object type: the object reference
	 * type provided by the runtime (outside of Flick) that will be used to
	 * define our interface-specific object type.  For example, in CORBA,
	 * all object references are defined in terms of `CORBA_Object'.
	 *
	 * Also, compute the name of the interface-specific object type.
	 */
	parent_ref = aoi_get_parent_scope(in_aoi, cur_aoi_idx);
	if (gen_client) {
		basic_object_type_name =
			calc_client_basic_object_type_scoped_name(
				parent_ref, interface_name);
		object_type_name =
			calc_client_interface_object_type_name(interface_name);
		
	} else if (gen_server) {
		basic_object_type_name =
			calc_server_basic_object_type_scoped_name(
				parent_ref, interface_name);
		object_type_name =
			calc_server_interface_object_type_name(interface_name);
		
	} else
		panic("In `pg_state::p_interface_def_typedef', "
		      "generating neither client nor server.");
	
	/*
	 * Now make our typedef.
	 */
	cdef = cast_add_def((cast_scope *)top_ptr(scope_stack),
			    cast_new_scoped_name(object_type_name, NULL),
			    CAST_SC_NONE,
			    CAST_TYPEDEF,
			    ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
			    CAST_PROT_NONE);
	c(cdef).u.cast_def_u_u.typedef_type =
		cast_new_type_scoped_name(basic_object_type_name);
	
	if (async_stubs)
		p_interface_async_msg_types(ai);
}

/*
 * If you override this method, you should also override `p_forward_type',
 * which handles forward interface declarations.
 */
void pg_state::p_interface_type(aoi_interface * /*ai*/,
				p_type_collection **out_ptc)
{
	/*
	 * Here we are handling *basic* interface types, not *typedef'ed*
	 * interface types.  This method is called by `p_type' when we have a
	 * direct reference to an AOI interface.  References to *specific*
	 * interface types always occur through AOI_INDIRECT nodes, and so we
	 * handle those cases in `p_indirect_type'.
	 */
	p_type_collection *ptc;
	pres_c_mapping map;
	p_type_node *ptn;
	cast_type ctype;

	ptc = p_new_type_collection(a(cur_aoi_idx).name);
	ptn = new p_type_node;
	ptn->set_name("definition");
	ptn->set_format("%s");
	map = pres_c_new_mapping(PRES_C_MAPPING_REFERENCE);
	map->pres_c_mapping_u_u.ref.ref_count = 1;
	map->pres_c_mapping_u_u.ref.arglist_name = ir_strlit("");
	
	if (gen_client) {
		/* default for client is sending copies */
		map->pres_c_mapping_u_u.ref.kind
			= PRES_C_REFERENCE_COPY;
		ctype =
			cast_new_type_name(
				calc_client_basic_object_type_name(
					a(cur_aoi_idx).name /* XXX ? */
					));
	} else if (gen_server) {
		/* default for server is returning its only copy */
		map->pres_c_mapping_u_u.ref.kind = PRES_C_REFERENCE_MOVE;
		ctype = cast_new_type_name(
				calc_server_basic_object_type_name(
					a(cur_aoi_idx).name /* XXX ? */
					));
	}
	else
		panic("In `pg_state::p_interface_type', "
		      "generating neither client nor server.");
	ptn->set_type(ctype);
	ptn->set_mapping(map);
	ptc->add_type("default", ptn);
	
	if( *out_ptc )
		(*out_ptc)->set_collection_ref(ptc);
	else
		*out_ptc = ptc;
}

void pg_state::p_interface_async_msg_types(aoi_interface * /*ai*/)
{
	char *interface_name = a(cur_aoi_idx).name;
	char *interface_type_name
		= calc_basic_message_type_name(interface_name);
	int cdef;
	
	cdef = cast_add_def(
		(cast_scope *)top_ptr(scope_stack),
		cast_new_scoped_name(calc_interface_message_request_type_name(
			interface_name), NULL),
		CAST_SC_NONE,
		CAST_TYPEDEF,
		ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		CAST_PROT_NONE);
	c(cdef).u.cast_def_u_u.typedef_type =
		cast_new_type_name(interface_type_name);
	
	cdef = cast_add_def(
		(cast_scope *)top_ptr(scope_stack),
		cast_new_scoped_name(calc_interface_message_reply_type_name(
			interface_name), NULL),
		CAST_SC_NONE,
		CAST_TYPEDEF,
		ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
		CAST_PROT_NONE);
	c(cdef).u.cast_def_u_u.typedef_type =
		cast_new_type_name(interface_type_name);
}

/* End of file. */
