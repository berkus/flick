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

void mem_mu_state::new_glob()
{
	assert(glob_size_expr == 0);
	assert(glob_size == 0);
	assert(chunk_size_expr == 0);
	assert(chunk_size == 0);
	
	/* Create the beginning-of-glob macro invocation. */
	char *name = flick_asprintf("flick_%s_%s_new_glob",
				    get_be_name(), get_buf_name());
	cast_expr cex = cast_new_expr_call(cast_new_expr_name(name),
					   (op & MUST_DECODE) ? 1 : 2);
	glob_size_expr = &cex->cast_expr_u_u.call.params.
			 cast_expr_array_val[0];
	/* When we are encoding we might need to allocate space,
	   so we need to pass an abort label in order to handle out
	   of memory errors. . */
	if( (op & MUST_ENCODE) ) {
		cex->cast_expr_u_u.call.params.cast_expr_array_val[1] =
			cast_new_expr_name( abort_block->use_current_label() );
	}
	add_stmt(cast_new_stmt_expr(cex));
	
	/* Note that `end_glob' produces CAST statements that share the
	   parameter list structure pointed to by `glob_size_expr'.  Moreover,
	   `end_glob' may change the expression in the parameter list --- and
	   thereby change the number that appears within this `new_glob'
	   statement.  Tricky, eh? */
	*glob_size_expr = cast_new_expr_lit_int(0, 0);
}

void mem_mu_state::end_glob()
{
	assert(glob_size_expr);
	assert(glob_size >= 0);
	
	/* Chunks can't extend beyond the end of a glob. */
	break_chunk();
	
	/* Don't change the glob size expression if the existing glob size
	   expression refers to a value not less than `glob_size'.  Why?  We
	   may have already generated an `end_glob' for the current glob in
	   some other generated code branch, and that branch may have a greater
	   glob size requirement. */
	assert (((*glob_size_expr)->kind == CAST_EXPR_LIT_PRIM) &&
		((*glob_size_expr)->cast_expr_u_u.lit_prim.u.kind ==
		 CAST_PRIM_INT));
	int prev_glob_size = (*glob_size_expr)->cast_expr_u_u.lit_prim.u.
			     cast_lit_prim_u_u.i;
	if (prev_glob_size >= glob_size) {
		/* Do nothing. */
	} else {
		/* Replace the existing expression with a new expression.
		   Note that all of the `new_glob()' and `end_glob()'
		   statements that refer to the current glob all reference the
		   same array of parameters, and that array is pointed to by
		   `glob_size_expr'.  Therefore, changing the expression within
		   that parameter array has the effect of changing the constant
		   that appears in *all* the statements that refer to the
		   current glob!  Tricky, eh? */
		*glob_size_expr = cast_new_expr_lit_int(glob_size, 0);
		
		/* Add this glob to the running total for the maximum message
		   size.  Of course, we have to subtract this glob's previous
		   size from the count! */
		if ((max_msg_size - prev_glob_size + glob_size) > max_msg_size)
			max_msg_size = max_msg_size - prev_glob_size +
				       glob_size;
		else
			max_msg_size = MAXUINT_MAX;
	}
	
	/* Create the end-of-chunk macro invocation. */
	char *name = flick_asprintf("flick_%s_%s_end_glob",
				    get_be_name(), get_buf_name());
	cast_expr cex = cast_new_expr_call(cast_new_expr_name(name), 0);
	/* Now change the list of parameters in `cex'.  In particular, our new
	   expression will share its list of parameters with the `new_glob'
	   expression we generated previously.  This sharing of structure makes
	   it possible for us to munge the parameters (i.e., the literal
	   integer that is the glob size) that appear in all of these
	   expressions. */
	cex->cast_expr_u_u.call.params.cast_expr_array_len = 1;
	cex->cast_expr_u_u.call.params.cast_expr_array_val = glob_size_expr;
	add_stmt(cast_new_stmt_expr(cex));

	glob_size_expr = 0;
	glob_size = 0;
}

void mem_mu_state::glob_grow(int amount)
{
	//printf("this %08x amt %d mgs %d\n", this, amount, max_glob_size);
	assert(amount >= 0);
	assert(amount <= (signed int)max_glob_size);
	
	make_glob();
	if (glob_size + amount > (signed int)max_glob_size) {
		end_glob();
		new_glob();
		assert(glob_size + amount <= (signed int)max_glob_size);
	}
	glob_size += amount;
	assert(glob_size <= (signed int)max_glob_size);
}

void mem_mu_state::glob_prim(int needed_align_bits, int prim_size)
{
	/* Calculate the maximum amount of padding we might have to insert
	   here. */
	/* maximum padding examples:
	   
	   needed:  3, have 2,0:
	   you have either 3,0 or 3,4.  3,4 is the 'worst case'
	   
	   needed:  3, have 1,1:
	   you have either 3,1, 3,3, 3,5, or 3,7.  3,1 is 'worst case'
	   
	   needed:  3, have 1,0:
	   you have either 3,0, 3,2, 3,4, or 3,6.  3,2 is 'worst case'
	   
	   needed:  2, have 3,2:
	   you have 2,2
	   
	   needed:  3, have 0,0:
	   you have either 3,0, 3,1, 3,2, ...  3,1 is 'worst case'
	   
	   */
	assert(align_ofs < (1 << align_bits));
	int needed_align_mask = (1 << needed_align_bits) - 1;
	int alignment = (needed_align_bits > align_bits) ? needed_align_bits : align_bits;
	int alignoffset = align_ofs;
	if (align_ofs == 0 && needed_align_bits > align_bits)
		alignoffset = (1 << align_bits);
	int max_padding = ((1 << alignment) - alignoffset) & needed_align_mask;
	
	/* Add it to the current glob, or start a new glob if necessary. */
	if (max_padding > 0)
		glob_grow(max_padding);
	
	/* Then add the space for the actual primitive. */
	glob_grow(prim_size);
}

