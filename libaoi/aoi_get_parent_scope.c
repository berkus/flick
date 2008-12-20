/*
 * Copyright (c) 1998, 1999 The University of Utah and
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

#include <string.h>
#include <mom/compiler.h>
#include <mom/libaoi.h>
#include <assert.h>

aoi_ref aoi_get_parent_scope(aoi *the_aoi, aoi_ref ref)
{
	aoi_ref retval;
	int last_scope;

	last_scope = the_aoi->defs.defs_val[ref].scope;
	if( last_scope > 0 ) {
		retval = ref;
		while( (retval >= 0) &&
		       (the_aoi->defs.defs_val[retval].scope >= last_scope) )
			retval--;
		if( retval < 0 )
			panic("In aoi_get_parent_scope, can't find"
			      " the parent scope.");
	}
	else
		retval = aoi_ref_null;
	return( retval );
}
