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

#include <mom/compiler.h>
#include <mom/c/pfe.hh>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include <mom/c/pg_corbaxx.hh>

/*
 * If you override this method, you should also override `p_interface_type',
 * which handles normal interface declarations.
 */
void pg_corbaxx::p_forward_type(p_type_collection **out_ptc)
{
	p_type_collection *ptc;
	p_type_node *ptn;
	cast_scoped_name scn, scn_cp;
	cast_type ctype;
	pres_c_mapping map, ref_map;
	tag_list *tl;
	tag_data *pt_td;
	union tag_data_u data;
	char *int_name;
	unsigned int lpc;
	cast_type u32 = cast_new_prim_type(CAST_PRIM_INT,
					   CAST_MOD_UNSIGNED);
	pres_c_inline alloc_inl =
		pres_c_new_inline(PRES_C_INLINE_ALLOCATION_CONTEXT);
	pres_c_inline_allocation_context *ac =
		&alloc_inl->pres_c_inline_u_u.acontext;
	
	ac->arglist_name = pres_c_make_arglist_name("objref");
	
	int_name = calc_name_from_ref(cur_aoi_idx);
	scn = cast_new_scoped_name(int_name, NULL);
	
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
	
	ctype = cast_new_type_scoped_name(scn);
	
	ptc = p_new_type_collection("");
	
	if( *out_ptc )
		(*out_ptc)->set_collection_ref(ptc);
	else
		*out_ptc = ptc;
	
	/* Construct the type collection for the interface */
	ptc = *out_ptc;
	pt_td = &(find_tag(out_pres->pres_attrs, "pres_type")->data);
	tl = create_tag_list(0);
	data.tl = tl;
	ptc->set_attr_index(append_tag_data(pt_td, data));
	ptc->set_tag_list(tl);
	add_tag(tl, "name", TAG_STRING, ptc->get_name());
	add_tag(tl, "idl_type", TAG_STRING, "interface");
	add_tag(tl, "size", TAG_STRING, "variable");
	
	ptn = new p_type_node;
	ptn->set_name("forward_declaration");
	ptn->set_format(int_name);
	ctype = cast_new_type(CAST_TYPE_CLASS_NAME);
	ctype->cast_type_u_u.class_name = scn;
	ptn->set_type(ctype);
	ptn->set_flags(PTF_REF_ONLY|PTF_NO_REF);
	ptc->add_type("default", ptn, 1, 0);
	
	ptn = new p_type_node;
	ptn->set_name("pointer");
	ptn->set_format("%s_ptr");
	scn_cp = cast_copy_scoped_name(&current_scope_name);
	if( scn_cp.cast_scoped_name_len > 1 )
		cast_prepend_scope_name(&scn_cp, "", null_template_arg_array);
	cast_add_scope_name(&scn_cp, int_name, null_template_arg_array);
	ctype = cast_new_type_scoped_name(scn_cp);
	ptn->set_type(cast_new_pointer_type(ctype));
	ptn->set_mapping(map);
	ptc->add_type("default", ptn);
	
	ptn = new p_type_node;
	ptn->set_name("definition");
	ptn->set_format("%s");
	ctype = cast_new_type_scoped_name(scn);
	ptn->set_type(ctype);
	ptn->set_mapping(map);
	ptn->set_flags(PTF_NO_REF);
	ptn->set_channel(ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL));
	ptc->add_type("default", ptn);

	pres_c_mapping xmap;
	
	ptn = new p_type_node;
	ptn->set_name("managed_object");
	ptn->set_format("%s");
	ptn->set_flags(PTF_REF_ONLY|PTF_NO_DEF);
	scn_cp = cast_copy_scoped_name(&current_scope_name);
	if( scn_cp.cast_scoped_name_len > 1 )
		cast_prepend_scope_name(&scn_cp, "", null_template_arg_array);
	cast_add_scope_name(&scn_cp, int_name, null_template_arg_array);
	ctype = cast_new_type_scoped_name(scn_cp);
	ptn->set_type(ctype);
	ref_map = pres_c_new_mapping(PRES_C_MAPPING_REFERENCE);
	ref_map->pres_c_mapping_u_u.ref.kind = PRES_C_REFERENCE_COPY;
	ref_map->pres_c_mapping_u_u.ref.ref_count = 1;
	ref_map->pres_c_mapping_u_u.ref.arglist_name = ir_strlit("");
	xmap = PRES_C_M_XLATE,
		PMA_InternalCType, cast_new_pointer_type(ctype),
		PMA_InternalMapping, ref_map,
		END_PRES_C;
	ptn->set_mapping(xmap);
	ptc->add_type("default", ptn, 1, 0);
	
	ptc->set_id(get_repository_id(cur_aoi_idx));
	
	p_type_var(ptc, a(cur_aoi_idx).binding);
	
	p_out_class(ptc, a(cur_aoi_idx).binding);
	
	cast_scope dummy_scope = cast_new_scope(0);
	aoi_type at;
	
	at = a(cur_aoi_idx).binding;
	
	/* If a forward interface has no definition we still
	   need to generate its interface functions for
	   reference in the back end */
	if( (at->kind == AOI_FWD_INTRFC) &&
	    (at->aoi_type_u_u.fwd_intrfc_def == -1) ) {
		a(cur_aoi_idx).idl_file = builtin_file;
		cast_add_scope_name(&current_scope_name,
				    name,
				    null_template_arg_array);
		p_interface_funcs(ptc, &dummy_scope);
		cast_del_scope_name(&current_scope_name);
	}
	p_poa_class(ptc);
	p_poa_tie(ptc);
	
	/* Set the format for the definition to "" so that it won't
	   be output, we just need to write out a forward class
	   definition.  If the interface is defined then it will
	   replace these definition's */
	ptc->find_type("definition")->set_format("");
	ptc->get_collection_ref()->find_type("definition")->set_format("");
}

/* End of file. */

