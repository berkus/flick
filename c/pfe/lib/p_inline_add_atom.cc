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

#include <mom/c/libpres_c.h>

#include "private.hh"

pres_c_inline pg_state::p_inline_add_atom(cast_type /* inl_ctype */,
					  char *atom_name,
					  cast_type atom_ctype,
					  pres_c_mapping atom_mapping)
{
	cast_scope *scope = (cast_scope *)top_ptr(scope_stack);
	int field_num;
	
	field_num = cast_add_def(scope,
				 cast_new_scoped_name(atom_name, NULL),
				 CAST_SC_NONE,
				 CAST_VAR_DEF,
				 ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
				 current_protection);
	scope->cast_scope_val[field_num].u.cast_def_u_u.var_def.type =
		atom_ctype;
	
	/* Create an `inline_atom' referring to that slot. */
	return pres_c_new_inline_atom(field_num, atom_mapping);
}

/* End of file. */

