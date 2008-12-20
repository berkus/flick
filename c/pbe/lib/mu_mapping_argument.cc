/*
 * Copyright (c) 1997, 1998, 1999 The University of Utah and
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

#include <mom/compiler.h>
#include <mom/libmint.h>
#include <mom/c/libcast.h>
#include <mom/c/pbe.hh>

/*
 * This handles PRES_C_MAPPING_ARGUMENT nodes, which tells the back end to save
 * the current CAST expression and type under a given key (`mamap->arg_name')
 * in a virtual argument list (`arglist').  Presumably, some parent of this
 * node will later retrieve the CAST expression and type in order to produce
 * code.
 *
 * In sum, a PRES_C_MAPPING_ARGUMENT mode is a way for a child to send some
 * information back up to its parent, so that the parent (or a sibling) can
 * deal with it in some way that the child cannot.
 */

void mu_state::mu_mapping_argument(cast_expr expr, cast_type ctype,
				   mint_ref itype, pres_c_mapping map)
{
	pres_c_mapping_argument *mamap = &map->pres_c_mapping_u_u.argument;
	
	assert(map->kind == PRES_C_MAPPING_ARGUMENT);
	assert(mamap->arg_name);
	
	/* No arglist?  Or nowhere to store the argument?  Then panic! */
	if (!arglist
	    || !(arglist->setargs(mamap->arglist_name, mamap->arg_name,
				  expr, ctype)))
		panic(("In `mu_state::mu_mapping_argument', "
		       "there is nowhere to save a \"%s\" argument."),
		      mamap->arg_name);
	
	if (mamap->map)
		mu_mapping(expr, ctype, itype, mamap->map);
}

/* End of file. */

