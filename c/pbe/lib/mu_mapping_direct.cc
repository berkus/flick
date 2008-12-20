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

#include <assert.h>

#include <mom/compiler.h>
#include <mom/libmint.h>
#include <mom/c/libcast.h>
#include <mom/c/pbe.hh>

/*
 * This handles PRES_C_MAPPING_DIRECT nodes, which indicate that the mapping
 * between the current ctype and the currend itype "should be obvious".  It
 * basically just hands control off to `mu_mapping_simple()', which is supposed
 * to be provided by more-specific code.
 *
 * XXX Not sure if this routine really has a reason for existence.
 */

void mu_state::mu_mapping_direct(cast_expr expr, cast_type ctype,
				 mint_ref itype)
{
	mint_def *idef = &pres->mint.defs.defs_val[itype];
	
	switch (idef->kind) {
	case MINT_VOID:
		assert(ctype->kind == CAST_TYPE_VOID);
		/* Nothing to be done.  */
		break;
		
	case MINT_INTEGER:
		/*
		 * If it's an integer type with range 0, then don't marshal
		 * anything.
		 */
		if (idef->mint_def_u.integer_def.range == 0) {
			/* If we're decoding, produce the correct constant. */
			if (op & MUST_DECODE)
				add_stmt(cast_new_stmt_expr(
					cast_new_expr_assign(
						expr,
						cast_new_expr_lit_int(
							(idef->mint_def_u.
							 integer_def.min),
							0)
						)
					)
					);
			break;
		}
		/* Otherwise, keep going... */
		
	case MINT_CHAR:
	case MINT_FLOAT:
	case MINT_SCALAR:
		mu_mapping_simple(expr, ctype, itype);
		break;
		
	default:
		panic("mu_state::mu_mapping_direct: unknown mint_def_kind %d",
		      idef->kind);
	}
}

/* End of file. */

