/*
 * Copyright (c) 1995, 1996, 1999 The University of Utah and
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

#include <mom/c/be/mem_mu_state.hh>

/* Methods to save and restore the state of the glob and chunk allocator. */

mu_memory_allocator_state *mem_mu_state::memory_allocator_state()
{
	return new mem_mu_memory_allocator_state(max_msg_size,
						 align_bits, align_ofs,
						 chunk_size_expr, chunk_size, chunk_count,
						 glob_size_expr, glob_size);
}

void mem_mu_state::set_memory_allocator_state(mu_memory_allocator_state
					      *memory_state)
{
	mem_mu_memory_allocator_state *mstate =
		(mem_mu_memory_allocator_state *) memory_state;
	
	max_msg_size = mstate->max_msg_size;
	
	align_bits = mstate->align_bits;
	align_ofs = mstate->align_ofs;
	
	chunk_size_expr = mstate->chunk_size_expr;
	chunk_size = mstate->chunk_size;
	chunk_count = mstate->chunk_count;
	
	/* Note that although we restore `glob_size_expr', we do *not* restore
	   the CAST expression that it was pointing to.  This is intentional.
	   This is what allows us to determine the maximum glob size that is
	   required by any code branch.  See `mem_glob.cc' for more insight. */
	glob_size_expr = mstate->glob_size_expr;
	glob_size = mstate->glob_size;
}

