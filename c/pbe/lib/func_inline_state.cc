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
#include <mom/c/libcast.h>
#include <mom/c/pbe.hh>

/*
 * This is the primary accessor method for a func inline_state.  Given a slot
 * index number (a pres_c_inline_index) into the "current" C function parameter
 * list, returns in `out_expr' a C expression that can be used in generated
 * code to access that parameter at runtime.  Also returns, in `out_type', the
 * C type of that parameter.
 *
 * If `slot' is >= 0, then it indicates a function parameter.
 * If `slot' is == -1, then it indicates the function's return value.
 */
void func_inline_state::slot_access(int slot,
				    cast_expr *out_expr,
				    cast_type *out_type)
{
	cast_type type;
	cast_expr expr;

	assert(func_type != 0);

	if (slot >= 0) {
		assert(slot < (signed int)func_type->params.params_len);
		assert(func_type->params.params_val[slot].name);

		type = func_type->params.params_val[slot].type;
		expr = cast_new_expr_name(
			func_type->params.params_val[slot].name);
	} else {
		/* Select the function return value.
		   It is always kept in a magic variable named `_return'.  */

		assert(slot == pres_c_func_return_index);

		type = func_type->return_type;
		expr = cast_new_expr_name("_return");
	}

	assert(expr != 0);
	assert(type != 0);

	*out_expr = expr;
	*out_type = type;
}

/* End of file. */
