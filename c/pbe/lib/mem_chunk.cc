/*
 * Copyright (c) 1995, 1996, 1998, 1999 The University of Utah and
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
#include <mom/c/be/mem_mu_state.hh>


void mem_mu_state::new_chunk()
{
	assert(chunk_size_expr == 0);
	assert(chunk_size == 0);
	
	/* Check for enough message data during a decode */
	if( current_span && (op & MUST_DECODE) ) {
		if( !c_block ) {
			c_block = cast_new_block(0,0);
		}
		current_chunk_span = new mu_msg_span;
		current_chunk_span->set_block(c_block);
		current_chunk_span->set_abort(abort_block);
		current_chunk_span->begin();
	}
	
	/* If there's no current glob, create one first.  */
	if (!glob_size_expr) new_glob();

	/* Create the beginning-of-chunk macro invocation.  */
	char *name = flick_asprintf("flick_%s_%s_new_chunk",
				    get_be_name(), get_buf_name());
	cast_expr cex = cast_new_expr_call(cast_new_expr_name(name), 1);
	chunk_size_expr = &cex->cast_expr_u_u.call.params.
			  cast_expr_array_val[0];
	add_stmt(cast_new_stmt_expr(cex));

	*chunk_size_expr = cast_new_expr_name("XXX");
	chunk_count++;
}

void mem_mu_state::new_chunk_align(int needed_bits)
{
	assert(chunk_size_expr == 0);
	assert(chunk_size == 0);

	/* If there's no current glob, create one first.  */
	if (!glob_size_expr) new_glob();

	/* Create the beginning-of-chunk macro invocation.  */
	char *name = flick_asprintf("flick_%s_%s_new_chunk_align",
				    get_be_name(), get_buf_name());
	cast_expr cex = cast_new_expr_call_4(
		cast_new_expr_name(name),
		0,
		cast_new_expr_lit_int(needed_bits, 0),
		cast_new_expr_lit_int(align_bits, 0),
		cast_new_expr_lit_int(align_ofs, 0));
	chunk_size_expr = &cex->cast_expr_u_u.call.params.
			  cast_expr_array_val[0];
	add_stmt(cast_new_stmt_expr(cex));

	/* Check for enough message data during a decode */
	if( current_span && (op & MUST_DECODE) ) {
		if( !c_block ) {
			c_block = cast_new_block(0,0);
		}
		current_chunk_span = new mu_msg_span;
		current_chunk_span->set_flags(current_chunk_span->get_flags() |
					      MSF_ALIGN);
		current_chunk_span->set_block(c_block);
		current_chunk_span->set_abort(abort_block);
		current_chunk_span->begin();
	}
	
	*chunk_size_expr = cast_new_expr_name("XXX");
	
	align_bits = needed_bits;
	align_ofs = 0;
	chunk_count++;
}

void mem_mu_state::end_chunk()
{
	assert(chunk_size_expr);
	assert(chunk_size >= 0);

	cast_expr size_expr = cast_new_expr_lit_int(chunk_size, 0);
	*chunk_size_expr = size_expr;

	/* Create the end-of-chunk macro invocation.  */
	char *name = flick_asprintf("flick_%s_%s_end_chunk",
				    get_be_name(), get_buf_name());
	cast_expr cex = cast_new_expr_call(cast_new_expr_name(name), 1);
	cex->cast_expr_u_u.call.params.cast_expr_array_val = chunk_size_expr;
	add_stmt(cast_new_stmt_expr(cex));
	
	if( current_span ) {
		current_chunk_span->grow(chunk_size);
		current_chunk_span->end();
		current_span->add_child(current_chunk_span);
		current_chunk_span = 0;
	}
	
	// if this chunk is empty, don't count it...
	if (!chunk_size)
		chunk_count--;
	
	chunk_size_expr = 0;
	chunk_size = 0;
}

int mem_mu_state::chunk_prim(int needed_align_bits, int prim_size)
{
	/* Do appropriate globification.
	   This might end the current chunk.  */
	glob_prim(needed_align_bits, prim_size);

	/* If known alignment must be increased, then finish the last chunk
	   and start a new one with new_chunk_align().  */
	if (needed_align_bits > align_bits)
	{
		if (chunk_size_expr)
			end_chunk();
		new_chunk_align(needed_align_bits);
	}
	else
	{
		/* Otherwise, just make sure we have a chunk.  */
		if (!chunk_size_expr)
			new_chunk();
	}

	/* Now the known bits of alignment should be fine
	   (we've taken care of variable-length runtime padding),
	   but we may still have to insert constant pad bytes to get align_ofs right.  */
	assert(needed_align_bits <= align_bits);
	int needed_align_mask = (1 << needed_align_bits) - 1;
	int pad = ((1 << align_bits) - align_ofs) & needed_align_mask;
	int prim_ofs = chunk_size + pad;
	chunk_size = prim_ofs + prim_size;
	
	/* Adjust the current known alignment appropriately.  */
	align_ofs = (((align_ofs + pad) & ~needed_align_mask) + prim_size)
		    & ((1 << align_bits) - 1);
	
#if 0
	// Old stuff
	chunk_size = (chunk_size + needed_align_mask) & ~needed_align_mask;

	/* Allocate room for the primitive.  */
	int prim_ofs = chunk_size;
	chunk_size += prim_size;

	/* Adjust the current known alignment appropriately.  */
	align_ofs = ((align_ofs & ~needed_align_mask) + prim_size)
		    & ((1 << align_bits) - 1);

	/* Return the position of the primitive in the chunk.  */
#endif
	
	return prim_ofs;
}

