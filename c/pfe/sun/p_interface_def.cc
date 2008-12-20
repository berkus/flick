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

#include <mom/compiler.h>
#include <mom/c/pfe.hh>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include "pg_sun.hh"

/* Generate a typedef for the object reference (interface name). */
void pg_sun::p_interface_def_typedef(aoi_interface *ai)
{
	/*
	 * Sun RPC doesn't generate typedef's for interfaces.
	 * Rather, it outputs a #define:
	 * #define <interface_name> ((unsigned long) <interface_code>).
	 */
	int cdef = cast_add_def((cast_scope *)top_ptr(scope_stack),
				null_scope_name,
				CAST_SC_NONE,
				CAST_DEFINE,
				ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
				CAST_PROT_NONE);
	cast_expr value = cast_new_expr(CAST_EXPR_LIT_PRIM);
	
	/* Fill out the value. */
	value->cast_expr_u_u.lit_prim.u.kind = CAST_PRIM_INT;
	value->cast_expr_u_u.lit_prim.mod    = (CAST_MOD_UNSIGNED |
						CAST_MOD_LONG);
	if (ai->parents.parents_len <= 0)
		/*
		 * The interface `a' represents a program.
		 * The discriminator for a program is a single integer:
		 * the program number.
		 */
		value->cast_expr_u_u.lit_prim.u.cast_lit_prim_u_u.i =
			ai->code->aoi_const_u_u.const_int;
	else
		/*
		 * The interface `a' represents a version.
		 * The discriminator for a version is a structure of two
		 * integers: The first is the program number, and the second
		 * is the version number.
		 * So, we dig out the version number...
		 */
		value->cast_expr_u_u.lit_prim.u.cast_lit_prim_u_u.i =
			ai->code->
			aoi_const_u_u.const_struct.aoi_const_struct_val[1]->
			aoi_const_u_u.const_int;
	
	/* Fill out the CAST defintion. */
	if (gen_client)
		c(cdef).name = cast_new_scoped_name(
			calc_client_interface_object_type_name(
				a(cur_aoi_idx).name),
			NULL);
	else if (gen_server)
		c(cdef).name = cast_new_scoped_name(
			calc_server_interface_object_type_name(
				a(cur_aoi_idx).name),
			NULL);
	else
		panic("In `pg_sun::p_interface_def_typedef', "
		      "generating neither client nor server.");
	
	c(cdef).u.cast_def_u_u.define_as = value;
}

/* End of file. */

