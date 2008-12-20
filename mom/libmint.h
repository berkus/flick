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

#ifndef _mom_libmint_h_
#define _mom_libmint_h_

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <mom/aoi.h>
#include <mom/mint.h>

void mint_1_check(mint_1 *mint);
void mint_1_readfh(mint_1 *dest, FILE *fh);
void mint_1_writefh(mint_1 *src, FILE *fh);

/* Check a mint_const for validity and internal consistency.  */
void mint_const_check(mint_const mc);

/* Compares two constants for equality.
   Assumes the two constants have the same type.
   Returns < 0 if c1 is less than c2,
   0 if c1 is exactly equal to c2,
   and > 0 if c1 is greater than c2.  */
int mint_const_cmp(mint_const c1, mint_const c2);

/* Given a constant representing a value of the appropriate type for this union's discriminator,
   find the applicable union case and return its type.
   If the union has no case that matches the supplied discriminator value,
   returns the type of the default case,
   or mint_ref_null if the union has no default case.  */
mint_ref mint_find_union_case(mint_1 *mint, mint_ref name_r, mint_const const_r);

/* From a min/range-style integer definition,
   determine the integer size in bits needed to represent it,
   and whether or not the integer needs to be signed.  */
void mint_get_int_size(mint_1 *mint, mint_ref itype, int *out_bits, int *out_is_signed);

/* Delve into an array definition and find the minimum and maximum array length.  */
void mint_get_array_len(mint_1 *mint, mint_ref itype, unsigned *min, unsigned *max);

/* Create the standard MINT type defintions. */
void mint_add_standard_defs(mint_1 *mint);
	
/* Add a new uninitialized definition to the specified mint.  */
mint_ref mint_add_def(mint_1 *mint);

/* Allocate a new union definition, with the specified initial number of cases.
   The union initially has no default case.  */
mint_ref mint_add_union_def(mint_1 *mint, mint_ref discrim_type, int len);

/* Find an integer_def with the specified parameters,
   or create a new one if necessary.  */
mint_ref mint_add_integer_def(mint_1 *mint, int min, unsigned range);

/* Find an array definition with the specified parameters,
   or create a new def if none already exists.  */
mint_ref mint_add_array_def(mint_1 *mint, mint_ref element_type,
			    unsigned min_len, unsigned max_len);

/* Add new uninitialized slots/cases to existing structs/unions, respectively.  */
mint_ref mint_add_struct_slot(mint_1 *mint, mint_ref struct_itype);
mint_ref mint_add_union_case(mint_1 *mint, mint_ref union_itype);


/* Constructors for mint_const nodes.  */
mint_const mint_new_const(mint_const_kind kind);
mint_const mint_new_const_int(int val);
mint_const mint_new_const_string(const char *val);
mint_const mint_new_const_from_aoi_const(aoi_const aoiconst);
mint_const mint_new_symbolic_const(mint_const_kind kind, const char *name);

/* Add a mint def with the values given in Attribute/Value(s)
   notation (tags) on the stack. */
mint_ref mint_add_def_tags(mint_def_kind kind, mint_1 *mint, int tag, ...);

/* Attribute tags for each mint type.  Names
   should be similar to struct names */
enum {
	MINT_TAG_DONE,     /* Special tag to identify the
			      end of the tag list */
	
	/* [Attributed ID] [Values needed] */
	MDA_Min,           /* int */
	MDA_Range,         /* unsigned */
	MDA_Bits,          /* int */
	MDA_Flags,         /* mint_*_flags */
	MDA_ElementType,   /* mint_ref */
	MDA_LengthType,    /* mint_ref */
	MDA_Slot,          /* mint_ref */
	MDA_Discrim,       /* mint_ref */
	MDA_Case,          /* mint_const, mint_ref */
	MDA_Default,       /* mint_ref */
	MDA_Tag,           /* mint_ref */
	MDA_Ref,           /* mint_ref */
	MDA_Right          /* mint_interface_right */
};

/* Macros to assist in creating the mint types.  (Note: these make a
   reference to the variable mint which should be a mint_1 * where you
   want the stuff added)  Example:
   
   mref = MINT_UNION_REF,
              MDA_Discrim, MINT_ARRAY_REF,
		  MDA_ElementType, MINT_CHAR_REF, END_REF,
		  END_REF,
              MDA_Case,
		  mint_new_const_string( "Flick" ), (This case's discrim value)
	          MINT_INTEGER_REF,                 (This case's variant)
                      MDA_Min, 0,
                      MDA_Range, 1,
      	              END_REF,
              END_REF;
*/
#define MINT_INTEGER_REF mint_add_def_tags( MINT_INTEGER, mint
#define MINT_VOID_REF mint_add_def_tags( MINT_VOID, mint
#define MINT_STRUCT_REF mint_add_def_tags( MINT_STRUCT, mint
#define MINT_UNION_REF mint_add_def_tags( MINT_UNION, mint
#define MINT_FLOAT_REF mint_add_def_tags( MINT_FLOAT, mint
#define MINT_BOOLEAN_REF mint_add_def_tags( MINT_BOOLEAN, mint
#define MINT_STRUCT_REF mint_add_def_tags( MINT_STRUCT, mint
#define MINT_CHAR_REF mint_add_def_tags( MINT_CHAR, mint
#define MINT_ARRAY_REF mint_add_def_tags( MINT_ARRAY, mint
#define MINT_TYPED_REF mint_add_def_tags( MINT_TYPED, mint
#define MINT_INTERFACE_REF mint_add_def_tags( MINT_INTERFACE, mint
#define MINT_SYSTEM_EXCEPTION_REF mint_add_def_tags( MINT_SYSTEM_EXCEPTION,mint
#define MINT_TYPE_TAG_REF mint_add_def_tags( MINT_TYPE_TAG, mint
#define MINT_ANY_REF mint_add_def_tags( MINT_ANY, mint
#define END_REF MINT_TAG_DONE )

#ifdef __cplusplus
}
#endif

#endif /* _mom_libmint_h_ */
