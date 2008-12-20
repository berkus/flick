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
#include <string.h>

#include <mom/compiler.h>
#include <mom/libaoi.h>		/* For `void_d'. */
#include <mom/libmint.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include <mom/c/pfe.hh>
#include "private.hh"

static void p_param_arg_mapping(pres_c_inline inl, int mint_struct_slot_index,
				int cast_slot_index, const char *name,
				pres_c_mapping orig_mapping)
{
	/* Create an argument mapping for the parameter. */	
	pres_c_mapping map = orig_mapping;
	pres_c_interpose_argument(&map, "params", name);
	
	/* Tell the back end that this is the ``root'' of the parameter. */
	pres_c_interpose_param_root(&map, 0, 0);
	
	int inline_slot_index;
	pres_c_inline_struct_slot *sslot = 0;
	
	switch (inl->kind) {
	case PRES_C_INLINE_ATOM:
		/*
		 * Special case: INLINE_ATOM (likely a target/client reference)
		 * In this case, the atom is already created, we just fill in
		 * the slots.  For all the others, we create a new atom.
		 */
		inl->pres_c_inline_u_u.atom.index = cast_slot_index;
		inl->pres_c_inline_u_u.atom.mapping = map;
		
		/* Done; no more processing. */
		return;
		
	case PRES_C_INLINE_STRUCT:
		inline_slot_index = pres_c_add_inline_struct_slot(inl);
		sslot = &(inl->pres_c_inline_u_u.struct_i.slots.
			  slots_val[inline_slot_index]);
		break;
		
	case PRES_C_INLINE_FUNC_PARAMS_STRUCT:
		/* We should never be handling return values here. */
		assert(strcmp(name, "return") != 0);
		
		inline_slot_index = pres_c_add_inline_struct_slot(inl);
		sslot = &(inl->pres_c_inline_u_u.func_params_i.slots.
			  slots_val[inline_slot_index]);
		break;
		
	case PRES_C_INLINE_HANDLER_FUNC:
		inline_slot_index = pres_c_add_inline_struct_slot(inl);
		sslot = &(inl->pres_c_inline_u_u.handler_i.slots.
			  slots_val[inline_slot_index]);
		break;
		
	default:
		panic("In `p_param_arg_mapping', "
		      "invalid inline kind (%d).", inl->kind);
		break;
	}
	assert(sslot);
	
	/* Fill in the struct slot. */
	sslot->mint_struct_slot_index = mint_struct_slot_index;
	sslot->inl = pres_c_new_inline_atom(cast_slot_index, map);
}

/*
 * Generate the parameter list of a client stub presentation for an AOI
 * interface operation.
 */
void pg_state::process_client_params(
	cast_func_type *cfunc,
	stub_special_params *specials,
	mint_ref request_ref, mint_ref reply_ref,
	aoi_operation *ao,
	pres_c_inline request_l4_inl, pres_c_inline reply_l4_inl,
	pres_c_inline target_inl, pres_c_inline /*client_inl*/)
{
	/*
	 * Process each parameter.
	 *
	 * `aoi_index' is the index into the (input) AOI list of parameters;
	 * `cast_index' is the index into the (output) CAST list of parameters.
	 *
	 * XXX --- This code makes two assumptions: (1) that the order of the
	 * slots in the request and reply MINT types corresponds to the order
	 * of the `in/inout' and `out/inout' parameters (respectively) of the
	 * AOI operation; and (2) that in the request and reply MINT structure
	 * types, there is exactly one slot for each appropriate AOI parameter
	 * (one request slot per `in/inout' parameter, and one reply slot per
	 * `out/inout' parameter).
	 *
	 * Are either of these assumptions bad?  They do mean that the AOI and
	 * MINT must be in very close correspondence.  Maybe we shouldn't
	 * assume so much behavior from the AOI-to-MINT translator.
	 *
	 * XXX --- Certain other code makes assumptions about correspondences
	 * between MINT and PRES_C.  See `pg_state::p_param_client_sid' for a
	 * long diatribe on this subject.
	 */
	int req_idx = 0, rep_idx = 0;
	int cast_params_len = cfunc->params.params_len;
	
	for (int aoi_index = 0, cast_index = 0;
	     cast_index < cast_params_len;
	     ++cast_index) {
		if (cast_index ==
		    (specials->
		     params[stub_special_params::object_ref].
		     index)) {
			/* This is the object reference parameter. */
			cfunc->params.params_val[cast_index].spec =
				specials->
				params[stub_special_params::object_ref].
				spec;
			cfunc->params.params_val[cast_index].name =
				ir_strlit(
					specials->
					params[stub_special_params::
					      object_ref].
					name
					);
			cfunc->params.params_val[cast_index].type =
				specials->
				params[stub_special_params::object_ref].
				ctype;
			cfunc->params.params_val[cast_index].default_value = 0;
			
			pres_c_mapping target_mapping
				= pres_c_new_mapping(PRES_C_MAPPING_REFERENCE);
			
			target_mapping->pres_c_mapping_u_u.ref.kind
				= PRES_C_REFERENCE_COPY;
			target_mapping->pres_c_mapping_u_u.ref.ref_count = 1;
			target_mapping->pres_c_mapping_u_u.ref.
				arglist_name = ir_strlit("");
			
			p_param_arg_mapping(((target_inl)?
					     target_inl : request_l4_inl),
					    mint_slot_index_null,
					    cast_index,
					    "object",
					    target_mapping);
			
		} else if (cast_index ==
			   (specials->
			    params[stub_special_params::environment_ref].
			    index)) {
			/* This is the environment reference parameter. */
			cfunc->params.params_val[cast_index].spec =
				specials->
				params[stub_special_params::environment_ref].
				spec;
			cfunc->params.params_val[cast_index].name =
				ir_strlit(
					specials->
					params[stub_special_params::
					      environment_ref].
					name
					);
			cfunc->params.params_val[cast_index].type =
				specials->
				params[stub_special_params::environment_ref].
				ctype;
			cfunc->params.params_val[cast_index].default_value = 0;
			
			p_param_arg_mapping(reply_l4_inl,
					    mint_slot_index_null,
					    cast_index,
					    "environment",
					    pres_c_new_mapping(
						    PRES_C_MAPPING_IGNORE));
			
		} else if (cast_index ==
			   (specials->
			    params[stub_special_params::return_ref].
			    index)) {
			panic("Can't change return type of regular "
			      "client/server function");
			
		} else if (cast_index ==
			   (specials->
			    params[stub_special_params::message_ref].
			    index)) {
			panic("Can't add message parameter to regular "
			      "client/server function");
			
		} else if (cast_index ==
			   (specials->
			    params[stub_special_params::client_sid].
			    index)) {
			/* This is the client SID parameter. */
			cfunc->params.params_val[cast_index].spec =
				specials->
				params[stub_special_params::client_sid].
				spec;
			cfunc->params.params_val[cast_index].name =
				ir_strlit(
					specials->
					params[stub_special_params::
					      client_sid].
					name
					);
			cfunc->params.params_val[cast_index].type =
				specials->
				params[stub_special_params::client_sid].
				ctype;
			cfunc->params.params_val[cast_index].default_value = 0;
			p_param_client_sid(cast_index, request_l4_inl);
			
		} else if (cast_index ==
			   (specials->
			    params[stub_special_params::required_server_sid].
			    index)) {
			/* This is the required server SID parameter. */
			cfunc->params.params_val[cast_index].spec =
				specials->
				params[stub_special_params::
				      required_server_sid].
				spec;
			cfunc->params.params_val[cast_index].name =
				ir_strlit(
					specials->
					params[stub_special_params::
					      required_server_sid].
					name
					);
			cfunc->params.params_val[cast_index].type =
				specials->
				params[stub_special_params::
				      required_server_sid].
				ctype;
			cfunc->params.params_val[cast_index].default_value = 0;
			p_param_required_server_sid(cast_index,
						    request_l4_inl);
			
		} else if (cast_index ==
			   (specials->
			    params[stub_special_params::actual_server_sid].
			    index)) {
			/* This is the actual server SID parameter. */
			cfunc->params.params_val[cast_index].spec =
				specials->
				params[stub_special_params::actual_server_sid].
				spec;
			cfunc->params.params_val[cast_index].name =
				ir_strlit(
					specials->
					params[stub_special_params::
					      actual_server_sid].
					name
					);
			cfunc->params.params_val[cast_index].type =
				specials->
				params[stub_special_params::actual_server_sid].
				ctype;
			cfunc->params.params_val[cast_index].default_value = 0;
			p_param_actual_server_sid(cast_index, reply_l4_inl);
			
		} else {
			/* This is a normal parameter. */
			mint_ref m_ref;
			aoi_direction dir = ao->params.params_val[aoi_index].
					    direction;
			
			/* Get the MINT type of this parameter. */
			if (dir == AOI_DIR_OUT) {
				/*
				 * KBF - This is now in a UNION before the
				 * structure.
				 *
				 * XXX --- The `0' below is the index of the
				 * ``no-exception reply'' variant.  It should
				 * not be hardcoded.
				 */
				m_ref = m(reply_ref).mint_def_u.union_def.
					cases.cases_val[0].var;
				m_ref = m(m_ref).mint_def_u.struct_def.slots.
					slots_val[rep_idx];
			} else
				m_ref = m(request_ref).mint_def_u.struct_def.
					slots.slots_val[req_idx];
			
			cfunc->params.params_val[cast_index] =
				p_param(&(ao->params.params_val[aoi_index]),
					cast_index, m_ref,
					request_l4_inl, reply_l4_inl,
					req_idx, rep_idx);
			
			req_idx += ((dir == AOI_DIR_INOUT)
				    || (dir == AOI_DIR_IN));
			rep_idx += ((dir == AOI_DIR_INOUT)
				    || (dir == AOI_DIR_OUT));
			
			++aoi_index;
		}
	}
}

/*
 * Generate the parameter list of a server work function presentation for an
 * AOI interface operation.
 */
void pg_state::process_server_params(
	cast_func_type *cfunc,
	stub_special_params *specials,
	mint_ref request_ref, mint_ref reply_ref,
	aoi_operation *ao,
	pres_c_inline request_l4_inl, pres_c_inline reply_l4_inl,
	pres_c_inline target_inl, pres_c_inline client_inl)
{
	process_client_params(cfunc,
			      specials,
			      request_ref, reply_ref,
			      ao,
			      request_l4_inl, reply_l4_inl,
			      target_inl, client_inl);
}

/*
 * Generate the parameter list of a client stub presentation for an AOI
 * interface operation.
 */
void pg_state::process_async_params(
	cast_func_type *cfunc,
	stub_special_params *specials,
	mint_ref mref,
	aoi_operation *ao,
	pres_c_inline l4_inl,
	pres_c_inline target_inl, pres_c_inline client_inl,
	int encode, int request)
{
	/*
	 * Process each parameter.
	 *
	 * `aoi_index' is the index into the (input) AOI list of parameters;
	 * `cast_index' is the index into the (output) CAST list of parameters.
	 *
	 * XXX --- See comment above for process_client_params().
	 */
	int idx = 0;
	int cast_params_len = cfunc->params.params_len;
	int cast_index_adjust = 0;
	
	if (!request
	    && m(mref).kind == MINT_UNION) {
		/*
		 * XXX --- The `0' below is the index of the
		 * ``no-exception reply'' variant.  It should
		 * not be hardcoded.
		 */
		mref = m(mref).mint_def_u.union_def.
		       cases.cases_val[0].var;
	}
	
	for (int aoi_index = 0, cast_index = 0;
	     cast_index < cast_params_len;
	     ++cast_index) {
		int cast_idx = cast_index - cast_index_adjust;
		
		if (cast_index ==
		    (specials->
		     params[stub_special_params::object_ref].
		     index)) {
			/* This is the object reference parameter. */
			cfunc->params.params_val[cast_idx].spec =
				specials->
				params[stub_special_params::object_ref].
				spec;
			cfunc->params.params_val[cast_idx].name =
				ir_strlit(
					specials->
					params[stub_special_params::
					      object_ref].
					name
					);
			cfunc->params.params_val[cast_idx].type =
				specials->
				params[stub_special_params::object_ref].
				ctype;
			cfunc->params.params_val[cast_idx].default_value = 0;
			
			pres_c_mapping target_mapping
				= pres_c_new_mapping(PRES_C_MAPPING_REFERENCE);
			
			target_mapping->pres_c_mapping_u_u.ref.kind
				= PRES_C_REFERENCE_COPY;
			target_mapping->pres_c_mapping_u_u.ref.ref_count = 1;
			target_mapping->pres_c_mapping_u_u.ref.
				arglist_name = ir_strlit("");
			
			p_param_arg_mapping(((target_inl)?
					     target_inl : l4_inl),
					    mint_slot_index_null,
					    cast_idx,
					    "object",
					    ((target_inl)?
					     target_mapping :
					     pres_c_new_mapping(
						     PRES_C_MAPPING_IGNORE)));
			
			/* Update to the true cast index of the parameter. */
			specials->params[stub_special_params::object_ref].
				index = cast_idx;
			
		} else if (cast_index ==
			   (specials->
			    params[stub_special_params::environment_ref].
			    index)) {
			/* This is the environment reference parameter. */
			cfunc->params.params_val[cast_idx].spec =
				specials->
				params[stub_special_params::environment_ref].
				spec;
			cfunc->params.params_val[cast_idx].name =
				ir_strlit(
					specials->
					params[stub_special_params::
					      environment_ref].
					name
					);
			cfunc->params.params_val[cast_idx].type =
				specials->
				params[stub_special_params::environment_ref].
				ctype;
			cfunc->params.params_val[cast_idx].default_value = 0;
			
			p_param_arg_mapping(l4_inl,
					    mint_slot_index_null,
					    cast_idx,
					    "environment",
					    pres_c_new_mapping(
						    PRES_C_MAPPING_IGNORE));
			
			/* Update to the true cast index of the parameter. */
			specials->params[stub_special_params::environment_ref].
				index = cast_idx;
			
		} else if (cast_index ==
			   (specials->
			    params[stub_special_params::return_ref].
			    index)) {
			/* This is the return value parameter. */
			int return_slot_index;
			mint_ref m_ref;
			
			/*
			 * Get the MINT type of this parameter.  The MINT type
			 * of the return value is stored in the last slot of
			 * the reply structure (thus, the hairy expression for
			 * `m_ref' below).  See `tam_operation_reply_struct' in
			 * the file `aoi_to_mint.c' for more information.
			 */
			return_slot_index
				= (m(mref).mint_def_u.struct_def.slots.
				   slots_len
				   - 1);
			m_ref = m(mref).mint_def_u.struct_def.
				slots.
				slots_val[return_slot_index];
			
			cfunc->params.params_val[cast_idx] =
				p_async_param_return(
					(specials->
					 params[stub_special_params::
					       return_ref].
					 name),
					ao->return_type,
					cast_idx,
					m_ref, l4_inl, return_slot_index,
					encode, request
					);
			
			/*
			 * If no parameter was generated, recover!  We don't
			 * want to leave holes in the CAST!
			 */
			if (cfunc->params.params_val[cast_idx].type == 0) {
				++cast_index_adjust; /* One less CAST param. */
			}
			
			/* Update to the true cast index of the parameter. */
			specials->params[stub_special_params::return_ref].
				index = cast_idx;
			
		} else if (cast_index ==
			   (specials->
			    params[stub_special_params::invocation_ref].
			    index)) {
			/* This is the invocation identifier parameter. */
			cfunc->params.params_val[cast_idx].spec =
				specials->
				params[stub_special_params::invocation_ref].
				spec;
			cfunc->params.params_val[cast_idx].name =
				ir_strlit(
					specials->
					params[stub_special_params::
					      invocation_ref].
					name
					);
			cfunc->params.params_val[cast_idx].type =
				specials->
				params[stub_special_params::invocation_ref].
				ctype;
			cfunc->params.params_val[cast_idx].default_value = 0;
			
			p_param_arg_mapping(l4_inl,
					    mint_slot_index_null,
					    cast_idx,
					    "invocation_id",
					    pres_c_new_mapping(
						    PRES_C_MAPPING_IGNORE));
			
			/* Update to the true cast index of the parameter. */
			specials->params[stub_special_params::invocation_ref].
				index = cast_idx;
			
		} else if (cast_index ==
			   (specials->
			    params[stub_special_params::client_ref].
			    index)) {
			/* This is the client reference parameter. */
			cfunc->params.params_val[cast_idx].spec =
				specials->
				params[stub_special_params::client_ref].
				spec;
			cfunc->params.params_val[cast_idx].name =
				ir_strlit(
					specials->
					params[stub_special_params::
					      client_ref].
					name
					);
			cfunc->params.params_val[cast_idx].type =
				specials->
				params[stub_special_params::client_ref].
				ctype;
			cfunc->params.params_val[cast_idx].default_value = 0;
			
			pres_c_mapping client_mapping
				= pres_c_new_mapping(PRES_C_MAPPING_REFERENCE);
			
			client_mapping->pres_c_mapping_u_u.ref.kind
				= PRES_C_REFERENCE_COPY;
			client_mapping->pres_c_mapping_u_u.ref.ref_count = 1;
			client_mapping->pres_c_mapping_u_u.ref.
				arglist_name = ir_strlit("");
			
			p_param_arg_mapping(((client_inl)?
					     client_inl : l4_inl),
					    mint_slot_index_null,
					    cast_idx,
					    "client",
					    ((client_inl)?
					     client_mapping :
					     pres_c_new_mapping(
						     PRES_C_MAPPING_IGNORE)));
			
			/* Update to the true cast index of the parameter. */
			specials->params[stub_special_params::client_ref].
				index = cast_idx;
			
		} else if (cast_index ==
			   (specials->
			    params[stub_special_params::message_ref].
			    index)) {
			/* This is the message reference parameter. */
			cfunc->params.params_val[cast_idx].spec =
				specials->
				params[stub_special_params::message_ref].
				spec;
			cfunc->params.params_val[cast_idx].name =
				ir_strlit(
					specials->
					params[stub_special_params::
					      message_ref].
					name
					);
			cfunc->params.params_val[cast_idx].type =
				specials->
				params[stub_special_params::message_ref].
				ctype;
			cfunc->params.params_val[cast_idx].default_value = 0;
			
			p_param_arg_mapping(l4_inl,
					    mint_slot_index_null,
					    cast_idx,
					    "message",
					    pres_c_new_mapping(
						    PRES_C_MAPPING_IGNORE));
			
			/* Update to the true cast index of the parameter. */
			specials->params[stub_special_params::message_ref].
				index = cast_idx;
			
		} else if (cast_index ==
			   (specials->
			    params[stub_special_params::continue_func_ref].
			    index)) {
			/* This is the continuation function parameter. */
			cfunc->params.params_val[cast_idx].spec =
				specials->
				params[stub_special_params::continue_func_ref].
				spec;
			cfunc->params.params_val[cast_idx].name =
				ir_strlit(
					specials->
					params[stub_special_params::
					      continue_func_ref].
					name
					);
			cfunc->params.params_val[cast_idx].type =
				specials->
				params[stub_special_params::continue_func_ref].
				ctype;
			cfunc->params.params_val[cast_idx].default_value = 0;
			
			p_param_arg_mapping(l4_inl,
					    mint_slot_index_null,
					    cast_idx,
					    "continue_func",
					    pres_c_new_mapping(
						    PRES_C_MAPPING_IGNORE));
			
			/* Update to the true cast index of the parameter. */
			specials->params[stub_special_params::
					continue_func_ref].
				index = cast_idx;
			
		} else if (cast_index ==
			   (specials->
			    params[stub_special_params::continue_data_ref].
			    index)) {
			/* This is the continuation function data parameter. */
			cfunc->params.params_val[cast_idx].spec =
				specials->
				params[stub_special_params::continue_data_ref].
				spec;
			cfunc->params.params_val[cast_idx].name =
				ir_strlit(
					specials->
					params[stub_special_params::
					      continue_data_ref].
					name
					);
			cfunc->params.params_val[cast_idx].type =
				specials->
				params[stub_special_params::continue_data_ref].
				ctype;
			cfunc->params.params_val[cast_idx].default_value = 0;
			
			p_param_arg_mapping(l4_inl,
					    mint_slot_index_null,
					    cast_idx,
					    "continue_data",
					    pres_c_new_mapping(
						    PRES_C_MAPPING_IGNORE));
			
			/* Update to the true cast index of the parameter. */
			specials->params[stub_special_params::
					continue_data_ref].
				index = cast_idx;
			
		} else if (cast_index ==
			   (specials->
			    params[stub_special_params::client_sid].
			    index)
			   && request) {
			/* This is the client SID parameter. */
			cfunc->params.params_val[cast_idx].spec =
				specials->
				params[stub_special_params::client_sid].
				spec;
			cfunc->params.params_val[cast_idx].name =
				ir_strlit(
					specials->
					params[stub_special_params::
					      client_sid].
					name
					);
			cfunc->params.params_val[cast_idx].type =
				specials->
				params[stub_special_params::client_sid].
				ctype;
			cfunc->params.params_val[cast_idx].default_value = 0;
			p_param_client_sid(cast_idx, l4_inl);
			
			/* Update to the true cast index of the parameter. */
			specials->params[stub_special_params::client_sid].
				index = cast_idx;
			
		} else if (cast_index ==
			   (specials->
			    params[stub_special_params::required_server_sid].
			    index)
			   && request) {
			/* This is the required server SID parameter. */
			cfunc->params.params_val[cast_idx].spec =
				specials->
				params[stub_special_params::
				      required_server_sid].
				spec;
			cfunc->params.params_val[cast_idx].name =
				ir_strlit(
					specials->
					params[stub_special_params::
					      required_server_sid].
					name
					);
			cfunc->params.params_val[cast_idx].type =
				specials->
				params[stub_special_params::
				      required_server_sid].
				ctype;
			cfunc->params.params_val[cast_idx].default_value = 0;
			p_param_required_server_sid(cast_idx, l4_inl);
			
			/* Update to the true cast index of the parameter. */
			specials->params[stub_special_params::
					required_server_sid].
				index = cast_idx;
			
		} else if (cast_index ==
			   (specials->
			    params[stub_special_params::actual_server_sid].
			    index)
			   && !request) {
			/* This is the actual server SID parameter. */
			cfunc->params.params_val[cast_idx].spec =
				specials->
				params[stub_special_params::actual_server_sid].
				spec;
			cfunc->params.params_val[cast_idx].name =
				ir_strlit(
					specials->
					params[stub_special_params::
					      actual_server_sid].
					name
					);
			cfunc->params.params_val[cast_idx].type =
				specials->
				params[stub_special_params::actual_server_sid].
				ctype;
			cfunc->params.params_val[cast_idx].default_value = 0;
			p_param_actual_server_sid(cast_idx, l4_inl);
			
			/* Update to the true cast index of the parameter. */
			specials->params[stub_special_params::
					actual_server_sid].
				index = cast_idx;
			
		} else {
			/* This is a normal parameter. */
			mint_ref m_ref;
			
			/* Get the MINT type of this parameter. */
			m_ref = m(mref).mint_def_u.struct_def.
				slots.slots_val[idx];
			
			cfunc->params.params_val[cast_idx] =
				p_async_param(&(ao->params.
						params_val[aoi_index]),
					      cast_idx,
					      m_ref, l4_inl, idx,
					      encode, request);
			
			++aoi_index;
			
			/*
			 * If no parameter was generated, recover!  We don't
			 * want to leave holes in the CAST!
			 */
			if (cfunc->params.params_val[cast_idx].type == 0) {
				++cast_index_adjust; /* One less CAST param. */
			} else {
				++idx;
			}
		}
	}
	
	/* Adjust the actual number of parameters, since
	   we may have skipped a few. */
	cfunc->params.params_len -= cast_index_adjust;
}

/* End of file. */

