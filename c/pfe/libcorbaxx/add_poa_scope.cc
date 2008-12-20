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
#include <assert.h>
#include <mom/c/pg_corbaxx.hh>
#include <mom/c/libcast.h>
#include "../macros.h"

/* Used to make a properly named addition to the POA scope.
   Any stuff in the POA needs to have POA_ prefixing the entire
   name, not just the tail.  Therefore we must add POA_ to
   any namespaces that come before this type, or we just add
   POA_ directly to the type name */
char *pg_corbaxx::add_poa_scope(char *the_name)
{
	char *scope_name;

	if( cast_scoped_name_is_empty(&current_poa_scope_name) )
		scope_name = flick_asprintf("POA_%s", the_name);
	else
		scope_name = the_name;
	return( scope_name );
}

/* End of file. */

