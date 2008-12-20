/*
 * Copyright (c) 1997 The University of Utah and
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
#include <mom/compiler.h>

#include "mach3.h"

/* This routine handles typed presentations.
 * First marshal/unmarshal the type tag,
 * then marshal/unmarshal the typed data.
 */
void mach3_mu_state::mu_inline_typed(inline_state *ist,
				     mint_ref itype, pres_c_inline inl)
{
	pres_c_inline_typed *tinl = &inl->pres_c_inline_u_u.typed;
	
	assert(itype >= 0);
	assert(itype < (signed int) pres->mint.defs.defs_len);
	
	mint_def *def = &pres->mint.defs.defs_val[itype];
	
	assert(def->kind == MINT_TYPED);
	
	mint_typed_def *tdef = &def->mint_def_u.typed_def;
	
	cast_expr saved_tag_cexpr = tag_cexpr;
	
	/* This call to mu_inline should return a CAST expression in
         * tag_cexpr that will be used when marshaling the following
	 * value.
	 */
	tag_cexpr = 0;
	marshaling_inline_typed = 1;
	
	mu_inline(ist, tdef->tag, tinl->tag);
	
	marshaling_inline_typed = 0;
	assert(tag_cexpr);
	
	mu_inline(ist, tdef->ref, tinl->inl);
	
	tag_cexpr = saved_tag_cexpr;
}

/* End of file. */

