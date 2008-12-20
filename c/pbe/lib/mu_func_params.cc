/*
 * Copyright (c) 1995, 1996, 1997, 1998 The University of Utah and
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
#include <mom/c/pbe.hh>

/*
 * Start descending the parallel CAST ond MINT type trees, beginning with the C
 * function prototype at index number `cfunc_idx' in the global cast_scope on
 * the C side.  The top-level network message type (`itype') could be anything.
 * The `inl' describes how to map the given `itype', whatever it is, onto the
 * parameters and/or return value of the C function.
 *
 * Basically, this routine just creates an inline_state describing the
 * parameters and return value of the function declaration, and then jumps into
 * the generic mu_inline decoding routine.
 */
void mu_state::mu_func_params(cast_ref cfunc_idx,
			      mint_ref itype,
			      pres_c_inline inl)
{
	assert(cfunc_idx >= 0);
	assert(cfunc_idx < ((signed int) pres->stubs_cast.cast_scope_len));
	
	cast_def *cfunc = &pres->stubs_cast.cast_scope_val[cfunc_idx];
	
	assert(cfunc->u.kind == CAST_FUNC_DECL);
	
	cast_func_type *cfunct = &(cfunc->u.cast_def_u_u.func_type);
	func_inline_state st(cfunct);
	
	/*
	 * Clear our SID expressions before we start inlining.  This is
	 * required for cases in which we use a single `mu_state' object to
	 * process more than one SID-bearing function parameter list, as we do
	 * when generating a server skeleton.  (A single `mu_state' is used
	 * to process all of the server functions; see `mu_decode_switch.cc'.)
	 */
	client_sid_cexpr = 0;
	server_sid_cexpr = 0;
	
	mu_inline(&st, itype, inl);
}

/* End of file. */

