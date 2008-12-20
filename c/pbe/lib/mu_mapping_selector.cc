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

/*
 * This method handles PRES_C_MAPPING_SELECTOR nodes, ``simple'' member
 * selections -- members that are part of a struct due to the presentation
 * style, but do not convey any message structure or data.
 *
 * Currently we assert that we have a CAST_TYPE_AGGREGATE from which we can
 * select a member.  Perhaps in the future this could be useful for selecting
 * a single element of an array as well.
 *
 * There is no MINT representation for this selection itself.  The MINT passed
 * in is intended for (and thus passed to) the subsequent mapping for the
 * selected member.
 */
void mu_state::mu_mapping_selector(
	cast_expr cexpr,
	cast_type ctype,
	mint_ref itype,
	pres_c_mapping_selector *smap)
{
	ctype = cast_find_typedef_type(&(pres->cast), ctype);
	assert(ctype);
	
	assert(ctype->kind == CAST_TYPE_AGGREGATE);
	cast_aggregate_type *at = &ctype->cast_type_u_u.agg_type;

	int slot = smap->index;
	assert(slot >= 0);
	assert(slot < (signed int)at->scope.cast_scope_len);
	assert(at->scope.cast_scope_val[slot].name.cast_scoped_name_val[0].
	       name);
	assert(at->scope.cast_scope_val[slot].u.kind == CAST_VAR_DEF);
	
	ctype = at->scope.cast_scope_val[slot].u.cast_def_u_u.var_def.type;
	cexpr = cast_new_expr_sel(cexpr, at->scope.cast_scope_val[slot].name);
	
	mu_mapping(cexpr, ctype, itype, smap->mapping);
}

/* End of file. */
