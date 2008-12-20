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

#include <mom/compiler.h>
#include <mom/c/pfe.hh>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>
#include <assert.h>

#include "private.hh"

/*
 * Generate pres_c_inline (pres_c_inline_atom) for an operation parameter.
 */
cast_func_param pg_state::p_param(
	aoi_parameter *ap,
	int cast_param_index,			// NOT a CAST ref!
	mint_ref mr,			
	pres_c_inline request_i,
	pres_c_inline reply_i,
	int request_mint_struct_slot_index,	// NOT a MINT ref!
	int reply_mint_struct_slot_index	// NOT a MINT ref!
	)
{
	cast_type param_ctype;
	pres_c_mapping param_map;
	char *old_name;
	cast_func_param res;
	
	/* Some initial assertions and local variable assignments. */
	assert(request_i->kind == PRES_C_INLINE_FUNC_PARAMS_STRUCT);
	assert(reply_i->kind == PRES_C_INLINE_FUNC_PARAMS_STRUCT);
	
	/* Save the original name context.  */
	old_name = name;
	name = calc_stub_param_name(ap->name);
	
	/* Set the parameter name. */
	res.name = name;
	
	/* Set the parameter type. */
	p_param_type(ap->type, mr, ap->direction, &param_ctype, &param_map);
	res.type = param_ctype;
	res.spec = 0;
	res.default_value = 0;
	
	/* Add the parameter to the request inline struct if appropriate. */
	if ((ap->direction == AOI_DIR_IN) ||
	    (ap->direction == AOI_DIR_INOUT)) {
		int inline_slot_index
			= pres_c_add_inline_struct_slot(request_i);
		
		request_i->pres_c_inline_u_u.func_params_i.slots.
			slots_val[inline_slot_index].
			mint_struct_slot_index
			= request_mint_struct_slot_index;
		
		request_i->pres_c_inline_u_u.func_params_i.slots.
			slots_val[inline_slot_index].
			inl
			= pres_c_new_inline_atom(cast_param_index, param_map);
	}
	
	if ((ap->direction == AOI_DIR_OUT)
	    && gen_server) {
		/*
		 *
		 */
		pres_c_mapping alloc_map;
		
		p_param_server_alloc_out(param_ctype, mr, param_map,
					 &alloc_map);
		if (alloc_map != 0) {
			int inline_slot_index
				= pres_c_add_inline_struct_slot(request_i);
			
			request_i->pres_c_inline_u_u.func_params_i.slots.
				slots_val[inline_slot_index].
				mint_struct_slot_index
				= mint_slot_index_null;
			
			request_i->pres_c_inline_u_u.func_params_i.slots.
				slots_val[inline_slot_index].
				inl
				= pres_c_new_inline_atom(cast_param_index,
							 alloc_map);
		}
	}
	
	/* Add the parameter to the reply inline struct if appropriate. */
	if ((ap->direction == AOI_DIR_INOUT) ||
	    (ap->direction == AOI_DIR_OUT)) {
		int inline_slot_index
			= pres_c_add_inline_struct_slot(reply_i);
		
		reply_i->pres_c_inline_u_u.func_params_i.slots.
			slots_val[inline_slot_index].
			mint_struct_slot_index
			= reply_mint_struct_slot_index;
		
		reply_i->pres_c_inline_u_u.func_params_i.slots.
			slots_val[inline_slot_index].
			inl
			= pres_c_new_inline_atom(cast_param_index, param_map);
	}
	
	/* Restore the name context.  */
	name = old_name;
	return res;
}

/*
 * Generate pres_c_inline (pres_c_inline_atom) for an operation parameter.
 * (asynchronous message marshal/unmarshal stub version)
 */
cast_func_param pg_state::p_async_param(
	aoi_parameter *ap,
	int cast_param_index,			// NOT a CAST ref!
	mint_ref mr,			
	pres_c_inline inl,
	int mint_struct_slot_index,		// NOT a MINT ref!
	int encode,				// 1=encode, 0=decode
	int request				// 1=request, 0=reply
	)
{
	cast_type param_ctype;
	pres_c_mapping param_map;
	char *old_name;
	cast_func_param res;
	
	/* Some initial assertions and local variable assignments. */
	assert(inl->kind == PRES_C_INLINE_FUNC_PARAMS_STRUCT);
	
	/* Save the original name context.  */
	old_name = name;
	name = calc_stub_param_name(ap->name);
	
	/* Set the parameter name. */
	res.name = name;
	res.type = 0;
	res.spec = 0;
	res.default_value = 0;
	
	/* Add the parameter to the inline struct if appropriate. */
	if ((request
	     && ((ap->direction == AOI_DIR_IN)
		 || (ap->direction == AOI_DIR_INOUT)))
	    || (!request
		&& ((ap->direction == AOI_DIR_INOUT) ||
		    (ap->direction == AOI_DIR_OUT)))) {
		
		/* Set the parameter type. */
		p_async_param_type(ap->type, mr, ap->direction,
				   &param_ctype, &param_map, encode);
		
		int inline_slot_index
			= pres_c_add_inline_struct_slot(inl);
		
		inl->pres_c_inline_u_u.func_params_i.slots.
			slots_val[inline_slot_index].
			mint_struct_slot_index
			= mint_struct_slot_index;
		
		inl->pres_c_inline_u_u.func_params_i.slots.
			slots_val[inline_slot_index].
			inl
			= pres_c_new_inline_atom(cast_param_index, param_map);
		
		res.type = param_ctype;
	}
	
	/* Restore the name context.  */
	name = old_name;
	return res;
}

/*
 * Generate pres_c_inline (pres_c_inline_atom) that presents an operation
 * return value as a parameter of an asynchronous message marshal/unmarshal
 * stub.
 */
cast_func_param pg_state::p_async_param_return(
	const char *param_name,
	aoi_type param_aoi_type,
	int cast_param_index,			// NOT a CAST ref!
	mint_ref mr,			
	pres_c_inline inl,
	int mint_struct_slot_index,		// NOT a MINT ref!
	int encode,				// 1=encode, 0=decode
	int request				// 1=request, 0=reply
	)
{
	cast_type param_ctype;
	pres_c_mapping param_map;
	char *old_name;
	
	pres_c_inline_struct_slot *return_struct_slot;
	
	cast_func_param res;
	
	/* Some initial assertions and local variable assignments. */
	assert(inl->kind == PRES_C_INLINE_FUNC_PARAMS_STRUCT);
	
	/* Save the original name context. */
	old_name = name;
	name = pg_state_name_strlit(param_name);
	
	if (request) {
		/*
		 * The return value does not appear in the request message.
		 */
		res.name = 0;
		res.type = 0;
		res.default_value = 0;
		res.spec = 0;
		
		/*
		 * Since the return value is not part of the message, we must
		 * clear the `return_slot' of the `inline_func_params_struct'
		 * that we were given.
		 */
		inl->pres_c_inline_u_u.func_params_i.return_slot = 0;
		
	} else {
		/*
		 * The return value is a component of the reply message.
		 */
		
		/* Determine the return parameter type and mapping. */
		p_async_param_return_type(param_aoi_type, mr,
					  &param_ctype, &param_map, encode);
		
		/*
		 * The return parameter mapping goes into an `inline_atom',
		 * which in turn goes into the `return_slot' of the `inl' we
		 * were given.
		 */
		return_struct_slot = (pres_c_inline_struct_slot *)
				     mustmalloc(
					     sizeof(pres_c_inline_struct_slot)
					     );
		
		return_struct_slot->mint_struct_slot_index
			= mint_struct_slot_index;
		
		/* XXX --- Should `cast_find_typedef_type' here. */
		
		if (param_ctype->kind == CAST_TYPE_VOID) {
			/*
			 * Special case: the return type is void.  In this
			 * case, we don't make a function parameter to hold the
			 * return value.  Additionally, we make a special
			 * inline atom that refers the (void) function return
			 * value in the inline state constructed by the BE.
			 */
			return_struct_slot->inl
				= pres_c_new_inline_atom(
					pres_c_func_return_index,
					param_map);
			
			res.name = 0;
			res.type = 0;
			res.default_value = 0;
			res.spec = 0;
			
		} else {
			/*
			 * Normal case: we present our return value as a
			 * function parameter.
			 */
			return_struct_slot->inl
				= pres_c_new_inline_atom(cast_param_index,
							 param_map);
			
			res.name = ir_strlit(param_name);
			res.type = param_ctype;
			res.default_value = 0;
			res.spec = 0;
		}
		
		/*
		 * Insert the PRES_C for handling the return value into the
		 * `return_slot' of the overall `inline_func_params_struct'.
		 */
		inl->pres_c_inline_u_u.func_params_i.return_slot
			= return_struct_slot;
	}
	
	/* Restore the name context. */
	name = old_name;
	return res;
}

/*
 * Generate pres_c_inline for a client SID parameter.
 */
void pg_state::p_param_client_sid(int param_index,
				  pres_c_inline request_inline)
{
	pres_c_mapping	param_map;
	int		slot_index;
	
	/* Make the PRES_C mapping for the client SID. */
	param_map = pres_c_new_mapping(PRES_C_MAPPING_SID);
	param_map->pres_c_mapping_u_u.sid.kind = PRES_C_SID_CLIENT;
	
	/* Tell the back end that this is the ``root'' of the parameter. */
	pres_c_interpose_param_root(&param_map, 0, 0);
	
	/*
	 * Now make an inline atom containing our SID's mapping, and add that
	 * atom to the request inline structure.
	 *
	 * By setting our `mint_struct_slot_index' to `mint_slot_index_null',
	 * we indicate that this SID parameter has no representation in the
	 * MINT structure that describes the request message.  That is, the SID
	 * is outside of the IDL-specified interface.
	 */
	slot_index = pres_c_add_inline_struct_slot(request_inline);
	request_inline->pres_c_inline_u_u.struct_i.slots.slots_val[slot_index].
		mint_struct_slot_index
		= mint_slot_index_null;
	request_inline->pres_c_inline_u_u.struct_i.slots.slots_val[slot_index].
		inl
		= pres_c_new_inline_atom(param_index, param_map);
}

/*
 * Generate pres_c_inline for a required server SID parameter.
 */
void pg_state::p_param_required_server_sid(int param_index,
					   pres_c_inline request_inline)
{
	pres_c_mapping	param_map;
	int		slot_index;
	
	/* Make the PRES_C mapping for the server SID. */
	param_map = pres_c_new_mapping(PRES_C_MAPPING_SID);
	param_map->pres_c_mapping_u_u.sid.kind = PRES_C_SID_SERVER;
	
	/* Tell the back end that this is the ``root'' of the parameter. */
	pres_c_interpose_param_root(&param_map, 0, 0);
	
	/*
	 * Now make an inline atom containing our SID's mapping, and add that
	 * atom to the request inline structure.
	 *
	 * By setting our `mint_struct_slot_index' to `mint_slot_index_null',
	 * we indicate that this SID parameter has no representation in the
	 * MINT structure that describes the request message.  That is, the SID
	 * is outside of the IDL-specified interface.
	 */
	slot_index = pres_c_add_inline_struct_slot(request_inline);
	request_inline->pres_c_inline_u_u.struct_i.slots.slots_val[slot_index].
		mint_struct_slot_index
		= mint_slot_index_null;
	request_inline->pres_c_inline_u_u.struct_i.slots.slots_val[slot_index].
		inl
		= pres_c_new_inline_atom(param_index, param_map);
}

/*
 * Generate pres_c_inline for an actual server SID parameter.
 */
void pg_state::p_param_actual_server_sid(int param_index,
					 pres_c_inline reply_inline)
{
	pres_c_mapping	param_map;
	int		slot_index;
	
	/* Make the PRES_C mapping for the server SID. */
	param_map = pres_c_new_mapping(PRES_C_MAPPING_SID);
	param_map->pres_c_mapping_u_u.sid.kind = PRES_C_SID_SERVER;
	
	/* Tell the back end that this is the ``root'' of the parameter. */
	pres_c_interpose_param_root(&param_map, 0, 0);
	
	/*
	 * Now make an inline atom containing our SID's mapping, and add that
	 * atom to the reply inline structure.
	 *
	 * By setting our `mint_struct_slot_index' to `mint_slot_index_null',
	 * we indicate that this SID parameter has no representation in the
	 * MINT structure that describes the request message.  That is, the SID
	 * is outside of the IDL-specified interface.
	 */
	slot_index = pres_c_add_inline_struct_slot(reply_inline);
	reply_inline->pres_c_inline_u_u.struct_i.slots.slots_val[slot_index].
		mint_struct_slot_index
		= mint_slot_index_null;
	reply_inline->pres_c_inline_u_u.struct_i.slots.slots_val[slot_index].
		inl
		= pres_c_new_inline_atom(param_index, param_map);
}


/*****************************************************************************/

/***** Auxiliary functions. *****/

/* This method determines how `p_param' processes a parameter type.
   Some presentation generators override this method. */
void pg_state::p_param_type(aoi_type at, mint_ref /*mr*/, aoi_direction dir,
			    cast_type *out_ctype, pres_c_mapping *out_mapping)
{
	p_type_collection *ptc = 0;
	p_type_node *ptn;
	
	/*
	 * The default behavior is to:
	 * (1) call `p_type' to compute the basic C type and PRES_C mapping;
	 * (2) interpose a pointer for `inout' and `out' parameters; and
	 * (3) encapsulate the mapping with information about the parameter
	 *     direction.
	 */
	
	p_type(at, &ptc);
	ptn = ptc->find_type("definition");
	*out_ctype = ptn->get_type();
	*out_mapping = ptn->get_mapping();
	
	if ((dir == AOI_DIR_INOUT) || (dir == AOI_DIR_OUT)) {
		// Set up the allocation semantics of the indirection pointer.
		pres_c_allocation alloc;
		
		/* Direction is InOut or Out, so other cases are invalid. */
		alloc.cases[PRES_C_DIRECTION_UNKNOWN].allow
			= PRES_C_ALLOCATION_INVALID;
		alloc.cases[PRES_C_DIRECTION_IN].allow
			= PRES_C_ALLOCATION_INVALID;
		alloc.cases[PRES_C_DIRECTION_RETURN].allow
			= PRES_C_ALLOCATION_INVALID;
		
		/*
		 * This is an indirection pointer:
		 *   Client side never allocs nor deallocs.
		 *   Server side always allocs and deallocs.
		 */
		alloc.cases[PRES_C_DIRECTION_INOUT].allow
			= PRES_C_ALLOCATION_ALLOW;
		if (gen_client)
			alloc.cases[PRES_C_DIRECTION_INOUT].
				pres_c_allocation_u_u.val.flags
				= PRES_C_ALLOC_NEVER | PRES_C_DEALLOC_NEVER;
		else if (gen_server)
			alloc.cases[PRES_C_DIRECTION_INOUT].
				pres_c_allocation_u_u.val.flags
				= PRES_C_ALLOC_ALWAYS | PRES_C_DEALLOC_ALWAYS;
		else
			panic("In pg_state::p_param_type: "
			      "Generating neither client nor server!");
		alloc.cases[PRES_C_DIRECTION_INOUT].pres_c_allocation_u_u.
			val.allocator.kind = PRES_C_ALLOCATOR_DONTCARE;
		alloc.cases[PRES_C_DIRECTION_INOUT].pres_c_allocation_u_u.
			val.alloc_init = 0;
		
		/* Out is the same as the InOut case */
		alloc.cases[PRES_C_DIRECTION_OUT]
			= alloc.cases[PRES_C_DIRECTION_INOUT];
		
		pres_c_interpose_indirection_pointer(out_ctype, out_mapping,
						     alloc);
	}
	
	/* Tell the back end that this is the ``root'' of the parameter. */
	pres_c_interpose_param_root(out_mapping, 0, 0);
	/* Tell the back end about this parameter's direction. */
	pres_c_interpose_direction(out_mapping, dir);
}

/* This method determines how `p_async_param' processes a parameter type.
   Some presentation generators override this method. */
void pg_state::p_async_param_type(aoi_type at, mint_ref mr,
				  aoi_direction dir,
				  cast_type *out_ctype,
				  pres_c_mapping *out_mapping,
				  int encode)
{
	p_type_collection *ptc = 0;
	p_type_node *ptn;
	
	/*
	 * The default behavior is to call `p_type' to compute the basic C type
	 * and PRES_C mapping.  If we are decoding the parameter, we also make
	 * it a pointer (pass-by-reference).
	 */
	p_type(at, &ptc);
	ptn = ptc->find_type("definition");
	*out_ctype = ptn->get_type();
	*out_mapping = ptn->get_mapping();
	
	if (!encode) {
		/*
		 * The semantics of `in' data is that it lives as long as the
		 * message in which it is contained.  Since the message can
		 * live on indefinitely, it may be relocated to free up
		 * precious resources for the network layer (e.g., buffer
		 * space, Trapeze receive rings, etc.).  Further, storage may
		 * be allocated to hold `in' data, but the user should not be
		 * forced to free it --- it should automatically be freed when
		 * the message dies.
		 *
		 * To allow for these semantics, we add an extra level of
		 * indirection, saving the pointer (into the message or to
		 * allocated space) within the message, and giving the user a
		 * ``handle'' to the data.
		 *
		 * NOTE: This only affects NON-primitive data types.
		 */
		cast_type ct = cast_find_typedef_type(
			(cast_scope *)top_ptr(scope_stack),
			*out_ctype);
		
		if (dir == AOI_DIR_IN
		    && ct
		    && (ct->kind == CAST_TYPE_POINTER
			|| ct->kind == CAST_TYPE_ARRAY)) {
			
			/*
			 * Fix the current CAST_TYPE so it will be allocated
			 * properly.
			 */
			if (ct && ct->kind == CAST_TYPE_ARRAY) {
				*out_ctype = cast_new_type(CAST_TYPE_POINTER);
				(*out_ctype)->cast_type_u_u.pointer_type.target
					= ct->cast_type_u_u.array_type.
					element_type;
			}
			
			if ((*out_mapping)->kind == PRES_C_MAPPING_STUB) {
				int stub_idx
					= pres_c_find_mu_stub(
						out_pres, mr, *out_ctype,
						*out_mapping,
						PRES_C_UNMARSHAL_STUB);
				assert(stub_idx >= 0);
				pres_c_mapping map
					= (pres_c_mapping)
					mustcalloc(sizeof *map);
				*map = *(out_pres->
					 stubs.stubs_val[stub_idx].
					 pres_c_stub_u.ustub.seethru_map);
				*out_mapping = map;
			}
			
#if 0
			/*
			 * XXX - #if 0'ed out because we don't have to dig to
			 * find the allocation.  It's now at a higher level and
			 * gets set correctly.
			 */
			pres_c_allocation *alloc = 0;
			switch ((*out_mapping)->kind) {
			case PRES_C_MAPPING_POINTER:
				alloc = &(*out_mapping)->pres_c_mapping_u_u.
					pointer.alloc;
				break;
			case PRES_C_MAPPING_OPTIONAL_POINTER:
				alloc = &(*out_mapping)->pres_c_mapping_u_u.
					optional_pointer.alloc;
				break;
			case PRES_C_MAPPING_FIXED_ARRAY:
				alloc = &(*out_mapping)->pres_c_mapping_u_u.
					fixed_array.alloc;
				break;
			case PRES_C_MAPPING_TERMINATED_ARRAY:
				alloc = &(*out_mapping)->pres_c_mapping_u_u.
					terminated_array.alloc;
				break;
			default:
				assert(!"Invalid mapping for pointer/array.");
			}
			assert(alloc);
			
			/* Force allocation/deallocation of every parameter. */
			pres_c_allocation ptr_alloc;
			pres_c_allocation_u dfault;
			dfault.allow = PRES_C_ALLOCATION_ALLOW;
			dfault.pres_c_allocation_u_u.val.flags
				= PRES_C_ALLOC_ALWAYS | PRES_C_DEALLOC_ALWAYS;
			dfault.pres_c_allocation_u_u.val.allocator.kind
				= PRES_C_ALLOCATOR_DONTCARE;
			dfault.pres_c_allocation_u_u.val.alloc_init = 0;
			
			ptr_alloc.cases[PRES_C_DIRECTION_UNKNOWN].allow
				= PRES_C_ALLOCATION_INVALID;
			ptr_alloc.cases[PRES_C_DIRECTION_IN] = dfault;
			ptr_alloc.cases[PRES_C_DIRECTION_INOUT] = dfault;
			ptr_alloc.cases[PRES_C_DIRECTION_OUT] = dfault;
			ptr_alloc.cases[PRES_C_DIRECTION_RETURN] = dfault;
			*alloc = ptr_alloc;

#endif
			/* Make handle_alloc for message handles. */
			pres_c_allocation handle_alloc;
			pres_c_allocation_u dfault;
			dfault.allow = PRES_C_ALLOCATION_ALLOW;
			dfault.pres_c_allocation_u_u.val.flags
				= PRES_C_ALLOC_ALWAYS | PRES_C_DEALLOC_ALWAYS;
			dfault.pres_c_allocation_u_u.val.allocator.kind
				= PRES_C_ALLOCATOR_NAME;
			dfault.pres_c_allocation_u_u.val.allocator.
				pres_c_allocator_u.name
				= ir_strlit("msg_handle");
			dfault.pres_c_allocation_u_u.val.alloc_init = 0;
			
			handle_alloc.cases[PRES_C_DIRECTION_UNKNOWN].allow
				= PRES_C_ALLOCATION_INVALID;
			handle_alloc.cases[PRES_C_DIRECTION_IN] = dfault;
			handle_alloc.cases[PRES_C_DIRECTION_INOUT] = dfault;
			handle_alloc.cases[PRES_C_DIRECTION_OUT] = dfault;
			handle_alloc.cases[PRES_C_DIRECTION_RETURN] = dfault;
			
			/* The ``handle'' to the data. */
			pres_c_interpose_indirection_pointer(out_ctype,
							     out_mapping,
							     handle_alloc);
		}
		
		/*
		 * One level of pointer indirection for all types.  This allows
		 * unmarshaled data to be returned like an `out' parameter.
		 */
		pres_c_allocation alloc;
		
		/*
		 * Direction is `in', `inout', or `out', so other cases are
		 * invalid.
		 */
		alloc.cases[PRES_C_DIRECTION_UNKNOWN].allow
			= PRES_C_ALLOCATION_INVALID;
		alloc.cases[PRES_C_DIRECTION_RETURN].allow
			= PRES_C_ALLOCATION_INVALID;
		
		/*
		 * This is an indirection pointer: never allocated nor
		 * deallocated.
		 */
		alloc.cases[PRES_C_DIRECTION_IN].allow
			= PRES_C_ALLOCATION_ALLOW;
		alloc.cases[PRES_C_DIRECTION_IN].pres_c_allocation_u_u.val.
			flags
			= (PRES_C_ALLOC_NEVER | PRES_C_DEALLOC_NEVER);
		alloc.cases[PRES_C_DIRECTION_IN].pres_c_allocation_u_u.val.
			allocator.kind
			= PRES_C_ALLOCATOR_DONTCARE;
		alloc.cases[PRES_C_DIRECTION_IN].pres_c_allocation_u_u.val.
			alloc_init = 0;
		
		/* `inout' and `out' are the same as the `in' case. */
		alloc.cases[PRES_C_DIRECTION_INOUT]
			= alloc.cases[PRES_C_DIRECTION_IN];
		alloc.cases[PRES_C_DIRECTION_OUT]
			= alloc.cases[PRES_C_DIRECTION_IN];
		
		pres_c_interpose_indirection_pointer(out_ctype, out_mapping,
						     alloc);
	}
	
	/* Tell the back end that this is the ``root'' of the parameter. */
	pres_c_interpose_param_root(out_mapping, 0, 0);
	/* Tell the back end about this parameter's direction. */
	pres_c_interpose_direction(out_mapping, dir);
}

void pg_state::p_async_param_return_type(aoi_type at, mint_ref /* mr */,
					 cast_type *out_ctype,
					 pres_c_mapping *out_mapping,
					 int encode)
{
	p_type_collection *ptc = 0;
	p_type_node *ptn;
	
	/*
	 * The default behavior is to call `p_type' to compute the
	 * basic C type and PRES_C mapping.  If we are decoding the
	 * parameter, we also make it a pointer (pass-by-reference).
	 */
	
	p_type(at, &ptc);
	ptn = ptc->find_type("definition");
	*out_ctype = ptn->get_type();
	*out_mapping = ptn->get_mapping();
	
	/* XXX --- Should `cast_find_typedef_type' here. */
	
	if ((*out_ctype)->kind == CAST_TYPE_VOID) {
		/*
		 * Special case: the return type is void.  Set up an `ignore'
		 * mapping rather than the usual `direct' mapping (although
		 * either should work).
		 */
		*out_mapping = pres_c_new_mapping(PRES_C_MAPPING_IGNORE);
		
	} else if (!encode) {
		/*
		 * A return value is presented as an `out' parameter, so we
		 * don't need all the code from `p_async_param_type' that
		 * handles `in' data.
		 */
		pres_c_allocation alloc;
		
		/*
		 * Direction is `out', so other cases are invalid.
		 */
		alloc.cases[PRES_C_DIRECTION_UNKNOWN].allow
			= PRES_C_ALLOCATION_INVALID;
		alloc.cases[PRES_C_DIRECTION_RETURN].allow
			= PRES_C_ALLOCATION_INVALID;
		alloc.cases[PRES_C_DIRECTION_IN].allow
			= PRES_C_ALLOCATION_INVALID;
		alloc.cases[PRES_C_DIRECTION_INOUT].allow
			= PRES_C_ALLOCATION_INVALID;
		
		/*
		 * This is an indirection pointer: never allocated nor
		 * deallocated.
		 */
		alloc.cases[PRES_C_DIRECTION_OUT].allow
			= PRES_C_ALLOCATION_ALLOW;
		alloc.cases[PRES_C_DIRECTION_OUT].pres_c_allocation_u_u.val.
			flags
			= (PRES_C_ALLOC_NEVER | PRES_C_DEALLOC_NEVER);
		alloc.cases[PRES_C_DIRECTION_OUT].pres_c_allocation_u_u.val.
			allocator.kind
			= PRES_C_ALLOCATOR_DONTCARE;
		alloc.cases[PRES_C_DIRECTION_OUT].pres_c_allocation_u_u.val.
			alloc_init = 0;

		pres_c_interpose_indirection_pointer(out_ctype, out_mapping,
						     alloc);
	}
	
	/* Indicate that this is the return value.  Some back ends use this. */
	pres_c_interpose_argument(out_mapping, "params", "return");
	if ((*out_ctype)->kind != CAST_TYPE_VOID) {
		/* Tell the BE that this is the ``root'' of the parameter. */
		pres_c_interpose_param_root(out_mapping, 0, 0);
	}
	/* Tell the back end about this parameter's direction. */
	pres_c_interpose_direction(out_mapping, AOI_DIR_OUT);
}

/*
 * THIS CODE HAS BEEN COPIED INTO `fe/mig/xlate_pres.c' AS
 * make_arg_server_alloc_out(). ANY CHANGES SHOULD BE REFLECTED THERE ALSO!
 * 
 * Decide if this is an `out' parameter for which the server must allocate
 * storage beyond the root.  If so, create an `alloc_mapping' for doing this.
 */
#define DEEP 0
void pg_state::p_param_server_alloc_out(cast_type param_ctype,
					mint_ref param_itype,
					pres_c_mapping param_mapping,
					pres_c_mapping *alloc_mapping)
{
#if DEEP
	mint_ref new_itype = mint_ref_null;
#endif
	cast_type new_ctype = 0;
	pres_c_mapping new_map = 0;
	mint_def *idef;
	
	/*****/
	
	*alloc_mapping = 0;
	assert(param_itype >= 0);
	assert(param_itype < (mint_ref)out_pres->mint.defs.defs_len);
	idef = &out_pres->mint.defs.defs_val[param_itype];
	
	switch (param_mapping->kind) {
	default:
		/*
		 * By default, we assume that the server dispatch function is
		 * not required to do anything special beyond allocating the
		 * ``root'' of the out parameter.
		 */
		warn("In `p_param_server_alloc_out', "
		     "unexpected mapping type %d.",
		     param_mapping->kind);
		break;
		
#if DEEP /* XXX --- These cases are never encountered, at least not now. */
	case PRES_C_MAPPING_IGNORE:
	case PRES_C_MAPPING_DIRECT:
	case PRES_C_MAPPING_REFERENCE:
	case PRES_C_MAPPING_MESSAGE_ATTRIBUTE:
	case PRES_C_MAPPING_STRUCT:
	case PRES_C_MAPPING_SID:
		/*
		 * Handle mappings for values presumably passed by reference.
		 *
		 * In these cases, there is nothing for the server dispatch
		 * function to do but allocate the parameter root.
		 * Thus, we change the mapping into a MAPPING_IGNORE.
		 */
		*alloc_mapping = pres_c_new_mapping(PRES_C_MAPPING_IGNORE);
		break;
#endif
		
	case PRES_C_MAPPING_POINTER:
		/*
		 * Descend the target mapping.
		 */
#if DEEP
		if (param_ctype->kind == CAST_TYPE_NAME) {
			param_ctype = cast_find_typedef_type(&(out_pres->cast),
							     param_ctype);
			if (!param_ctype)
				panic(("In `p_param_server_alloc_out(), can't "
				       "locate `typedef' for a named type."));
		}
		assert(param_ctype->kind == CAST_TYPE_POINTER);
		new_ctype = param_ctype->cast_type_u_u.pointer_type.target;
		
		p_param_server_alloc_out(new_ctype, param_itype,
					 (param_mapping->pres_c_mapping_u_u.
					  pointer.target),
					 &new_map);
		
		/* Reinsert the pointer. */
		assert(new_map);
#else
		new_map = pres_c_new_mapping(PRES_C_MAPPING_IGNORE);
#endif
		*alloc_mapping = pres_c_new_mapping(PRES_C_MAPPING_POINTER);
		(*alloc_mapping)->pres_c_mapping_u_u.pointer
			= param_mapping->pres_c_mapping_u_u.pointer;
		(*alloc_mapping)->pres_c_mapping_u_u.pointer.target
			= new_map;
		break;
		
	case PRES_C_MAPPING_INTERNAL_ARRAY:
		/*
		 * Descend the element mapping.
		 */
#if DEEP
		if (param_ctype->kind == CAST_TYPE_NAME) {
			param_ctype = cast_find_typedef_type(&(out_pres->cast),
							     param_ctype);
			if (!param_ctype)
				panic(("In `p_param_server_alloc_out(), can't "
				       "locate `typedef' for a named type."));
		}
		if (param_ctype->kind == CAST_TYPE_POINTER) {
			new_ctype = (param_ctype->cast_type_u_u.pointer_type.
				     target);
		} else if (param_ctype->kind == CAST_TYPE_ARRAY) {
			new_ctype = (param_ctype->cast_type_u_u.array_type.
				     element_type);
		} else
			panic(("In p_param_server_alloc_out(), "
			       "internal array mapping is not associated with "
			       "a CAST array or CAST pointer."));
		
		assert(idef->kind == MINT_ARRAY);
		new_itype = idef->mint_def_u.array_def.element_type;
		
		p_param_server_alloc_out(new_ctype, param_itype,
					 (param_mapping->pres_c_mapping_u_u.
					  internal_array.element_mapping),
					 &new_map);
		
		/* Reinsert the internal array. */
		assert(new_map);
#else
		new_map = pres_c_new_mapping(PRES_C_MAPPING_IGNORE);
#endif
		
		*alloc_mapping
			= pres_c_new_mapping(PRES_C_MAPPING_INTERNAL_ARRAY);
		(*alloc_mapping)->pres_c_mapping_u_u.internal_array
			= param_mapping->pres_c_mapping_u_u.internal_array;
		(*alloc_mapping)->pres_c_mapping_u_u.internal_array.
			element_mapping
			= new_map;
		break;
		
	case PRES_C_MAPPING_VAR_REFERENCE:
		/*
		 * Descend the referent mapping.
		 */
#if DEEP
		if (param_ctype->kind == CAST_TYPE_NAME) {
			param_ctype = cast_find_typedef_type(&(out_pres->cast),
							     param_ctype);
			if (!param_ctype)
				panic(("In `p_param_server_alloc_out(), can't "
				       "locate `typedef' for a named type."));
		}
		assert(param_ctype->kind == CAST_TYPE_REFERENCE);
		new_ctype = param_ctype->cast_type_u_u.reference_type.target;
		
		p_param_server_alloc_out(new_ctype, param_itype,
					 (param_mapping->pres_c_mapping_u_u.
					  var_ref.target),
					 &new_map);
		
		/* Reinsert the reference. */
		assert(new_map);
#else
		new_map = pres_c_new_mapping(PRES_C_MAPPING_IGNORE);
#endif
		
		*alloc_mapping
			= pres_c_new_mapping(PRES_C_MAPPING_VAR_REFERENCE);
		(*alloc_mapping)->pres_c_mapping_u_u.var_ref
			= param_mapping->pres_c_mapping_u_u.var_ref;
		(*alloc_mapping)->pres_c_mapping_u_u.var_ref.target
			= new_map;
		break;
		
	case PRES_C_MAPPING_STUB:
		/*
		 * Descend into the stub's mapping.
		 */
		new_ctype = param_ctype;
		new_map = param_mapping;
		
		pres_descend_mapping_stub(out_pres, param_itype,
					  &new_ctype, &new_map);
		p_param_server_alloc_out(new_ctype, param_itype, new_map,
					 alloc_mapping);
		break;
		
	case PRES_C_MAPPING_XLATE:
		/*
		 * Descend into the translation.
		 */
		new_ctype = param_mapping->pres_c_mapping_u_u.xlate.
			    internal_ctype;
		new_map = param_mapping->pres_c_mapping_u_u.xlate.
			  internal_mapping;
		
		p_param_server_alloc_out(new_ctype, param_itype, new_map,
					 alloc_mapping);
		
		/* Reinsert the translation node. */
#if DEEP
		assert(*alloc_mapping);
#else
		if (!*alloc_mapping)
			*alloc_mapping
				= pres_c_new_mapping(PRES_C_MAPPING_IGNORE);
#endif
		new_map = pres_c_new_mapping(PRES_C_MAPPING_XLATE);
		*new_map = *param_mapping;
		
		new_map->pres_c_mapping_u_u.xlate.internal_mapping
			= *alloc_mapping;
		*alloc_mapping = new_map;
		break;
		
	case PRES_C_MAPPING_DIRECTION:
		/*
		 * Skip past the direction mapping.
		 */
		p_param_server_alloc_out(param_ctype, param_itype,
					 (param_mapping->pres_c_mapping_u_u.
					  direction.mapping),
					 alloc_mapping);
		
		/* Reinsert the direction node. */
#if DEEP
		assert(*alloc_mapping);
#else
		if (!*alloc_mapping)
			*alloc_mapping
				= pres_c_new_mapping(PRES_C_MAPPING_IGNORE);
#endif
		new_map = pres_c_new_mapping(PRES_C_MAPPING_DIRECTION);
		new_map->pres_c_mapping_u_u.direction
			= param_mapping->pres_c_mapping_u_u.direction;
		new_map->pres_c_mapping_u_u.direction.mapping
			= *alloc_mapping;
		*alloc_mapping = new_map;
		break;
		
	case PRES_C_MAPPING_ARGUMENT:
		/*
		 * Skip past the argument mapping.
		 */
		p_param_server_alloc_out(param_ctype, param_itype,
					 (param_mapping->pres_c_mapping_u_u.
					  argument.map),
					 alloc_mapping);
		
		/* Reinsert the argument node. */
#if DEEP
		assert(*alloc_mapping);
#else
		if (!*alloc_mapping)
			*alloc_mapping
				= pres_c_new_mapping(PRES_C_MAPPING_IGNORE);
#endif
		pres_c_interpose_argument(alloc_mapping,
					  (param_mapping->
					   pres_c_mapping_u_u.argument.
					   arglist_name),
					  (param_mapping->
					   pres_c_mapping_u_u.argument.
					   arg_name));
		break;
		
	case PRES_C_MAPPING_PARAM_ROOT:
		/*
		 * Skip past the ``parameter root'' mapping.
		 */
		new_ctype = param_mapping->pres_c_mapping_u_u.param_root.ctype;
		
		p_param_server_alloc_out(new_ctype ? new_ctype : param_ctype,
					 param_itype,
					 (param_mapping->pres_c_mapping_u_u.
					  param_root.map),
					 alloc_mapping);
		
		/*
		 * Reinsert the ``parameter root'' node.  This node must always
		 * be inserted, or else the back end won't notice the parameter
		 * root at all!
		 */
#if DEEP
		assert(*alloc_mapping);
#else
		if (!(*alloc_mapping))
			*alloc_mapping
				= pres_c_new_mapping(PRES_C_MAPPING_IGNORE);
#endif
		pres_c_interpose_param_root(alloc_mapping, new_ctype,
					    (param_mapping->pres_c_mapping_u_u.
					     param_root.init));
		break;
		
	case PRES_C_MAPPING_SINGLETON: {
		/*
		 * Skip past the singleton mapping.  Unfortunately, this
		 * returns us to inline mode, and we really aren't prepared to
		 * handle inlines at this point.  The only case that we should
		 * encounter is a PRES_C_INLINE_ALLOCATION_CONTEXT node, which
		 * we need to pluck off also, then a PRES_C_INLINE_ATOM in the
		 * ptr slot that should refer to slot 0 of the singleton, and
		 * then we can continue looking for the mapping.  Phew!
		 */
		pres_c_inline_allocation_context *ac;
		pres_c_inline inl;
		
		assert(param_mapping->pres_c_mapping_u_u.singleton.inl->kind
		       == PRES_C_INLINE_ALLOCATION_CONTEXT);
		ac = &param_mapping->pres_c_mapping_u_u.singleton.inl->
		     pres_c_inline_u_u.acontext;
		assert(ac->ptr->kind == PRES_C_INLINE_ATOM);
		assert(ac->ptr->pres_c_inline_u_u.atom.index == 0);
		
		/* Go for the *real* mapping. */
		p_param_server_alloc_out(param_ctype, param_itype,
					 (ac->ptr->pres_c_inline_u_u.atom.
					  mapping),
					 alloc_mapping);
		
		/* Reinsert a copy of all the junk we just stripped off. */
		assert(*alloc_mapping);
		inl = pres_c_new_inline(PRES_C_INLINE_ALLOCATION_CONTEXT);
		inl->pres_c_inline_u_u.acontext = *ac;
		inl->pres_c_inline_u_u.acontext.ptr
			= pres_c_new_inline_atom(0, *alloc_mapping);
		
		/* We may also have to do the length mapping. */
		if (ac->length->kind == PRES_C_INLINE_ATOM) {
			assert(ac->length->pres_c_inline_u_u.atom.index == 0);
			p_param_server_alloc_out(param_ctype, param_itype,
						 (ac->length->
						  pres_c_inline_u_u.atom.
						  mapping),
						 alloc_mapping);
			assert(*alloc_mapping);
			inl->pres_c_inline_u_u.acontext.length
				= pres_c_new_inline_atom(0, *alloc_mapping);
		}
		
		*alloc_mapping
			= pres_c_new_mapping(PRES_C_MAPPING_SINGLETON);
		(*alloc_mapping)->pres_c_mapping_u_u.singleton.inl = inl;
		break;
	}
	
	case PRES_C_MAPPING_TEMPORARY:
		/*
		 * Descend the temporary mapping.
		 */
		new_ctype = param_mapping->pres_c_mapping_u_u.temp.ctype;
		
		p_param_server_alloc_out(new_ctype, param_itype,
					 (param_mapping->pres_c_mapping_u_u.
					  temp.map),
					 &new_map);
		
		/* Reinsert the temporary. */
#if DEEP
		assert(new_map);
#else
		if (!new_map)
			new_map = pres_c_new_mapping(PRES_C_MAPPING_IGNORE);
#endif
		
		*alloc_mapping
			= pres_c_new_mapping(PRES_C_MAPPING_TEMPORARY);
		(*alloc_mapping)->pres_c_mapping_u_u.temp
			= param_mapping->pres_c_mapping_u_u.temp;
		(*alloc_mapping)->pres_c_mapping_u_u.temp.map
			= new_map;
		break;
	}
}

/* End of file. */

