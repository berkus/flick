/*
 * Copyright (c) 1999 The University of Utah and
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
#include <mom/c/libcast.h>

/*
 * Compare two CAST expressions, and return their ``difference''
 * (return value of 0 means they are equivalent).
 */
int cast_cmp_expr(cast_expr a, cast_expr b)
{
	int diff;
	unsigned i;
	
	/* Easy things first */
	if (a == b)
		return 0;
	assert(a);
	assert(b);
	if (a->kind != b->kind)
		return (int)a->kind - (int)b->kind;
	switch (a->kind)
	{
	case CAST_EXPR_NAME:
	case CAST_EXPR_CONST_NAME:
		return cast_cmp_scoped_names(&a->cast_expr_u_u.name,
					     &b->cast_expr_u_u.name);
	case CAST_EXPR_LIT_PRIM:
		if ((diff = (int)a->cast_expr_u_u.lit_prim.u.kind
		     - (int)b->cast_expr_u_u.lit_prim.u.kind))
			return diff;
		if ((diff = (int)a->cast_expr_u_u.lit_prim.mod
		     - (int)b->cast_expr_u_u.lit_prim.mod))
			return diff;
		switch(a->cast_expr_u_u.lit_prim.u.kind) {
		case CAST_PRIM_CHAR:
			return (int)a->cast_expr_u_u.lit_prim.u.
				cast_lit_prim_u_u.c
				- (int)b->cast_expr_u_u.lit_prim.u.
				cast_lit_prim_u_u.c;
		case CAST_PRIM_INT:
			return (int)a->cast_expr_u_u.lit_prim.u.
				cast_lit_prim_u_u.i
				- (int)b->cast_expr_u_u.lit_prim.u.
				cast_lit_prim_u_u.i;
		case CAST_PRIM_FLOAT:
			return (int)a->cast_expr_u_u.lit_prim.u.
				cast_lit_prim_u_u.f
				- (int)b->cast_expr_u_u.lit_prim.u.
				cast_lit_prim_u_u.f;
		case CAST_PRIM_DOUBLE:
			return (int)a->cast_expr_u_u.lit_prim.u.
				cast_lit_prim_u_u.d
				- (int)b->cast_expr_u_u.lit_prim.u.
				cast_lit_prim_u_u.d;
		case CAST_PRIM_BOOL:
			return (int)a->cast_expr_u_u.lit_prim.u.
				cast_lit_prim_u_u.b
				- (int)b->cast_expr_u_u.lit_prim.u.
				cast_lit_prim_u_u.b;
		default:
			panic("In cast_cmp_expr(), invalid primitive kind!");
		}
	case CAST_EXPR_LIT_STRING:
		return strcmp(a->cast_expr_u_u.lit_string,
			      b->cast_expr_u_u.lit_string);
	case CAST_EXPR_CALL:
		if ((diff = cast_cmp_expr(a->cast_expr_u_u.call.func,
					  b->cast_expr_u_u.call.func)))
			return diff;
		if ((diff = a->cast_expr_u_u.call.params.cast_expr_array_len
		     - b->cast_expr_u_u.call.params.cast_expr_array_len))
			return diff;
		for (i = 0;
		     i < a->cast_expr_u_u.call.params.cast_expr_array_len;
		     i++) {
			if ((diff = cast_cmp_expr(
				(a->cast_expr_u_u.call.params.
				 cast_expr_array_val[i]),
				(b->cast_expr_u_u.call.params.
				 cast_expr_array_val[i]))))
				return diff;
		}
		return 0;
	case CAST_EXPR_UNARY:
		return ((a->cast_expr_u_u.unary.op
			 - b->cast_expr_u_u.unary.op)
			|| cast_cmp_expr(a->cast_expr_u_u.unary.expr,
					 b->cast_expr_u_u.unary.expr));
	case CAST_EXPR_CAST:
	case CAST_EXPR_CONST_CAST:
	case CAST_EXPR_DYNAMIC_CAST:
	case CAST_EXPR_REINTERPRET_CAST:
	case CAST_EXPR_STATIC_CAST:
		return (cast_cmp_type(a->cast_expr_u_u.cast.type,
				      b->cast_expr_u_u.cast.type)
			|| cast_cmp_expr(a->cast_expr_u_u.cast.expr,
					 b->cast_expr_u_u.cast.expr));
	case CAST_EXPR_SIZEOF_EXPR:
	case CAST_EXPR_TYPEID_EXPR:
		return cast_cmp_expr(a->cast_expr_u_u.sizeof_expr,
				     b->cast_expr_u_u.sizeof_expr);
	case CAST_EXPR_SIZEOF_TYPE:
	case CAST_EXPR_TYPEID_TYPE:
	case CAST_EXPR_TYPE:
		return cast_cmp_type(a->cast_expr_u_u.sizeof_type,
				     b->cast_expr_u_u.sizeof_type);
	case CAST_EXPR_BINARY:
	case CAST_EXPR_OP_ASSIGN:
		return ((a->cast_expr_u_u.binary.op
			 - b->cast_expr_u_u.binary.op)
			|| cast_cmp_expr(a->cast_expr_u_u.binary.expr[0],
					 b->cast_expr_u_u.binary.expr[0])
			|| cast_cmp_expr(a->cast_expr_u_u.binary.expr[0],
					 b->cast_expr_u_u.binary.expr[0]));
	case CAST_EXPR_COND:
		return (cast_cmp_expr(a->cast_expr_u_u.cond.test,
				      b->cast_expr_u_u.cond.test)
			|| cast_cmp_expr(a->cast_expr_u_u.cond.true_expr,
					 b->cast_expr_u_u.cond.true_expr)
			|| cast_cmp_expr(a->cast_expr_u_u.cond.false_expr,
					 b->cast_expr_u_u.cond.false_expr));
	case CAST_EXPR_OP_NEW:
		return (cast_cmp_expr(a->cast_expr_u_u.op_new.placement,
				      b->cast_expr_u_u.op_new.placement)
			|| cast_cmp_type(a->cast_expr_u_u.op_new.type,
					 b->cast_expr_u_u.op_new.type)
			|| cast_cmp_init(a->cast_expr_u_u.op_new.init,
					 b->cast_expr_u_u.op_new.init));
	case CAST_EXPR_OP_DELETE:
		return (cast_cmp_expr(a->cast_expr_u_u.op_delete.expr,
				      b->cast_expr_u_u.op_delete.expr)
			|| (a->cast_expr_u_u.op_delete.array
			    - b->cast_expr_u_u.op_delete.array));
		
	default:
		panic("cast_cmp_expr: unknown cast_expr kind %d", a->kind);
	}
	panic("cast_cmp_expr: should have returned in switch statement\n");
	return 0;
}

