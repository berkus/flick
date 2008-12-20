/*
 * Copyright (c) 1999 The University of Utah and
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

/*
 * This routine handles an allocation context.
 */
void mu_state::mu_inline_allocation_context(inline_state *ist,
					    mint_ref itype,
					    pres_c_inline inl)
{
	/* Set the default length type to be an unsigned 32-bit integer. */
	mint_ref lenref = pres->mint.standard_refs.unsigned32_ref;
	mint_ref boolref = pres->mint.standard_refs.bool_ref;
	/* Set the default element type to be the current itype. */
	mint_ref elemref = itype;
	pres_c_inline_allocation_context *acinl
		= &(inl->pres_c_inline_u_u.acontext);
	
	/* Get the length of arrays. */
	assert(itype >= 0);
	assert(itype < (signed int) pres->mint.defs.defs_len);
	mint_def *idef = &(pres->mint.defs.defs_val[itype]);

	/* For arrays, we can determine the length and element itypes. */
	if (idef->kind == MINT_ARRAY) {
		lenref = idef->mint_def_u.array_def.length_type;
		elemref = idef->mint_def_u.array_def.element_type;
	}
	/* For MINT_VOIDs, we are strictly allocating or deallocating.
	   All refs we pass down will also be MINT_VOIDs. */
	if (idef->kind == MINT_VOID) {
		lenref = boolref = elemref = pres->mint.standard_refs.void_ref;
	}
	
	/* Setup an arglist for this context, named after the context. */
	mu_state_arglist *oldlist = arglist;
	arglist = new mu_state_arglist(acinl->arglist_name, oldlist);
	arglist->add(acinl->arglist_name,
		     "offset");    /* a.k.a. the first element of the array */
	arglist->add(acinl->arglist_name,
		     "length");    /* a.k.a. the # of items in the array */
	arglist->add(acinl->arglist_name,
		     "min_len");   /* a.k.a. the hard minimum length */
	arglist->add(acinl->arglist_name,
		     "max_len");   /* a.k.a. the hard maximum length */
	arglist->add(acinl->arglist_name,
		     "alloc_len"); /* a.k.a. the allocated length */
	arglist->add(acinl->arglist_name,
		     "min_alloc_len"); /* a.k.a. the min allocated length */
	arglist->add(acinl->arglist_name,
		     "max_alloc_len"); /* a.k.a. the max allocated length */
	arglist->add(acinl->arglist_name,
		     "release");   /* a.k.a. the owned buffer release flag */
	arglist->add(acinl->arglist_name,
		     "terminator");/* a.k.a. the buffer's termination value */
	arglist->add(acinl->arglist_name,
		     "mustcopy");  /* a.k.a. the must-copy-to-keep flag */
	
	/*
	 * Here we set up the `inline_alloc_context' to communicate more
	 * semantic allocation information down to mu_pointer_alloc() and
	 * mu_pointer_dealloc() (eventually called from one of our child nodes,
	 * such as a mapping_internal_array or mapping_pointer).
	 */
	mu_inline_alloc_context ac;
	ac.name = acinl->arglist_name;
	ac.overwrite = acinl->overwrite;
	ac.owner = acinl->owner;
	ac.alloc = &acinl->alloc;
	ac.parent_context = inline_alloc_context;
	inline_alloc_context = &ac;
	
	/*
	 * Here we (optionally) marshal/unmarshal the inlines within the
	 * allocation context.  Each inline that exists should contain an
	 * appropriate PRES_C_MAPPING_ARGUMENT, so the environment we just set
	 * up can be filled in and provide an ``environment'' or ``context''
	 * for the sub inline below us (usually an array of some sort).  This
	 * allows the allocation semantics to have more complete knowledge of
	 * what's really going on.
	 *
	 * Note: The length is not an optional inline; it must ALWAYS exist.
	 */
	cast_expr cexpr = 0;
	cast_type ctype = 0;
	
	/* Set the flag to indicate we're handling length entities. */
	ac.is_length = 1;
	
	if (acinl->offset) {
		mu_inline(ist, lenref, acinl->offset);
		/* Make sure we defined the argument. */
		assert(arglist->getargs(acinl->arglist_name, "length",
					&cexpr, &ctype) && cexpr && ctype);
	}
	assert(acinl->length);
	mu_inline(ist, lenref, acinl->length);
	/* Make sure we defined the argument. */
	assert(arglist->getargs(acinl->arglist_name, "length",
				&cexpr, &ctype) && cexpr && ctype);
	if (acinl->min_len) {
		mu_inline(ist, lenref, acinl->min_len);
		/* Make sure we defined the argument. */
		assert(arglist->getargs(acinl->arglist_name, "min_len",
					&cexpr, &ctype) && cexpr && ctype);
	}
	if (acinl->max_len) {
		mu_inline(ist, lenref, acinl->max_len);
		/* Make sure we defined the argument. */
		assert(arglist->getargs(acinl->arglist_name, "max_len",
					&cexpr, &ctype) && cexpr && ctype);
	}
	if (acinl->alloc_len) {
		mu_inline(ist, lenref, acinl->alloc_len);
		/* Make sure we defined the argument. */
		assert(arglist->getargs(acinl->arglist_name, "alloc_len",
					&cexpr, &ctype) && cexpr && ctype);
	}
	if (acinl->min_alloc_len) {
		mu_inline(ist, lenref, acinl->min_alloc_len);
		/* Make sure we defined the argument. */
		assert(arglist->getargs(acinl->arglist_name, "min_alloc_len",
					&cexpr, &ctype) && cexpr && ctype);
	}
	if (acinl->max_alloc_len) {
		mu_inline(ist, lenref, acinl->max_alloc_len);
		/* Make sure we defined the argument. */
		assert(arglist->getargs(acinl->arglist_name, "max_alloc_len",
					&cexpr, &ctype) && cexpr && ctype);
	}
	
	/* Done handling length entities, reset the flag. */
	ac.is_length = 0;
	
	if (acinl->release) {
		mu_inline(ist, boolref, acinl->release);
		/* Make sure we defined the argument. */
		assert(arglist->getargs(acinl->arglist_name, "release",
					&cexpr, &ctype) && cexpr && ctype);
	}
	if (acinl->terminator) {
		mu_inline(ist, elemref, acinl->terminator);
		/* Make sure we defined the argument. */
		assert(arglist->getargs(acinl->arglist_name, "terminator",
					&cexpr, &ctype) && cexpr && ctype);
	}
	if (acinl->mustcopy) {
		mu_inline(ist, boolref, acinl->mustcopy);
		/* Make sure we defined the argument. */
		assert(arglist->getargs(acinl->arglist_name, "mustcopy",
					&cexpr, &ctype) && cexpr && ctype);
	}
	
	/* Marshal/unmarshal the array/pointer contents. */
	mu_inline(ist, itype, acinl->ptr);
	
	/*
	 * Restore the old `inline_alloc_context' and old `arglist'.
	 */
	assert(inline_alloc_context == &ac);
	inline_alloc_context = ac.parent_context;
	delete arglist;
	arglist = oldlist;
}

/* End of file. */

