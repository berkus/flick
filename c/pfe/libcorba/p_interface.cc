/*
 * Copyright (c) 1995, 1996, 1997, 1998 The University of Utah and
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

#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include <mom/c/pg_corba.hh>

void pg_corba::p_interface_def_typedef(aoi_interface *ai)
{
	aoi_type_u at;
	
	at.kind                       = AOI_INTERFACE;
	at.aoi_type_u_u.interface_def = *ai;
	
	/* Define the interface type as a data object with real marshal and
	   unmarshal functions. */
	p_typedef_def(&at);
	
	if (async_stubs)
		p_interface_async_msg_types(ai);
}

void pg_corba::p_forward_type(p_type_collection **out_ptc)
{
	pres_c_mapping map;
	p_type_node *ptn;
	unsigned int lpc;
	cast_type u32 = cast_new_prim_type(CAST_PRIM_INT,
					   CAST_MOD_UNSIGNED);
	pres_c_inline alloc_inl =
		pres_c_new_inline(PRES_C_INLINE_ALLOCATION_CONTEXT);
	pres_c_inline_allocation_context *ac =
		&alloc_inl->pres_c_inline_u_u.acontext;
	
	pg_state::p_forward_type(out_ptc);
	
	ac->arglist_name = pres_c_make_arglist_name("objref");
	
	ac->alloc = p_get_allocation();
	for( lpc = 0; lpc < PRES_C_DIRECTIONS; lpc++ ) {
		switch( ac->alloc.cases[lpc].allow ) {
		case PRES_C_ALLOCATION_ALLOW:
			ac->alloc.cases[lpc].pres_c_allocation_u_u.val.
				flags &= ~(PRES_C_ALLOC_ALWAYS|
					   PRES_C_RUN_CTOR|
					   PRES_C_RUN_DTOR);
			ac->alloc.cases[lpc].pres_c_allocation_u_u.val.
				allocator.pres_c_allocator_u.name =
				ir_strlit("CORBA_object");
			break;
		default:
			break;
		}
	}
	ac->alloc.cases[PRES_C_DIRECTION_UNKNOWN].
		pres_c_allocation_u_u.val.flags &=
		~PRES_C_DEALLOC_ALWAYS;
	if( gen_server ) {
		ac->alloc.cases[PRES_C_DIRECTION_RETURN].
			pres_c_allocation_u_u.val.flags |=
			PRES_C_DEALLOC_ALWAYS;
		ac->alloc.cases[PRES_C_DIRECTION_INOUT].
			pres_c_allocation_u_u.val.flags |=
			PRES_C_DEALLOC_ALWAYS;
		ac->alloc.cases[PRES_C_DIRECTION_OUT].
			pres_c_allocation_u_u.val.flags |=
			PRES_C_DEALLOC_ALWAYS;
	} else if( gen_client ) {
		ac->alloc.cases[PRES_C_DIRECTION_INOUT].
			pres_c_allocation_u_u.val.flags |=
			PRES_C_DEALLOC_ALWAYS;
	}
	map = pres_c_new_mapping(PRES_C_MAPPING_REFERENCE);
	map->pres_c_mapping_u_u.ref.kind = PRES_C_REFERENCE_COPY;
	map->pres_c_mapping_u_u.ref.ref_count = 1;
	map->pres_c_mapping_u_u.ref.arglist_name = ac->arglist_name;
	ac->ptr = pres_c_new_inline_atom(0, map);
	ac->length
		= PRES_C_I_TEMPORARY,
		    PIA_Name, "array_len",
		    PIA_CType, u32,
		    PIA_Value, cast_new_expr_lit_int(1, 0),
		    PIA_IsConst, 1,
		    PIA_TempType, TEMP_TYPE_ENCODED,
		    PIA_Mapping, PRES_C_M_ARGUMENT,
		      PMA_ArgList, ac->arglist_name,
		      PMA_Name, "length",
		      PMA_Mapping, NULL,
		      END_PRES_C,
		    END_PRES_C;
	ac->min_len
		= PRES_C_I_TEMPORARY,
		    PIA_Name, "array_len",
		    PIA_CType, u32,
		    PIA_Value, cast_new_expr_lit_int(1, 0),
		    PIA_IsConst, 1,
		    PIA_Mapping, PRES_C_M_ARGUMENT,
		      PMA_ArgList, ac->arglist_name,
		      PMA_Name, "min_len",
		      PMA_Mapping, NULL,
		      END_PRES_C,
		    END_PRES_C;
	
	ac->max_len
		= PRES_C_I_TEMPORARY,
		    PIA_Name, "array_len",
		    PIA_CType, u32,
		    PIA_Value, cast_new_expr_lit_int(1, 0),
		    PIA_IsConst, 1,
		    PIA_Mapping, PRES_C_M_ARGUMENT,
		      PMA_ArgList, ac->arglist_name,
		      PMA_Name, "max_len",
		      PMA_Mapping, NULL,
		      END_PRES_C,
		    END_PRES_C;
	ac->min_alloc_len = PRES_C_I_TEMPORARY,
			      PIA_Name, "array_len",
			      PIA_CType, u32, 
			      PIA_Value, cast_new_expr_lit_int(1, 0),
			      PIA_IsConst, 1,
			      PIA_Mapping, PRES_C_M_ARGUMENT,
			        PMA_ArgList, ac->arglist_name,
			        PMA_Name, "min_alloc_len",
			        PMA_Mapping, NULL,
			        END_PRES_C,
			      END_PRES_C;
	
	map = pres_c_new_mapping(PRES_C_MAPPING_SINGLETON);
	map->pres_c_mapping_u_u.singleton.inl = alloc_inl;
	
	ptn = (*out_ptc)->get_collection_ref()->find_type("definition");
	ptn->set_mapping(map);
}

void pg_corba::p_interface_type(aoi_interface * /*ai*/,
				p_type_collection **out_ptc)
{
	/* Call the method that we use for forward interface declarations. */
	p_forward_type(out_ptc);
}

/* End of file. */

