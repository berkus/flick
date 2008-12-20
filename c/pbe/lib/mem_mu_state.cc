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

#include <mom/c/be/mem_mu_state.hh>
#include <mom/c/libcast.h>

mem_mu_state::mem_mu_state(be_state *_state, mu_state_op _op, int _assumptions,
			   int init_align_bits, int init_align_ofs,
			   int init_max_glob_size, const char *which)
:	mu_state(_state, _op, _assumptions, which),
	max_glob_size(init_max_glob_size)
{
	max_msg_size = 0;
	align_bits = init_align_bits;
	align_ofs = init_align_ofs;
	chunk_count = 0;
	chunk_size_expr = 0;
	chunk_size = 0;
	glob_size_expr = 0;
	glob_size = 0;
}

mem_mu_state::mem_mu_state(const mem_mu_state &must)
:	mu_state(must),
	max_glob_size(must.max_glob_size)
{
	max_msg_size = must.max_msg_size;
	align_bits = must.align_bits;
	align_ofs = must.align_ofs;
	chunk_count = must.chunk_count;
	/* Force a copy of the chunk_size_expr so the original doesn't
           get clobbered from a cloned state. */
	if (must.chunk_size_expr) {
		chunk_size_expr
			= (cast_expr *) mustcalloc(sizeof(*chunk_size_expr));
		*chunk_size_expr
			= (cast_expr) mustcalloc(sizeof(**chunk_size_expr));
		**chunk_size_expr = **must.chunk_size_expr;
	} else
		chunk_size_expr = 0;
	chunk_size = must.chunk_size;
	/* Likewise, force a copy of the glob_size_expr. */
	if (must.glob_size_expr) {
		glob_size_expr
			= (cast_expr *) mustcalloc(sizeof(*glob_size_expr));
		*glob_size_expr
			= (cast_expr) mustcalloc(sizeof(**glob_size_expr));
		**glob_size_expr = **must.glob_size_expr;
	} else
		glob_size_expr = 0;
	glob_size = must.glob_size;
	array_one_glob = must.array_one_glob;
	union_align_bits = must.union_align_bits;
	union_align_ofs = must.union_align_ofs;
	union_glob_size = must.union_glob_size;
	union_one_glob = must.union_one_glob;
	abort_block = must.abort_block;
	current_span = must.current_span;
}

cast_expr mem_mu_state::mapping_noconv(cast_expr /*expr*/,
				       cast_type /*ctype*/,
				       mint_ref /*itype*/,
				       pres_c_mapping /*map*/)
{
	/* XXX chars are always safe */
	return 0;
}

cast_expr mem_mu_state::inline_noconv(inline_state * /*ist*/,
				      mint_ref /*itype*/,
				      pres_c_inline /*inl*/)
{
	return 0;
}

/* End of file. */

