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

#include <assert.h>
#include <mom/libmint.h>
#include <mom/c/libcast.h>
#include <mom/c/pbe.hh>

/* This function contains generic array processing code
   for all varieties and presentations of arrays;
   it is called by specific array-handling routines
   such as mu_mapping_internal_array and mu_mapping_pointer.

   When this function is called, the array length must already be known:
   we're ready to generate the code to m/u the actual array elements.
   If unmarshaling, the memory block into which to unmarshal the array
   will be allocated, if necessary.  Likewise, when marshaling, the
   memory block from which the array is marshaled will be deallocated.

   `array_expr' is a C expression evaluating (at run-time)
   to a pointer to the first element of the array buffer
   (i.e. the buffer that is "presented to" or gotten from the user program).
   From this expression will be built the actual expressions
   used to marshal/unmarshal the elements of the array.

   `elem_ctype' is the C (presentation) type of an element of the array.
   (The C type of the `array_expr' is a pointer to this type.)

   `elem_itype' is the MINT type of an element of the array.

   `elem_map' is the pres_c_mapping node
   representing the mapping of each element of the array
   (_not_ the mapping of the array as a whole).

   NOTE: `len_expr' and `len_ctype' were removed as explicit parameters,
   since they are now available through context information (the arglist).
   Passing these explicitly is redundant, and as we add other allocation-
   related context parameters (such as maximum length, release flags, etc.),
   it becomes tedious and expensive to add them explicitly to the parameter
   list of this and other related functions.  However, these are important
   [pseudo] parameters that are used here, and I felt their descriptions
   should remain.
   
   `len_expr' is a C expression evaluating to the length of the array.
   For fixed-length arrays, this is generally a constant expression;
   for variable-length arrays it will presumably refer to some variable
   containing the actual length of the array
   as determined dynamically, at runtime.
   (Again, any array length determination code necessary
   has already been produced by the time this function is called;
   `len_expr' simply encapsulates that length
   for use in the rest of the array processing code.)

   `len_ctype' specifies the C type of the array length -
   it is always some kind of integer,
   but may vary in size according to the maximum length of the array
   or other factors.
   The array processing code uses this C type
   when declaring variables to hold array indexes and counts;
   in particular, it is used for the array iterator variable
   that this routine declares
   in order to iterate over the elements of the array.

   `len_min' and `len_max' indicate the array length minimum and
   maximum values.  These are equal if it is a fixed array.

   */

void mu_state::mu_array(
	cast_expr array_expr,
	cast_type array_ctype,
	cast_type elem_ctype,
	mint_ref elem_itype,
	pres_c_mapping elem_map,
	char *cname)
{
	int gotarg = 0;
	
	/* First, we MUST find the length's CAST expr and type. */
	cast_expr len_expr;
	cast_type len_ctype;
	gotarg = arglist->getargs(cname, "length", &len_expr, &len_ctype);
	assert(gotarg);
	assert(len_expr);
	assert(len_ctype);
	
	/* Determine (as best we can) the array's bounds. */
	unsigned int len_min, len_max;
	mu_array_get_encoded_bounds(&len_min, &len_max, cname);
	
	/* Bounds check the array, but only for *real* arrays. */
	if (array_data.is_valid)
		mu_array_check_bounds(cname);
	
	if (len_max > 1) {
		/*
		 * Speculatively see if anything will be produced when we m/u
		 * the array.  If not, we can safely assume there is no reason
		 * to produce a loop for the array.
		 */
		mu_state *sub = clone();
		sub->abort_block = new mu_abort_block();
		sub->abort_block->set_kind(MABK_THREAD);
		sub->abort_block->begin();
		sub->current_span = 0;
		sub->c_block = 0;
		sub->mu_state::mu_array_elem(/* XXX */array_expr,
					     elem_ctype, elem_itype, elem_map,
					     1, 1 /* exactly one element */);
		if (!sub->c_block
		    || sub->c_block->kind != CAST_STMT_BLOCK
		    || (sub->c_block->cast_stmt_u_u.block.stmts.
			stmts_len == 0)) {
			/* Nothing produced; no loop necessary. */
			len_min = len_max = 0;
		}
		sub->abort_block->end();
		delete sub;
	}
	
	/* Allocate space */
	if ((op & MUST_ALLOCATE) &&
	    (array_ctype->kind == CAST_TYPE_POINTER))
		mu_pointer_alloc(array_expr, elem_ctype, cname);
	
	if (len_max > 1 || len_min != len_max) {
		assert(len_ctype);
		/*
		 * So we don't create unnecessary temporary variables, we
		 * create our own cblock to declare a temporary variable, and
		 * add the loop statement.  This is in addition to the new
		 * cblock we create for the loop body (below).
		 */
		cast_stmt orig_cblock = c_block;
		c_block = cast_new_block(0, 0);
		
		/* Declare an iterator variable.  */
		cast_expr iter_var_expr
			= add_temp_var("array_iter", len_ctype);
		
		/* From the array-as-a-whole pointer expression,
		   produce a current-element-of-the-array expression,
		   by adding the current array index to the pointer
		   and dereferencing it.
		   (The combination is equivalent to C's [] operator.)  */
		cast_expr elem_expr =
			cast_new_unary_expr(
				CAST_UNARY_DEREF,
				cast_new_binary_expr(CAST_BINARY_ADD,
						     array_expr,
						     iter_var_expr));
		
		/*
		 * Save off the CAST block so we can capture the loop body.
		 */
		cast_stmt the_c_block = c_block;
		c_block = 0;
		
		/* Since an array might be of a variable length, we need
		   to add a span node in order to check for this. */
		mu_msg_span *array_span = 0, *parent_span = 0;
		if( current_span ) {
			parent_span = current_span;
			array_span = new mu_msg_span;
			array_span->set_kind(MSK_ARRAY);
			array_span->set_block(orig_cblock);
			array_span->set_abort(abort_block);
			array_span->begin();
			current_span = new mu_msg_span;
			current_span->set_kind(MSK_SEQUENTIAL);
			current_span->set_block(c_block);
			current_span->set_abort(abort_block);
			current_span->begin();
		}
		
		cast_stmt goto_loop, loop_test;
		cast_expr test_expr;
		
		/* We need to build up the abort code so that it can handle
		   an abort in the middle of the loop, and after its executed.
		   So we need to make a loop out of goto's and simply make
		   it decrement the loop counter.  Then, at the end we
		   add a statement to reset the loop counter to the length
		   of the array, so we can rollback and undo everything. */
		struct mu_abort_block *mab_par, *mab_thr;
		
		mab_thr = new mu_abort_block();
		mab_thr->set_kind(MABK_THREAD);
		mab_par = abort_block;
		abort_block = mab_thr;
		mab_thr->begin();
		mu_array_elem(elem_expr, elem_ctype, elem_itype, elem_map,
			      len_min, len_max);
		mab_thr->end();
		abort_block = mab_par;
		
		add_stmt(cast_new_continue());
		if( mab_thr->stmt_count() ) {
			struct mu_abort_block *mab_con;
			
			mab_con = new mu_abort_block();
			mab_con->set_kind(MABK_THREAD);
			mab_con->begin();
			/* Add the loop tester */
			test_expr = cast_new_binary_expr(
				CAST_BINARY_GT,
				cast_new_expr_cast(
					iter_var_expr,
					cast_new_prim_type(CAST_PRIM_INT,
							   CAST_MOD_UNSIGNED)),
				cast_new_expr_lit_int(0,0));
			goto_loop = cast_new_goto(0);
			loop_test = cast_new_if(test_expr, goto_loop, 0);
			mab_con->add_stmt(loop_test);
			mab_con->add_child(mab_thr, MABF_INLINE);
			mab_con->add_stmt(
				cast_new_stmt_expr(
					cast_new_unary_expr(
						CAST_UNARY_POST_DEC,
						iter_var_expr)));
			goto_loop->cast_stmt_u_u.goto_label =
				mab_con->use_current_label();
			mab_con->add_stmt(
				cast_new_stmt_expr(
					cast_new_expr_assign(iter_var_expr,
							     len_expr)));
			mab_con->end();
			add_stmt(mab_con->get_block_label());
			abort_block->add_child(mab_con, MABF_OUT_OF_LINE);
		} else if( mab_thr->get_current_label()->
			   cast_stmt_u_u.s_label.users ) {
			add_stmt(mab_thr->get_block_label());
			abort_block->add_child(mab_thr, MABF_OUT_OF_LINE);
		}
		cast_stmt loopbody = c_block;
		c_block = the_c_block;
		
		/* add only if there's something to add */
		if (loopbody) {
			add_stmt(cast_new_for(
				cast_new_expr_assign(
					iter_var_expr,
					cast_new_expr_lit_int(0, 0)),
				cast_new_binary_expr(CAST_BINARY_LT,
						     iter_var_expr, len_expr),
				cast_new_unary_expr(CAST_UNARY_POST_INC,
						    iter_var_expr),
				loopbody));
			
			/* If no original cblock, make one. */
			if (!orig_cblock)
				orig_cblock = cast_new_block(0, 0);
			
			/*
			 * Since we actually did something useful, we
			 * ``absorb'' the cblock (temp var & loop) into the
			 * original cblock.
			 */
			assert(orig_cblock);
			assert(orig_cblock->kind == CAST_STMT_BLOCK);
			cast_block_absorb_stmt(
				&orig_cblock->cast_stmt_u_u.block,
				c_block);
		}
		
		/*
		 * Restore the original block.  Note that this now contains
		 * the for loop we made, if there was a body to it.
		 */
		c_block = orig_cblock;
		
		if( parent_span ) {
			/* Multiply the loop span size by the
			   length of the array */
			array_span->set_size(len_expr);
			current_span->end();
			array_span->add_child(current_span);
			array_span->end();
			parent_span->add_child(array_span);
			current_span = parent_span;
		}
	} else if (len_max == 1) {
		assert(len_min == 1);
		
		/* There is only one element to m/u, thus a simple
		   dereference of the pointer will produce the correct
		   element expression.
		   (This is equivalent to ptr[0] or *ptr in C) */
		cast_expr elem_expr =
			cast_new_unary_expr(
				CAST_UNARY_DEREF,
				array_expr);
		
		/*
		 * We directly call mu_mapping() instead of mu_array_elem().
		 * Normally, mu_array_elem() will make sure that the code it
		 * generates is loop-friendly, i.e. it can be placed in a loop
		 * without adverse effects (e.g. ensuring the globbing/chunking
		 * state is always the same at the beginning and end of each
		 * loop iteration).  With only a single element, we don't need
		 * to be this smart about it, nor do we want the unfortunate
		 * side-effects it might create (such as breaking a glob).
		 */
		mu_mapping(elem_expr, elem_ctype, elem_itype, elem_map);
	}
	
	/* Terminate the array, but only for *real* arrays. */
	if (array_data.is_valid)
		mu_array_terminate(array_expr, elem_ctype, cname);
	
	/* Deallocate space */
	if ((op & MUST_DEALLOCATE) &&
	    (array_ctype->kind == CAST_TYPE_POINTER))
		mu_pointer_free(array_expr, elem_ctype, cname);
}

/*
 * mu_array_get_pres_bounds() attempts to determine from the arglist the min
 * and max size of the array, returning both the (unsigned) integer values as
 * well as the CAST expressions.  If we can't determine the integer values
 * (since they conceptually could be runtime quantities), we return
 * conservative values for the purpose of Flick's optimizations (min=0U,
 * max=~0U).
 */
void mu_state::mu_array_get_pres_bounds(unsigned int *len_min,
					cast_expr *min_len_expr,
					unsigned int *len_max,
					cast_expr *max_len_expr,
					char *cname)
{
	cast_expr len_expr;
	cast_type len_ctype;
	cast_type toss_ctype;
	int gotarg = 0;
	
	/*
	 * We try to find the CAST expr and type for the actual array length
	 * (required), minimum length (optional, unspecified means zero), and
	 * maximum length (optional, unspecified means unbounded).  We also
	 * ensure that the min and max lengths have the same CAST type as the
	 * actual length.
	 */
	gotarg = arglist->getargs(cname, "length", &len_expr, &len_ctype);
	assert(gotarg);
	assert(len_expr);
	assert(len_ctype);
	gotarg = arglist->getargs(cname, "min_len", min_len_expr, &toss_ctype);
	assert(gotarg);
	if ((*min_len_expr)) {
		assert(toss_ctype);
		/*
		 * XXX - It would be nice to make sure the CAST types are
		 * compatible, but cast_cmp_type() fails us in this regard --
		 * it can only determine if they are *identical*.  For now, we
		 * just have to assume the presentation generator knows what
		 * it's doing.
		 */
		//assert(cast_cmp_type(toss_ctype, len_ctype) == 0);
	}
	gotarg = arglist->getargs(cname, "max_len", max_len_expr, &toss_ctype);
	assert(gotarg);
	if ((*max_len_expr)) {
		assert(toss_ctype);
		/*
		 * See XXX comment above regarding comparing CAST types.
		 */
		//assert(cast_cmp_type(toss_ctype, len_ctype) == 0);
	}
	
	/*
	 * See if the min & max lengths are constants.  If we have them, we
	 * might be able to use them for optimizing the output code.  If we
	 * don't, we set them to conservative values.
	 */
	*len_min = 0;
	if (*min_len_expr) {
		switch ((*min_len_expr)->kind) {
		case CAST_EXPR_LIT_PRIM:
			switch ((*min_len_expr)->
				cast_expr_u_u.lit_prim.u.kind) {
			case CAST_PRIM_CHAR:
				*len_min = (*min_len_expr)->cast_expr_u_u.
					   lit_prim.u.cast_lit_prim_u_u.c;
				break;
			case CAST_PRIM_INT:
				*len_min = (*min_len_expr)->cast_expr_u_u.
					   lit_prim.u.cast_lit_prim_u_u.i;
				break;
			case CAST_PRIM_BOOL:
				*len_min = (*min_len_expr)->cast_expr_u_u.
					   lit_prim.u.cast_lit_prim_u_u.b;
				break;
			default:
				/* We can't handle float or double lengths! */
				warn(("In mu_state::mu_array(), "
				      "strange minimum length type."));
			}
			break;
			
		case CAST_EXPR_CONST_NAME:
			/*
			 * At least it's a constant, but we don't know the
			 * value.
			 * XXX - Currently we don't communicate this knowledge
			 * beyond this point, although it may be useful to
			 * know that it's at least constant even if we don't
			 * know the exact value.
			 */
			break;
			
		default:
			// We don't know how to get the value; use the default.
			break;
		}
	}
	
	*len_max = ~0U;
	if (*max_len_expr) {
		switch((*max_len_expr)->kind) {
		case CAST_EXPR_LIT_PRIM:
			switch ((*max_len_expr)->cast_expr_u_u.lit_prim.u.
				kind) {
			case CAST_PRIM_CHAR:
				*len_max = (*max_len_expr)->cast_expr_u_u.
					   lit_prim.u.cast_lit_prim_u_u.c;
				break;
			case CAST_PRIM_INT:
				*len_max = (*max_len_expr)->cast_expr_u_u.
					   lit_prim.u.cast_lit_prim_u_u.i;
				break;
			case CAST_PRIM_BOOL:
				*len_max = (*max_len_expr)->cast_expr_u_u.
					   lit_prim.u.cast_lit_prim_u_u.b;
				break;
			default:
				/* We can't handle float or double lengths! */
				warn(("In mu_state::mu_array(), "
				      "strange maximum length type."));
			}
			break;
			
		case CAST_EXPR_CONST_NAME:
			/*
			 * At least it's a constant, but we don't know the
			 * value.
			 * XXX - Currently we don't communicate this knowledge
			 * beyond this point, although it may be useful to
			 * know that it's at least constant even if we don't
			 * know the exact value.
			 */
			break;
			
		default:
			// We don't know how to get the value; use the default.
			break;
		}
	}
}

/*
 * mu_array_get_encoded_bounds() attempts to determine from the arglist the min
 * and max size of the array, returning both the (unsigned) integer values as
 * well as the CAST expressions.  If we can't determine the integer values
 * (since they conceptually could be runtime quantities), we return
 * conservative values for the purpose of Flick's optimizations (min=0U,
 * max=~0U).
 */
void mu_state::mu_array_get_encoded_bounds(unsigned int *len_min,
					   unsigned int *len_max,
					   char *cname)
{
	/* Try to get the MINT length. */
	if (array_data.is_valid) {
		*len_min = array_data.mint_len_min;
		*len_max = array_data.mint_len_max;
	} else {
		/* Settle for presented length. */
		cast_expr min_len_expr, max_len_expr;
		mu_array_get_pres_bounds(len_min, &min_len_expr,
					 len_max, &max_len_expr, cname);
	}
	
	if (mu_array_is_terminated(cname)
	    && mu_array_encode_terminator(cname)) {
		if (*len_min != ~0U) (*len_min)++;
		if (*len_max != ~0U) (*len_max)++;
	}
}

/*
 * mu_array_get_pres_length() returns the *PRESENTED* length of an array.
 * Usually, and by default, this is the same as the encoded length of the
 * array, as given by the argument "length" in the arglist.  However, a back
 * end (e.g. Sun) can override this if the presented and encoded lengths of an
 * array are different (e.g. Sun's strings).
 */
void mu_state::mu_array_get_pres_length(char *cname,
					cast_expr *len_expr,
					cast_type *len_ctype)
{
	int gotarg = arglist->getargs(cname, "length", len_expr, len_ctype);
	assert(gotarg);
	assert(len_expr);
	assert(len_ctype);
}

/*
 * mu_array_check_bounds() emits runtime, presented array bounds check(s), as
 * necessary, for variable-sized, bounded arrays.
 */
void mu_state::mu_array_check_bounds(char *cname)
{
	/* Determine (as best we can) the array's bounds. */
	cast_expr min_len_expr, max_len_expr;
	unsigned int len_min, len_max;
	mu_array_get_pres_bounds(&len_min, &min_len_expr,
				 &len_max, &max_len_expr,
				 cname);
	
	/* Now, we MUST find the length's *PRESENTED* CAST expr and type. */
	cast_expr len_expr;
	cast_type len_ctype;
	mu_array_get_pres_length(cname, &len_expr, &len_ctype);
	assert(len_expr);
	assert(len_ctype);
	
	/*
	 * Bounds check the array length (only for variable-sized arrays).  It
	 * is variable-sized iff min_len_expr and max_len_expr do NOT evaluate
	 * to the same cast_expr.  If neither are specified, it is unbounded;
	 * if only one is specified, it is bounded on one end; if both are
	 * specified but different, it is bounded on both ends, but variable.
	 * (If they evaluate to identical exprs, it is a fixed-size array.)
	 */
	if (!max_len_expr || !min_len_expr
	    || (cast_cmp_expr(min_len_expr, max_len_expr) != 0)) {
		if (max_len_expr) // bounded above
			add_stmt(
				cast_new_if(
					cast_new_binary_expr(CAST_BINARY_GT,
							     len_expr,
							     max_len_expr),
					make_error(FLICK_ERROR_OUT_OF_BOUNDS),
					0));
		if (min_len_expr) // bounded below
			add_stmt(
				cast_new_if(
					cast_new_binary_expr(CAST_BINARY_LT,
							     len_expr,
							     min_len_expr),
					make_error(FLICK_ERROR_OUT_OF_BOUNDS),
					0));
	}
}

/*
 * mu_array_terminate() emits any necessary array termination code, such as
 * adding the NUL character onto the end of encoded/decoded strings.
 */
void mu_state::mu_array_terminate(cast_expr expr, cast_type ctype,
				  char *cname)
{
	/* See if there's a terminator. */
	cast_expr terminator_expr;
	cast_type terminator_type;
	int gotarg = arglist->getargs(cname, "terminator",
				      &terminator_expr, &terminator_type);
	assert(gotarg);
	if (!terminator_expr) {
		/* Nothing to do! */
		return;
	}
	
	/* Ensure the terminator is the same type as the array elements,
	   ignoring any qualifiers. */
	assert(terminator_type);
	while (ctype->kind == CAST_TYPE_QUALIFIED)
		ctype = ctype->cast_type_u_u.qualified.actual;
	while (terminator_type->kind == CAST_TYPE_QUALIFIED)
		terminator_type
			= terminator_type->cast_type_u_u.qualified.actual;
	if (cast_cmp_type(ctype, terminator_type) != 0)
		panic(("In mu_state::mu_array_terminate(): "
		       "terminator does not match element CAST type."));
	
	/* Now, we MUST find the length's CAST expr and type. */
	cast_expr len_expr;
	cast_type len_ctype;
	gotarg = arglist->getargs(cname, "length", &len_expr, &len_ctype);
	assert(gotarg);
	assert(len_expr);
	assert(len_ctype);
	
	/*
	 * If we encode the terminator, then we don't have to worry about it,
	 * because the encoded length will include the terminator of the
	 * presented array.  Thus, by encoding or decoding the array, we also
	 * encode the terminator.  However, if we aren't supposed to encode the
	 * terminator, then the encoded length *EXCLUDES* the terminator.  In
	 * that case, we must add it onto the presented array to make it
	 * properly terminated.  This might seem a little backwards, but it
	 * does make sense.
	 */
	if (!mu_array_encode_terminator(cname)) {
		/*
		 * If we're decoding, produce code to add the terminator to the
		 * array.
		 */
		if (op & MUST_DECODE) {
			add_stmt(cast_new_stmt_expr(cast_new_expr_assign(
				cast_new_unary_expr(
					CAST_UNARY_DEREF,
					cast_new_binary_expr(
						CAST_BINARY_ADD,
						expr,
						len_expr)),
				terminator_expr)));
		}
	}
}

/*
 * mu_array_is_terminated() determines if the allocation context `cname'
 * describes a terminated array.
 */
int mu_state::mu_array_is_terminated(char *cname)
{
	cast_expr expr;
	cast_type ctype;
	int gotarg = arglist->getargs(cname, "terminator", &expr, &ctype);
	assert(gotarg);
	return (!!expr);
}

/*
 * mu_array_is_string() determines if the current array (identified by the
 * allocation context name `cname') describes a null-terminated string.
 */
int mu_state::mu_array_is_string(char *cname)
{
	cast_expr expr;
	cast_type ctype;
	int gotarg = arglist->getargs(cname, "terminator", &expr, &ctype);
	assert(gotarg);
	if (expr
	    && expr->kind == CAST_EXPR_LIT_PRIM
	    && expr->cast_expr_u_u.lit_prim.u.kind == CAST_PRIM_CHAR
	    && expr->cast_expr_u_u.lit_prim.u.cast_lit_prim_u_u.c == 0) {
		assert(ctype
		       && ctype->kind == CAST_TYPE_PRIMITIVE
		       && (ctype->cast_type_u_u.primitive_type.kind
			   == CAST_PRIM_CHAR));
		return 1;
	}
	return 0;
}

/* End of file. */

