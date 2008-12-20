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
#include <mom/c/libpres_c.h>
#include <mom/c/libcast.h>

#include "private.hh"

/* presentation generation routine for AOI_UNION type */
pres_c_inline pg_state::p_inline_struct_union(aoi_union *au,
					      p_type_collection */*inl_ptc*/,
					      cast_type inl_ctype)
{
	char *old_name;
	
	pres_c_inline inl;
	pres_c_inline_struct_union *suinl;
	
	p_type_collection *ptc;
	p_type_node *ptn;
	cast_type discrim_ctype, union_ctype, variant_ctype;
	pres_c_mapping discrim_map, union_map, variant_map;
	
	int union_ctype_cases_len, union_ctype_cases_idx;
	unsigned int i;
	int cdef;

	cast_scope *scope;
	
	/* Save the original name context for when we return. */
	old_name = name;
	
	/* Count the number of non-void union variants in our AOI_UNION. */
	union_ctype_cases_len = 0;
	for (i = 0; i < au->cases.cases_len; ++i)
		if (au->cases.cases_val[i].var.type->kind != AOI_VOID)
			++union_ctype_cases_len;
	if (au->dfault && (au->dfault->type->kind != AOI_VOID))
		++union_ctype_cases_len;
	
	/* Create the pres_c_inline_struct_union which will be the return
	   value of this function.  This object will describe how the pieces of
	   our discriminated union (AOI_UNION) match up with the pieces within
	   the C structure definition we are creating (`inl_ctype'). */
	inl = pres_c_new_inline_struct_union(union_ctype_cases_len);
	suinl = &(inl->pres_c_inline_u_u.struct_union);
	
	/* Fill out `suinl'. */
	/* `discrim.index' is the index of the discriminator in the CAST
	   structure definition (`inl_ctype') that we are constructing to
	   represent the current AOI_UNION.  The discriminator will be the
	   zeroth element of that structure (i.e., the first slot). */
	suinl->discrim.index = 0;
	/* `union_index' is the index of the CAST union within the CAST
	   structure defintion that we are constructing.  The slots within this
	   union will correspond to the variants of the AOI_UNION we are
	   processing. */
	suinl->union_index = 1;
	/* `cases' is the array that describe how variants of our discriminated
	   union match up with the variants within the C presentation of that
	   union.  These correspondences are established later in this function
	   (in the `for' loop below). */
	suinl->cases.cases_len = au->cases.cases_len;
	suinl->cases.cases_val =
		(pres_c_inline_struct_union_case *)
		mustmalloc(sizeof(pres_c_inline_struct_union_case) *
			   au->cases.cases_len);
	
	/* Prepare the discriminator mapping, save it in `suinl', and add the
	   slot for the discriminator to our structure definition `inl_ctype'.
	   */
	name = calc_struct_slot_name(au->discriminator.name);
	ptc = 0;
	p_type(au->discriminator.type, &ptc);
	ptn = ptc->find_type("definition");
	discrim_ctype = ptn->get_type();
	discrim_map = ptn->get_mapping();
	suinl->discrim.mapping = discrim_map;
	
	/* The following invocation of `p_inline_add_atom' has the side effect
	   of adding the discriminator slot to the structure definition. */
	p_inline_add_atom(inl_ctype,
			  calc_struct_slot_name(au->discriminator.name),
			  discrim_ctype,
			  discrim_map);
	
	/* Create a cast_union_type to contain the slots that correspond to the
	   variants of our discriminated union. */
	union_ctype = cast_new_aggregate_type(union_aggregate_type, 0);
	union_ctype->cast_type_u_u.agg_type.name =
		cast_new_scoped_name(
			calc_struct_union_tag_name(au->union_label), NULL);
	push_ptr(scope_stack, &union_ctype->cast_type_u_u.agg_type.scope);
	
	scope = (cast_scope *)top_ptr(scope_stack);
	
	/* Compute the name of the union member; this will be used by
	   `p_inline_add_atom' below. */
	name = calc_struct_slot_name(au->union_label);
	
	/* Now process the variants of our discriminated union. */
	for (i = 0, union_ctype_cases_idx = 0; i < au->cases.cases_len; i++) {
		/*
		 * XXX --- We must set `name' here for the benefit of `pg_sun::
		 * p_variable_array_type'.  Grrr.
		 */
		name = calc_struct_slot_name(au->cases.cases_val[i].var.name);
		ptc = 0;
		p_type(au->cases.cases_val[i].var.type,
		       &ptc);
		ptn = ptc->find_type("definition");
		variant_ctype = ptn->get_type();
		variant_map = ptn->get_mapping();
		suinl->cases.cases_val[i].mapping = variant_map;
		
		if (au->cases.cases_val[i].var.type->kind == AOI_VOID) {
			/* In the PRES_C description, we must indicate that
			   this variant of the union doesn't correspond to
			   any member of the `union_ctype'.  (There are no
			   void members!) */
			suinl->cases.cases_val[i].index = -1;
		} else {
			/* Otherwise, we must remember which member of the
			   `union_ctype' corresponds to this AOI union case.
			   */
			suinl->cases.cases_val[i].index =
				union_ctype_cases_idx;
			
			/* Add the member to the `union_ctype'. */
			cdef = cast_add_def(
				scope,
				cast_new_scoped_name(
					calc_struct_slot_name(au->cases.
							      cases_val[i].
							      var.name),
					NULL),
				CAST_SC_NONE,
				CAST_VAR_DEF,
				ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
				current_protection);
			scope->cast_scope_val[cdef].u.cast_def_u_u.var_def.
				type = variant_ctype;
			++union_ctype_cases_idx;
		}
	}
	if (au->dfault) {
		suinl->dfault = (pres_c_inline_struct_union_case *)
				mustmalloc(
					sizeof(pres_c_inline_struct_union_case)
					);
		
		/*
		 * XXX --- We must set `name' here for the benefit of `pg_sun::
		 * p_variable_array_type'.  Grrr.
		 */
		name = calc_struct_slot_name(au->dfault->name);
		ptc = 0;
		p_type(au->dfault->type, &ptc);
		ptn = ptc->find_type("definition");
		variant_ctype = ptn->get_type();
		variant_map = ptn->get_mapping();
		suinl->dfault->mapping = variant_map;
		
		if (au->dfault->type->kind == AOI_VOID) {
			/* As described above, remember that there is no
			   member of the `union_ctype' that corresponds to this
			   case. */
			suinl->dfault->index = -1;
		} else {
			/* Remember which member of the `union_ctype'
			   corresponds to this AOI union case. */
			suinl->dfault->index = union_ctype_cases_len - 1;
			
			/* Add the member to the `union_ctype'. */
			cdef = cast_add_def(
				scope,
				cast_new_scoped_name(
					calc_struct_slot_name(au->dfault->
							      name),
					NULL),
				CAST_SC_NONE,
				CAST_VAR_DEF,
				ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
				current_protection);
			scope->cast_scope_val[cdef].u.cast_def_u_u.var_def.
				type = variant_ctype;
		}
	} else
		/* Remember that there is no default case. */
		suinl->dfault = 0;
	
	/* Create a direct pres_c_mapping for the C union, and add the union
	   slot to the structure definition we are creating (`inl_ctype'). */
	union_map = (pres_c_mapping)
		    mustcalloc(sizeof(struct pres_c_mapping_u));
	union_map->kind = PRES_C_MAPPING_DIRECT;
	
	pop_ptr(scope_stack);
	
	/* The following invocation of `p_inline_add_atom' has the side effect
	   of adding the union slot to the structure definition. */
	p_inline_add_atom(inl_ctype,
			  calc_struct_slot_name(au->union_label),
			  union_ctype,
			  union_map);
	
	/* Restore the name context. */
	name = old_name;
	
	return inl;
}

/* End of file. */

