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

#include "iiop.h"

void iiop_mu_state::mu_mapping_reference(cast_expr expr,
					 cast_type /* ctype */,
					 mint_ref itype,
					 pres_c_mapping_reference *rmap)
{
	cast_expr macro_call_cexpr;
	cast_expr cleanup_call_cexpr;
	
	int ref_count_adjust;
	int mark_for_cleanup;
	
	/*****/
	
	/*
	 * In IIOP, object references are encoded as Interoperable Object
	 * References (IORs).  Section 10.6.2 of the CORBA 2.0 spec describes
	 * IORs in general; Section 12.7.2 describes the IIOP-specific parts.
	 *
	 * We produce special macro calls to process IORs.  These macros expect
	 * to be invoked outside of any active glob or chunk; they manage the
	 * stream themselves.
	 */
	break_glob();
	
	mu_mapping_reference_get_attributes(itype,
					    rmap, 
					    &ref_count_adjust,
					    &mark_for_cleanup);
	
	macro_call_cexpr
		= cast_new_expr_call_3(
			cast_new_expr_name(
				flick_asprintf("flick_%s_%s_IOR",
					       get_encode_name(),
					       get_buf_name())),
			cast_new_expr_name(get_be_name()),
			expr,
			cast_new_expr_lit_int(ref_count_adjust, 0));
	
	if( op & MUST_ENCODE )
		cast_add_expr_array_value(
			&macro_call_cexpr->cast_expr_u_u.call.params,
			cast_new_expr_name(abort_block->use_current_label()));
	
	add_stmt(cast_new_stmt_expr(macro_call_cexpr));
	
	if (mark_for_cleanup) {
		cleanup_call_cexpr
			= cast_new_expr_call_2(
				cast_new_expr_name(
					flick_asprintf(
						("flick_%s_mark_port_for_"
						 "cleanup"),
						get_be_name())),
				expr,
				cast_new_expr_lit_int(ref_count_adjust, 0));
		
		add_stmt(cast_new_stmt_expr(cleanup_call_cexpr));
	}
	if( (op & MUST_DEALLOCATE) && strlen(rmap->arglist_name) ) {
		mu_pointer_free(expr,
				cast_new_type(CAST_TYPE_VOID),
				rmap->arglist_name);
	}
	
	/*
	 * Fix our memory alignment state.  After processing an IOR, we are
	 * left with just 1-byte alignment.  Sigh.
	 */
	max_msg_size = MAXUINT_MAX;
	align_bits = 0;
	align_ofs = 0;
}

/* End of file. */

