/*
 * Copyright (c) 1996, 1997 The University of Utah and
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

#include <mom/libaoi.h>

#include "pg_fluke.hh"

void pg_fluke::p_typedef_def(aoi_type at)
{
	/*
	 * A typedef for an interface is really a typedef for an object
	 * reference (i.e., to an entity that implements the interface).  In
	 * the Fluke presentation, object references differ between client
	 * stubs and server work functions.
	 *
	 * On the client, all object references are presented as `mom_ref_t',
	 * and no typedef is ever provided to ``hide'' that fact.  The Fluke PG
	 * versions of `p_client_stub_object_type' and `p_indirect_type' are
	 * what set the client-side object reference types.  In the code below,
	 * we avoid making any typedefs for object references on the client
	 * side.
	 *
	 * In a server work function, a target object reference is a pointer
	 * to a user-defined structure.  (A ``target'' object is an object upon
	 * which an operation is being invoked.)  The typedef that associates
	 * the interface name with the user-defined structure is defined in the
	 * Fluke PG's `p_interface_type' method.  The type of the target object
	 * reference is set by `p_server_func_object_type'.
	 *
	 * Non-target object references passed to a server work function are
	 * presented as `mom_ref_t'.  This type is set by the Fluke PG
	 * `p_indirect_type' method as mentioned previously.  Since this same
	 * type is used for all non-target object references in the server-side
	 * presentation, we don't need to emit any typedefs for interfaces that
	 * are forward-declared: we only need a typedef when an interface is
	 * actually defined.  In more concrete terms, for server presentations,
	 * we need to avoid making typedef's for AOI_FWD_INTRFCs but allow
	 * typedefs for AOI_INTERFACEs to go through.
	 */
	
	if (gen_client
	    && ((at->kind == AOI_INTERFACE) || (at->kind == AOI_FWD_INTRFC)))
		; /* Do nothing! */
	else if (gen_server
		 && (at->kind == AOI_FWD_INTRFC))
		; /* Do nothing! */
	else
		pg_corba::p_typedef_def(at);
}

/* End of file. */

