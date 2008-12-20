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
 * Generate code to free the memory pointed to by a pointer variable or
 * parameter, as dictated by the deallocation semantics defined in the passed
 * `alloc' structure.
 *
 * `pexpr' is a C expression evaluating to the pointer whose memory is being
 * freed.
 *
 * `target_type' is the C type of whatever the pointer points to.  (The
 * pointer's type itself would be a CAST_TYPE_POINTER referring to that type).
 *
 * `cname' is the name of the PRES_C_INLINE_ALLOCATION_CONTEXT node that is
 * associated with this pointer.
 */
void mu_state::mu_pointer_free(cast_expr pexpr, cast_type target_type,
			       char *cname)
{
	cast_expr free_expr;
	
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
	
	if (!(op & MUST_DEALLOCATE)) /* XXX */
		return;
	
	if ((alloc->flags & PRES_C_DEALLOC_EVER) == PRES_C_DEALLOC_NEVER)
		return;
	
	unsigned int len_min, len_max;
	cast_expr min_expr, max_expr, length_expr;
	mu_array_get_pres_bounds(&len_min, &min_expr, &len_max, &max_expr,
				 cname);
	length_expr = get_allocation_length(cname, palloc);
	
	/*
	 * Generate a statement to deallocate the storage pointed to by the
	 * pointer.
	 *
	 * XXX --- We ought to replace this `switch' with some kind of
	 * allocator method dispatch function.
	 */
	switch (alloc->allocator.kind) {
	case PRES_C_ALLOCATOR_STATIC:
	case PRES_C_ALLOCATOR_DONTCARE:
		/* Nothing ever needs to be done. */
		return;
	case PRES_C_ALLOCATOR_OUTOFLINE:
	case PRES_C_ALLOCATOR_NAME: {
		if( alloc->flags & PRES_C_RUN_DTOR ) {
			int is_array = !((length_expr->kind ==
					  CAST_EXPR_LIT_PRIM) &&
					 (length_expr->cast_expr_u_u.
					  lit_prim.u.kind == CAST_PRIM_INT) &&
					 (length_expr->cast_expr_u_u.lit_prim.
					  u.cast_lit_prim_u_u.i == 1));
			free_expr = cast_new_expr_op_delete(is_array, pexpr);
		}
		else {
			/* Call a function to deallocate the storage. */
			const char *name = get_deallocator(palloc);
			
			free_expr = cast_new_expr_call_1(cast_new_expr_name(name),
							 pexpr);
		}
		/* Nullify the pointer expr if this is an inout/out
		   parameter.  This is needed so the abort code won't
		   try and free something twice. */
		if( (current_param_dir == PRES_C_DIRECTION_INOUT) ||
		    (current_param_dir == PRES_C_DIRECTION_OUT) ||
		    (current_param_dir == PRES_C_DIRECTION_RETURN) )
			alloc->flags |= PRES_C_DEALLOC_NULLIFY;
		break;
	}
	
	default:
		panic(("In `mu_state::mu_pointer_free', "
		       "unknown allocator kind (%d)."),
		      alloc->allocator.kind);
	}
	
	cast_stmt free_stmt = cast_new_stmt_expr(free_expr);
	
	/* Decide if we should set the allocated length. */
	cast_stmt set_alen_stmt = 0;
	cast_expr alen_cexpr;
	cast_type alen_ctype;
	int gotarg = arglist->getargs(cname, "alloc_len",
				      &alen_cexpr, &alen_ctype);
	assert(gotarg);
	if (alen_cexpr) {
		set_alen_stmt = cast_new_stmt_expr(
			cast_new_expr_assign(alen_cexpr,
					     cast_new_expr_lit_int(0, 0)));
	}
	
	/* Decide if we should honor/set ownership (release flag). */
	cast_stmt release_stmt = 0;
	cast_expr rel_cexpr;
	cast_type rel_ctype;
	gotarg = arglist->getargs(cname, "release", &rel_cexpr, &rel_ctype);
	assert(gotarg);
	if (rel_cexpr) {
		free_stmt = cast_new_if(rel_cexpr, free_stmt, 0);
		release_stmt = cast_new_stmt_expr(
		    cast_new_expr_assign(rel_cexpr,
					 cast_new_expr_lit_int(0, 0)));
	}
	
	/* Now decide when that statement should be invoked. */
	if (alloc->flags & PRES_C_DEALLOC_ALWAYS) {
		add_stmt(free_stmt);
		if (set_alen_stmt) add_stmt(set_alen_stmt);
		if (release_stmt) add_stmt(release_stmt);
		
		/* Nullify the pointer if requested.  */
		if (alloc->flags & PRES_C_DEALLOC_NULLIFY)
			add_stmt(cast_new_stmt_expr(
				cast_new_expr_assign(
					pexpr, 
					cast_new_expr_cast(cast_new_expr_lit_int(0, 0),
							   cast_new_pointer_type(target_type)))));
	} else {
		/* We currently don't have any conditional dealloc cases.
		   This is where they would go if we did. */
		panic(("In mu_pointer_free(): "
		       "Conditional dealloc not handled!"));
	}
}

/*
 * This helper function simply returns the name of the deallocator function for
 * a given `pres_c_allocation'.
 */
const char *mu_state::get_deallocator(pres_c_allocation *palloc)
{
	/* Find the appropriate allocation case, and make sure it's valid. */
	pres_c_allocation_u *ualloc = &palloc->cases[current_param_dir];
	assert(ualloc->allow != PRES_C_ALLOCATION_INVALID);
	pres_c_allocation_case *alloc = &(ualloc->pres_c_allocation_u_u.val);
	
	if (alloc->flags & PRES_C_DEALLOC_ALWAYS)
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
			panic(("In `mu_state::get_deallocator', "
			       "unknown allocator kind (%d)."),
			      alloc->allocator.kind);
			break;
		}
	
	return "null_flick_free";
}

/*
 * This helper function simply returns the appropriate (de)allocator for
 * a given `pres_c_allocation' (as opposed to the deallocator _function_).
 */
pres_c_allocator mu_state::get_deallocator_kind(pres_c_allocation *alloc)
{
	assert(alloc->cases[current_param_dir].allow
	       == PRES_C_ALLOCATION_ALLOW);
	return alloc->cases[current_param_dir].pres_c_allocation_u_u.val.
		allocator;
}

/*
 * This helper function simply returns the appropriate deallocation flags for
 * a given `pres_c_allocation'.
 */
pres_c_alloc_flags mu_state::get_deallocator_flags(pres_c_allocation *alloc)
{
	assert(alloc->cases[current_param_dir].allow
	       == PRES_C_ALLOCATION_ALLOW);
	return alloc->cases[current_param_dir].pres_c_allocation_u_u.val.flags;
}

/* End of file. */

