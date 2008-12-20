/*
 * Copyright (c) 1998 The University of Utah and
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

#include <string.h>
#include <mom/compiler.h>
#include <mom/c/libcast.h>

/* Check whether a cast expression is constant or not */
int cast_expr_const(cast_expr expr)
{
	int retval = 1;
	
	switch( expr->kind ) {
	case CAST_EXPR_NAME:
		retval = 0;
		break;
	case CAST_EXPR_LIT_PRIM:
		break;
	case CAST_EXPR_LIT_STRING:
		break;
	case CAST_EXPR_CALL:
		retval = 0;
		break;
	case CAST_EXPR_SEL:
		retval = 0;
		break;
	case CAST_EXPR_UNARY:
		switch( expr->cast_expr_u_u.unary.op ) {
		case CAST_UNARY_PRE_INC:
		case CAST_UNARY_POST_INC:
		case CAST_UNARY_PRE_DEC:
		case CAST_UNARY_POST_DEC:
			retval = 0;
			break;
		default:
			retval = cast_expr_const(expr->cast_expr_u_u.
						 unary.expr);
			break;
		}
		break;
	case CAST_EXPR_CAST:
		break;
	case CAST_EXPR_SIZEOF_EXPR:
		break;
	case CAST_EXPR_SIZEOF_TYPE:
		break;
	case CAST_EXPR_TYPEID_EXPR:
		break;
	case CAST_EXPR_TYPEID_TYPE:
		break;
	case CAST_EXPR_BINARY:
		retval = cast_expr_const(expr->cast_expr_u_u.binary.expr[0]) &&
			 cast_expr_const(expr->cast_expr_u_u.binary.expr[1]);
		break;
	case CAST_EXPR_OP_ASSIGN:
		retval = cast_expr_const(expr->cast_expr_u_u.
					 op_assign.expr[1]);
		break;
	case CAST_EXPR_COND:
		retval = 0;
		break;
	case CAST_EXPR_CONST_NAME:
		break;
	case CAST_EXPR_DYNAMIC_CAST:
	case CAST_EXPR_REINTERPRET_CAST:
	case CAST_EXPR_STATIC_CAST:
	case CAST_EXPR_CONST_CAST:
		break;
	case CAST_EXPR_OP_NEW:
		retval = 0;
		break;
	case CAST_EXPR_OP_DELETE:
		retval = 0;
		break;
	case CAST_EXPR_TYPE:
		retval = 0;
		break;
	}
	return retval;
}
