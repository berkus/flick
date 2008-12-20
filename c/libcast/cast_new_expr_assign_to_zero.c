/*
 * Copyright (c) 1997, 1998, 1999 The University of Utah and
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

/*
 * Two auxiliary functions: one to manufacture a call to `memset', and one to
 * determine if a CAST expression is a call to `memset'.
 */
static cast_expr make_memset(cast_expr expr, cast_type ctype)
{
	return cast_new_expr_call_3(cast_new_expr_name("memset"),
				    ((ctype->kind == CAST_TYPE_ARRAY) ?
				     expr :
				     cast_new_unary_expr(CAST_UNARY_ADDR,
							 expr)),
				    cast_new_expr_lit_int(0, 0),
				    cast_new_expr_sizeof_type(ctype));
}

static int is_memset(cast_expr expr)
{
	return ((expr->kind == CAST_EXPR_CALL)
		&& (expr->cast_expr_u_u.call.func->kind == CAST_EXPR_NAME)
		&& (!strcmp(expr->cast_expr_u_u.call.func->cast_expr_u_u.name.
			    cast_scoped_name_val[0].name,
			    "memset"))
		);
}

/*
 * Create and return a CAST expression that sets `expr' to zero.  The resultant
 * expression may be an assignment, a procedure call (to `memset'), or a comma
 * expression that combines multiple assignments and/or `memset' calls.
 */
cast_expr cast_new_expr_assign_to_zero(cast_expr expr,
				       cast_type ctype,
				       cast_scope *scope)
{
	cast_expr assign_expr = 0;
	cast_expr zero_expr;
	unsigned int i;
			
	/*****/
	
	switch (ctype->kind) {
	case CAST_TYPE_PRIMITIVE:
		/*
		 * Create an assignment with the right kind of zero.
		 */
		switch (ctype->cast_type_u_u.primitive_type.kind) {
		case CAST_PRIM_CHAR:
			zero_expr = cast_new_expr_lit_char(0, 0);
			break;
		case CAST_PRIM_INT:
			zero_expr = cast_new_expr_lit_int(0, 0);
			break;
		case CAST_PRIM_FLOAT:
			zero_expr = cast_new_expr_lit_float(0.0);
			break;
		case CAST_PRIM_DOUBLE:
			zero_expr = cast_new_expr_lit_double(0.0, 0);
			break;
		default:
			panic("In `cast_new_expr_assign_to_zero', "
			      "unrecognized CAST primitive type.");
			break;
		}
		
		assign_expr = cast_new_expr_assign(expr, zero_expr);
		break;
		
	case CAST_TYPE_POINTER:
	case CAST_TYPE_FUNCTION: /* XXX? */
		assign_expr
			= cast_new_expr_assign(expr,
					       cast_new_expr_lit_int(0, 0));
		break;
		
	case CAST_TYPE_ENUM:
	case CAST_TYPE_ENUM_NAME: {
		/*
		 * Find the zero-valued member of the enumeration, or failing
		 * that, any member of the enumeration.
		 */
		cast_enum_type *enum_type = cast_find_enum_type(scope, ctype);
		
		if (!enum_type)
			/* We couldn't find the enumeration definition. */
			zero_expr = cast_new_expr_lit_int(0, 0);
		else {
			/* Find the zero-valued member of the enumeration. */
			for (i = 0; i < enum_type->slots.slots_len; ++i)
				if ((enum_type->slots.slots_val[i].val->kind
				     == CAST_EXPR_LIT_PRIM)
				    &&
				    (enum_type->slots.slots_val[i].val->
				     cast_expr_u_u.lit_prim.u.kind
				     == CAST_PRIM_INT)
				    &&
				    (enum_type->slots.slots_val[i].val->
				     cast_expr_u_u.lit_prim.u.
				     cast_lit_prim_u_u.i
				     == 0))
					break;
			
			if (i >= enum_type->slots.slots_len) {
				/*
				 * We didn't find a zero value, so we'll just
				 * use the first member of the enumeration.
				 */
				if (enum_type->slots.slots_len > 0)
					i = 0;
				else
					panic("Empty enumeration!");
			}
			
			zero_expr = cast_new_expr_name(enum_type->slots.
						       slots_val[i].name);
		}
		
		assign_expr = cast_new_expr_assign(expr, zero_expr);
		break;
	}
	
	case CAST_TYPE_AGGREGATE:
	case CAST_TYPE_STRUCT_NAME: {
		/*
		 * Make a comma expression to zero the structure members.
		 */
		cast_aggregate_type *agg_type =
			cast_find_struct_type(scope,
					      ctype);
		int will_memset;
		
		if (!agg_type || (agg_type->kind == CAST_AGGREGATE_UNION)) {
			/* We couldn't find the structure definition. */
			assign_expr = make_memset(expr, ctype);
			break;
		}
		/* Iterate through the structure slots. */
		will_memset = 0;
		for (i = 0;
		     i < agg_type->scope.cast_scope_len;
		     ++i) {
			switch(agg_type->scope.
			       cast_scope_val[i].u.kind) {
			case CAST_VAR_DEF:
			case CAST_VAR_DECL:
				zero_expr = cast_new_expr_assign_to_zero(
					cast_new_expr_sel(
						expr,
						agg_type->scope.
						cast_scope_val[i].
						name),
					(agg_type->scope.
					 cast_scope_val[i].u.kind ==
					 CAST_VAR_DEF) ?
					(agg_type->scope.cast_scope_val[i].
					 u.cast_def_u_u.var_def.type) :
					(agg_type->scope.
					 cast_scope_val[i].u.cast_def_u_u.
					 var_type),
					scope);
				
				if (is_memset(zero_expr)) {
					/*
					 * If we have to `memset' any member,
					 * we might as well `memset' the entire
					 * structure.
					 */
					assign_expr = make_memset(expr,
								  ctype);
					will_memset = 1;
					break;
				}
				
				if (!assign_expr)
					assign_expr = zero_expr;
				else
					assign_expr =
						cast_new_binary_expr(
							CAST_BINARY_COMMA,
							assign_expr,
							zero_expr);
				break;
			default:
				break;
			}
			if (will_memset)
				/* We're done iterating over the slots. */
				break;
		}
		if( !assign_expr ) {
			assign_expr = make_memset(expr, ctype);
		}
		break;
	}
	
	case CAST_TYPE_CLASS_NAME:
	case CAST_TYPE_UNION_NAME:
	case CAST_TYPE_ARRAY:
		/*
		 * Use `memset' to zero the expression.
		 */
		assign_expr = make_memset(expr, ctype);
		break;
		
	case CAST_TYPE_NAME: {
		/*
		 * Dereference the named type and call this function again.
		 */
		cast_type real_ctype = cast_find_typedef_type(scope, ctype);
		
		if (!real_ctype)
			/* We couldn't find the type's definition.  Punt. */
			assign_expr = make_memset(expr, ctype);
		else {
			assign_expr = cast_new_expr_assign_to_zero(expr,
								   real_ctype,
								   scope);
			if (is_memset(assign_expr))
				/*
				 * After finding the real C type, we still made
				 * a call to `memset'.  For pretty code, we
				 * should use the given `ctype' in the call,
				 * not the `real_ctype'.
				 */
				assign_expr = make_memset(expr, ctype);
		}
		break;
	}
	
	case CAST_TYPE_QUALIFIED:
		/*
		 * Dig through the qualifiers and call this function again.
		 */
		while (ctype->kind == CAST_TYPE_QUALIFIED)
			ctype = ctype->cast_type_u_u.qualified.actual;
		
		assign_expr = cast_new_expr_assign_to_zero(expr, ctype, scope);
		break;
		
	case CAST_TYPE_VOID:
		panic("In `cast_new_expr_assign_to_zero', "
		      "cannot assign a void-typed expression to zero.");
		break;
		
	default:
		panic("In `cast_new_expr_assign_to_zero', "
		      "unrecognized CAST type %d.", ctype->kind);
		break;
	}
	
	return assign_expr;
}

/* End of file. */

