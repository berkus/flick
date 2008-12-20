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
#include <mom/c/be/mem_mu_state.hh>
#include <string.h>

/* This method overrides (actually extends)
   the default behavior of mu_array
   in order to generate more optimal marshaling/unmarshaling code
   for simple arrays of bytes that need no translation
   (i.e. a byte of message data maps to a byte in memory).
   It handles such arrays all in one shot
   by simply emitting a block copy statement.
   Arrays that don't meet the eligibility requirements
   simply get passed on to the default mu_array implementation,
   which iterates through the array elements individually.

   This code should really be able to optimize more varied data types this way:
   for example, a struct that contains only bytes should work too
   (or, for that matter, most any C data type
   that consists of just one chunk of memory
   and happens to be laid out in the same format
   as the message being marshaled/unmarshaled).
   This could be done by creating a generic method
   to be implemented (partly?) by more-specific code
   that recursively tests a complete itype/ctype/mapping subtree
   and returns true if the message and memory layouts are equivalent
   for that entire type subtree.
*/

#define DID_BCOPY	0x00000100
#define DID_MPTR	0x00000200
#define DID_LOOP	0x00000001
#define DID_OPTIMIZE	0xffffff00

static int mu_array_helper(
	mem_mu_state *must,
	cast_expr ptr_expr,
	cast_type ptr_ctype,
	pres_c_allocation *ptr_alloc,
	cast_type target_ctype,
	mint_ref target_itype,
	pres_c_mapping target_map,
	cast_expr len_expr, cast_type len_ctype,
	char *cname,
	unsigned int len_min,
	unsigned int len_max);

void mem_mu_state::mu_array(
	cast_expr array_expr,
	cast_type array_ctype,
	cast_type elem_ctype,
	mint_ref elem_itype,
	pres_c_mapping elem_map,
	char *cname)
{
	/* Get the encoded length for this array. */
	unsigned int len_min;
	unsigned int len_max;
	mu_array_get_encoded_bounds(&len_min, &len_max, cname);
	
	/*
	 * We have no need to worry about optimizing, globbing, or chunking a
	 * MINT_VOID element.  However, ignoring it completely is also wrong
	 * (perhaps we want to allocate and initialize an array).  We pass it
	 * to the standard mu_state::mu_array() for processing.
	 *
	 * XXX - Could we ever call memset or bzero for initialization?
	 *
	 * We also pass simple pointers (actually all fixed-size arrays of one
	 * element) to mu_state::mu_array() for processing, since they do not
	 * require the extra processing for optimization.
	 */
	assert(elem_itype >= 0);
	assert((unsigned) elem_itype < pres->mint.defs.defs_len);
	if (pres->mint.defs.defs_val[elem_itype].kind == MINT_VOID
	    || (len_max <= 1 && len_min == len_max)) {
		/*
		 * We set `array_one_glob' since we can easily fit a single
		 * element, or an array of 0-byte elements into our glob.
		 * Ideally, it shouldn't matter.  However, mem_mu_state's
		 * version of mu_array_elem() will break the glob if
		 * `array_one_glob' is NOT set, and thus will generate a loop,
		 * breaking the current glob for every element of the array
		 * (very bad).
		 */
		int old_array_one_glob = array_one_glob;
		array_one_glob = 1;
		mu_state::mu_array(array_expr, array_ctype,
				   elem_ctype, elem_itype, elem_map, cname);
		array_one_glob = old_array_one_glob;
		return;
	}
	
	/*
	 * For all other (non-void) arrays, make sure some parent node has set
	 * up the allocation context information we need to process the array.
	 */
	mu_inline_alloc_context *iac = inline_alloc_context;
	while (iac) {
		if (strcmp(iac->name, cname) == 0)
			break;
		iac = iac->parent_context;
	}
	if (!iac)
		panic(("In `mem_mu_state::mu_array', "
		       "allocation context `%s' not available."), cname);
	
	/* Grab the information we need from the slots set up by our parent. */
	pres_c_allocation *aalloc = iac->alloc;
	
	/* Now, we MUST find the length's CAST expr and type. */
	cast_expr len_expr;
	cast_type len_ctype;
	int gotarg = arglist->getargs(cname, "length", &len_expr, &len_ctype);
	assert(gotarg);
	assert(len_expr);
	assert(len_ctype);
	
	/* We need to figure out the optimal globbing and chunking. */
	
	int old_array_one_glob = array_one_glob;
	int elem_size;
	int elem_align_bits;
	unsigned int array_glob_size;
	
	/* see if a bcopy is possible for this data */
	cast_expr bcopy_poss = mu_get_sizeof(elem_itype,
					     elem_ctype,
					     elem_map,
					     &elem_size,
					     &elem_align_bits);
	if (bcopy_poss && elem_size > 0) {
//		chunk_prim(elem_align_bits, 0);
	}
	
	/*
	 * Supposing (conservatively) each array element needs only one byte,
	 * if the maximum possible array would already be too long for a glob,
	 * don't even bother taking a dry run to find the actual element size.
	 * An array of zero-byte elements is actually possible (e.g. of void),
	 * but since it's useless, there's no need to bother optimizing that
	 * case.
	 */
	mem_mu_state *sub = 0;
	cast_expr *element_glob_size_expr;
	
	/*
	 * Take a dry run to determine whether or not
	 * we can marshal the array as one big glob,
	 * and if so, whether it can be merged into the current glob
	 * or we need to start a new glob before the array.
	 */
	
	sub = (mem_mu_state*)clone();
	sub->abort_block = new mu_abort_block();
	sub->abort_block->set_kind(MABK_THREAD);
	sub->abort_block->begin();
	sub->current_span = 0;
	sub->break_glob();
	sub->make_glob();
	element_glob_size_expr = sub->glob_size_expr;
	assert(element_glob_size_expr);
	sub->array_one_glob = 1;
	/*
	 * We call mu_array_elem() here, since we are really only interested in
	 * what happens with a single element -- it's size, ending alignment,
	 * etc.  We don't want any optimizations (like we would get with
	 * mu_array()), since that may affect the single element length we want
	 * to determine (a bcopy would show the *entire* size of the array).
	 */
	sub->mu_array_elem(/* XXX - for lack of something better */ array_expr,
			   elem_ctype, elem_itype, elem_map,
			   1, 1 /* Do exactly one element */);
	
	if (elem_size < 0) 
		elem_size = sub->glob_size;
	array_glob_size = elem_size * len_max;
	// Simple overflow check:
	if (array_glob_size < len_max)
		array_glob_size = ~0U;
	
	/*
	 * First - check if the element glob was broken
	 * if it was broken, there's no way we'll fit the array in memory,
	 * so we just break the glob here...
	 * If it wasn't broken, but the array could be too long,
	 * break it, as well.
	 */
	
	// unrestricted
	//      we have to break the glob and start a new glob for each element
	// element-per-chunk
	//	the outer glob should be (element_size * [actual len of array])
	// bounded-globbable
	//	the outer glob should be (element_glob_size * max len of array)
	
	
	/* Perhaps this illustrates it better:
	   UNRESTRICTED
	   sequence<not-globbable, whatever...>
	   end-glob
	   for (;;) {
	    new-glob
	    mu
	    end-glob
	   }
	   
	   UNRESTRICTED
	   sequence<one-glob(multiple chunk), (one-glob * max > max_glob_size)>
	   end-glob
	   for (;;) {
	    new-glob
	    mu
	    end-glob
	   }
	   
	   CHUNK_PER_ELEMENT
	   sequence<one-chunk, (one-chunk * max > max_glob_size)>
	   new-glob(_length * one-chunk)
	   for (;;) {
	    mu
	   }
	   
	   BOUNDED_GLOBBABLE
	   sequence<one-glob, globbable>
	   new-glob(_max * one-glob)
	   for (;;) {
	    mu
	   }
	   
	   BOUNDED_GLOBBABLE
	   sequence<one-chunk, globbable>
	   new-glob(_max * one-chunk)
	   for (;;) {
	    mu
	   }
	   */
	enum {UNRESTRICTED, CHUNK_PER_ELEMENT, BOUNDED_GLOBBABLE} arr_type;
	
	if (sub->glob_size_expr != element_glob_size_expr) {
		arr_type = UNRESTRICTED;
	} else if (!sub->elem_one_chunk) {
		arr_type = (array_glob_size > max_glob_size) ?
			   UNRESTRICTED :
			   BOUNDED_GLOBBABLE;
	} else {
		arr_type = (array_glob_size > max_glob_size) ?
			   CHUNK_PER_ELEMENT :
			   BOUNDED_GLOBBABLE;
	}
	
	array_one_glob = (arr_type != UNRESTRICTED);
	switch (arr_type) {
	case UNRESTRICTED:
		break_glob();
		break;
	case CHUNK_PER_ELEMENT:
		break_glob();
		if (array_glob_size > 0)
			make_glob();
		break;
	case BOUNDED_GLOBBABLE:
		if (glob_size + array_glob_size > max_glob_size)
			break_glob();
		if (array_glob_size > 0)
			make_glob();
		break;
	}
	
	int orig_glob_size = glob_size;
	
	/* m/u the array, looking for optimizations. */
	int did_what = mu_array_helper(this, array_expr, array_ctype, aalloc,
				       elem_ctype, elem_itype, elem_map,
				       len_expr, len_ctype, cname,
				       len_min, len_max);
	assert(bcopy_poss || !(did_what & DID_OPTIMIZE));
	
	/* It is possible that we produce an alignment inside of a loop, in
	   which case, we need to make the glob size big enough to handle
	   the alignment on each iteration. */
	if ((array_glob_size < sub->glob_size * len_max)
	    && (did_what & ~DID_OPTIMIZE)) {
		elem_size = sub->glob_size;
		array_glob_size = elem_size * len_max;
	}
	
	switch (arr_type) {
	case UNRESTRICTED:
		break_glob();
		max_msg_size = MAXUINT_MAX;
		break;
		
	case CHUNK_PER_ELEMENT:
	{
		int arr_size;
		assert(glob_size_expr);
		cast_expr *orig_glob_size_expr = glob_size_expr;
		unsigned int orig_glob_size = glob_size;
		break_glob();
		// we change the glob expression to be the real glob expression
		cast_expr var_size;
		if (len_expr->kind == CAST_EXPR_LIT_PRIM &&
		    len_expr->cast_expr_u_u.lit_prim.u.kind == CAST_PRIM_INT) {
			var_size = cast_new_expr_lit_int(
				arr_size =
				(len_expr->cast_expr_u_u.lit_prim.u.
				 cast_lit_prim_u_u.i * elem_size),
				0);
			if ((did_what & DID_OPTIMIZE)
			    && orig_glob_size
			    && ((*orig_glob_size_expr)->kind
				== CAST_EXPR_LIT_PRIM)
			    && ((*orig_glob_size_expr)->cast_expr_u_u.
				lit_prim.u.kind == CAST_PRIM_INT)) {
				var_size = cast_new_expr_lit_int(
					((*orig_glob_size_expr)->cast_expr_u_u.
					 lit_prim.u.cast_lit_prim_u_u.i),
					0);
			}
			*orig_glob_size_expr = var_size;
			if ((max_msg_size + arr_size) < max_msg_size)
				/* overflow */
				max_msg_size = MAXUINT_MAX;
			else
				max_msg_size += arr_size;
		} else {
			var_size = cast_new_binary_expr(
				CAST_BINARY_MUL,
				cast_new_expr_lit_int(elem_size, 0),
				len_expr);
			if ((did_what & DID_OPTIMIZE) && orig_glob_size)
				*orig_glob_size_expr = cast_new_binary_expr(
					CAST_BINARY_ADD,
					*orig_glob_size_expr,
					var_size);
			else
				*orig_glob_size_expr = var_size;
			
			if ((max_msg_size + array_glob_size) < max_msg_size)
				/* overflow */
				max_msg_size = MAXUINT_MAX;
			else
				max_msg_size += array_glob_size;
		}
		break;
	}
	
	case BOUNDED_GLOBBABLE:
		if (did_what & DID_OPTIMIZE) {
			/* bcopy may add extra padding for alignment, but will
			   not add the array size to the glob, so do it now */
			glob_size += array_glob_size;
		} else {
			/*
			 * If our glob is gone, the original size was fine.
			 * Otherwise, we need to fix the size since non-bcopy
			 * will update the glob_size only for a single
			 * element.  What we want is the whole array.
			 */
			if (glob_size_expr) {
				glob_size = orig_glob_size + array_glob_size;
			}
		}
		break;
	}
	
	array_one_glob = old_array_one_glob;
	sub->abort_block->end();
	delete sub;
}


static inline int min(int a, int b) {
	return (a < b) ? a : b;
};

static int mu_array_helper(
	mem_mu_state *must,
	cast_expr ptr_expr,
	cast_type ptr_ctype,
	pres_c_allocation *ptr_alloc,
	cast_type target_ctype,
	mint_ref target_itype,
	pres_c_mapping target_map,
	cast_expr len_expr,
	cast_type /*len_ctype*/,
	char *cname,
	unsigned int len_min,
	unsigned int len_max)
{
	int res = 0;
	cast_expr bcopy_poss;
	int target_size;
	int target_align_bits;
	mu_msg_span *union_span = 0, *parent_span = 0;
	mu_msg_span *reg_span = 0, *bcopy_span = 0;
	int add_union = 0;
	
	/* see if a bcopy is possible for this data */
	bcopy_poss = must->mu_get_sizeof(target_itype, target_ctype,
					 target_map,
					 &target_size, &target_align_bits);
	int do_bcopy = !!bcopy_poss; // By default, if we can, we will.
	
	cast_stmt bcopy_if = 0, bcopy_else = 0, bcopy_endif = 0;
	struct mu_abort_block *mab_par, *mab_con = 0, *mab_thr = 0;
	int starting_glob_size = -1;
	
	/* We need to make some special abort stuff here because of the
	   #if/#else/#endif stuff that can be put in here. */
	mab_par = must->abort_block;
	mab_con = new mu_abort_block();
	mab_con->set_kind(MABK_CONTROL);
	mab_con->begin();
	if (bcopy_poss) {
		/* Possible optimization for a bcopy.
		   Depending on transport and byte-ordering at compile
		   time, this optimization may or may not be used */
		
		bcopy_if = must->mu_bit_translation_necessary(0, target_itype);
		assert(bcopy_if);
		assert(bcopy_if->kind == CAST_STMT_TEXT);
		
		if (strncmp("#if 0", bcopy_if->cast_stmt_u_u.text, 5) == 0) {
			/* If we're just going to spit out an #if 0, then
			   there's no reason to spit out the loop code. */
			bcopy_if = bcopy_else = bcopy_endif = 0;
		} else if (strncmp("#if 1",
				   bcopy_if->cast_stmt_u_u.text, 5) == 0) {
			/* Likewise, if we're just going to spit out an #if 1,
			   then there's no reason to spit out bcopy code. */
			bcopy_else = bcopy_endif = 0;
			do_bcopy = 0; // No, don't do the bcopy
		} else {
			/* Otherwise, prepare the compile-time check */
			bcopy_else = must->mu_bit_translation_necessary(
				1, target_itype);
			assert(bcopy_else);
			assert(bcopy_else->kind == CAST_STMT_TEXT);
			bcopy_endif = must->mu_bit_translation_necessary(
				2, target_itype);
			assert(bcopy_endif);
			assert(bcopy_endif->kind == CAST_STMT_TEXT);
		}
	}
	
	if (!do_bcopy || bcopy_if) {
		/*
		 * Always create a chunk break before and after arrays; each
		 * element must be treated as one or more separate chunks.
		 * This really is only necessary when generating loops; no
		 * loops are generated for pointers or 1-element fixed-size
		 * arrays.
		 */
		if (len_max > 1 || len_min != len_max)
			must->break_chunk();
		starting_glob_size = must->glob_size;
	}
	if (do_bcopy && bcopy_if) {
		must->add_stmt(bcopy_if);
		assert(bcopy_if->kind == CAST_STMT_TEXT);
		/* Add the bcopy_if stmt to the abort control block
		   and make a new thread block for actual abort code to
		   go into. */
		mab_con->add_stmt(bcopy_if);
		mab_thr = new mu_abort_block();
		mab_thr->set_kind(MABK_THREAD);
		mab_thr->begin();
		must->abort_block = mab_thr;
		/* insert an "#if 0" around array_iter definition */
		must->add_direct_code(
			flick_asprintf("%s\n",
				       bcopy_if->cast_stmt_u_u.text));
	}
	if( must->current_span ) {
		union_span = new mu_msg_span;
		union_span->set_kind(MSK_UNION);
		union_span->set_block(must->c_block);
		union_span->set_abort(must->abort_block);
		parent_span = must->current_span;
	}
	/* only do it if we have to */
	if (!do_bcopy || bcopy_if) {
		if( do_bcopy && must->current_span ) {
			union_span->begin();
			must->current_span = reg_span = new mu_msg_span;
			must->current_span->set_kind(MSK_SEQUENTIAL);
			must->current_span->set_block(must->c_block);
			must->current_span->set_abort(must->abort_block);
			must->current_span->begin();
		}
		must->mu_state::mu_array(ptr_expr, ptr_ctype,
					 target_ctype, target_itype,
					 target_map, cname);
		res |= DID_LOOP;
		
		if (do_bcopy)
			/* to ensure we end up in the same chunking
			   state as the bcopy, make sure the chunk
			   is broken here */
			must->break_chunk();
		if (bcopy_poss) {
			/* set the glob size back to what it was
			   before we m/u-ed the array
			   (this ensures the correct glob size) */
			assert(starting_glob_size >= 0);
			must->glob_size = starting_glob_size;
		}
		if( do_bcopy && must->current_span ) {
			must->current_span->end();
			add_union++;
			must->current_span = parent_span;
		}
	}
	
	if (do_bcopy &&
	    (must->op & (MUST_ENCODE | MUST_DECODE))) {
		if (bcopy_else) {
			must->add_stmt(bcopy_else);
			/* Add a thread block for the #else */
			mab_thr->end();
			mab_con->add_child(mab_thr, MABF_INLINE);
			mab_con->add_stmt(bcopy_else);
			mab_thr = new mu_abort_block();
			mab_thr->set_kind(MABK_THREAD);
			mab_thr->begin();
			must->abort_block = mab_thr;
			assert(bcopy_endif);
			assert(bcopy_endif->kind == CAST_STMT_TEXT);
			/* insert a "#else" after array_iter definition */
			must->add_direct_code(
				flick_asprintf(
					"%s\n",
					bcopy_else->cast_stmt_u_u.text));
		}
		
		/*
		 * Bounds check the array, but only for *real* arrays.
		 * Normally, this is taken care of by mu_array(), but
		 * bcopy/msgptr optimization never sees that code.
		 */
		if (must->array_data.is_valid)
			must->mu_array_check_bounds(cname);
		
		/*
		 * If we're decoding a variable-length array, we probably just
		 * unmarshaled the length, and thus can't use the same chunk we
		 * are in (doing so would mean we use the array length to start
		 * the same chunk in which we decode the length!).
		 */
		if ((len_min != len_max) &&
		    (must->op & MUST_DECODE) &&
		    (must->chunk_size > 0))
			must->break_chunk();
		
		cast_expr size_expr;
		if (len_expr->kind == CAST_EXPR_LIT_PRIM &&
		    len_expr->cast_expr_u_u.lit_prim.u.kind == CAST_PRIM_INT) {
			size_expr = cast_new_expr_lit_int(
				target_size *
				(len_expr->cast_expr_u_u.lit_prim.u.
				 cast_lit_prim_u_u.i),
				0);
		} else {
			size_expr = cast_new_binary_expr(CAST_BINARY_MUL,
							 bcopy_poss,
							 len_expr);
			if( must->current_span ) {
				if( !add_union )
					union_span->begin();
				bcopy_span = new mu_msg_span;
				must->current_span = bcopy_span;
				must->current_span->set_block(must->c_block);
				must->current_span->set_abort(must->
							      abort_block);
				must->current_span->begin();
				must->current_span->set_size(size_expr);
				must->current_span->end();
				add_union++;
				must->current_span = parent_span;
			}
		}
		
		/* align ourselves for the upcoming data */
		assert(target_size >= 0);
		assert(target_align_bits >= 0);
		int offset = must->chunk_prim(target_align_bits, 0);
		cast_expr ofs_expr = cast_new_expr_lit_int(offset, 0);
		
		pres_c_alloc_flags ptr_alloc_flags
			= must->get_allocator_flags(ptr_alloc);
		if ((must->op & MUST_DECODE)
		    /*
		     * ...and this object is `in'.  (XXX --- Bogus check.  What
		     * we really want to know is that the receiver won't try to
		     * modify or reallocate the object.)
		     */
		    && (must->current_param_dir == PRES_C_DIRECTION_IN)
		    /*
		     * ...and there's no special allocator
		     */
		    && (must->get_allocator_kind(ptr_alloc).kind
			== PRES_C_ALLOCATOR_DONTCARE)
		    /*
		     * ...and we are responsible for allocating the object.
		     */
		    && ((ptr_alloc_flags & PRES_C_ALLOC_EVER)
			!= PRES_C_ALLOC_NEVER)
		    /*
		     * ...and we are responsible for later deallocating the
		     * object, too.
		     */
		    && ((ptr_alloc_flags & PRES_C_DEALLOC_EVER)
			== PRES_C_DEALLOC_ALWAYS)
		    /*
		     * ...and the presentation is through a pointer.
		     */
		    && (ptr_ctype->kind == CAST_TYPE_POINTER)) {
		       	/* Optimization for a pointer into the
			   message buffer */
			must->mu_array_do_msgptr(ofs_expr, ptr_expr,
						 ptr_ctype, target_ctype,
						 len_expr, size_expr, cname);
			res |= DID_MPTR;
		} else {
			must->mu_array_do_bcopy(ofs_expr, ptr_expr,
						ptr_ctype, target_ctype,
						len_expr, size_expr, cname);
			res |= DID_BCOPY;
		}
		
		/* figure out what we know about the alignment at this point.
		   (This is normally taken care of by mu_array_elem(),
		   but the bcopy/msgptr optimizations sometimes
		   eliminate the need to ever run it, so do it now) */
		
		if (len_min == len_max) {
			/* constant length, so we can calculate
			   the exact size of the whole array */
			int mask = (1 << must->align_bits) - 1;
			int ofs = target_size * len_max + must->align_ofs;
			must->align_ofs = ofs & mask;
			must->chunk_size = offset + target_size * len_max;
		} else {
			/* variable length, but we know the
			   element size, so judge from that */
			int bits = 0;
			while ((target_size & (1 << bits)) == 0)
				bits++;
			must->align_bits = min(bits, must->align_bits);
			must->align_ofs &= ((1 << must->align_bits) - 1);
			cast_expr *chunk = must->chunk_size_expr;
			must->break_chunk();
			/* add to current chunk size if > 0 */
			if (offset > 0)
				*chunk = cast_new_binary_expr(CAST_BINARY_ADD,
							      *chunk,
							      size_expr);
			else
				*chunk = size_expr;
		}
		
		/*
		 * Terminate the array, but only for *real* arrays.
		 * Normally, this is taken care of by mu_array(), but
		 * bcopy/msgptr optimization never sees that code.
		 */
		if (must->array_data.is_valid)
			must->mu_array_terminate(ptr_expr, target_ctype,
						 cname);
		
		if (bcopy_endif) {
			/* have to break the chunk here so we can be
			   in the same chunking state as the other
			   case of the #if */
			must->break_chunk();
			must->add_stmt(bcopy_endif);
			/* insert a "#endif" after array_iter definition */
			must->add_direct_code(
				flick_asprintf(
					"%s\n",
					bcopy_endif->cast_stmt_u_u.text));
			/* Finish the control block and add
			   it to the original parent */
			mab_thr->end();
			mab_con->add_child(mab_thr, MABF_INLINE);
			mab_con->add_stmt(bcopy_endif);
			mab_con->end();
			mab_par->add_child(mab_con, MABF_INLINE);
			must->abort_block = mab_par;
		}
	}
	if( must->current_span ) {
		must->current_span = parent_span;
		if( do_bcopy ) {
			if( reg_span ) {
				reg_span->collapse();
				reg_span->drop();
				reg_span->commit();
			}
			if( bcopy_span ) {
				union_span->add_child( bcopy_span );
				parent_span->add_child( union_span );
			} else {
				union_span->commit();
			}
		} else {
			if( reg_span )
				parent_span->add_child( reg_span );
			union_span->commit();
		}
	}
	return res;
}

void mem_mu_state::mu_array_do_bcopy(cast_expr ofs_expr,
				     cast_expr ptr_expr,
				     cast_type ptr_ctype,
				     cast_type target_ctype,
				     cast_expr /*len_expr*/,
				     cast_expr size_expr,
				     char *cname)
{
	
	/* Allocate space */
	if ((op & MUST_ALLOCATE) && (ptr_ctype->kind == CAST_TYPE_POINTER))
		mu_pointer_alloc(ptr_expr, target_ctype, cname);
	
	cast_expr macro = cast_new_expr_name(
		flick_asprintf("flick_%s_%s_bcopy",
			       get_encode_name(),
			       get_buf_name()));
	
	cast_expr call = cast_new_expr_call_3(macro, ofs_expr,
					      ptr_expr, size_expr);
	add_stmt(cast_new_stmt_expr(call));
	
	/* Deallocate space */
	if ((op & MUST_DEALLOCATE) && (ptr_ctype->kind == CAST_TYPE_POINTER))
		mu_pointer_free(ptr_expr, target_ctype, cname);
}

void mem_mu_state::mu_array_do_msgptr(cast_expr ofs_expr,
				      cast_expr ptr_expr,
				      cast_type ptr_ctype,
				      cast_type /*target_ctype*/,
				      cast_expr len_expr,
				      cast_expr /*size_expr*/,
				      char *cname)
{
	/*
	 * There is no need to allocate space here.  However, since we just
	 * subverted the pointer allocation code, we now have to handle the
	 * maximum size and ownership stuff here.
	 */
	
	/*
	 * Normally, we would honor the previous release flag here and free the
	 * buffer if necessary, but that wouldn't be correct.  First of all,
	 * we're decoding an in parameter, which means we're on the server
	 * side.  Likely it hasn't been initialized, meaning the release flag
	 * is likely some random non-zero value, and freeing the reandom buffer
	 * would be catastrophic.  Secondly, another condition for a msgptr is
	 * that we have a ``don't care'' allocator, meaning it will likely be
	 * allocated on the stack.  Thus, it wouldn't require a free anyway.
	 */
	
	cast_expr type_expr = cast_new_expr_type(ptr_ctype);
	cast_expr macro = cast_new_expr_name(
		flick_asprintf("flick_%s_%s_msgptr",
			       get_encode_name(),
			       get_buf_name()));
	
	cast_expr call = cast_new_expr_call_3(macro,
					      ofs_expr, ptr_expr, type_expr);
	add_stmt(cast_new_stmt_expr(call));
	
	/* Decide if we should set the allocated length. */
	cast_expr max_cexpr;
	cast_type max_ctype;
	int gotarg = arglist->getargs(cname, "alloc_len",
				      &max_cexpr, &max_ctype);
	assert(gotarg);
	if (max_cexpr) {
		add_stmt(cast_new_stmt_expr(cast_new_expr_assign(max_cexpr,
								 len_expr)));
	}
	
	cast_expr rel_cexpr;
	cast_type rel_ctype;
	gotarg = arglist->getargs(cname, "release", &rel_cexpr, &rel_ctype);
	assert(gotarg);
	if (rel_cexpr) {
		/*
		 * We UNset ownership for msgptr optimization, since the
		 * containing structure doesn't own the buffer (it's part of
		 * the message).
		 */
		add_stmt(cast_new_stmt_expr(cast_new_expr_assign(
			rel_cexpr,
			cast_new_expr_lit_int(0, 0))));
	}
}

/* End of file. */
