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

#ifndef _mom_libaoi_h_
#define _mom_libaoi_h_

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif
  
#include <mom/aoi.h>
#include <mom/mint.h>

/*****************************************************************************/

/* compute the fully scoped name, using a given separator */
char *aoi_get_scoped_name(aoi_ref aref, const char *separator);

/* AOI-to-MINT translation. */
void translate_aoi_to_mint(void);
	
/*
 * `mint_custom_op_const' and `mint_custom_attrib_const' allow a presentation
 * generator to control how the AOI-to-MINT translator creates MINT constants
 * for operation and attribute discriminators.
 *
 * The `#define'd values tell the customization functions what kind of constant
 * they must generate.
 */

#define MAKE_OP_REQUEST			(0)
#define MAKE_OP_REPLY			(1)
#define MAKE_ATTRIB_READ_REQUEST	(0)
#define MAKE_ATTRIB_READ_REPLY		(1)
#define MAKE_ATTRIB_WRITE_REQUEST	(2)
#define MAKE_ATTRIB_WRITE_REPLY		(3)

extern mint_const (*mint_custom_op_const)(aoi_interface *this_interface,
					  char *this_interface_name,
					  aoi_interface *derived_interface,
					  char *derived_interface_name,
					  aoi_operation *op,
					  int type_of_constant);
extern mint_const (*mint_custom_attrib_const)(aoi_interface *this_interface,
					      char *this_interface_name,
					      aoi_interface *derived_interface,
					      char *derived_interface_name,
					      aoi_attribute *attrib,
					      int type_of_constant);
extern mint_const (*mint_custom_exception_const)(aoi_ref exception_ref,
						 unsigned int exception_num);
	
extern mint_ref mint_custom_exception_discrim_ref;

/*****/

void tam_interface_record_mint_discrims(aoi_interface *interface,
					void *op_or_attr,
					mint_const request_discrim,
					mint_const reply_discrim);

int lookup_interface_mint_discrims(aoi_interface *interface,
				   void *op_or_attr,
				   /* OUT */ mint_const *request_discrim,
				   /* OUT */ mint_const *reply_discrim);


/*****************************************************************************/
	
void aoi_check(aoi *dest);

void aoi_readfh(aoi *dest, FILE *fh);
void aoi_writefh(aoi *dest, FILE *fh);

/* Print out the aoi supplied on stdout */
void print_all(aoi inaoi);


/*****************************************************************************/

/* Constructors for `aoi_const' nodes.  */
aoi_const aoi_new_const(aoi_const_kind kind);
aoi_const aoi_new_const_char(char c);
aoi_const aoi_new_const_int(int val);
aoi_const aoi_new_const_float(double val);
aoi_const aoi_new_const_string(const char *s);
aoi_const aoi_new_const_string_cat(const char *s1, const char *s2);
	
/*
 * Return non-zero if the given AOI constants are equal.
 */
int aoi_const_eq(aoi_const c1, aoi_const c2);

/*
 * From a min/range-style integer definition, determine the integer size in
 * bits needed to represent it, and whether or not the integer needs to be
 * signed.
 */
void aoi_get_int_size(aoi_integer *ai, int *out_bits, int *out_is_signed);

/*
 * Given an array type definition, return the minimum and maximum possible
 * array lengths.
 */
void aoi_get_array_len(aoi *a, aoi_array *at, unsigned *min, unsigned *max);

aoi_ref aoi_get_parent_scope(aoi *the_aoi, aoi_ref ref);
int aoi_def_has_member(aoi *in_aoi, aoi_def *d, const char *name);
int aoi_def_find_member(aoi *in_aoi, aoi_def *d, const char *name,
			aoi_def **out_d, int *out_memindex);

/*
 * Descend through any AOI_INDIRECT type nodes and return the `aoi_type' that
 * they eventually lead to.  (`aoi_indir_1' descends only one level.)
 */
aoi_type aoi_indir(aoi *a, aoi_type t);
aoi_type aoi_indir_1(aoi *a, aoi_type t);

/*
 * Given an `aoi_ref' to an AOI_INTERFACE or an AOI_FWD_INTRFC, return a
 * reference to an AOI_INTERFACE.  (That is, dereference the AOI_FWD_INTRFC as
 * necessary.)
 */
aoi_ref aoi_deref_fwd(aoi *a, aoi_ref fwd);

#ifdef __cplusplus
}
#endif

#endif /* _mom_libaoi_h_ */

/* End of file. */

