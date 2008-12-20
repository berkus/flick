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
#include <mom/libmint.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

/*
 * Create a simple C type corresponding to a particular itype,
 * for internal use within the backend.
 * Can't deal with really complex or recursive types.
 */
cast_type mint_to_ctype(mint_1 *mint, mint_ref itype)
{
	mint_def *def = &mint->defs.defs_val[itype];
	
	switch (def->kind) {
	case MINT_STRUCT: {
		mint_struct_def *sdef = &def->mint_def_u.struct_def;
		cast_type ctype = cast_new_struct_type(sdef->slots.slots_len);
		cast_scope *scope = &ctype->cast_type_u_u.agg_type.scope;
		
		for (int i = 0; i < (signed int)sdef->slots.slots_len; i++) {
			scope->cast_scope_val[i].sc = CAST_SC_NONE;
			scope->cast_scope_val[i].u.kind = CAST_VAR_DEF;
			scope->cast_scope_val[i].u.cast_def_u_u.var_def.
				type = mint_to_ctype(mint,
						     sdef->slots.slots_val[i]);
			scope->cast_scope_val[i].name =
				cast_new_scoped_name(flick_asprintf("el%d", i),
						     NULL);
		}
		return ctype;
	}
	
	default:
		return cast_new_type_name(mint_to_ctype_name(mint, itype));
	}
}

/* End of file. */

