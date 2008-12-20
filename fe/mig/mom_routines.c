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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <rpc/types.h>
#include <mom/libmint.h>
#include <mom/compiler.h>

#include <mom/pres_c.h>

#include "mom_routines.h"

#define m(n) (out_pres_c.mint.defs.defs_val[n])

extern pres_c_1 out_pres_c;

/* Allocate a new (uninitialized) definition in the out_mint.  */
int get_def()
{
	return mint_add_def(&out_pres_c.mint);
}

/* Allocate a new union definition in the out_mint. */
mint_ref get_union_def(int len)
{
	return mint_add_union_def(&out_pres_c.mint,
				  out_pres_c.mint.standard_refs.signed32_ref,
				  len);
}

/* Add more cases to the union */
int expand_union_cases(mint_ref r)
{
	int i = m(r).mint_def_u.union_def.cases.cases_len++;
	m(r).mint_def_u.union_def.cases.cases_val =
		realloc(m(r).mint_def_u.union_def.cases.cases_val,
			sizeof(mint_union_case)*(i+1));
	return i;
}

int xl_int(int min, unsigned range)
{
	return mint_add_integer_def(&out_pres_c.mint, min, range);
}

/* Make a MINT array reference */
int xl_array(mint_ref type_d, int min_len, int max_len)
{
	return mint_add_array_def(&out_pres_c.mint, type_d, min_len, max_len);
}

/* End of file. */

