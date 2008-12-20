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

#include <mom/c/pbe.hh>

/* Base implementation of mu_array_elem:
   just calls the generic marshaling code generator for the array element type.
   Backend code will generally override this method
   and then recursively invoke it in the appropriate place(s).  */
void mu_state::mu_array_elem(cast_expr elem_expr, cast_type elem_ctype,
			     mint_ref elem_itype, pres_c_mapping elem_map,
			     unsigned long /*len_min*/,
			     unsigned long /*len_max*/)
{
	mu_mapping(elem_expr, elem_ctype, elem_itype, elem_map);
}

