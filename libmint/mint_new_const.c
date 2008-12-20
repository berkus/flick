/*
 * Copyright (c) 1995, 1996, 1998, 1999 The University of Utah and
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

#include <mom/compiler.h>
#include <mom/libmint.h>

mint_const mint_new_const(mint_const_kind kind)
{
	mint_const mc = mustcalloc(sizeof(*mc));
	
	mc->kind = kind;
	return mc;
}

mint_const mint_new_symbolic_const(mint_const_kind kind, const char *name)
{
	mint_const res = mint_new_const(kind);
	char *nm = ir_strlit(name);
	
	switch (kind) {
	case MINT_CONST_INT:
		res->mint_const_u_u.const_int.kind = MINT_CONST_SYMBOLIC;
		res->mint_const_u_u.const_int.mint_const_int_u_u.name = nm;
		break;
		
	case MINT_CONST_CHAR:
		res->mint_const_u_u.const_char.kind = MINT_CONST_SYMBOLIC;
		res->mint_const_u_u.const_char.mint_const_char_u_u.name = nm;
		break;
		
	case MINT_CONST_FLOAT:
		res->mint_const_u_u.const_float.kind = MINT_CONST_SYMBOLIC;
		res->mint_const_u_u.const_float.mint_const_float_u_u.name = nm;
		break;
		
	default:
		assert(0);
		break;
	}
	return res;
}

/* End of file. */

