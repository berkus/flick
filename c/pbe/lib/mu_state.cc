/*
 * Copyright (c) 1995, 1996, 1997, 1998, 1999 The University of Utah and
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

#include <string.h>
#include <stdlib.h>

#include <mom/c/libcast.h>

#include <mom/c/pbe.hh>

struct h_table *mu_state::translation_handlers;

mu_state::mu_state(be_state *_state, mu_state_op _op, int _assumptions,
		   const char *which)
{
	int i;
	
	state = _state;
	pres = _state->get_pres();
	stub_inline_depth = (int *) mustmalloc(sizeof(int) *
					       pres->stubs.stubs_len);
	for (i = 0; i < (signed int)pres->stubs.stubs_len; ++i)
		stub_inline_depth[i] = 0;
	
	which_stub = which;
	op = _op;
	current_param_dir = PRES_C_DIRECTION_UNKNOWN;
	assumptions = _assumptions;
	now_packing = 0;
	c_block = 0;
	cdecl_block = 0;
	array_data.is_valid = 0;
	
	/* Nullify our allocation context. */
	inline_alloc_context = 0;
	
	/* Nullify our formal and actual function invocation expressions. */
	formal_func_invocation_cexpr = 0;
	actual_func_invocation_cexpr = 0;
	
	/* Set our SID (security identifier) CAST expressions to null. */
	client_sid_cexpr = 0;
	server_sid_cexpr = 0;
	
	/* no arglist initially */
	arglist = NULL;
	
	abort_block = 0;
	
	current_span = 0;
	current_chunk_span = 0;
}

mu_state::mu_state(const mu_state &must)
{
	unsigned int i;
	
	state = must.state;
	pres = must.pres;
	stub_inline_depth = (int *) mustmalloc(sizeof(int) *
					       pres->stubs.stubs_len);
	for (i = 0; i < pres->stubs.stubs_len; ++i)
		stub_inline_depth[i] = must.stub_inline_depth[i];
	
	which_stub = must.which_stub;
	op = must.op;
	current_param_dir = must.current_param_dir;
	assumptions = must.assumptions;
	now_packing = must.now_packing;
	
	/*
	 * Do not deep copy the `actual_func_invocation_cexpr'.  We want
	 * modifications to be shared.  (Often, it is a child `mu_state' that
	 * figures out the substitution of actuals for formals in the function
	 * parameter list!)
	 */
	formal_func_invocation_cexpr = must.formal_func_invocation_cexpr;
	actual_func_invocation_cexpr = must.actual_func_invocation_cexpr;
	
	client_sid_cexpr =	must.client_sid_cexpr;
	server_sid_cexpr =	must.server_sid_cexpr;
	
	/*
	 * Since the `c_block' is built incrementally while marshaling, we must
	 * create a copy of the `cast_block'.  However, the copy doesn't need
	 * to go any deeper than that.
	 */
	if (must.c_block) {
		cast_block *ob = &(must.c_block->cast_stmt_u_u.block);
		
		c_block = cast_new_block(ob->scope.cast_scope_len,
					 ob->stmts.stmts_len);

		cast_block *nb = &(c_block->cast_stmt_u_u.block);
		
		for (i = 0; i < ob->scope.cast_scope_len; ++i)
			nb->scope.cast_scope_val[i] = ob->
						      scope.cast_scope_val[i];
		for (i = 0; i < ob->stmts.stmts_len; ++i)
			nb->stmts.stmts_val[i] = ob->stmts.stmts_val[i];
		
		nb->flags = ob->flags;
	} else
		c_block = 0;
	
	/*
	 * Now do the same for `cdecl_block'.
	 */
	if (must.cdecl_block) {
		cast_block *ob = &(must.cdecl_block->cast_stmt_u_u.block);
		
		cdecl_block = cast_new_block(ob->scope.cast_scope_len,
					     ob->stmts.stmts_len);

		cast_block *nb = &(cdecl_block->cast_stmt_u_u.block);
		
		for (i = 0; i < ob->scope.cast_scope_len; ++i)
			nb->scope.cast_scope_val[i] = ob->
						      scope.cast_scope_val[i];
		/* There should be no statements in the `cdecl_block'. */
		for (i = 0; i < ob->stmts.stmts_len; ++i)
			nb->stmts.stmts_val[i] = ob->stmts.stmts_val[i];
		
		nb->flags = ob->flags;
	} else
		cdecl_block = 0;
	
	array_data = must.array_data;			/* Structure copy. */
	inline_alloc_context
		= must.inline_alloc_context;		/* Pointer copy. */
	
	arglist = must.arglist;
	
	abort_block = 0;
	
	current_span = 0;
	current_chunk_span = 0;
}

mu_state::~mu_state()
{
	free(stub_inline_depth);
}

const char *mu_state::get_which_stub()
{
	return which_stub;
}

/* End of file. */

