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

#include <mom/c/libcast.h>
#include <mom/c/pbe.hh>

/*
 * Define the maximum length of an array that we are willing to preallocate on
 * the stack.  Essentially, this is the amount of space we are willing to waste
 * for any given array in order to avoid the overhead of a dynamic allocation.
 *
 * XXX - This number should be based on *SIZE*, not length!
 */
static const unsigned MAX_PREALLOC_LEN = 256;

/*
 * This is a utility function used to build logical-OR expressions
 * intelligently.
 *
 * To use it, initialize a `cast_expr' variable (e.g., `expr') to 0.  Each time
 * you want to add a new term to the expression, call this routine with the old
 * `expr' as `old_cond' and the new term as `new_cond', and assign the result
 * back to `expr'.  This will automatically handle the case of there being only
 * one term.  (Of course, if there are _no_ terms, then `expr' will remain null
 * --- so be careful.)
 */
cast_expr if_or(cast_expr old_cond, cast_expr new_cond)
{
	if (old_cond == 0)
		return new_cond;
	else
		return cast_new_binary_expr(CAST_BINARY_LOR,
					    old_cond, new_cond);
}

/*
 * Generate code to allocate memory to be pointed to by a pointer variable or
 * parameter, as dictated by the allocation semantics defined in the passed
 * `alloc' structure.
 *
 * `pexpr' is a C expression representing the pointer variable memory is being
 * allocated for; naturally, it must be an lvalue so that it can be assigned
 * to.
 *
 * `target_type' is the C type of whatever the pointer points to.  (The
 * pointer's type itself would be a CAST_TYPE_POINTER referring to that type.)
 *
 * `cname' is the name of the PRES_C_INLINE_ALLOCATION_CONTEXT node that is
 * associated with this pointer.
 */
void mu_state::mu_pointer_alloc(cast_expr pexpr, cast_type target_type,
				char *cname)
{
	cast_expr alloc_expr;
	int abort_stmt = 0;
	int can_fail = 1; // True if allocation can fail (default true).
	
	/* We need to find our allocation semantics established by some parent
           INLINE_ALLOCATION_CONTEXT node. */
	mu_inline_alloc_context *iac = inline_alloc_context;
	while (iac) {
		if (strcmp(iac->name, cname) == 0)
			break;
		iac = iac->parent_context;
	}
	assert(iac);
	pres_c_allocation *palloc = iac->alloc;
	
	/* Find the appropriate allocation case, and make sure it's valid. */
	pres_c_allocation_u *ualloc = &palloc->cases[current_param_dir];
	assert(ualloc->allow != PRES_C_ALLOCATION_INVALID);
	pres_c_allocation_case *alloc = &(ualloc->pres_c_allocation_u_u.val);
	
	if (!(op & MUST_ALLOCATE))
		return;
	
	if ((alloc->flags & PRES_C_ALLOC_EVER) == PRES_C_ALLOC_NEVER)
		return;
	
	/*
	 * Special case: If we would try to allocate a `void', we change the
	 * target type to something allocable --- `char' will do.
	 */
	if (target_type->kind == CAST_TYPE_VOID) {
		target_type = cast_new_type(CAST_TYPE_PRIMITIVE);
		target_type->
			cast_type_u_u.primitive_type.kind = CAST_PRIM_CHAR;
		target_type->
			cast_type_u_u.primitive_type.mod = 0;
	}
	
	unsigned int len_min, len_max;
	cast_expr min_expr, max_expr, length_expr;
	mu_array_get_pres_bounds(&len_min, &min_expr, &len_max, &max_expr,
				 cname);
	length_expr = get_allocation_length(cname, palloc);
	
	/*
	 * If there is a typecast attached to the pointer, remove it.
	 * Though gcc handles casts-as-lvalues, standard C forbids it.
	 *
	 * XXX - This is not the place to deal with invalid C code...
	 * mu_pointer_alloc() should recieve a proper lvalue expression.
	 * The problem stems from the fact that allocation happens as a side
	 * effect to normal decoding: we are requesting to decode data into
	 * the expression, which may need to be viewed through a typecast.
	 * The typecast is determined at a high level, but allocation for
	 * the data is made just before actually decoding it, using the same
	 * expression (including typecast).
	 */
	while (pexpr->kind == CAST_EXPR_CAST) {
		pexpr = pexpr->cast_expr_u_u.cast.expr;
	}
	
	/*
	 * Generate a statement to allocate storage and assign the pointer.
	 *
	 * XXX --- We ought to replace this `switch' with some kind of
	 * allocator method dispatch function.
	 */
	switch (alloc->allocator.kind) {
	case PRES_C_ALLOCATOR_STATIC: {
		
		cast_type static_target_type;
		
		/*
		 * The static allocator can allocate only positive, constant
		 * amounts of storage!
		 */
		assert(length_expr->kind
		       == CAST_EXPR_LIT_PRIM);
		assert(length_expr->cast_expr_u_u.lit_prim.u.kind
		       == CAST_PRIM_INT);
		assert(length_expr->cast_expr_u_u.lit_prim.u.cast_lit_prim_u_u.
		       i
		       > 0);
		
		/*
		 * Declare a static variable to provide the storage and assign
		 * the pointer to point to that.
		 *
		 * XXX --- It's a little goofy to have two variables, one for
		 * static storage and one to point to it.  But this is how
		 * things must be done until the back end is smarter.  To be
		 * smarter, we need to be able to determine `pexpr' here and
		 * then pass it up.  But implementing that feature correctly
		 * would seem to mean that *all* mapping functions should be
		 * able to determine their own CAST reference expressions.
		 */
		if (length_expr->cast_expr_u_u.lit_prim.u.cast_lit_prim_u_u.i
		    != 1) {
			/* `static_target_type' will be an array. */
			static_target_type = cast_new_type(CAST_TYPE_ARRAY);
			static_target_type->
				cast_type_u_u.array_type.element_type =
					target_type;
			static_target_type->
				cast_type_u_u.array_type.length =
				length_expr;
		} else
			/* `static_target_type' will be the `target_type'. */
			static_target_type = target_type;
		
		cast_expr static_var_expr = add_temp_var("storage",
							 static_target_type,
							 0,
							 CAST_SC_STATIC);
		
		if (static_target_type != target_type)
			/* `alloc_expr' is the name of the local array. */
			alloc_expr = static_var_expr;
		else
			/* `alloc_expr' is the address of the local var. */
			alloc_expr = cast_new_unary_expr(CAST_UNARY_ADDR,
							 static_var_expr);
		can_fail = 0; // Static storage can't fail to be alloced!
		break;
	}
	
	case PRES_C_ALLOCATOR_DONTCARE:
	case PRES_C_ALLOCATOR_OUTOFLINE:
		
		/*
		 * Stack allocation
		 *   (AKA `auto' or `dontcare')
		 */
		if (length_expr->kind == CAST_EXPR_LIT_PRIM) {
			
			cast_type auto_target_type;
			
			/*
			 * The auto allocator can deal with requests for
			 * variable amounts of storage, but such requests are
			 * not handled here.  Here, we must have a positive
			 * constant request.
			 */
			assert(length_expr->cast_expr_u_u.lit_prim.u.kind
			       == CAST_PRIM_INT);
			assert(length_expr->cast_expr_u_u.lit_prim.u.
			       cast_lit_prim_u_u.i
			       > 0);
			
			/*
			 * Declare an automatic variable to provide the storage
			 * and assign the pointer to point to that.
			 *
			 * XXX --- See the comment in the ``static'' case about
			 * why we must currently have two variables (one for
			 * the pointer and a second for storage) instead of
			 * just one.
			 */
			if (length_expr->cast_expr_u_u.lit_prim.u.
			    cast_lit_prim_u_u.i != 1) {
				/* `auto_target_type' will be an array. */
				auto_target_type
					= cast_new_type(CAST_TYPE_ARRAY);
				auto_target_type->
					cast_type_u_u.array_type.element_type =
					target_type;
				auto_target_type->
					cast_type_u_u.array_type.length =
					length_expr;
			} else
				/* `auto_target_type' will be `target_type'. */
				auto_target_type = target_type;
			
			cast_expr auto_var_expr
				= add_temp_var("storage",
					       auto_target_type,
					       0,
					       CAST_SC_AUTO);
			
			if (auto_target_type != target_type)
				/* `alloc_expr' is the name of the local
                                   array. */
				alloc_expr = auto_var_expr;
			else
				/* `alloc_expr' is the address of the local
                                   var. */
				alloc_expr = cast_new_unary_expr(
					CAST_UNARY_ADDR,
					auto_var_expr);
			can_fail = 0; // Stack storage can't fail!
			break;
		}
		/* else fall through (we don't know the exact length) */
		
	case PRES_C_ALLOCATOR_NAME: {
		if( alloc->flags & PRES_C_RUN_CTOR ) {
			cast_type type;
			
			if ((length_expr->kind == CAST_EXPR_LIT_PRIM)
			    && (length_expr->cast_expr_u_u.lit_prim.u.kind ==
				CAST_PRIM_INT)
			    && (length_expr->cast_expr_u_u.lit_prim.u.
				cast_lit_prim_u_u.i
				== 1)
			    )
				type = target_type;
			else
				type = cast_new_array_type(length_expr,
							   target_type);
			alloc_expr = cast_new_expr_op_new(0, type,
							  alloc->alloc_init);
		}
		else {
			cast_expr size_expr;
			/* Call a function to allocate the storage. */
			const char *name = get_allocator(palloc);
			
			if ((length_expr->kind == CAST_EXPR_LIT_PRIM)
			    && (length_expr->cast_expr_u_u.lit_prim.u.kind ==
				CAST_PRIM_INT)
			    && (length_expr->cast_expr_u_u.lit_prim.u.
				cast_lit_prim_u_u.i
				== 1)
			    )
				/* Special case:
				     Don't multiply the `sizeof' by 1. */
				size_expr = cast_new_expr_sizeof_type(
					target_type);
			else
				/* General case: Multiply by `length_expr'. */
				size_expr = cast_new_binary_expr(
					CAST_BINARY_MUL,
					length_expr,
					cast_new_expr_sizeof_type(
						target_type));
			alloc_expr = cast_new_expr_call_1(
				cast_new_expr_name(name),
				size_expr);
			if( alloc->flags & PRES_C_DEALLOC_ON_FAIL )
				abort_stmt = 1;
		}
		can_fail = 1; // Named allocators *can* fail.
		break;
	}
	
	default:
		panic("In mu_state::mu_pointer_alloc(), "
		      "Unknown allocator kind (%d)",
		      alloc->allocator.kind);
	}
	
	cast_type alloc_type = cast_new_pointer_type(target_type);
	
	alloc_expr = cast_new_expr_cast(alloc_expr, alloc_type);
	
	/* Decide if we should set the allocated length. */
	cast_stmt set_alen_stmt = 0;
	cast_expr alen_cexpr;
	cast_type alen_ctype;
	int gotarg = arglist->getargs(cname, "alloc_len",
				      &alen_cexpr, &alen_ctype);
	assert(gotarg);
	if (alen_cexpr) {
		set_alen_stmt = cast_new_stmt_expr(
			cast_new_expr_assign(alen_cexpr, length_expr));
	}
	
	/* Decide if we should honor/set ownership (release flag). */
	cast_stmt set_release_stmt = 0;
	cast_stmt release_stmt = 0;
	cast_expr rel_cexpr;
	cast_type rel_ctype;
	gotarg = arglist->getargs(cname, "release", &rel_cexpr, &rel_ctype);
	assert(gotarg);
	if (rel_cexpr) {
		/*
		 * Honor previous ownership & free the buffer if necessary.
		 * This is only necessary if we have realloc flags set (e.g. an
		 * inout parameter on the client side).  If no realloc flags
		 * are set, it is likely the buffer has not been initialized
		 * yet, so checking the release flag or freeing the buffer
		 * would have unpredictable results.  This places the
		 * responsibility on the presentation generators to correctly
		 * identify when/if reallocation should occur for each entity.
		 *
		 * XXX - THE PGs CURRENTLY DON'T SET ANY REALLOC FLAGS!
		 */
		if (alloc->flags & PRES_C_REALLOC_EVER) {
			/*
			 * This gets kinda ugly.  What we want to do is
			 * deallocate the buffer, and we would like to use
			 * mu_pointer_free() to do so.  However, that requires
			 * us to munge ``op'' and the current alloc flags to
			 * ensure deallocation (since this branch will only be
			 * taken in valid cases; we don't want to duplicate
			 * conditionals).
			 */
			mu_state_op old_op = op;
			op = MUST_DEALLOCATE;
			pres_c_alloc_flags old_flags = alloc->flags;
			alloc->flags = PRES_C_DEALLOC_ALWAYS;
			cast_stmt old_block = c_block;
			c_block = 0;
			mu_pointer_free(pexpr, target_type, cname);
			if (c_block) {
				release_stmt = c_block;
				assert(release_stmt->kind == CAST_STMT_BLOCK);
				release_stmt->cast_stmt_u_u.block.flags
					|= CAST_BLOCK_INLINE;
			}
			c_block = old_block;
			alloc->flags = old_flags;
			op = old_op;
		}
		/* Setting ownership, since we just allocated it. */
		set_release_stmt = cast_new_stmt_expr(
			cast_new_expr_assign(rel_cexpr,
					     cast_new_expr_lit_int(can_fail,
								   0)));
	}
	
	/* Construct the allocation statement. */
	cast_expr temp_alloc_var = cast_new_expr_name("_t_alloc_ptr");
	cast_stmt alloc_block_stmt = cast_new_block(0, 0);
	cast_block *alloc_block = &alloc_block_stmt->cast_stmt_u_u.block;
	if (can_fail) {
		/*
		 * We use a temporary variable to allocate storage, so we can
		 * check for allocation errors without obliterating the
		 * original pointer.  But we ONLY do this if the allocation
		 * can fail (stack allocation just doesn't fail).
		 */
		cast_block_add_var(alloc_block,
				   (temp_alloc_var->cast_expr_u_u.
				    name.cast_scoped_name_val[0].name),
				   alloc_type, 0, CAST_SC_AUTO);
		cast_block_add_stmt(alloc_block,
				    cast_new_stmt_expr(
					    cast_new_expr_assign(
						    temp_alloc_var,
						    alloc_expr)));
		cast_block_add_stmt(
			alloc_block,
			cast_new_if(
				cast_new_unary_expr(CAST_UNARY_LNOT,
						    temp_alloc_var),
				make_error(FLICK_ERROR_NO_MEMORY),
				0));
		
		if (abort_stmt)
			abort_block->add_stmt(cast_new_stmt_expr(
				cast_new_expr_call_1(
					cast_new_expr_name(
						get_abort_deallocator(palloc)),
					pexpr)));
	} else {
		/*
		 * We don't need a temporary variable for allocation, since it
		 * can't fail (must've been static or stack).
		 */
		temp_alloc_var = alloc_expr;
	}
	if (release_stmt) cast_block_add_stmt(alloc_block, release_stmt);
	cast_block_add_stmt(alloc_block,
			    cast_new_stmt_expr(
				    cast_new_expr_assign(pexpr,
							 temp_alloc_var)));
	if (set_alen_stmt) cast_block_add_stmt(alloc_block, set_alen_stmt);
	if (set_release_stmt)
		cast_block_add_stmt(alloc_block, set_release_stmt);
	
	/* Now decide when that statement should be invoked. */
	if (alloc->flags & PRES_C_ALLOC_ALWAYS) {
		/*
		 * If there was no reason for the block statement (no temp var
		 * declared), just ``absorb'' the block into the current block.
		 * There is no particular reason for this, other than it makes
		 * the code prettier.
		 */
		assert(alloc_block_stmt->kind == CAST_STMT_BLOCK);
		if (temp_alloc_var == alloc_expr) {
			absorb_stmt(alloc_block_stmt);
		} else
			add_stmt(alloc_block_stmt);
		
	} else {
		cast_expr if_test = 0;
		
		if (alloc->flags & PRES_C_ALLOC_IF_NULL)
			if_test = if_or(if_test,
					cast_new_unary_expr(CAST_UNARY_LNOT,
							    pexpr));
		if (alloc->flags & PRES_C_ALLOC_IF_TOO_SMALL)
			assert(0); /* XXX */
		if (alloc->flags & PRES_C_ALLOC_IF_TOO_LARGE)
			assert(0); /* XXX */
		
		assert(if_test != 0);
		
		add_stmt(cast_new_if(if_test, alloc_block_stmt, 0));
		
	}
}

/*
 * This helper function simply returns the name of the allocator function for
 * a given `pres_c_allocation'.
 */
const char *mu_state::get_allocator(pres_c_allocation *palloc)
{
	/* Find the appropriate allocation case, and make sure it's valid. */
	pres_c_allocation_u *ualloc = &palloc->cases[current_param_dir];
	assert(ualloc->allow != PRES_C_ALLOCATION_INVALID);
	pres_c_allocation_case *alloc = &(ualloc->pres_c_allocation_u_u.val);
	
	if (alloc->flags & PRES_C_ALLOC_ALWAYS)
		switch (alloc->allocator.kind) {
		case PRES_C_ALLOCATOR_DONTCARE:
		case PRES_C_ALLOCATOR_STATIC:
			/* Name of the default (stack) allocator. */
			return "auto_flick_alloc";
		case PRES_C_ALLOCATOR_OUTOFLINE:
			return flick_asprintf(
				"%s_flick_alloc",
				alloc->allocator.pres_c_allocator_u.ool_name);
		case PRES_C_ALLOCATOR_NAME:
			return flick_asprintf(
				"%s_flick_alloc",
				alloc->allocator.pres_c_allocator_u.name);
		default:
			panic(("In `mu_state::get_allocator', "
			       "unknown allocator kind (%d)."),
			      alloc->allocator.kind);
			break;
		}
	
	return "null_flick_alloc";
}

/*
 * This helper function simply returns the name of the allocator function for
 * a given `pres_c_allocation'.
 */
const char *mu_state::get_abort_deallocator(pres_c_allocation *palloc)
{
	/* Find the appropriate allocation case, and make sure it's valid. */
	pres_c_allocation_u *ualloc = &palloc->cases[current_param_dir];
	assert(ualloc->allow != PRES_C_ALLOCATION_INVALID);
	pres_c_allocation_case *alloc = &(ualloc->pres_c_allocation_u_u.val);
	
	if (alloc->flags & PRES_C_ALLOC_ALWAYS)
		switch (alloc->allocator.kind) {
		case PRES_C_ALLOCATOR_DONTCARE:
		case PRES_C_ALLOCATOR_STATIC:
			/* Name of the default (stack) deallocator. */
			return "auto_flick_free";
		case PRES_C_ALLOCATOR_OUTOFLINE:
			return flick_asprintf(
				"%s_flick_free",
				alloc->allocator.pres_c_allocator_u.ool_name);
		case PRES_C_ALLOCATOR_NAME:
			return flick_asprintf(
				"%s_flick_free",
				alloc->allocator.pres_c_allocator_u.name);
		default:
			panic(("In `mu_state::get_abort_deallocator', "
			       "unknown allocator kind (%d)."),
			      alloc->allocator.kind);
			break;
		}
	
	return "null_flick_free";
}

/*
 * This helper function simply returns the appropriate allocator for
 * a given `pres_c_allocation' (as opposed to the allocator _function_).
 */
pres_c_allocator mu_state::get_allocator_kind(pres_c_allocation *alloc)
{
	assert(alloc->cases[current_param_dir].allow
	       == PRES_C_ALLOCATION_ALLOW);
	return alloc->cases[current_param_dir].pres_c_allocation_u_u.val.
		allocator;
}

/*
 * This helper function simply returns the appropriate allocation flags for
 * a given `pres_c_allocation'.
 */
pres_c_alloc_flags mu_state::get_allocator_flags(pres_c_allocation *alloc)
{
	assert(alloc->cases[current_param_dir].allow
	       == PRES_C_ALLOCATION_ALLOW);
	return alloc->cases[current_param_dir].pres_c_allocation_u_u.val.flags;
}

/*
 * Find the legnth we will use to allocate or deallocate the data.
 */
cast_expr mu_state::get_allocation_length(char *cname,
					  pres_c_allocation *palloc)
{
	/* Find the appropriate allocation case, and make sure it's valid. */
	pres_c_allocation_u *ualloc = &palloc->cases[current_param_dir];
	assert(ualloc->allow != PRES_C_ALLOCATION_INVALID);
	pres_c_allocation_case *alloc = &(ualloc->pres_c_allocation_u_u.val);
	
	/*
	 * The argument `min_alloc_len' specifies the minimum allocated length
	 * for the array.  We guarantee that we will never make an allocation
	 * for fewer elements than this.  If it doesn't exist, it is assumed to
	 * be zero.
	 *
	 * The argument `max_alloc_len' specifies the maximum allocated length
	 * for the array.  We guarantee that we will never make an allocation
	 * for more elements than this.  If it doesn't exist, it is assumed to
	 * be the maximum bound of the array.
	 */
	cast_expr length_expr;
	cast_type length_type;
	cast_expr allocmin_expr;
	cast_type allocmin_type;
	cast_expr allocmax_expr;
	cast_type allocmax_type;
	cast_expr min_expr;
	cast_expr max_expr;
	mu_array_get_pres_length(cname, &length_expr, &length_type);
	
	int gotarg;
	unsigned int len_min, len_max;
	mu_array_get_pres_bounds(&len_min, &min_expr, &len_max, &max_expr,
				 cname);
	
	gotarg = arglist->getargs(cname, "min_alloc_len",
				  &allocmin_expr, &allocmin_type);
	assert(gotarg);
	gotarg = arglist->getargs(cname, "max_alloc_len",
				  &allocmax_expr, &allocmax_type);
	assert(gotarg);
	/*
	 * XXX - The `max_alloc_len' slot is not yet supported.  The exact
	 * semantic details of this value haven't been nailed down yet.
	 */
	assert(!allocmax_expr);assert(!allocmax_type);
	
	if (allocmin_expr) {
		if (max_expr
		    && cast_cmp_expr(allocmin_expr, max_expr) == 0) {
			/*
			 * Here, `min_alloc_len' is the same as the array's
			 * maximum bound.  Since the length can never be larger
			 * than the bound, we never need to allocate more than
			 * `allocmin_expr' elements.  Note, however, that it
			 * could still be a variable value.
			 */
			length_expr = allocmin_expr;
		} else {
			/*
			 * We need to figure out which length we should use.
			 */
			assert(0);
			length_expr
				= cast_new_expr_cond(
					cast_new_binary_expr(
						CAST_BINARY_LT,
						length_expr,
						allocmin_expr),
					allocmin_expr,
					length_expr);
		}
	}
	
	/* Check for possible stack allocation. */
	if (alloc->allocator.kind == PRES_C_ALLOCATOR_DONTCARE
	    || alloc->allocator.kind == PRES_C_ALLOCATOR_OUTOFLINE) {
		/*
		 * Optimization: If we have low variance in the length (for a
		 * bounded array), it's probably not bad to over-allocate some
		 * space on the stack, versus wasting time allocating the exact
		 * length required.  Non-stack allocation is not affected by
		 * this optimization.
		 *
		 * XXX - This check for MAX_PREALLOC_LEN should probably be
		 * based on *SIZE* rather than *LENGTH*; an array of 1MB
		 * structs just isn't going to fit on the stack!
		 */
		if ((len_max - len_min) <= MAX_PREALLOC_LEN) {
			assert(max_expr);
			length_expr = max_expr;
		}
	}
	
	assert(length_expr);
	return length_expr;
}

/* End of file. */

