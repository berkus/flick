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

#include <mom/compiler.h>

#include "private.h"

mint_ref mint_add_union_def(mint_1 *mint, mint_ref discrim_type, int len)
{
	mint_ref r = mint_add_def(mint);
	m(r).kind = MINT_UNION;
	m(r).mint_def_u.union_def.discrim = discrim_type;
	m(r).mint_def_u.union_def.cases.cases_len = len;
	m(r).mint_def_u.union_def.cases.cases_val = mustmalloc(len ? len*sizeof(mint_union_case) : 1);
	m(r).mint_def_u.union_def.dfault = mint_ref_null;
	return r;
}

