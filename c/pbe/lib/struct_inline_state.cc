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
 * This is the primary accessor method for a struct inline_state.  Given a slot
 * index number (a pres_c_inline_index) into the "current" C structure, returns
 * in `out_expr' a C expression that can be used in generated code to access
 * that slot at runtime.  Also returns, in `out_type', the C type of that slot.
 *
 * `slot' must be >= 0, indicating the structure member.
 */
void struct_inline_state::slot_access(int slot,
				      cast_expr *out_expr,
				      cast_type *out_type)
{
	cast_type type;
	cast_expr expr;

	assert(struct_type != 0);
	assert(var_expr != 0);

	assert(slot >= 0);
	assert(slot < (signed int)struct_type->scope.cast_scope_len);
	assert(struct_type->scope.cast_scope_val[slot].name.
		cast_scoped_name_val[0].name);

	/* `slot' selects a member of a structure,
	   and `var_expr' is an expression
	   referring to the structure as a whole.
	   Build an expression to access that member.  */
	type = struct_type->scope.cast_scope_val[slot].u.
		cast_def_u_u.var_def.type;
	expr = cast_new_expr_sel(var_expr,
				 struct_type->scope.
				 cast_scope_val[slot].
				 name);

	assert(expr != 0);
	assert(type != 0);

	*out_expr = expr;
	*out_type = type;
}

/* End of file. */
