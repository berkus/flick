/*
 * Copyright (c) 1999 The University of Utah and
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

#include <mom/libmeta.h>
#include <mom/c/libcast.h>
#include <mom/c/pg_corba.hh>

/*
 * This method allows a presentation generator to create some initial CAST.
 * The library version creates an `#include' statement for a file that
 * defines presentation-specific things (e.g., the basic object type such as
 * `CORBA_Object').
 *
 * Here, we add the `CORBA_Environment' type to our CAST tree for later
 * reference.
 */

void pg_corba::build_init_cast(void)
{
	// Do the pg_state thing...
	pg_state::build_init_cast();
	
	/* Add the environment structure to the top-level CAST. */
	cast_type env = p_get_env_struct_type();
	int cdef = cast_add_def(&out_pres->cast,
				env->cast_type_u_u.agg_type.name,
				CAST_SC_NONE,
				CAST_TYPEDEF,
				pg_channel_maps[PG_CHANNEL_CLIENT_DECL]
				[builtin_file],
				CAST_PROT_NONE);
	out_pres->cast.cast_scope_val[cdef].u.cast_def_u_u.typedef_type = env;
}

/* End of file. */

