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
 * Compare two CAST initializations, and return their ``difference''
 * (return value of 0 means they are equivalent).
 */
int cast_cmp_init(cast_init a, cast_init b)
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
	case CAST_INIT_EXPR:
		return cast_cmp_expr(a->cast_init_u_u.expr,
				     b->cast_init_u_u.expr);
	case CAST_INIT_AGGREGATE:
		if ((diff = a->cast_init_u_u.subs.cast_init_array_len
		     - b->cast_init_u_u.subs.cast_init_array_len))
			return diff;
		for (i = 0;
		     i < a->cast_init_u_u.subs.cast_init_array_len;
		     i++) {
			if ((diff = cast_cmp_init(
				a->cast_init_u_u.subs.cast_init_array_val[i],
				b->cast_init_u_u.subs.cast_init_array_val[i])))
				return diff;
		}
		return 0;
	case CAST_INIT_CONSTRUCT:
		if ((diff = a->cast_init_u_u.exprs.cast_expr_array_len
		     - b->cast_init_u_u.exprs.cast_expr_array_len))
			return diff;
		for (i = 0;
		     i < a->cast_init_u_u.exprs.cast_expr_array_len;
		     i++) {
			if ((diff = cast_cmp_expr(
				a->cast_init_u_u.exprs.cast_expr_array_val[i],
				(b->cast_init_u_u.exprs.
				 cast_expr_array_val[i]))))
				return diff;
		}
		return 0;
		
	default:
		panic("cast_cmp_init: unknown cast_init kind %d", a->kind);
	}
	panic("cast_cmp_init: should have returned in switch statement\n");
	return 0;
}

