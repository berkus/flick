/*
 * Copyright (c) 1997 The University of Utah and
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

cast_expr cast_new_expr_op_assign(cast_binary_op op, cast_expr a, cast_expr b)
{
	cast_expr expr = cast_new_expr(CAST_EXPR_OP_ASSIGN);
	expr->cast_expr_u_u.op_assign.op = op;
	expr->cast_expr_u_u.op_assign.expr[0] = a;
	expr->cast_expr_u_u.op_assign.expr[1] = b;
	return expr;
}

