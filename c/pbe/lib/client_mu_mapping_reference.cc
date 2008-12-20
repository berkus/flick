/*
 * Copyright (c) 1998, 1999 The University of Utah and
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
#include <mom/c/be/client_mu_state.hh>

void client_mu_state::mu_mapping_reference(cast_expr expr,
					   cast_type /*ctype*/,
					   mint_ref /*itype*/,
					   pres_c_mapping_reference */*rmap*/)
{
	/* Find the marshaling parameters for this port type.  */
	char *macro_name = flick_asprintf("flick_%s_%s_%s_client",
					  get_be_name(), 
					  get_which_stub(), 
					  get_buf_name());
	cast_expr macro_expr = cast_new_expr_name(macro_name);
	
	/* Spit out the marshal/unmarshal macro call.  */
	cast_expr cex = cast_new_expr_call_4(
		macro_expr,
		expr,
		cast_new_expr_lit_int(0 /*XXX enforce copy semantic*/, 0),
		cast_new_expr_name(get_encode_name()),
		/* Some of the macros might need an abort label. */
		cast_new_expr_name(abort_block->use_current_label()));
	
	add_stmt(cast_new_stmt_expr(cex));
}

/* End of file. */

