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

/* These methods override the default do_nothing versions in mu_state,
   providing typically appropriate behavior when marshaling unions
   to/from memory-based marshaling streams.

   This code assumes that unions are handled
   by first marshaling the union discriminator,
   and then marshaling the union branch it selects.
   Only the amount of message space
   needed by the selected union branch is reserved -
   i.e. a marshaled union is generally variable-length.
   This contrasts with C unions,
   in which unions always occupy the amount of memory
   needed to hold the largest branch of the union.

   We use the variable `union_end_align' in the parent mu_state
   to calculate the minimum alignment of the union as a whole.
   This is the minimum of the final alignments of each possible branch.
   For example, if each branch in the union ends with an `int'
   aligned at a 4-byte boundary,
   then no matter which branch is taken at run time,
   we know the message stream will be 4-byte aligned
   after the full union is marshaled.
   If the next item we marshal after the union
   needs 4-byte alignment or less,
   we don't have to produce any run-time alignment checking code for it.
   However, if one of the union branches ends
   with a variable-length array of chars,
   then no particular alignment is guaranteed,
   and a run-time alignment check might be needed.
*/

#include <assert.h>
#include <mom/c/be/mem_mu_state.hh>

void mem_mu_state::mu_union(functor *f)
{
	/* Save off the union state we are about to use.  */
	
	int old_union_align_bits = union_align_bits;
	int old_union_align_ofs = union_align_ofs;
	int old_union_glob_size = union_glob_size;
	int old_union_one_glob = union_one_glob;
	maxuint old_union_msg_size = union_msg_size;
	
	/* Always break the chunk before and after unions.
	   Technically, it might be possible to avoid this
	   if all the union cases happened to be exactly the same size,
	   but it almost certainly isn't worth the trouble.  */
	break_chunk();
	
	/* Take a dry run to see if the union will fit all in one glob.  */
	mem_mu_state *sub = (mem_mu_state*)clone();
	sub->abort_block = new mu_abort_block();
	sub->abort_block->set_kind(MABK_THREAD);
	sub->abort_block->begin();
	sub->current_span = 0;
	sub->break_glob();
	sub->new_glob();
	assert(sub->glob_size_expr != 0); assert(sub->glob_size == 0);
	sub->union_one_glob = 1;
	sub->union_glob_size = 0;
	sub->mu_state::mu_union(f);
	
	union_one_glob = sub->union_one_glob;
	if (union_one_glob) {
		assert(sub->glob_size <= sub->union_glob_size);
		assert(sub->union_glob_size <= (signed int)max_glob_size);
		
		/* Even if the union will all fit in one glob,
		   it may not all fit into _this_ glob... */
		if (glob_size + sub->union_glob_size > (signed int)max_glob_size)
			break_glob();
		make_glob();
	} else {
		/* break_glob(); */
		/* We used to break the current glob here, but we don't really
		   need to do so.  All that we need to do is ensure that every
		   union case results in a glob/chunk state that is consistent
		   with the states produced by all the other cases.  Currently,
		   we ensure this consistency in `mem_mu_union_case'.  If
		   `union_one_glob' is false, then every case ends by breaking
		   the then-current glob.  This means that no matter what union
		   variant we process, we'll end up in a consistent state: one
		   in which we have no current glob. */
	}
	
	cast_expr *orig_glob_size_expr = glob_size_expr;
	union_glob_size = glob_size;
	
	/* Initial alignment informantion: -1 means no cases seen yet. */
	union_align_bits = -1;
	
	/* Initial union message size */
	union_msg_size = max_msg_size;
	
	/* Output the actual union handling code. */
	mu_state::mu_union(f);
	
	/* Set the final post-union alignment information. */
	assert(union_align_bits >= 0);
	assert(union_align_ofs < (1 << union_align_bits));
	align_bits = union_align_bits;
	align_ofs = union_align_ofs;
	
	/* Set the post-union glob size. */
	/* We used to make the following assertion at this point, but it is no
	   longer true when `union_glob_size' is false.  We may have started
	   with a glob, but when `union_glob_size' is false, every case results
	   in a state in which there is no current glob. */
	// assert(glob_size_expr == orig_glob_size_expr);
	if (union_one_glob) {
		assert(glob_size_expr == orig_glob_size_expr);
		assert(glob_size_expr != 0);
		assert(union_glob_size >= glob_size);
		glob_size = union_glob_size;
		assert(glob_size <= (signed int)max_glob_size);
	} else {
		assert(glob_size_expr == 0);
		assert(glob_size == 0);
	}
	
	/* Set the post-union message size */
	assert(union_msg_size >= max_msg_size);
	max_msg_size = union_msg_size;
	
	/* Restore the union state, for the benefit of any parent unions.  */
	union_align_bits = old_union_align_bits;
	union_align_ofs = old_union_align_ofs;
	union_glob_size = old_union_glob_size;
	union_one_glob = old_union_one_glob;
	union_msg_size = old_union_msg_size;
	sub->abort_block->end();
	delete sub;
}

