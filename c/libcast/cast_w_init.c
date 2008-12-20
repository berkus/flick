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

#include <mom/compiler.h>
#include <mom/c/libcast.h>

void cast_w_init(cast_init init, int indent)
{
	int i;

	switch (init->kind)
	{
		case CAST_INIT_EXPR:
			w_printf(" = ");
			cast_w_expr_noncomma(init->cast_init_u_u.expr, 0);
			break;
		case CAST_INIT_AGGREGATE:
			w_printf(" = {");
			for (i = 0;
			     i < (signed int)init->cast_init_u_u.subs.
				 cast_init_array_len;
			     i++)
			{
				if (i > 0)
					w_putc(',');
				cast_w_init(init->cast_init_u_u.subs.
					    cast_init_array_val[i], indent);
			}
			w_putc('}');
			break;
		case CAST_INIT_CONSTRUCT:
			w_putc('(');
			for (i = 0;
			     i < (signed int)init->cast_init_u_u.exprs.
				 cast_expr_array_len;
			     i++) {
				if (i > 0)
					w_putc(',');
				cast_w_expr_noncomma(init->cast_init_u_u.exprs.
						     cast_expr_array_val[i],
						     0);
			}
			w_putc(')');
			break;
		default:
			panic("cast_w_init: unknown kind %d\n", init->kind);
	}
}

