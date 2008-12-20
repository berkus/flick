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
#include <stdlib.h>
#include <string.h>

#include <mom/compiler.h>
#include <mom/libmint.h>
#include <mom/c/pfe.hh>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>
#include <mom/libaoi.h>

#include <mom/c/pg_corba.hh>

/*
 * Generate the parameter list of a client stub presentation for an AOI
 * interface operation.
 */
void pg_corba::process_client_params(
	cast_func_type *cfunc,
	stub_special_params *specials,
	mint_ref request_ref, mint_ref reply_ref,
	aoi_operation *ao,
	pres_c_inline request_l4_inl, pres_c_inline reply_l4_inl,
	pres_c_inline target_inl, pres_c_inline client_inl)
{
	int env_ref_index;
	
	pg_state::process_client_params(cfunc,
					specials,
					request_ref, reply_ref,
					ao,
					request_l4_inl, reply_l4_inl,
					target_inl, client_inl);
	
	/*
	 * Create the inline and mapping to connect the environment reference
	 * parameter to the server-side request message.  Yes, the *request*.
	 * Part of the server's job in handling a request is to allocate the
	 * CORBA environment storage.
	 *
	 * XXX --- The PG library should do this for us in some fashion.
	 */
	env_ref_index = specials->
			params[stub_special_params::environment_ref].index;
	
	if (gen_server
	    && !async_stubs
	    /*
	     * Check that the environment parameter exists.  It is always there
	     * in the standard CORBA mapping, but we allow that it might not be
	     * present in some CORBA-derived presentation.
	     */
	    && (env_ref_index > 0)) {
		pres_c_mapping alloc_map
			= pres_c_new_mapping(PRES_C_MAPPING_IGNORE);
		
		pres_c_allocation alloc;
		
		alloc.cases[PRES_C_DIRECTION_UNKNOWN].allow
			= PRES_C_ALLOCATION_ALLOW;
		alloc.cases[PRES_C_DIRECTION_UNKNOWN].pres_c_allocation_u_u.
			val.flags
			= PRES_C_ALLOC_ALWAYS | PRES_C_DEALLOC_ALWAYS;
		alloc.cases[PRES_C_DIRECTION_UNKNOWN].pres_c_allocation_u_u.
			val.allocator.kind = PRES_C_ALLOCATOR_DONTCARE;
		alloc.cases[PRES_C_DIRECTION_UNKNOWN].pres_c_allocation_u_u.
			val.alloc_init = 0;
		
		alloc.cases[PRES_C_DIRECTION_IN].allow
			= PRES_C_ALLOCATION_INVALID;
		alloc.cases[PRES_C_DIRECTION_INOUT].allow
			= PRES_C_ALLOCATION_INVALID;
		alloc.cases[PRES_C_DIRECTION_OUT].allow
			= PRES_C_ALLOCATION_INVALID;
		alloc.cases[PRES_C_DIRECTION_RETURN].allow
			= PRES_C_ALLOCATION_INVALID;
		
		/*
		 * XXX --- Instead of using MAPPING_IGNORE, we should make some
		 * kind of mapping to initialize the environment structure.  Or
		 * is it the runtime's job to init the environment for us?
		 */
		
		/*
		 * Add the allocation context.  (We operate on a bogus ctype
		 * here because the special parameter already has it correct.)
		 */
		cast_type ct = cast_new_type(CAST_TYPE_VOID);
		pres_c_interpose_indirection_pointer(&ct, &alloc_map, alloc);
		/* Tell the back end that this parameter is the environment. */
		pres_c_interpose_argument(&alloc_map, "params", "environment");
		/* Tell the back end that this is the parameter ``root''. */
		pres_c_interpose_param_root(&alloc_map, 0, 0);
		
		int inline_slot_index
			= pres_c_add_inline_struct_slot(request_l4_inl);
		
		request_l4_inl->pres_c_inline_u_u.func_params_i.slots.
			slots_val[inline_slot_index].mint_struct_slot_index
			= mint_slot_index_null;
		
		request_l4_inl->pres_c_inline_u_u.func_params_i.slots.
			slots_val[inline_slot_index].inl
			= pres_c_new_inline_atom(env_ref_index, alloc_map);
	}
	
	/*
	 * The PRES_C for handling the environment in the reply is constructed
	 * by `pg_corba::p_do_exceptional_case' and its associates.
	 *
	 * We used to connect the exception to the `reply_l4_inl' (which is
	 * used when handling normal, non-exceptional replies), but thais was
	 * done only because `mu_server_func' in the back end used to examine
	 * the `reply_l4_inl' for `out' data that required allocation.  But
	 * now we take a more sensible approach and make allocation of `out'
	 * parameters part of the *request* PRES_C, not the reply.
	 */
}

/*
 * Generate the parameter list of a server work function presentation for an
 * AOI interface operation.
 */
void pg_corba::process_server_params(
	cast_func_type *cfunc,
	stub_special_params *specials,
	mint_ref request_ref, mint_ref reply_ref,
	aoi_operation *ao,
	pres_c_inline request_l4_inl, pres_c_inline reply_l4_inl,
	pres_c_inline target_inl, pres_c_inline client_inl)
{
	/*
	 * Call the `pg_corba' version of this method!  This is important!
	 * Who knows what a derived class might do differently between client
	 * and server!
	 */
	pg_corba::process_client_params(cfunc,
					specials,
					request_ref, reply_ref,
					ao,
					request_l4_inl, reply_l4_inl,
					target_inl, client_inl);
}

/* End of file. */

