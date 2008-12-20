/*
 * Copyright (c) 1995, 1996, 1997 The University of Utah and
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

#include <mom/compiler.h>

#include "private.h"

mint_ref mint_add_integer_def(mint_1 *mint, int min, unsigned range)
{
	unsigned int i;

	/* See if there's already a matching int defined.  */
	for (i = 0; i < mint->defs.defs_len; i++) {
		if ((m(i).kind == MINT_INTEGER)
		    && (m(i).mint_def_u.integer_def.min == min)
		    && (m(i).mint_def_u.integer_def.range == range))
			return i;
	}
	
	/* If not, create one. */
	i = mint_add_def(mint);
	m(i).kind = MINT_INTEGER;
	m(i).mint_def_u.integer_def.min = min;
	m(i).mint_def_u.integer_def.range = range;
	
	return i;
}

/* End of file. */

