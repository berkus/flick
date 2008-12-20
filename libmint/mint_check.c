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

#include <rpc/types.h>
#include <rpc/xdr.h>
#include <mom/mint.h>
#include <mom/compiler.h>
#include <mom/libmint.h>

#define PTR_CHECK(ptr) \
	if ((ptr) == 0) \
		panic("mint_1_check(%d): null pointer in def %d", __LINE__, i);

#define REF_CHECK(ref)							\
	if (((ref) < 0)							\
	    || ((ref) >= (signed int) mint->defs.defs_len))		\
		panic("mint_1_check(%d): bad reference %d in def %d",	\
		      __LINE__, (ref), i);
		
#define OPT_REF_CHECK(ref)						  \
	if ((((ref) < 0)						  \
	     || ((ref) >= (signed int) mint->defs.defs_len))		  \
	    && ((ref) != mint_ref_null))				  \
		panic("mint_1_check(%d): bad opt_reference %d in def %d", \
		      __LINE__, (ref), i);

#define STD_REF_CHECK(ref, type)					     \
	if (((ref) < 0)							     \
	    || ((ref) >= (signed int) mint->defs.defs_len))		     \
		panic("mint_1_check(%d): bad standard reference %d",	     \
		      __LINE__, (ref));					     \
	if (mint->defs.defs_val[(ref)].kind != (type))			     \
		panic("mint_1_check(%d): wrong type for standard reference", \
		      __LINE__);

/*
 * Currently this routine basically only checks pointer integrity.
 * It would be better if it also checked invariants and such.
 */
void mint_1_check(mint_1 *mint)
{
	unsigned int i;
	
	for (i = 0; i < mint->defs.defs_len; i++) {
		mint_def *d = &(mint->defs.defs_val[i]);
		
		switch (d->kind) {
		case MINT_INTEGER:
#if 0 /*XXX doesn't work right*/
			if (d->mint_def_u.integer_def.min
			    > (d->mint_def_u.integer_def.min
			       + d->mint_def_u.integer_def.range))
				panic("mint_1_check: illegal integer range "
				      "%d+%u (wraps around)",
				      d->mint_def_u.integer_def.min,
				      d->mint_def_u.integer_def.range);
#endif
			break;
			
		case MINT_FLOAT:
			if ((d->mint_def_u.float_def.bits != 32)
			    && (d->mint_def_u.float_def.bits != 64))
				panic("mint_1_check: illegal number "
				      "of float bits %d in def %d",
				      d->mint_def_u.float_def.bits, i);
			break;
			
		case MINT_SCALAR:
			if ((d->mint_def_u.char_def.bits != 1)
			    && (d->mint_def_u.char_def.bits != 8)
			    && (d->mint_def_u.char_def.bits != 16)
			    && (d->mint_def_u.char_def.bits != 32)
			    && (d->mint_def_u.char_def.bits != 64))
				panic("mint_1_check: illegal number "
				      "of scalar bits %d in def %d",
				      d->mint_def_u.scalar_def.bits,
				      i);
			break;
			
		case MINT_CHAR:
			if ((d->mint_def_u.char_def.bits != 8)
			    && (d->mint_def_u.char_def.bits != 16))
				panic("mint_1_check: illegal number "
				      "of char bits %d in def %d",
				      d->mint_def_u.char_def.bits,
				      i);
			break;
			
		case MINT_ARRAY:
			REF_CHECK(d->mint_def_u.array_def.element_type);
			REF_CHECK(d->mint_def_u.array_def.length_type);
			break;
			
		case MINT_STRUCT: {
			mint_struct_def *sd = &(d->mint_def_u.struct_def);
			unsigned int j;
			
			for (j = 0; j < sd->slots.slots_len; j++)
				REF_CHECK(sd->slots.slots_val[j]);
			break;
		}
		
		case MINT_UNION: {
			mint_union_def *ud = &(d->mint_def_u.union_def);
			unsigned int j;
			
			REF_CHECK(ud->discrim);
			for (j = 0; j < ud->cases.cases_len; j++) {
				mint_const_check(ud->cases.cases_val[j].val);
				OPT_REF_CHECK(ud->cases.cases_val[j].var);
			}
			OPT_REF_CHECK(ud->discrim);
			break;
		}
		
		case MINT_VOID:
			break;
			
		case MINT_ANY:
			break;
			
		case MINT_TYPED:
			REF_CHECK(d->mint_def_u.typed_def.tag);
			REF_CHECK(d->mint_def_u.typed_def.ref);
			break;
			
		case MINT_TYPE_TAG:
			break;
			
		case MINT_INTERFACE:
		        if ((d->mint_def_u.interface_def.right
			     != MINT_INTERFACE_NAME)
			    && (d->mint_def_u.interface_def.right
				!= MINT_INTERFACE_INVOKE)
			    && (d->mint_def_u.interface_def.right
				!= MINT_INTERFACE_INVOKE_ONCE)
			    && (d->mint_def_u.interface_def.right
				!= MINT_INTERFACE_SERVICE))
				panic("mint_1_check: illegal mint interface "
				      "right type %d in def %d",
				      d->mint_def_u.interface_def.right,
				      i);
			break;
			
		case MINT_SYSTEM_EXCEPTION:
			break;
			
		default:
			panic("mint_1_check: unknown kind %d", d->kind);
		}
	}
	
	/*
	 * Check that all of the standard types are defined, and that they
	 * refer to MINT definitions of the correct types.
	 */
	STD_REF_CHECK(mint->standard_refs.bool_ref, MINT_INTEGER);
	
	STD_REF_CHECK(mint->standard_refs.signed8_ref, MINT_INTEGER);
	STD_REF_CHECK(mint->standard_refs.signed16_ref, MINT_INTEGER);
	STD_REF_CHECK(mint->standard_refs.signed32_ref, MINT_INTEGER);
	STD_REF_CHECK(mint->standard_refs.unsigned8_ref, MINT_INTEGER);
	STD_REF_CHECK(mint->standard_refs.unsigned16_ref, MINT_INTEGER);
	STD_REF_CHECK(mint->standard_refs.unsigned32_ref, MINT_INTEGER);
	
	STD_REF_CHECK(mint->standard_refs.char8_ref, MINT_CHAR);
	
	STD_REF_CHECK(mint->standard_refs.float32_ref, MINT_FLOAT);
	
	STD_REF_CHECK(mint->standard_refs.interface_name_ref,
		      MINT_INTERFACE);
	STD_REF_CHECK(mint->standard_refs.interface_invoke_ref,
		      MINT_INTERFACE);
	STD_REF_CHECK(mint->standard_refs.interface_invoke_once_ref,
		      MINT_INTERFACE);
	STD_REF_CHECK(mint->standard_refs.interface_service_ref,
		      MINT_INTERFACE);
	
	STD_REF_CHECK(mint->standard_refs.system_exception_ref,
		      MINT_SYSTEM_EXCEPTION);
}

/* End of file. */

