/*
 * Copyright (c) 1997, 1998, 1999 The University of Utah and
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

#include <mom/c/be/mem_mu_state.hh>

/*
 * This method handles PRES_C_MAPPING_REFERENCE presentations, which are used
 * to pass object references.
 *
 * XXX --- A ``mapping reference'' node contains a reference adjustment count,
 * but it does not always reflect the adjustment that we really want to make.
 * Most notably, when a client sends a reference, we generally want to *copy*,
 * not *move*, the reference to the server.
 *
 * The PG phase should be more careful about creating appropriate PRES_C, but
 * until that happens, we implement a particular reference-management policy
 * here.
 */

void mem_mu_state::mu_mapping_reference(cast_expr expr,
					cast_type /*ctype*/,
					mint_ref itype,
					pres_c_mapping_reference *rmap)
{
	cast_expr macro_call_cexpr;
	cast_expr ofs_cexpr;
	cast_expr cleanup_call_cexpr;
	
	int ref_size, ref_align_bits;
	char *macro_name;
	
	int ref_count_adjust;
	int mark_for_cleanup;
	
	/*****/
	
	get_prim_params(itype, &ref_size, &ref_align_bits, &macro_name);
	if (!macro_name) {
		panic(("In `mem_mu_state::mu_mapping_reference', "
		       "invalid MINT type (%d) encountered."),
		      itype);
	}
	
	ofs_cexpr = cast_new_expr_lit_int(chunk_prim(ref_align_bits, ref_size),
					  0);
	
	mu_mapping_reference_get_attributes(itype, rmap, 
					    &ref_count_adjust,
					    &mark_for_cleanup);

	macro_call_cexpr = cast_new_expr_call_3(
		cast_new_expr_name(macro_name),
		ofs_cexpr,
		expr,
		cast_new_expr_lit_int(ref_count_adjust, 0));
	
	add_stmt(cast_new_stmt_expr(macro_call_cexpr));
	
	if (mark_for_cleanup) {
		cleanup_call_cexpr = cast_new_expr_call_2(
			cast_new_expr_name(
				flick_asprintf(
					"flick_%s_mark_port_for_cleanup",
					get_be_name())),
			expr,
			cast_new_expr_lit_int(ref_count_adjust, 0));
		
		add_stmt(cast_new_stmt_expr(cleanup_call_cexpr));
	}
}

/* End of file. */

