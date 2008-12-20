/*
 * Copyright (c) 1997, 1999 The University of Utah and
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
 * This method handles `PRES_C_MAPPING_TYPE_TAG' presentations.
 *
 * Our handling of type tags is similar to our handling of type-tagged values;
 * see `mu_state::mu_mapping_typed'.
 */
void
mu_state::mu_mapping_type_tag(
	cast_expr cexpr,
	cast_type /* ctype */,
	mint_ref itype)
{
	mint_def *idef;
	
	cast_expr macro_call_expr;
	
	/*
	 * Check the MINT: make sure that it's a `MINT_TYPE_TAG'.
	 */
	assert(itype >= 0);
	assert(itype < (signed int)pres->mint.defs.defs_len);
	
	idef = &(pres->mint.defs.defs_val[itype]);
	assert(idef->kind == MINT_TYPE_TAG);
	
	/*
	 * We simply produce a special macro call to process the type tag.
	 * The arguments to the macro are:
	 *
	 *   1. the name of this back end, to be used in the macroexpansion
	 *      (to create calls to other macros);
	 *   2. the CAST expression for the value to be marshaled/unmarshaled;
	 *   3. a fresh label, which may be used in the macroexpansion (see
	 *      <flick/link/iiopxx.h>); and
	 *   4. the current abort label.
	 */
	macro_call_expr =
		cast_new_expr_call_4(
			cast_new_expr_name(
				flick_asprintf(
					"flick_%s_%s_type_tag",
					get_encode_name(),
					get_buf_name()
					)),
			cast_new_expr_name(get_be_name()),
			cexpr,
			cast_new_expr_name(add_label()),
			cast_new_expr_name(abort_block->use_current_label())
			);
	
	add_stmt(cast_new_stmt_expr(macro_call_expr));
}

/* End of file. */

