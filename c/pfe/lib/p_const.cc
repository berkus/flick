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

#include "private.hh"

// this builds a '#define' statement for the constant mapping
void pg_state::p_const_def(aoi_const_def *ac)
{
	cast_scope *deep_scope = (cast_scope *)top_ptr(scope_stack);
	cast_scoped_name scn =
		cast_new_scoped_name(calc_const_name(a(cur_aoi_idx).
						     name), NULL);
	if( cast_find_def(&deep_scope, scn, CAST_DEFINE) != -1 )
		return;
	
	/* Define the C type with a #define statement. */
	int cdef = cast_add_def((cast_scope *)top_ptr(scope_stack),
				scn,
				CAST_SC_NONE,
				CAST_DEFINE,
				ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
				CAST_PROT_NONE);
       	c(cdef).u.cast_def_u_u.define_as = p_const_translate(ac);
}

cast_expr pg_state::p_const_translate(aoi_const_def *acd)
{
	aoi_const ac = acd->value;
	cast_expr res;
	
	res = (cast_expr) mustcalloc(sizeof(cast_expr_u));
	
	switch(ac->kind) {
	case AOI_CONST_INT:
		res = cast_new_expr_lit_int(ac->aoi_const_u_u.const_int,
					    (CAST_MOD_SIGNED | CAST_MOD_LONG));
		break;
		
	case AOI_CONST_CHAR:
		res = cast_new_expr_lit_char(ac->aoi_const_u_u.const_char,
					     CAST_MOD_SIGNED);
		break;
		
	case AOI_CONST_FLOAT:
		res = cast_new_expr_lit_float(ac->aoi_const_u_u.const_float);
		break;
		
	case AOI_CONST_ARRAY: {
		/* XXX Currently, this code can handle strings. */
		aoi_const_array array = ac->aoi_const_u_u.const_array;
		int len = ac->aoi_const_u_u.const_array.aoi_const_array_len;
		int pos;
		char *thename;
		
		/* AOI_CONST_ARRAY's are not NUL terminated, so we must add one
		   to the array length for malloc. */
		thename = (char *) mustmalloc(sizeof(char) * (len + 1));
		for (pos = 0; pos < len; ++pos) {
			if (array.aoi_const_array_val[pos]->kind !=
			    AOI_CONST_CHAR)
				panic("Unable to translate const array element of type %d.\n",
				      array.aoi_const_array_val[pos]->kind);
			thename[pos] = array.aoi_const_array_val[pos]->
				       aoi_const_u_u.const_char;
		}
		thename[len] = 0;
		res = cast_new_expr_lit_string(thename);
		break;
	}
	
	case AOI_CONST_STRUCT:
		panic("Don't know how to deal with constant structures!\n");
		break;
		
	default:
		panic("Unknown AOI_CONST type %d\n",ac->kind);
		break;
	}
	
	return res;
}

cast_type pg_state::p_const_type(aoi_const_def *acd)
{
	p_type_collection *ptc = 0;
	cast_type retval;
	
	p_type(acd->type, &ptc);
	retval = ptc->find_type("definition")->get_type();
	return( retval );
}

/* End of file. */

