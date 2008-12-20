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
#include <mom/compiler.h>
#include <mom/libmint.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>
#include <mom/c/pbe.hh>

void mu_state::mu_encode_const(mint_const iconst, mint_ref itype)
{
	cast_type ctype; cast_expr cexpr;
	mint_def *def;
	assert(op & MUST_ENCODE);
	assert((itype >= 0)
	       && (itype < (signed int) pres->mint.defs.defs_len));
	def = &(pres->mint.defs.defs_val[itype]);
	
	switch (def->kind) {
	case MINT_VOID:
		break;
		
	case MINT_INTEGER:
		ctype = mint_to_ctype(&pres->mint, itype);
		assert(iconst->kind == MINT_CONST_INT);
		switch (iconst->mint_const_u_u.const_int.kind) {
		case MINT_CONST_LITERAL:
			cexpr = cast_new_expr_lit_int((iconst->mint_const_u_u.
						       const_int.
						       mint_const_int_u_u.
						       value),
						      0);
			break;
		case MINT_CONST_SYMBOLIC:
			cexpr = cast_new_expr_name(iconst->mint_const_u_u.
						   const_int.
						   mint_const_int_u_u.name);
			break;
		default:
			panic("Unknown MINT constant category!");
			break;
		}
		mu_mapping_direct(cexpr, ctype, itype);
		break;
		
	case MINT_CHAR:
		ctype = mint_to_ctype(&pres->mint, itype);
		assert(iconst->kind == MINT_CONST_CHAR);
		switch(iconst->mint_const_u_u.const_int.kind) {
		case MINT_CONST_LITERAL:
			cexpr = cast_new_expr_lit_char((iconst->mint_const_u_u.
							const_char.
							mint_const_char_u_u.
							value),
						       0);
			break;
		case MINT_CONST_SYMBOLIC:
			cexpr = cast_new_expr_name(iconst->mint_const_u_u.
						   const_char.
						   mint_const_char_u_u.name);
			break;
		default:
			panic("Unknown MINT constant category!");
			break;
		}
		mu_mapping_direct(cexpr, ctype, itype);
		break;
		
	case MINT_FLOAT:
		ctype = mint_to_ctype(&pres->mint, itype);
		assert(iconst->kind == MINT_CONST_FLOAT);
		switch(iconst->mint_const_u_u.const_int.kind) {
		case MINT_CONST_LITERAL:
			cexpr = cast_new_expr_lit_double((iconst->
							  mint_const_u_u.
							  const_float.
							  mint_const_float_u_u.
							  value),
							 0);
			break;
		case MINT_CONST_SYMBOLIC:
			cexpr = cast_new_expr_name(iconst->mint_const_u_u.
						   const_float.
						   mint_const_float_u_u.name);
			break;
		default:
			panic("Unknown MINT constant category!");
			break;
		}
		mu_mapping_direct(cexpr, ctype, itype);
		break;
		
	case MINT_ARRAY:
	{
		mint_array_def *adef = &def->mint_def_u.array_def;
		mint_const_array *aconst = &iconst->mint_const_u_u.const_array;
		
		assert(iconst->kind == MINT_CONST_ARRAY);
		
		/* Marshal the array length.  */
		mint_const_u length_const;
		length_const.kind = MINT_CONST_INT;
		length_const.mint_const_u_u.const_int.kind
			= MINT_CONST_LITERAL;
		length_const.mint_const_u_u.const_int.mint_const_int_u_u.value
			= aconst->mint_const_array_len;
		mu_encode_const(&length_const, adef->length_type);
		
		/* XXX Assert length falls within array limits */
		
		/* XXX call backend hooks */
		
		for (unsigned int i = 0; i < aconst->mint_const_array_len; i++)
			mu_encode_const(aconst->mint_const_array_val[i],
					adef->element_type);
		
		break;
	}
	
	case MINT_STRUCT:
	{
		mint_struct_def *sdef = &def->mint_def_u.struct_def;
		mint_const_struct *sconst = &iconst->mint_const_u_u.const_struct;
		
		assert(iconst->kind == MINT_CONST_STRUCT);
		
		/* XXX call backend hooks */
		
		for (unsigned int i = 0; i < sconst->mint_const_struct_len; i++)
			mu_encode_const(sconst->mint_const_struct_val[i],
					sdef->slots.slots_val[i]);
		
		break;
	}
	
	default:
		panic("mu_encode_const: unknown mint_kind %d", def->kind);
	}
}

