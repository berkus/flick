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

#include <mom/libaoi.h>

#include "private.hh"

void pg_state::p_array_type(aoi_array *aa,
			    p_type_collection **out_ptc)
{
	/* Find out whether this is a fixed or variable-length array,
	   and call the appropriate routine.  */
	unsigned min, max;
	aoi_get_array_len(in_aoi, aa, &min, &max);
	if (min == max)
		p_fixed_array_type(aa, out_ptc);
	else
		p_variable_array_type(aa, out_ptc);
}

