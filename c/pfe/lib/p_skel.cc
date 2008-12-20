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
#include <stdlib.h>

#include <mom/compiler.h>
#include <mom/libaoi.h>
#include <mom/libmint.h>
#include <mom/c/pfe.hh>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include "private.hh"

#define FLICK_CLIENT_SKEL_TYPE_NAME "flick_dispatch_t"
#define FLICK_SERVER_SKEL_TYPE_NAME "flick_dispatch_t"

/* Generate server skeleton stub presentation for an AOI interface. */
int pg_state::p_skel(aoi_interface * /*ai*/)
{
	pres_c_skel *skel;
	char *skel_name;
	int skel_index;
	
	aoi_ref saved_parent_ref, saved_derived_ref;
	
	/* Save the original name context for when we return. */
	char *old_name = name;
	
	/*
	 * Set the `pg_state' interface reference data members to point at the
	 * interfaces under consideration so that `pg_state::calc_name' can
	 * compute names.
	 */
	saved_parent_ref = parent_interface_ref;
	saved_derived_ref = derived_interface_ref;
	
	parent_interface_ref = cur_aoi_idx;
	derived_interface_ref = cur_aoi_idx;
	
	/* Allocate a PRES_C_STUB for this interface. */
	skel_index = p_add_stub(out_pres);
	
	/*
	 * Initialize the skeleton.
	 * Prepare the skeleton name.
	 * Set `skel->c_def' to the C declaration for this skeleton.
	 */
	if (gen_server) {
		s(skel_index).kind = PRES_C_SERVER_SKEL;
		skel = &s(skel_index).pres_c_stub_u.sskel;
		skel_name = calc_server_skel_name("");
		skel->c_def = p_skel_cdef(skel_name,
					  FLICK_SERVER_SKEL_TYPE_NAME);
	} else {
		s(skel_index).kind = PRES_C_CLIENT_SKEL;
		skel = &s(skel_index).pres_c_stub_u.cskel;
		skel_name = calc_client_skel_name("");
		skel->c_def = p_skel_cdef(skel_name,
					  FLICK_CLIENT_SKEL_TYPE_NAME);
	}
	out_pres->stubs_cast.cast_scope_val[skel->c_def].
		channel = ch(cur_aoi_idx, PG_CHANNEL_SERVER_IMPL);
	name = skel_name;
	
	/*
	 * Set `skel->request_itype' and `skel->reply_itype' to the top-level
	 * MINT message union.
	 */
	skel->request_itype = top_union;
	skel->reply_itype = top_union;
	
	/* Generate presentations for the work functions. */
	skel->funcs.funcs_len = 0;
	skel->funcs.funcs_val = 0;
	p_skel_internal(cur_aoi_idx, cur_aoi_idx, skel_index);
	
	/* Restore the name context */
	name = old_name;
	
	/*
	 * Finally, restore the `pg_state' AOI interface references that we
	 * changed previously.
	 */
	parent_interface_ref = saved_parent_ref;
	derived_interface_ref = saved_derived_ref;
	return( skel_index );
}

void pg_state::p_skel_internal(aoi_ref this_ref,
			       aoi_ref derived_ref,
			       int skel_index)
{
	aoi_interface *this_interface, *derived_interface;
	pres_c_skel *skel;
	aoi_ref saved_parent_ref, saved_derived_ref;
	aoi_ref saved_cur_idx;
	
	if (gen_server)
		skel = &s(skel_index).pres_c_stub_u.sskel;
	else
		skel = &s(skel_index).pres_c_stub_u.cskel;
	
	u_int ops_len;
#ifdef ATTRIBUTE_STUBS
	u_int attribs_len;
#endif /* ATTRIBUTE_STUBS */
	u_int parents_len;
	
	u_int i;
	
	int flags;
	
	pres_c_func *funcs;
	u_int funcs_size;
	u_int funcs_count;
	
	/*
	 * Set `this_interface' and `derived_interface' to point at the AOI
	 * interfaces under consideration.  We also save the cur_aoi_idx
	 * so that we can generate the proper names when
	 * server_funcs_for_inherited_operations == 0.
	 */
	assert((this_ref >= 0)
	       && (this_ref < ((aoi_ref) in_aoi->defs.defs_len)));
	assert(a(this_ref).binding
	       && (a(this_ref).binding->kind == AOI_INTERFACE));
	
	this_interface = &(a(this_ref).binding->aoi_type_u_u.interface_def);
	
	assert((derived_ref >= 0)
	       && (derived_ref < ((aoi_ref) in_aoi->defs.defs_len)));
	assert(a(derived_ref).binding
	       && (a(derived_ref).binding->kind == AOI_INTERFACE));
	
	derived_interface = &(a(derived_ref).binding->aoi_type_u_u.
			      interface_def);
	
	saved_cur_idx = cur_aoi_idx;
	if( !server_funcs_for_inherited_operations )
		cur_aoi_idx = this_ref;
	
	/*
	 * Set the `pg_state' interface reference data members to point at the
	 * interfaces under consideration so that `pg_state::calc_name' can
	 * compute names.
	 */
	saved_parent_ref = parent_interface_ref;
	saved_derived_ref = derived_interface_ref;
	
	parent_interface_ref = this_ref;
	/*
	 * If we aren't supposed to generate separate server funcs then
	 * we set it to the current interface to fake out calc_name
	 */
	if( server_funcs_for_inherited_operations )
		derived_interface_ref = derived_ref;
	else
		derived_interface_ref = this_ref;
	
	/* Get the lengths of arrays that are part of `this_interface'. */
	ops_len = this_interface->ops.ops_len;
#ifdef ATTRIBUTE_STUBS
	attribs_len = this_interface->attribs.attribs_len;
#endif
	parents_len = this_interface->parents.parents_len;
	
	/*
	 * Count the number of server functions that we might generate as part
	 * of `this_interface' and allocate a vector to hold the PRES_C stubs.
	 */
	funcs_size = ops_len;
#ifdef ATTRIBUTE_STUBS
	for (i = 0; i < attribs_len; i++)
		funcs_size +=
			((this_interface->attribs.attribs_val[i].readonly) ?
			 1 : /* Read-only attributes result in one stub. */
			 2); /* Read/write attributes result in two. */
#endif /* ATTRIBUTE_STUBS */
	
	funcs = (pres_c_func *)
		 mustmalloc(sizeof(pres_c_func) * funcs_size);
	funcs_count = 0;
	
	/* Generate the send message function for asynchronous messages. */
	if (async_stubs && derived_interface == this_interface) {
		p_send_stub(derived_interface, 1 /* request */);
		p_send_stub(derived_interface, 0 /* reply */);
		p_continue_func_types(derived_interface);
	}
	
	/*
	 * Generate the stubs for the operations defined by `this_interface',
	 * which are inherited by `derived_interface'.
	 */
	for (i = 0; i < ops_len; ++i) {
		aoi_operation *ao = &(this_interface->ops.ops_val[i]);
		
		/*
		 * We must check if we have already generated a stub for this
		 * (interface, operation) pair.  This may occur, for example,
		 * when `this_interface' is inherited through multiple paths
		 * to `derived_interface'.
		 */
		flags = p_interface_table_get_value(derived_interface, ao);
		
		if (!(flags & P_SERVER_SKEL_OP_MARK)) {
			p_interface_table_set_value(derived_interface, ao,
						    (flags
						     | P_SERVER_SKEL_OP_MARK));
			
			/*
			 * Generate message m/u and continuation stubs
			 * for the server side.
			 * (Client side is done in p_client_stub.)
			 */
			if (async_stubs) {
				if (gen_server) {
					p_message_marshal_stub(
						derived_interface, ao,
						0 /* server */,
						1 /* request */);
					p_message_marshal_stub(
						derived_interface, ao,
						0 /* server */,
						0 /* reply */);
					
					p_continue_stub(derived_interface, ao,
							1 /* request */);
					/*
					 * p_message_marshal_stub() and
					 * p_continue_stub() make new stubs,
					 * and can thus realloc the stubs
					 * list.  Here we reset sskel to point
					 * to the reallocated list.
					 */
					skel = &s(skel_index).pres_c_stub_u.
					       sskel;
				}
				
				funcs[funcs_count] =
					p_receive_func(derived_interface, ao,
						       ((gen_server) ?
							1 /* request */ :
							0 /* reply */));
				/*
				 * Make sure a func was made.
				 * (``oneway'' ops will not generate a func.)
				 */
				if (funcs[funcs_count].kind != -1)
					funcs_count++;
				
			} else {
				funcs[funcs_count++] =
					p_server_func(derived_interface, ao);
			}
		}
	}
	
#ifdef ATTRIBUTE_STUBS
	/*
	 * Generate the stubs for the attributes defined by `this_interface',
	 * which are inherited by `derived_interface'.
	 */
	for (i = 0; i < attribs_len; ++i) {
		aoi_attribute *aa = &(this_interface->attribs.attribs_val[i]);
		
		/*
		 * As above, we must screen out attributes that we have already
		 * processed.
		 */
		flags = p_interface_table_get_value(derived_interface, aa);
		
		if (!(flags & P_SERVER_SKEL_OP_MARK)) {
			p_interface_table_set_value(derived_interface, aa,
						    (flags
						     | P_SERVER_SKEL_OP_MARK));
			funcs[funcs_count++] =
				p_server_attrib_func(derived_interface,
						     aa,
						     /* XXX: READ */);
			if (!(aa->readonly))
				funcs[funcs_count++] =
					p_server_attrib_func(derived_interface,
							     aa,
							     /* XXX: WRITE */);
		}
	}
#endif /* ATTRIBUTE_STUBS */
	
	/* Incorporate the work functions that we just made into `skel'. */
	if (funcs_count > 0) {
		skel->funcs.funcs_val =
			(pres_c_func *)
			mustrealloc(skel->funcs.funcs_val,
				    (sizeof(pres_c_func)
				     * (skel->funcs.funcs_len
					+ funcs_count)));
		
		for (i = 0; i < funcs_count; ++i)
			skel->funcs.funcs_val[i + skel->funcs.funcs_len] =
				funcs[i];
		
		skel->funcs.funcs_len += funcs_count;
	}
	free(funcs);
	
	/*
	 * Generate the stubs for the operations and attributes that are
	 * inherited from parents of `this_interface'.
	 */
	for (i = 0; i < parents_len; ++i) {
		aoi_type parent_val = this_interface->parents.parents_val[i];
		aoi_ref  parent_ref;
		aoi_interface *parent_interface;
		
		/*
		 * All parent references must be through indirects so that
		 * we can find the name to go with the parent interface!
		 */
		assert(parent_val->kind == AOI_INDIRECT);
		
		parent_ref = aoi_deref_fwd(in_aoi, parent_val->
					   aoi_type_u_u.indirect_ref);
		
		assert(parent_ref != -1);
		parent_interface = &(a(parent_ref).binding->aoi_type_u_u.
				     interface_def);
		
		/*
		 * As above, we check to see if we've already processed this
		 * parent.  (If we didn't do this check we would still generate
		 * correct code because all of the parent's operations and
		 * attributes are marked separately.  But doing this check can
		 * save us time.)
		 */
		flags = p_interface_table_get_value(derived_interface,
						    parent_interface);
		if (!(flags & P_SERVER_SKEL_PARENT_MARK)) {
			p_interface_table_set_value(
				derived_interface,
				parent_interface,
				(flags | P_SERVER_SKEL_PARENT_MARK));
			p_skel_internal(parent_ref,
					derived_ref,
					skel_index);
		}
	}
	/* NOTE: The recursive call to p_skel_internal() may invalidate the
           variable ``skel''. */
	
	/* Let the presentation generator add its own server functions */
	if (gen_server)
		p_add_builtin_server_func(this_interface,
					  a(cur_aoi_idx).name,
					  &s(skel_index).pres_c_stub_u.sskel);
	
	/*
	 * Finally, restore the `pg_state' AOI interface references that we
	 * changed previously.
	 */
	cur_aoi_idx = saved_cur_idx;
	parent_interface_ref = saved_parent_ref;
	derived_interface_ref = saved_derived_ref;
}

/***** Auxiliary functions. *****/

/*
 * `p_skel_cdef' allocates and returns an index to the CAST declaration
 * of a skeleton (a.k.a. a client or server dispatch function).
 */
cast_ref pg_state::p_skel_cdef(const char *skel_name,
			       const char *skel_type_name)
{
	cast_ref  cdef;
	cast_ref  skel_cref = cast_add_def((cast_scope *)top_ptr(scope_stack),
					   cast_new_scoped_name(skel_name,
								NULL),
					   CAST_SC_EXTERN,
					   CAST_VAR_DECL,
					   ch(cur_aoi_idx,
					      PG_CHANNEL_SERVER_DECL),
					   current_protection);
	cast_def *skel_cdef = &(((cast_scope *)top_ptr(scope_stack))->
				cast_scope_val[skel_cref]);
	skel_cdef->u.cast_def_u_u.var_type =
		cast_new_type_name(skel_type_name);
	cdef = cast_add_def(
		&out_pres->stubs_cast,
		gen_server ? calc_server_skel_scoped_name(cur_aoi_idx,
							  skel_name) :
		calc_client_skel_scoped_name(cur_aoi_idx,
					    skel_name),
		CAST_SC_EXTERN,
		CAST_VAR_DECL,
		ch(cur_aoi_idx, PG_CHANNEL_SERVER_IMPL),
		CAST_PROT_NONE);
	out_pres->stubs_cast.cast_scope_val[cdef].u = skel_cdef->u;
	return cdef;
}

/* End of file. */

