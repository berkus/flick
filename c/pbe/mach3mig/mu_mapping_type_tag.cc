/*
 * Copyright (c) 1997, 1998 The University of Utah and
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
#include <mom/libmint.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include "mach3.h"

void mach3_mu_state::mu_mapping_type_tag(cast_expr expr, cast_type /*ctype*/, mint_ref /*itype*/)
{
	if (marshaling_inline_typed)
	{
		/* if descending the type tag branch of an inline_typed,
		   marshaling_inline_typed is true, so mu_mapping_type_tag
		   should simply store the tag's cast_expr into tag_cexpr
		   for later use instead of explicitly [un]marshaling. */
		tag_cexpr = expr;
	} else {
		/* Find the marshaling parameters for this port type.  */
		char *macro_name = flick_asprintf(
			"flick_%s_%s_type_tag", get_be_name(), get_buf_name());
		cast_expr macro_expr = cast_new_expr_name(macro_name);
		cast_expr ofs_expr
			= cast_new_expr_lit_int(chunk_prim(2, 4), 0);
		
		/* Spit out the marshal/unmarshal macro call.  */
		cast_expr cex = cast_new_expr_call_2(macro_expr,
						     ofs_expr,
						     expr);
		add_stmt(cast_new_stmt_expr(cex));
	}
}

void mach3_target_mu_state::mu_mapping_type_tag(cast_expr expr,
						cast_type /*ctype*/,
						mint_ref /*itype*/)
{
	target_remote = expr;
	target_local = cast_new_expr_lit_int(0, 0);
}

void mach3_client_mu_state::mu_mapping_type_tag(cast_expr expr,
						cast_type /*ctype*/,
						mint_ref /*itype*/)
{
	client_remote = expr;
//	client_local = cast_new_expr_lit_int(0, 0); // already "0"
}

/* End of file. */
