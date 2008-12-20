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

#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include <mom/c/pg_corba.hh>

/*
 * XXX --- Note that this code is largely ineffectual because our current CORBA
 * front end always makes EMPTY enumerations!  It makes an `aoi_def' for each
 * member, and those are handled by `p_const_def'.  See `fe/newcorba/parser.yy'
 * for the gory details of why things are done this way.
 *
 * The upshot, however, is that we can't control the names of our enumeration
 * members through `calc_enum_member_name'.
 */
void pg_corba::p_enum_type(aoi_enum *ae,
			   p_type_collection **out_ptc)
{
	int curval;

	char *member_name;
	int cdef;
	
	/*****/
	
	curval = 0;
	for (unsigned int i = 0; i < ae->defs.defs_len; i++) {
		member_name = calc_enum_member_name(ae->defs.defs_val[i].name);
		cdef = cast_add_def((cast_scope *)top_ptr(scope_stack),
				    cast_new_scoped_name(member_name, NULL),
				    CAST_SC_NONE,
				    CAST_DEFINE,
				    ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
				    CAST_PROT_NONE);
		
		if (ae->defs.defs_val[i].type->kind != AOI_CONST)
			c(cdef).u.cast_def_u_u.define_as =
				cast_new_expr_lit_int(curval,
						      (CAST_MOD_SIGNED
						       | CAST_MOD_LONG));
		else {
			c(cdef).u.cast_def_u_u.define_as =
				p_const_translate(&ae->defs.defs_val[i].type->
						  aoi_type_u_u.const_def);
			curval = ae->defs.defs_val[i].type->
				 aoi_type_u_u.const_def.value->aoi_const_u_u.
				 const_int;
		}
		curval++;
	}
	
	if (*out_ptc)
		(*out_ptc)->set_collection_ref(
			prim_collections[PRIM_COLLECTION_ENUM]
			);
	else
		*out_ptc = prim_collections[PRIM_COLLECTION_ENUM];
}

/* End of file. */

