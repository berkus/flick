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
#include <mom/c/libcast.h>
#include <mom/c/pbe.hh>
#include <mom/c/libpres_c.h>

/*
 * When we encounter one of these nodes, we set up an inline state containing
 * only the CAST expr/ctype passed in.  This allows us to return to inline mode
 * (most useful for processing an inline_allocation_context node).
 */
void mu_state::mu_mapping_singleton(cast_expr cexpr,
				    cast_type ctype,
				    mint_ref itype,
				    pres_c_mapping_singleton *singleton)
{
	singleton_inline_state is(cexpr, ctype);
	mu_inline(&is, itype, singleton->inl);
}

/* End of file. */
