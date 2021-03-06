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

#include <mom/c/libcast.h>

cast_expr cast_new_expr_lit_char(char val, cast_primitive_modifier mod)
{
	cast_expr expr = cast_new_expr(CAST_EXPR_LIT_PRIM);
	expr->cast_expr_u_u.lit_prim.u.kind = CAST_PRIM_CHAR;
	expr->cast_expr_u_u.lit_prim.u.cast_lit_prim_u_u.c = val;
	expr->cast_expr_u_u.lit_prim.mod = mod;
	return expr;
}

