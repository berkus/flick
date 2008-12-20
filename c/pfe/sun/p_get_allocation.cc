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

#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include "pg_sun.hh"

/*
 * p_get_allocation() returns a pres_c_allocation structure describing this
 * presentation's default allocation semantics for various parameter roles.
 *
 * XXX - See comment in c/pfe/lib/p_get_allocation.cc.
 */
pres_c_allocation pg_sun::p_get_allocation(void)
{
	pres_c_allocation alloc;
	
	/* Set up the default allocation flags and allocators */
	
	/*
	 * The unknown direction should never occur in practice.
	 */
	alloc.cases[PRES_C_DIRECTION_UNKNOWN].allow
		= PRES_C_ALLOCATION_INVALID;
	
	if (gen_client) {
		/* `In' parameters are never allocated nor deallocated. */
		alloc.cases[PRES_C_DIRECTION_IN].allow
			= PRES_C_ALLOCATION_ALLOW;
		alloc.cases[PRES_C_DIRECTION_IN].pres_c_allocation_u_u.val.
			flags = PRES_C_ALLOC_NEVER | PRES_C_DEALLOC_NEVER;
		alloc.cases[PRES_C_DIRECTION_IN].pres_c_allocation_u_u.val.
			allocator.kind = PRES_C_ALLOCATOR_DONTCARE;
		alloc.cases[PRES_C_DIRECTION_IN].pres_c_allocation_u_u.
			val.alloc_init = 0;

		/*
		 * Base `return' values are always allocated statically;
		 * however, deep allocation may be required using the
		 * presentation's allocator.
		 */
		alloc.cases[PRES_C_DIRECTION_RETURN].allow
			= PRES_C_ALLOCATION_ALLOW;
		alloc.cases[PRES_C_DIRECTION_RETURN].pres_c_allocation_u_u.val.
			flags = PRES_C_ALLOC_ALWAYS | PRES_C_DEALLOC_NEVER |
			PRES_C_DEALLOC_ON_FAIL;
		alloc.cases[PRES_C_DIRECTION_RETURN].pres_c_allocation_u_u.val.
			allocator = p_get_allocator();
		alloc.cases[PRES_C_DIRECTION_RETURN].pres_c_allocation_u_u.val.
			alloc_init = 0;
		
		/*
		 * The in `inout' and `out' cases are never used in the ONC RPC
		 * presentation currently, since the original IDL specification
		 * doesn't allow for them.  However, the back end may be asked
		 * to produce all m/u stubs, which blindly produces them for
		 * all directions, whether they make sense or not.
		 */
		alloc.cases[PRES_C_DIRECTION_INOUT]
			= alloc.cases[PRES_C_DIRECTION_OUT]
			= alloc.cases[PRES_C_DIRECTION_RETURN];
		
	} else if (gen_server) {
		/*
		 * `In' parameters are always allocated and deallocated.
		 * The particular allocator doesn't matter because the work
		 * functions won't realloc or free `in's.
		 */
		alloc.cases[PRES_C_DIRECTION_IN].allow
			= PRES_C_ALLOCATION_ALLOW;
		alloc.cases[PRES_C_DIRECTION_IN].pres_c_allocation_u_u.val.
			flags = PRES_C_ALLOC_ALWAYS | PRES_C_DEALLOC_ALWAYS
			| PRES_C_DEALLOC_ON_FAIL;
		alloc.cases[PRES_C_DIRECTION_IN].pres_c_allocation_u_u.val.
			allocator.kind = PRES_C_ALLOCATOR_DONTCARE;
		alloc.cases[PRES_C_DIRECTION_IN].pres_c_allocation_u_u.val.
			alloc_init = 0;
		
		/*
		 * Base `return' parameters are statically allocated by the
		 * work function; ``deep'' allocation/deallocation is done
		 * entirely by the work function.
		 * (Yes: A Sun server skeleton *never* deallocates pointers in
		 * a reply!  That's how `rpcgen'-generated code works.)
		 */
		
		alloc.cases[PRES_C_DIRECTION_RETURN].allow
			= PRES_C_ALLOCATION_ALLOW;
		alloc.cases[PRES_C_DIRECTION_RETURN].pres_c_allocation_u_u.val.
			flags = PRES_C_ALLOC_NEVER | PRES_C_DEALLOC_NEVER;
		alloc.cases[PRES_C_DIRECTION_RETURN].pres_c_allocation_u_u.val.
			allocator.kind = PRES_C_ALLOCATOR_DONTCARE;
		alloc.cases[PRES_C_DIRECTION_RETURN].pres_c_allocation_u_u.val.
			alloc_init = 0;
		
		/*
		 * The in `inout' and `out' cases are never used in the ONC RPC
		 * presentation currently, since the original IDL specification
		 * doesn't allow for them.  However, the back end may be asked
		 * to produce all m/u stubs, which blindly produces them for
		 * all directions, whether they make sense or not.
		 */
		alloc.cases[PRES_C_DIRECTION_INOUT]
			= alloc.cases[PRES_C_DIRECTION_OUT]
			= alloc.cases[PRES_C_DIRECTION_IN];
		
	} else {
		panic("In pg_sun::p_get_allocation: "
		      "Generating neither client nor server!");
	}
	
	return alloc;
}

/* End of file. */

