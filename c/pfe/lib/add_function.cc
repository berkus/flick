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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Flick; see the file COPYING.  If not, write to
 * the Free Software Foundation, 59 Temple Place #330, Boston, MA 02111, USA.
 */

#include <mom/c/libpres_c.h>
#include <mom/c/pfe.hh>

/* A simple helper function to call pres_function */
int pg_state::add_function(tag_list *tl,
			   cast_scoped_name fname,
			   int tag, ...)
{
	va_list arg_addr;
	int retval;
	
	va_start( arg_addr, tag );
	retval = vpres_function(out_pres, tl, current_scope_name,
				fname, tag, arg_addr);
	va_end( arg_addr );
	return( retval );
}
