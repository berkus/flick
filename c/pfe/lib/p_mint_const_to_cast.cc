/*
 * Copyright (c) 1995, 1996, 1997 The University of Utah and
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
#include <string.h>

#include <mom/libmint.h>
#include <mom/c/libcast.h>
#include <mom/c/pfe.hh>

/* Given a MINT constant, this function creates and returns an
   equivalent CAST expression.  It is used for PRES_C_MAPPING_ASSIGN_EXPR's. */
cast_expr pg_state::p_mint_const_to_cast(mint_const mint_literal)
{
	cast_expr cast_literal;
	
	switch (mint_literal->kind) {
	case MINT_CONST_INT:
		switch (mint_literal->mint_const_u_u.const_int.kind) {
		case MINT_CONST_LITERAL:
			cast_literal =
				cast_new_expr_lit_int((mint_literal->
						       mint_const_u_u.
						       const_int.
						       mint_const_int_u_u.
						       value),
						      0);
			break;
		case MINT_CONST_SYMBOLIC:
			cast_literal =
				cast_new_expr_name(mint_literal->
						   mint_const_u_u.const_int.
						   mint_const_int_u_u.name);
			break;
		default:
			panic("Unknown MINT constant category.");
			break;
		}
		break;
		
	case MINT_CONST_CHAR:
		switch (mint_literal->mint_const_u_u.const_char.kind) {
		case MINT_CONST_LITERAL:
			cast_literal =
				cast_new_expr_lit_char((mint_literal->
							mint_const_u_u.
							const_char.
							mint_const_char_u_u.
							value),
						       0);
			break;
		case MINT_CONST_SYMBOLIC:
			cast_literal =
				cast_new_expr_name(mint_literal->
						   mint_const_u_u.const_char.
						   mint_const_char_u_u.name);
			break;
		default:
			panic("Unknown MINT constant category.");
			break;
		}
		break;
		
	case MINT_CONST_ARRAY: {
		unsigned int i;
		mint_const_array *mcarray = &(mint_literal->
					      mint_const_u_u.
					      const_array);
		char *s = (char *)mustmalloc(
			sizeof(char) * mcarray->mint_const_array_len+1);
		for (i = 0; i < mcarray->mint_const_array_len; i++)
			if ((mcarray->mint_const_array_val[i]->kind
			     == MINT_CONST_CHAR)
			    && (mcarray->mint_const_array_val[i]->
				mint_const_u_u.const_char.kind
				== MINT_CONST_LITERAL))
				s[i] = mcarray->mint_const_array_val[i]
				       ->mint_const_u_u.const_char.
				       mint_const_char_u_u.value;
			else {
				panic("Converting non-string MINT "
				      "arrays to CAST "
				      "not yet supported.\n");
				return 0;
			}
		s[i] = 0; /* Add a terminating NULL just in case */
		cast_literal = cast_new_expr_lit_string(s);
		break;
	}
	default:
		panic("MINT constant type %d not supported as discrimintor.",
		      mint_literal->kind);
	}
	
	return cast_literal;
}

/* End of file. */

