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

#include <mom/c/pfe.hh>
#include <mom/libaoi.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include "private.hh"

void pg_state::p_marshal_stub_conn(pres_c_marshal_stub *mstub,
				   int func_ref)
{
	cast_func_type *cfunc = &out_pres->stubs_cast.cast_scope_val[func_ref].
		u.cast_def_u_u.func_type;
	cast_func_param *cfield;
	int p;
	
	pres_c_mapping conn_ref_map;
	
	/* Build a C parameter in cfunc->params.params_val[p] that refers to
	   the marshal/unmarshal stream.  Note that the type of the `_stream'
	   argument is `flick_marshal_stream'.  This is an abstract type that
	   will be implemented by the runtime associated with the transport
	   that is eventually chosen.
	   
	   NOTE: It is an extremely bad idea to specialize this method within a
	   particular presentation generator.  If you do so (e.g., to change
	   the type of the `_stream' argument), then your new presentation
	   generator will produce code that can't be used with Flick's runtime
	   libraries.
	   
	   At one time, I thought that it would be a good idea to change the
	   type of `_stream' to be a C pointer.  This is actually a *bad* idea
	   because it constrains the possible ways in which the Flick runtimes
	   can implement streams.  A runtime might implement want to implement
	   `flick_marshal_stream_t' an an integer index into an array of
	   structres, for example.
	   
	   The only restriction that we impose here on the implementation of
	   `flick_marshal_stream_t' is that streams must be passable by value
	   to marshal and unmarshal functions.  Practically, this means that
	   `flick_marshal_stream_t' must be some kind of pointer or index to a
	   structure, but we don't *force* a choice by making `_stream' be a
	   pointer. */
	p = cast_func_add_param(cfunc);
	cfield = &cfunc->params.params_val[p];
	cfield->name = ir_strlit("_stream");
	cfield->type = cast_new_type_name("flick_marshal_stream_t");
	
	/* Make a reference mapping for the `cfield'. */
	/*
	 * XXX --- The `target_i' field for a marshal/unmarshal stub seems to
	 * be obsolete --- perhaps it is a holdover from some past time when
	 * m/u stubs received target object references?  Or was it simply
	 * intended to be a way to find the marshal stream argument (through
	 * the inline atom index `p')?
	 *
	 * In either case, `cfield' does not now refer to an object reference,
	 * so it is wrong to make a PRES_C_MAPPING_REFERENCE for it.  Let's
	 * make a PRES_C_MAPPING_IGNORE instead.
	 */
	conn_ref_map = pres_c_new_mapping(PRES_C_MAPPING_IGNORE);
	
	/* Finally, set the target inline (`target_i') for this stub.  The
           appropriate inline is an inline atom. */
	mstub->target_i = pres_c_new_inline_atom(p, conn_ref_map);
}

void pg_state::p_marshal_stub_data(cast_type ctype, pres_c_mapping map,
				   pres_c_stub_kind kind,
				   pres_c_marshal_stub *mstub,
				   int func_ref)
{
	cast_func_type *cfunc = &out_pres->stubs_cast.cast_scope_val[func_ref].
		u.cast_def_u_u.func_type;
	cast_func_param *cfield;
	pres_c_inline inl;
	int p;
	
	cfunc->return_type = cast_new_prim_type(CAST_PRIM_INT,0);
	
	/* Build a C parameter in cfunc->params.params_val[p] to refer to the
	   data to be marshaled/unmarshaled.  NOTE that the type of this
	   parameter will be modified later on in this function to be a pointer
	   to the data to be processed. */
	p = cast_func_add_param(cfunc);
	cfield = &cfunc->params.params_val[p];
	cfield->name = ir_strlit("_data");
	switch (kind) {
	case PRES_C_MARSHAL_STUB:
		cfield->type = ctype;
		break;
	case PRES_C_UNMARSHAL_STUB:
		cfield->type = ctype;
		break;
	default:
		panic("In `pg_state::p_marshal_stub_data', "
		      "the marshal stub type is unrecognized.");
		break;
	}
	
	/*
	 * Modify the parameter and mapping by inserting a level of indirection
	 * through a C pointer.
	 *
	 * We do *not* want allocation for indirection pointers (they are
	 * already provided to us correctly when params are passed by ref).
	 *
	 * Since m/u stubs are specialized by the back end, we must set each
	 * valid case.  UNKNOWN should never be associated with a generated m/u
	 * stub, so we set it to invalid.
	 */
	pres_c_allocation alloc;
	pres_c_allocation_u indir;

	indir.allow = PRES_C_ALLOCATION_ALLOW;
	indir.pres_c_allocation_u_u.val.flags
		= PRES_C_ALLOC_NEVER | PRES_C_DEALLOC_NEVER;
	indir.pres_c_allocation_u_u.val.allocator.kind
		= PRES_C_ALLOCATOR_DONTCARE;
	indir.pres_c_allocation_u_u.val.alloc_init = 0;
	
	alloc.cases[PRES_C_DIRECTION_IN]
		= alloc.cases[PRES_C_DIRECTION_INOUT]
		= alloc.cases[PRES_C_DIRECTION_OUT]
		= alloc.cases[PRES_C_DIRECTION_RETURN]
		= indir;
	
	alloc.cases[PRES_C_DIRECTION_UNKNOWN].allow
		= PRES_C_ALLOCATION_INVALID;
	
	pres_c_interpose_indirection_pointer(&(cfield->type), &map, alloc);
	
	/* Build a PRES_C inline atom for this parameter. */
	inl = pres_c_new_inline_atom(p, map);
	
	/* Save the inline that describes how to process the `_data' parameter
           along with the marshal/unmarshal stub's definition. */
	
	mstub->i = inl;
}

void pg_state::p_mu_stub(cast_type ctype_name, pres_c_mapping map,
			 mint_ref itype,
			 pres_c_stub_kind kind, char *func_name,
			 pres_c_mapping seethru_map)
{
	p_type_collection *ptc;
	p_type_node *ptn;
	int func_cdef;
	int stub;
	pres_c_marshal_stub *mstub;
	cast_type ctype;
	int aidx;
	
	/*
	 * Create the stub function declaration and stub mapping definition.
	 * Note that this much like a *template* for a stub.  If/when the back
	 * end decides a stub is necessary for this data type, it will generate
	 * a specialized stub (currently this means the direction will be
	 * known, and the allocation semantics will thus be correct).  This is
	 * why we set the inclusion flag to SUPPRESSED.
	 */
	cast_scoped_name scn = cast_new_scoped_name(func_name, NULL);
	aidx = aoi_get_parent_scope(in_aoi, cur_aoi_idx);
	func_cdef = cast_add_def(&out_pres->stubs_cast,
				 scn,
				 CAST_SC_NONE,
				 CAST_FUNC_DECL,
				 (pg_channel_maps[(gen_client)?
						 PG_CHANNEL_CLIENT_IMPL:
						 PG_CHANNEL_SERVER_IMPL]
				  [builtin_file]),
				 current_protection);
	stub = p_add_stub(out_pres);
	s(stub).kind = kind;
	mstub = &s(stub).pres_c_stub_u.mstub;
	mstub->c_func = func_cdef;
	mstub->itype = itype;
	mstub->seethru_map = seethru_map;
	
	/* Figure out how the stub will find the marshal/unmarshal stream. */
	p_marshal_stub_conn(mstub, func_cdef);
	
	/* Add a parameter pointing to the actual data to marshal. */
	p_marshal_stub_data(ctype_name, map, kind, mstub, func_cdef);
	
	ptc = p_type_collection::find_collection(&type_collections,
						 cur_aoi_idx);
	ptn = new p_type_node;
	ptn->set_flags(PTF_REF_ONLY|PTF_NO_REF);
	ptn->set_name((kind == PRES_C_MARSHAL_STUB) ?
		      "marshal_stub" :
		      "unmarshal_stub");
	ptn->set_format(func_name);
	ctype = cast_new_type(CAST_TYPE_FUNCTION);
	ctype->cast_type_u_u.func_type = out_pres->stubs_cast.
		cast_scope_val[func_cdef].u.cast_def_u_u.func_type;
	ptn->set_type(ctype);
	ptc->add_type("root", ptn, 1, 0);
	ptn->set_channel((pg_channel_maps[(gen_client)?
					 PG_CHANNEL_CLIENT_DECL:
					 PG_CHANNEL_SERVER_DECL]
			  [builtin_file]));
}

/* End of file. */

