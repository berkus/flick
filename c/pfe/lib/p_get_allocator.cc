/*
 * Copyright (c) 1997, 1998 The University of Utah and
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

pres_c_allocator pg_state::p_get_allocator(void)
{
	/*
	 * XXX --- Eventually we must do better.  We must have access to the
	 * target CAST type in order to make type-specific allocators.  We also
	 * need a `p_get_deallocator' method.
	 */
	pres_c_allocator res = { PRES_C_ALLOCATOR_NAME,
				 { calc_allocator_function_name("") } };
	return res;
}

/* End of file. */

