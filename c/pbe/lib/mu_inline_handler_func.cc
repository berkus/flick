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

#include <assert.h>

#include <mom/libmint.h>
#include <mom/c/libcast.h>
#include <mom/c/pbe.hh>

/*
 * This method handles `PRES_C_INLINE_HANDLER_FUNC' presentation nodes,
 * which are used to link an entire received message with a work function
 * (as is the case in a decomposed presentation).
 */
void mu_state::mu_inline_handler_func(inline_state *ist,
				      mint_ref itype,
				      pres_c_inline inl)
{
	pres_c_inline_handler_func *inlh
		= &(inl->pres_c_inline_u_u.handler_i);
	
	/*****/
	
	assert(inl->kind == PRES_C_INLINE_HANDLER_FUNC);
	
	/* Make sure the MINT is a structure (it should be the
         * parameters of the request message or parameters and return
         * value of the reply message).  We do not process the MINT
         * further; it is the responsibility of the work function to
	 * decode the message as necessary.
	 */
	assert(pres->mint.defs.defs_val[itype].kind == MINT_STRUCT);
	
	/*
	 * Handle the message and invocation identifier inlines.
	 * These should contain MAPPING_ARGUMENTS so we can get
	 * the cexpr and ctype.
	 */
	for (unsigned int i = 0; i < inlh->slots.slots_len; ++i) {
		/*
		 * None of the PRES_C slots refer to any MINT structure slot.
		 * In order to keep inlining, we need to pass down a reference
		 * to a MINT_VOID.
		 */
		assert(inlh->slots.slots_val[i].mint_struct_slot_index
			== mint_slot_index_null);
		
		mu_inline(ist, pres->mint.standard_refs.void_ref,
			  inlh->slots.slots_val[i].inl);
	}
	
	/* Pick off the arguments we need */
	cast_expr msg_cexpr = 0;
	cast_type msg_ctype = 0;
	cast_expr invid_cexpr = 0;
	cast_type invid_ctype = 0;
	int gotarg = 0;
	gotarg = arglist->getargs("params", "message", &msg_cexpr, &msg_ctype);
	assert(gotarg);
	gotarg = arglist->getargs("params", "invocation_id",
				  &invid_cexpr, &invid_ctype);
	assert(gotarg);
	assert(msg_cexpr);
	assert(msg_ctype);
	assert(invid_cexpr);
	assert(invid_ctype);
	
	/* Extract the invocation ID from the message */
	add_stmt(cast_new_stmt_expr(
		cast_new_expr_call_1(
			cast_new_expr_name(
				flick_asprintf(
					"flick_%s_extract_invocation_id",
					get_be_name())),
			invid_cexpr)));
	
	/* Convert the stream to a bundled message */
	add_stmt(cast_new_stmt_expr(
		cast_new_expr_call_1(
			cast_new_expr_name(
				flick_asprintf(
					"flick_%s_%s_stream_to_msg",
					get_be_name(),
					get_buf_name())),
			msg_cexpr)));
}

/* End of file. */

