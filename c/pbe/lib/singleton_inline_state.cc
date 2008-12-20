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
#include <mom/c/pbe.hh>

/*
 * This is the primary accessor method for a singleton inline_state.  The whole
 * point of having a singleton mapping is to return to inline mode, but in
 * order to do so, we must have an inline state.  Thus we have a singleton
 * state, containing only the CAST expr/type of the entity we were previously
 * mapping.  This method returns that expression and type.
 *
 * `slot' must == 0, indicating our only CAST expr/type.
 */
void singleton_inline_state::slot_access(int slot,
					 cast_expr *out_expr,
					 cast_type *out_type)
{
	assert(slot == 0);
	*out_expr = cexpr;
	*out_type = ctype;
}

/* End of file. */
