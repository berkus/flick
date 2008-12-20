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

#include <mom/libaoi.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include <mom/c/pg_corbaxx.hh>

void pg_corbaxx::p_const_def(aoi_const_def *acd)
{
	aoi_ref parent_ref;
	cast_type const_type;
	cast_def_kind def_kind;
	char *const_name;
	
	/* Define the C type with a #define statement. */
	parent_ref = aoi_get_parent_scope(in_aoi, cur_aoi_idx);
	const_type = p_const_type(acd);
	const_type = cast_new_qualified_type(const_type, CAST_TQ_CONST);
	if( parent_ref >= 0 ) {
		switch(a(parent_ref).binding->kind) {
		case AOI_NAMESPACE:
			def_kind = CAST_VAR_DEF;
			break;
		case AOI_STRUCT:
		case AOI_INTERFACE:
			def_kind = CAST_VAR_DECL;
			break;
		default:
			panic( "Unknown aoi scope %d",
			       a(parent_ref).binding->kind );
			break;
		}
	}
	else {
		def_kind = CAST_VAR_DEF;
	}
	int cdef = cast_add_def((cast_scope *)top_ptr(scope_stack),
				cast_new_scoped_name(
					const_name =
					calc_const_name(a(cur_aoi_idx).name),
					NULL),
				CAST_SC_STATIC,
				def_kind,
				ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
				CAST_PROT_NONE);
	if( def_kind == CAST_VAR_DEF ) {
		c(cdef).u.cast_def_u_u.var_def.type = const_type;
		c(cdef).u.cast_def_u_u.var_def.init =
			cast_new_init_expr(p_const_translate(acd));
	} else {
		c(cdef).u.cast_def_u_u.var_type = const_type;
		if( gen_client ) {
			cdef = cast_add_def(&out_pres->pres_cast,
					    calc_const_scoped_name(parent_ref,
								   const_name),
					    CAST_SC_NONE,
					    CAST_VAR_DEF,
					    ch(cur_aoi_idx,
					       PG_CHANNEL_CLIENT_IMPL),
					    CAST_PROT_NONE);
			out_pres->pres_cast.cast_scope_val[cdef].u.
				cast_def_u_u.var_def.type = const_type;
			out_pres->pres_cast.cast_scope_val[cdef].u.
				cast_def_u_u.var_def.init =
				cast_new_init_expr(p_const_translate(acd));
		}
	}
}

/* End of file. */

