/*
 * Copyright (c) 1998, 1999 The University of Utah and
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
#include <string.h>

#include <mom/compiler.h>
#include <mom/libaoi.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include "pg_fluke.hh"

/*
 * The Fluke PG version of `p_variable_array_type' looks for sequences with a
 * certain magic name (stored in `fdev_seq_type_name') and gives them a special
 * presentation.
 *
 * The special presentation differs from the normal sequence presentation in
 * that (1) the `_buffer' is alloced/freed with a special allocation function
 * and (2) the `_buffer' of an `in' sequence will not be freed by a server
 * dispatch function when the invoked server work function exits (i.e., the
 * server may hold a reference to the buffer data indefinitely and must free
 * the buffer itself).
 *
 * Coincidentally, the Fluke BE will use ``gathering'' to marshal instances of
 * the special sequence type; see `c/pbe/fluke/mu_array.cc'.
 *
 * XXX --- All of this magic exists for the benefit of KVM's user-mode device
 * driver work.  When Flick has a real presentation modification facility, KVM
 * can use that to get all the special sequence handling he needs.  But until
 * then, we hack!
 */
void pg_fluke::p_variable_array_type(aoi_array *array,
				     p_type_collection **out_ptc)
{
	pres_c_mapping map;
	p_type_node *ptn;
	cast_type ctype;
	pres_c_inline_allocation_context *iac;
	
	/* Do the normal CORBA thing... */
	pg_corba::p_variable_array_type(array, out_ptc);
	
	/*
	 * CORBA specifies that bounded sequences and strings must always be
	 * allocated to the maximum size, regardless of the actual size (see
	 * sections 17.11 & 17.12 of the CORBA 2.2 manual).  For Fluke, we undo
	 * this limitation, since this can waste a lot of space with little
	 * benefit.
	 */
	ptn = (*out_ptc)->find_type("definition");
	ctype = ptn->get_type();
	/*
	 * If there is a collection_ref, then ptn's mapping is just a stub.
	 * What we want is the real mapping.
	 */
	if ((*out_ptc)->get_collection_ref())
		map =(*out_ptc)->get_collection_ref()->
		     find_type("definition")->get_mapping();
	else
		map = ptn->get_mapping();
	
	/*
	 * We can get two things out: a sequence or a string.  Sequences are
	 * structures, and thus have a mapping_struct and return to inline
	 * mode.  Strings also have to return to inline mode for the allocation
	 * context, but they do this through a mapping_singleton instead.  We
	 * figure out which one to dig through.
	 */
	assert(map);
	if (map->kind == PRES_C_MAPPING_STRUCT) {
		assert(map->pres_c_mapping_u_u.struct_i->kind
		       == PRES_C_INLINE_ALLOCATION_CONTEXT);
		iac = &map->pres_c_mapping_u_u.struct_i->pres_c_inline_u_u.
		      acontext;
	} else {
		assert(map->kind == PRES_C_MAPPING_SINGLETON);
		assert(map->pres_c_mapping_u_u.singleton.inl->kind
		       == PRES_C_INLINE_ALLOCATION_CONTEXT);
		iac = &map->pres_c_mapping_u_u.singleton.inl->
		      pres_c_inline_u_u.acontext;
	}
	
	/* Remove the restrictive minimum allocated length inline. */
	iac->min_alloc_len = 0;
	
	/* ... and return if the sequence type name is not magic. */
	if (strcmp(name, fdev_seq_type_name) != 0)
		return;

	/*********************************************************************/

	/*
	 * Modify the presentation of this sequence type as needed by KVM.  Use
	 * a special allocator to get special memory, and don't cause the
	 * server dispatch function to free an `in' instance of this type.
	 */

	pres_c_allocation *new_alloc = &iac->alloc;
	
	assert(ctype->kind == CAST_TYPE_NAME);
	
	/*
	 * Tweak the new mapping's allocation semantics.
	 * One set of flags works for both client and server.
	 * We copy the allocator name to avoid warnings about `const'ness.
	 */
	for (int i = 0; i < PRES_C_DIRECTIONS; i++) {
		if (new_alloc->cases[i].allow == PRES_C_ALLOCATION_INVALID)
			continue;
		
		new_alloc->cases[i].pres_c_allocation_u_u.val.flags
			= (PRES_C_ALLOC_ALWAYS | PRES_C_DEALLOC_NEVER);
		new_alloc->cases[i].pres_c_allocation_u_u.val.allocator.
			pres_c_allocator_u.name
			= flick_asprintf("%s", fdev_seq_alloc_name);
	}
	
	/*
	 * Finally, emit an `#include' directive to get the prototypes for the
	 * special allocator functions.
	 */
	p_emit_include_stmt(flick_asprintf("%s", fdev_seq_alloc_header), 1);
}

/* End of file. */

