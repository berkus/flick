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

int mint_const_eq(mint_const, mint_const);

int mint_const_cmp(mint_const c1, mint_const c2)
{
	return !mint_const_eq(c1, c2);
}

/* Return 1 if the constants are equal and 0 of they are not. */

/* Currently, literal and symbolic constants are treated as separate classes.
   No literal constant is ever equal to a symbolic constant, and vice versa.
   Also, two symbllic constants are equal if and only if their names are
   identical.  Perhaps we will need to improve this situation someday. */

static void mint_const_eq_mismatch(char *type)
{
	warn("A literal constant %s was compared to a symbolic constant %s.  "
	     "Assuming they are not equal.",
	     type, type);
}
	
int mint_const_eq(mint_const c1, mint_const c2)
{
	if (c1->kind != c2->kind)
		return 0;
	
	switch (c1->kind) {
	case MINT_CONST_INT:
	{
		mint_const_int_u *c1_int = &(c1->mint_const_u_u.const_int);
		mint_const_int_u *c2_int = &(c2->mint_const_u_u.const_int);
		
		if (c1_int->kind != c2_int->kind) {
			mint_const_eq_mismatch("integer");
			return 0;
		}
		switch (c1_int->kind){
		case MINT_CONST_LITERAL:
			return (c1_int->mint_const_int_u_u.value ==
				c2_int->mint_const_int_u_u.value);
		case MINT_CONST_SYMBOLIC:
			return !strcmp(c1_int->mint_const_int_u_u.name,
				       c2_int->mint_const_int_u_u.name);
		}
	}
	break;
	
	case MINT_CONST_CHAR:
	{
		mint_const_char_u *c1_char = &(c1->mint_const_u_u.const_char);
		mint_const_char_u *c2_char = &(c2->mint_const_u_u.const_char);
		
		if (c1_char->kind != c2_char->kind) {
			mint_const_eq_mismatch("character");
			return 0;
		}
		switch (c1_char->kind){
		case MINT_CONST_LITERAL:
			return (c1_char->mint_const_char_u_u.value ==
				c2_char->mint_const_char_u_u.value);
		case MINT_CONST_SYMBOLIC:
			return !strcmp(c1_char->mint_const_char_u_u.name,
				       c2_char->mint_const_char_u_u.name);
		}
	}
	break;
	
	case MINT_CONST_FLOAT:
	{
		mint_const_float_u *c1_float = &(c1->
						 mint_const_u_u.const_float);
		mint_const_float_u *c2_float = &(c2->
						 mint_const_u_u.const_float);
		
		if (c1_float->kind != c2_float->kind) {
			mint_const_eq_mismatch("float");
			return 0;
		}
		switch (c1_float->kind){
		case MINT_CONST_LITERAL:
			return (c1_float->mint_const_float_u_u.value ==
				c2_float->mint_const_float_u_u.value);
		case MINT_CONST_SYMBOLIC:
			return !strcmp(c1_float->mint_const_float_u_u.name,
				       c2_float->mint_const_float_u_u.name);
		}
	}
	break;
	
	case MINT_CONST_STRUCT:
	{
		mint_const_struct *cs1 = &c1->mint_const_u_u.const_struct;
		mint_const_struct *cs2 = &c2->mint_const_u_u.const_struct;
		unsigned int i;
		
		if (cs1->mint_const_struct_len != cs2->mint_const_struct_len) {
			warn("In `mint_const_eq', "
			     "comparing a struct of length %d with a struct "
			     "of length %d.",
			     cs1->mint_const_struct_len,
			     cs2->mint_const_struct_len);
			return 0;
		}
		
		for (i = 0; i < cs1->mint_const_struct_len; i++) {
			int same = mint_const_eq(
				cs1->mint_const_struct_val[i],
				cs2->mint_const_struct_val[i]);
			if (!same)
				return 0;
		}
		return 1;
	}
	
	case MINT_CONST_ARRAY:
	{
		mint_const_array *ca1 = &c1->mint_const_u_u.const_array;
		mint_const_array *ca2 = &c2->mint_const_u_u.const_array;
		unsigned l1 = ca1->mint_const_array_len;
		unsigned l2 = ca2->mint_const_array_len;
		unsigned i;
		
		if (l1 != l2) {
			/* No warning message: We often compare arrays of
			   different lengths (variable-length strings). */
			return 0;
		}
		
		for (i = 0; i < l1; i++) {
			int same = mint_const_eq(
				ca1->mint_const_array_val[i],
				ca2->mint_const_array_val[i]);
			if (!same)
				return 0;
		}
		return 1;
	}
	
	default:
		panic("mint_const_eq: unknown mint_const_kind %d\n", c1->kind);
	}
	
	panic("mint_const_eq: should have returned in switch statement\n");
	return 0;
}

/* End of file. */

