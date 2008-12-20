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

#include <mom/c/pfe.hh>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

/*
 * p_get_allocation() returns a pres_c_allocation structure describing this
 * presentation's default allocation semantics for various parameter roles.
 *
 * XXX - It is awful that we need to declare a structure describing every
 * case of possible allocation.  The presentation generator should be
 * responsible for specifying the _exact_ allocation semantic needed, given
 * the parameter role in question.
 */
pres_c_allocation pg_state::p_get_allocation(void)
{
	pres_c_allocation alloc;
	
	/* Set up the default allocation flags and allocators */
	if (async_stubs) {
		pres_c_allocation alloc;
		
		/* Set up the default allocation flags and allocators */
		pres_c_allocation_u dfault;
		dfault.allow = PRES_C_ALLOCATION_ALLOW;
		/*
		 * We do want deallocation, but it is handled by the runtime;
		 * what we express here is what the *generated code* is
		 * responsible for.
		 */
		dfault.pres_c_allocation_u_u.val.flags
			= PRES_C_ALLOC_ALWAYS | PRES_C_DEALLOC_NEVER;
		dfault.pres_c_allocation_u_u.val.allocator.kind
			= PRES_C_ALLOCATOR_DONTCARE; // ? p_get_allocator();
		dfault.pres_c_allocation_u_u.val.alloc_init = 0;
		
		/*
		 * XXX- The unknown direction SHOULD never occur in practice.
		 * However, currently exceptions have no associated direction,
		 * and thus this case shows up when mapping an exception
		 * (particularly user exceptions that may contain arbitrary
		 * amounts and kinds of data).
		 */
		alloc.cases[PRES_C_DIRECTION_UNKNOWN] = dfault;
		
		/*
		 * All directions can use the semantics above.
		 */
		alloc.cases[PRES_C_DIRECTION_IN] = dfault;
		alloc.cases[PRES_C_DIRECTION_INOUT] = dfault;
		alloc.cases[PRES_C_DIRECTION_OUT] = dfault;
		alloc.cases[PRES_C_DIRECTION_RETURN] = dfault;
		
		return alloc;
	} else if (gen_client) {
		pres_c_allocation_u dfault;
		dfault.allow = PRES_C_ALLOCATION_ALLOW;
		dfault.pres_c_allocation_u_u.val.flags
			= PRES_C_ALLOC_ALWAYS | PRES_C_DEALLOC_NEVER |
			PRES_C_DEALLOC_ON_FAIL;
		dfault.pres_c_allocation_u_u.val.allocator
			= p_get_allocator();
		dfault.pres_c_allocation_u_u.val.alloc_init = 0;
		
		/*
		 * XXX - The unknown direction SHOULD never occur in practice.
		 * However, currently exceptions have no associated direction,
		 * and thus this case shows up when mapping an exception
		 * (particularly user exceptions that may contain arbitrary
		 * amounts and kinds of data).
		 */
		alloc.cases[PRES_C_DIRECTION_UNKNOWN] = dfault;

		/* By default, never allocate `in' parameters. */
		alloc.cases[PRES_C_DIRECTION_IN].allow
			= PRES_C_ALLOCATION_ALLOW;
		alloc.cases[PRES_C_DIRECTION_IN].pres_c_allocation_u_u.
			val.flags = PRES_C_ALLOC_NEVER | PRES_C_DEALLOC_NEVER;
		alloc.cases[PRES_C_DIRECTION_IN].pres_c_allocation_u_u.
			val.allocator = p_get_allocator();
		alloc.cases[PRES_C_DIRECTION_IN].pres_c_allocation_u_u.
			val.alloc_init = 0;
		
		/*
		 * XXX --- These pointer flags are *wrong* for `inout'
		 * sequences.  Currently, the client is expected to
		 * free the `in' version and allocate the `out' version
		 * --- it won't reuse space.
		 */
		alloc.cases[PRES_C_DIRECTION_INOUT].allow
			= PRES_C_ALLOCATION_ALLOW;
		alloc.cases[PRES_C_DIRECTION_INOUT].pres_c_allocation_u_u.
			val.flags
			= PRES_C_ALLOC_ALWAYS | PRES_C_DEALLOC_NEVER |
			PRES_C_DEALLOC_ON_FAIL;
		alloc.cases[PRES_C_DIRECTION_INOUT].pres_c_allocation_u_u.
			val.allocator = p_get_allocator();
		alloc.cases[PRES_C_DIRECTION_INOUT].pres_c_allocation_u_u.
			val.alloc_init = 0;
		
		alloc.cases[PRES_C_DIRECTION_OUT] = dfault;
		alloc.cases[PRES_C_DIRECTION_RETURN] = dfault;

	} else if (gen_server) {
		/* Specify the default allocation semantics. */
		pres_c_allocation_u dfault;
		dfault.allow = PRES_C_ALLOCATION_ALLOW;
		dfault.pres_c_allocation_u_u.val.flags
			= PRES_C_ALLOC_ALWAYS | PRES_C_DEALLOC_ALWAYS |
			PRES_C_DEALLOC_ON_FAIL;
		dfault.pres_c_allocation_u_u.val.allocator
			= p_get_allocator();
		dfault.pres_c_allocation_u_u.val.alloc_init = 0;
		
		/*
		 * XXX - The unknown direction SHOULD never occur in practice.
		 * However, currently exceptions have no associated direction,
		 * and thus this case shows up when mapping an exception
		 * (particularly user exceptions that may contain arbitrary
		 * amounts and kinds of data).
		 */
		alloc.cases[PRES_C_DIRECTION_UNKNOWN] = dfault;
		
		/* Default allocation, but with a "don't care" allocator. */
		alloc.cases[PRES_C_DIRECTION_IN] = dfault;
		alloc.cases[PRES_C_DIRECTION_IN].pres_c_allocation_u_u.val.
			allocator.kind = PRES_C_ALLOCATOR_DONTCARE;
		
		alloc.cases[PRES_C_DIRECTION_INOUT] = dfault;
		alloc.cases[PRES_C_DIRECTION_OUT] = dfault;
		alloc.cases[PRES_C_DIRECTION_RETURN] = dfault;
	} else {
		panic("In pg_state::p_get_allocation: "
		      "Generating neither client nor server!");
	}
	return alloc;
}

/* End of file. */

