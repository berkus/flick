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
#include <string.h>

#include <mom/compiler.h>
#include <mom/libmint.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include "fluke.h"

/*
 * Initialize the static member constant `fluke_mu_state::gather_allocator',
 * which is used here.  ANSI C++ doesn't let us initialize this static constant
 * member in the class declaration --- blech!
 */
const char * const fluke_mu_state::gather_allocator = "fdev";

/*
 * The Fluke version of `mu_array' recognizes and exploits opportunities for
 * ``scatter/gather'' opimizations.
 *
 * XXX --- The current Fluke BE and runtime don't support scatter at all.
 *
 * XXX --- There are three problems with the current gather implementation that
 * prevent us from doing gather in the general case:
 *
 *   1. The gathered objects must be the last objects marshaled in the message.
 *      If normal data is marshaled after gathered data, the effect is that the
 *      normal data will appear in the received message before the gathered
 *      data --- i.e., the marshaled data arrives out of order.
 *
 *   2. Fluke IPC requires that some minimum number of words be marshaled into
 *      the principal message buffer --- so not everything in the message can
 *      be gathered.
 *
 *   3. When the server gathers data for the reply, it must arrange for the
 *      gather buffers to be freed after the reply has been sent.  This is can
 *      be handled through the MOM infrastructure's `cleanup' facility.
 *      However, the current implementation of ``gather'' doesn't record the
 *      functions required to free each individual gather buffer.
 *
 * Due to these limitations, the gather optimization is currently applied only
 * to objects using a particular magic allocator named in `gather_allocator'.
 * The magic allocator is bestowed by the Fluke PG on objects of certain types.
 * Issue (1) is "solved" through users' knowledge.  Issue (2) is "solved"
 * because the magic allocator is applied only to sequences, and any message
 * encoding a sequence will have at least the minimum number of words encoded
 * in the main message buffer (the request/reply code and the sequence length).
 * Issue (3) is "solved" because gathering is done only on objects using a
 * particular allocator --- so the runtime can know what deallocator to call to
 * free the gathered buffers.
 */
void fluke_mu_state::mu_array(
	cast_expr array_expr, cast_type array_ctype,
	cast_type elem_ctype, mint_ref elem_itype, pres_c_mapping elem_map,
	char *cname)
{
	cast_expr sizeof_expr;
	int elem_size, elem_align_bits;
	cast_stmt if_xlate_stmt;
	int always_gather_p;
	
	/*
	 * `sizeof_expr' will be non-null only if the array elements are
	 * `bcopy'able, ignoring endianness conversion (handled later on).
	 */
	sizeof_expr = mu_get_sizeof(elem_itype, elem_ctype, elem_map,
				    &elem_size, &elem_align_bits);
	
	/*
	 * Determine if we can gather the array data.  If not, we'll process it
	 * normally.
	 */
	mu_inline_alloc_context *iac = inline_alloc_context;
	while (iac) {
		if (strcmp(iac->name, cname) == 0)
			break;
		iac = iac->parent_context;
	}
	assert(iac);
	pres_c_allocator allocer = get_allocator_kind(iac->alloc);
	if (/* We're not using the magic allocator... */
	    !((allocer.kind == PRES_C_ALLOCATOR_NAME)
	      && (strcmp(allocer.pres_c_allocator_u.name,
			 gather_allocator) == 0))
	    /* ...or we're not marshaling... */
	    || !(op & MUST_ENCODE)
	    /* ...or the object isn't `bcopy'able. */
	    || !sizeof_expr
	    ) {
		mem_mu_state::mu_array(array_expr, array_ctype,
				       elem_ctype, elem_itype, elem_map,
				       cname);
		return;
	}
	
	/* We need the length expression, so grab it from the arglist. */
	cast_expr len_expr;
	cast_type len_type;
	int gotarg = arglist->getargs(cname, "length", &len_expr, &len_type);
	assert(gotarg);
	assert(len_expr);
	assert(len_type);
	
	cast_stmt gather_if = 0, gather_else = 0, gather_endif = 0;
	struct mu_abort_block *mab_par, *mab_con = 0, *mab_thr = 0;
	/*
	 * Emit the array marshaling code.  First, get the `#if' preprocessor
	 * statement that determines if we must perform endianness conversion
	 * on the data.
	 */
	if_xlate_stmt = mu_bit_translation_necessary(0 /* `#if' */,
						     elem_itype);
	gather_if = if_xlate_stmt;
	always_gather_p = (if_xlate_stmt->kind == CAST_STMT_TEXT)
			  && (strncmp(if_xlate_stmt->cast_stmt_u_u.text,
				      "#if 0",
				      (sizeof("#if 0") - 1))
			      == 0);
	
	mab_par = abort_block;
	if (!always_gather_p) {
		gather_else = mu_bit_translation_necessary(1, elem_itype);
		gather_endif = mu_bit_translation_necessary(2, elem_itype);
		/*
		 * There is a compile-time possibility that we won't be able to
		 * gather the data due to endianness conversion.  So emit the
		 * normal marshaling code, too.
		 *
		 * XXX --- This code is untested because the Fluke BE always
		 * uses native endian encoding.  Am I overlooking some glob/
		 * chunk management issues here?
		 */
		add_stmt(if_xlate_stmt);
		
		/* We need to create a child abort block here to handle
		   the #if/#else/#endif stuff. */
		mab_con = new mu_abort_block();
		mab_con->set_kind(MABK_CONTROL);
		mab_con->begin();
		mab_con->add_stmt(gather_if);
		mab_thr = new mu_abort_block();
		mab_thr->set_kind(MABK_THREAD);
		mab_thr->begin();
		abort_block = mab_thr;
		mem_mu_state::mu_array(array_expr, array_ctype,
				       elem_ctype, elem_itype, elem_map,
				       cname);
		mab_thr->end();
		mab_con->add_child(mab_thr, MABF_INLINE);
		mab_con->add_stmt(gather_else);
		mab_thr = new mu_abort_block();
		mab_thr->set_kind(MABK_THREAD);
		mab_thr->begin();
		abort_block = mab_thr;
		add_stmt(gather_else);
	}
	
	/*
	 * Bounds check the array, but only for *real* arrays.
	 * Normally, this is taken care of by the standard mu_array(), but
	 * we aren't running that here.
	 */
	if (array_data.is_valid)
		mu_array_check_bounds(cname);
	
	/* Construct and emit the call to the gathering macro. */
	add_stmt(cast_new_stmt_expr(
		cast_new_expr_call_3(
			cast_new_expr_name(flick_asprintf("flick_%s_%s_gather",
							  get_be_name(),
							  get_buf_name())),
			array_expr,
			cast_new_binary_expr(CAST_BINARY_MUL,
					     len_expr,
					     cast_new_expr_lit_int(elem_size,
								   0)),
			cast_new_expr_name(abort_block->use_current_label())
			)
		));
	
	/*
	 * Terminate the array, but only for *real* arrays.
	 * Normally, this is taken care of by the standard mu_array(), but
	 * we aren't running that here.
	 */
	if (array_data.is_valid)
		mu_array_terminate(array_expr, elem_ctype, cname);
	
	if (!always_gather_p) {
		add_stmt(gather_endif);
		mab_thr->end();
		mab_con->add_child(mab_thr, MABF_INLINE);
		mab_con->add_stmt(gather_endif);
		mab_con->end();
		mab_par->add_child(mab_con, MABF_INLINE);
		abort_block = mab_par;
	}
}

/*
 * For Fluke strings, we encode the terminator as part of the string.
 */
int fluke_mu_state::mu_array_encode_terminator(char *cname)
{
	return mu_array_is_string(cname);
}

/* End of file. */

