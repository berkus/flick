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

#include <mom/c/pg_corbaxx.hh>
#include <mom/c/libpres_c.h>

/*
 * p_get_allocation() returns a pres_c_allocation structure describing this
 * presentation's default allocation semantics for various parameter roles.
 *
 * XXX - see comment in c/pfe/lib/p_get_allocation.cc
 */
pres_c_allocation pg_corbaxx::p_get_allocation(void)
{
	pres_c_allocation alloc;
	int lpc;
	
	alloc = pg_state::p_get_allocation();
	for( lpc = 0; lpc < PRES_C_DIRECTIONS; lpc++ ) {
		switch( alloc.cases[lpc].allow ) {
		case PRES_C_ALLOCATION_ALLOW:
			if (alloc.cases[lpc].pres_c_allocation_u_u.val.flags
			    | PRES_C_ALLOC_EVER)
				alloc.cases[lpc].pres_c_allocation_u_u.val.
					flags |= PRES_C_RUN_CTOR;
			if (alloc.cases[lpc].pres_c_allocation_u_u.val.flags
			    | PRES_C_DEALLOC_EVER)
				alloc.cases[lpc].pres_c_allocation_u_u.val.
					flags |= PRES_C_RUN_DTOR;
			break;
		default:
			break;
		}
	}
	return alloc;
}

/* End of file. */

