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

#include <mom/compiler.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>
#include <mom/c/pbe.hh>

/*
 * This routine "inlines" a function call to a separate marshal/unmarshal stub,
 * if it is discovered that a PRES_C_MAPPING_STUB node can or should be inlined
 * (see `mu_state::mu_mapping_stub').
 *
 * The code inlining is done by calling the generic `pres_descend_mapping_stub'
 * routine, provided by libpres_c, to translate the "reference" ctype and
 * mapping (probably a CAST_TYPE_NAME and a PRES_C_MAPPING_STUB, respectively)
 * to the "actual" ctype and mapping they represent, by digging into the target
 * stub's definition.
 * 
 * How this happens is not something you want to know about; but once it's done
 * all we need to do is replace the old ctype and mapping with the new ones and
 * keep descending.
 */

void mu_state::mu_mapping_stub_inline(cast_expr expr,
				      cast_type ctype,
				      mint_ref itype,
				      pres_c_mapping map)
{
	assert(map->kind == PRES_C_MAPPING_STUB);
	// assert((op & MUST_ENCODE) || (op & MUST_DECODE));
	if (!((op & MUST_ENCODE) || (op & MUST_DECODE))) {
#if 0
		warn("Ignoring op %d in `mu_mapping_stub_inline'.", op);
#endif
		return;
	}
	
	/* XXX encode/decode? */
	
	/*
	 * XXX --- Start of the ``array slice hack,'' part 1.
	 *
	 * The code from here up to the call to `pres_descend_mapping_stub' is
	 * a hack for handling CORBA-style ``array slice'' presentations, in
	 * which an array type is presented as a pointer to the array's first-
	 * level element type.  (E.g., `int [5]' presented as `int *', and
	 * `int [4][5]' presented as `(int [5]) *').  This code also comes into
	 * play when `mu_server_func' transforms array parameter types into
	 * pointer-to-element types.
	 *
	 * The hack here is that we want to call the marshal stub associated
	 * with the array type, but the presented CAST type doesn't exactly
	 * match.  We have to ``promote'' our pointer type to the appropriate
	 * named array type in order for `pres_descend_mapping_stub' to work.
	 */
	cast_type ctype_temp = ctype;
	cast_type ctype_orig = 0;
	
	int is_pointer_to_array_slice = 0;
	
	while (ctype_temp->kind == CAST_TYPE_QUALIFIED)
		ctype_temp = ctype_temp->cast_type_u_u.qualified.actual;
	
	if (ctype_temp->kind == CAST_TYPE_POINTER) {
		/*
		 * Save our original-but-unqualified C type; we need this in
		 * order to get back on track later on.  Certain things, e.g.,
		 * `mu_mapping_fixed_array' care whether we are working with a
		 * pointer type or an array type.
		 */
		ctype_orig = ctype_temp;
		
		/*
		 * Find the marshal stub corresponding to the current MINT
		 * type (itype).  Note that `pres_c_find_mu_stub' doesn't
		 * examine the given `ctype' --- good thing, since we need to
		 * change it below (in order for `pres_descend_mapping_stub' to
		 * work).
		 */
		int stub_num = pres_c_find_mu_stub(pres, itype,
						   ctype_temp, map,
						   PRES_C_MARSHAL_STUB);
		pres_c_marshal_stub *mstub;
		cast_def *cfunc;
		
		if (stub_num < 0)
			panic("In `mu_state::mu_mapping_stub_inline', "
			      "can't find marshal stub for MINT type %d",
			      itype);
		
		mstub = &(pres->stubs.stubs_val[stub_num].pres_c_stub_u.
			  mstub);
		
		/*
		 * Check that the stub implements a fixed array mapping.
		 *
		 * XXX - We can't assert this easily anymore, since the
		 * allocation context is likely interposed here.  We'd
		 * have to dig pretty far just to see if it's a fixed
		 * array.  We'll just take it on faith at this point.
		 *
		assert(mstub->seethru_map->kind == PRES_C_MAPPING_FIXED_ARRAY);
		 */
		
		/* Get a pointer to the marshal function definition. */
		cfunc = &(pres->stubs_cast.cast_scope_val[mstub->c_func]);
		
		/*
		 * Get the type of the argument at index 1.  After we strip
		 * away the (known) pointer and any qualifiers, `ctype_temp'
		 * will be a named type, naming the array type.
		 *
		 * XXX --- Yes, we should dig through the stub's inline to find
		 * the index of the parameter that is to be marshaled, but
		 * we're already inside a hack.  Another hack can't hurt much.
		 */
		ctype_temp = cfunc->u.cast_def_u_u.func_type.
			     params.params_val[1].type;
		
		/* Dig down through the top-level pointer. */
		assert(ctype_temp->kind == CAST_TYPE_POINTER);
		ctype_temp = ctype_temp->cast_type_u_u.pointer_type.target;
		
		/* Dig down through any qualifiers. */
		while (ctype_temp->kind == CAST_TYPE_QUALIFIED)
			ctype_temp = ctype_temp->cast_type_u_u.qualified.
				     actual;
		
		/*
		 * Set `ctype' to the named type.  `pres_descend_mapping_stub'
		 * will effectively un-`typedef' the name later on, leaving us
		 * with the basic array type.
		 */
		ctype = ctype_temp;
		
		/*
		 * Remember that we are processing an array slice, so that we
		 * can make further hacks below.
		 */
		is_pointer_to_array_slice = 1;
	}
	/* XXX --- End of the ``array slice hack,'' part 1. */
	
	/* Descend into the stub and continue traversing the mapping tree. */
	pres_descend_mapping_stub(pres, itype, &ctype, &map);
	
	/*
	 * XXX --- Start of the ``array slice hack,'' part 2.
	 *
	 * First, if we had a pointer type when we came in, then get a pointer
	 * type again.  `mu_mapping_fixed_array' cares about the differences
	 * between arrays and pointers.
	 *
	 * Second, if we had a pointer type, and we are constructing a server
	 * dispatch function, assume that the current CAST expression refers
	 * to an operation parameter.  Munge the mapping for `in' and `inout'
	 * parameters so that their storage will be auto-allocated.  The map
	 * created by `p_fixed_array_type' always specifies some non-auto
	 * allocator, because it can't make different policies for different
	 * uses of a type.  (We used to munge the allocation flags, too, but
	 * now those are set correctly the the presentation generator.)
	 *
	 * It is probably a bad assumption to assume that only top-level
	 * parameter types get munged between array types and pointer types,
	 * but this is true for all currently implemented presentation styles.
	 */
	if (is_pointer_to_array_slice
	    && (ctype_orig->kind == CAST_TYPE_POINTER)) {
		assert(ctype->kind == CAST_TYPE_ARRAY);
		ctype = cast_new_pointer_type(ctype->cast_type_u_u.array_type.
					      element_type);
	}
#if 0
	/*
	 * XXX --- We no longer need to "fix" the allocator.  The presentation
	 * generator now specifies different policies for different uses of
	 * a type, and thus it defaults to DONTCARE (`auto') appropriately.
	 */
	if (is_pointer_to_array_slice
	    && (ctype->kind == CAST_TYPE_POINTER)
	    && (!strcmp(which_stub, "server"))
	    && (current_param_dir != PRES_C_DIRECTION_RETURN)
	    && (current_param_dir != PRES_C_DIRECTION_UNKNOWN)) {
		/*
		 * Construct a fixed array mapping with `auto' allocation.
		 * Trust that the presentation generator set the allocation
		 * flags correctly.
		 */
		pres_c_mapping new_map
			= pres_c_new_mapping(PRES_C_MAPPING_FIXED_ARRAY);
		
		*new_map = *map;
		new_map->pres_c_mapping_u_u.fixed_array.alloc.allocator
			= "auto";
		
		map = new_map;
	}
#endif
#if 0
	/*
	 * XXX --- We used to do this, before the presentation generator was
	 * changed to emit the correct allocation flags.
	 */
	/*
	 * The function `p_mapping_fixed_array' in the presentation generator
	 * library always specifies (PRES_C_ALLOC_NEVER | PRES_C_DEALLOC_NEVER)
	 * allocation semantics, because it can't specify different policies
	 * for different *uses* of a single type.  So here we fix the flags,
	 * based on the current parameter direction and stub kind.
	 *
	 * Note that this code is similar to what you would find in the various
	 * `p_param_type' methods in the presentation generators.  But those
	 * methods can't change the allocation flags like we can, because they
	 * can't ``reach inside'' a stub mapping for a named type.  We can, by
	 * returning a mapping that is slightly different from the mapping
	 * found by `pres_descend_mapping_stub'.
	 */
	if (is_pointer_to_array_slice
	    && ((current_param_dir == PRES_C_DIRECTION_OUT)
		|| (current_param_dir == PRES_C_DIRECTION_RETURN))) {
		/*
		 * Construct a fixed array mapping with the flags we need for
		 * an `out' or `return' parameter.  Copy the `seethru_map' that
		 * we got from the stub, but change the pointer allocation.
		 */
		pres_c_mapping new_map
			= pres_c_new_mapping(PRES_C_MAPPING_FIXED_ARRAY);
		
		*new_map = *map;
		if (!strcmp(which_stub, "client")) {
			new_map->pres_c_mapping_u_u.fixed_array.alloc.flags
				= (PRES_C_ALLOC_ALWAYS | PRES_C_DEALLOC_NEVER);
		} else if (!strcmp(which_stub, "server")) {
			new_map->pres_c_mapping_u_u.fixed_array.alloc.flags
				= (PRES_C_ALLOC_NEVER | PRES_C_DEALLOC_ALWAYS);
		} else
			panic("In `mu_state::mu_mapping_stub_inline', "
			      "generating neither client stub nor server "
			      "skeleton.");
		
		map = new_map;
	}
#endif
	/* XXX --- End of the ``array slice hack,'' part 2. */
	
	mu_mapping(expr, ctype, itype, map);
}

/* End of file. */

