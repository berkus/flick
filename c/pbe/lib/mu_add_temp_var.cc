/*
 * Copyright (c) 1995, 1996 The University of Utah and
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
#include <mom/c/libcast.h>
#include <mom/c/pbe.hh>

/* Create a new temporary variable in the current m/u code block.
   The variable will be available (in scope)
   throughout the current code block, but not outside of it.

   `tag' is an arbitrary C identifier fragment
   that will be embedded in the temporary variable name:
   its purpose is purely to make the generated code
   a little more understandable by humans
   by giving some indication of what the variable is to be used for.
   These tags don't have to be unique in any respect:
   uniqueness of variable names is assured through other means.

   `type' is the C type to give the temporary variable.

   `sc' is the storage class for the variable -
   e.g. CAST_SC_NONE for a normal local variable,
   or CAST_SC_STATIC for a static variable.
*/
cast_expr mu_state::add_temp_var(const char *tag, cast_type type,
				 cast_init init, cast_storage_class sc)
{
	static int temp_counter;

	char *name = flick_asprintf("_t%d_%s", temp_counter++, tag);

	add_var(name, type, init, sc);

	return cast_new_expr_name(name);
}

