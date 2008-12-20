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

#include "mach3.h"

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
void mach3_mu_state::mu_pointer_free(cast_expr pexpr, cast_type target_type,
				     char *cname)
{
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
	
	cast_expr free_expr;
	
	if (!(op & MUST_DEALLOCATE)) /* XXX */
		return;
	
	if ((alloc->flags & PRES_C_DEALLOC_EVER) == PRES_C_DEALLOC_NEVER)
		return;
	
	/*
	 * Now we need to find the allocation length:
	 *   If we know the maximum presented length, then use it.
	 *   Otherwise, use the presented length.
	 */
	cast_expr lexpr;
	cast_type ltype;
	int gotarg = arglist->getargs(cname, "max_len", &lexpr, &ltype);
	assert(gotarg);
	if (!lexpr)
		mu_array_get_pres_length(cname, &lexpr, &ltype);
	assert(lexpr);
	
	switch (alloc->allocator.kind) {
	case PRES_C_ALLOCATOR_OUTOFLINE:
		/*
		 * The deallocation has been taken care of by the
		 * deallocate flag in the IPC message.
		 * => Do nothing, not even the default mu_pointer_free().
		 */
		break;
		
	case PRES_C_ALLOCATOR_NAME:
		
		/*
		 * For mach_vm frees, we have to pass the size.
		 * For others, just run the default mu_pointer_free()
		 */
		if (strcmp(alloc->allocator.pres_c_allocator_u.name,
			   "mach_vm") == 0) {
			
			/* Call a function to deallocate the storage. */
			const char *name = get_deallocator(palloc);
			cast_expr size_expr;
			
			if ((lexpr->kind == CAST_EXPR_LIT_PRIM)
			    && (lexpr->cast_expr_u_u.lit_prim.u.kind ==
				CAST_PRIM_INT)
			    && (lexpr->cast_expr_u_u.lit_prim.u.
				cast_lit_prim_u_u.i
				== 1)
				)
				/* Special case:
				   Don't multiply the `sizeof' by 1. */
				size_expr = cast_new_expr_sizeof_type(
					target_type);
			else
				/* General case: Multiply by `lexpr'. */
				size_expr = cast_new_binary_expr(
					CAST_BINARY_MUL,
					lexpr,
					cast_new_expr_sizeof_type(
						target_type));
			
			free_expr = cast_new_expr_call_2(
				cast_new_expr_name(name),
				pexpr,
				size_expr);
			cast_stmt free_stmt = cast_new_stmt_expr(free_expr);
			
			/* Now decide when that statement should be invoked. */
			if (alloc->flags & PRES_C_DEALLOC_ALWAYS) {
				add_stmt(free_stmt);
			}
			
			/* Nullify the pointer if requested.  */
			if (alloc->flags & PRES_C_DEALLOC_NULLIFY)
				add_stmt(cast_new_stmt_expr(
					cast_new_expr_assign(
						pexpr,
						cast_new_expr_lit_int(0, 0))
					));
			break;
		}
		/* else fall through */
		
	default:
		mu_state::mu_pointer_free(pexpr, target_type, cname);
	}
}

/* End of file. */

