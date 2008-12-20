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

#include <assert.h>
#include <string.h>
#include <mom/compiler.h>

#include "private.h"

mint_ref mint_add_struct_slot(mint_1 *mint, mint_ref struct_itype)
{
	mint_struct_def *udef = &m(struct_itype).mint_def_u.struct_def;
	int i;

	assert(m(struct_itype).kind == MINT_STRUCT);
	i = udef->slots.slots_len++;
	udef->slots.slots_val = mustrealloc(udef->slots.slots_val,
		udef->slots.slots_len*sizeof(*udef->slots.slots_val));

	memset(&udef->slots.slots_val[i], 0, sizeof(udef->slots.slots_val[i]));

	return i;
}

