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

/*
 * These functions are used in mom/c/pfe/ programs.
 *
 * See comments in mom/mom/aoi.x and mom/mom/mint.x, as well as the
 * "Flick Internals" document for a description of AOI and MINT.
 */


#include <stdlib.h>
#include <mom/compiler.h>
#include <mom/libaoi.h>
#include <mom/libmint.h>
#include <assert.h>

/* global variables */
mint_1 out_mint;
mint_ref mom_msg = mint_ref_null;
mint_ref *aoi_to_mint_association;
aoi in_aoi;

static int mint_max = 0;
extern int custom_const;
static char *a_name;

/* Quickie lookup macro */
#define m(n) (out_mint.defs.defs_val[n])
#define a(n) (in_aoi.defs.defs_val[n])

mint_const (*mint_custom_op_const)(aoi_interface *this_interface,
				   char *this_interface_name,
				   aoi_interface *derived_interface,
				   char *derived_interface_name,
				   aoi_operation *op,
				   int type_of_constant) = 0;
mint_const (*mint_custom_attrib_const)(aoi_interface *this_interface,
				       char *this_interface_name,
				       aoi_interface *derived_interface,
				       char *derived_interface_name,
				       aoi_attribute *attrib,
				       int type_of_constant) = 0;
mint_const (*mint_custom_exception_const)(aoi_ref exception_ref,
					  unsigned int exception_num) = 0;

mint_ref mint_custom_exception_discrim_ref = 0;

/* prototype declarations for static functions defined in this file */
void translate_aoi_to_mint(void);
static void tam_aoi_type(aoi_type aoi_type, mint_ref *mr);
static mint_ref tam_ref(aoi_ref ref);
static mint_ref tam_integer(aoi_integer *integer_def);
static mint_ref tam_float(aoi_float *float_def);
static mint_ref tam_char(aoi_char *char_def);
static mint_ref tam_scalar(aoi_scalar *scalar_def);
static mint_ref tam_array(aoi_array *array_def);
static mint_ref tam_struct(aoi_struct *struct_def);
static mint_ref tam_union(aoi_union *union_def);

static void tam_interface(aoi_interface *aoi_interface_def);
static void tam_interface_contents(aoi_interface *this_interface,
				   char *this_interface_name,
				   aoi_interface *derived_interface,
				   char *derived_interface_name,
				   mint_ref interface_union);
static void tam_interface_contents_add(mint_ref interface_union,
				       mint_const discrim,
				       mint_ref type,
				       char *interface_name,
				       char *type_name);

static mint_ref tam_operation_request_struct(aoi_operation *ops_val);
static mint_ref tam_operation_reply_struct(aoi_operation *ops_val,
					   mint_ref req);

static mint_ref tam_attribute_read_request_struct(aoi_attribute *attribs_val);
static mint_ref tam_attribute_read_reply_struct(aoi_attribute *attribs_val);
static mint_ref tam_attribute_write_request_struct(aoi_attribute *attribs_val);
static mint_ref tam_attribute_write_reply_struct(aoi_attribute *attribs_val);

/*
 * Presentation generators need to look up MINT request and reply
 * discriminator values, so we record these types during translation.
 *
 * We keep a table of `interface_mint_table_elem's so that we can map from a
 * given AOI (interface, operation/attribute) pair onto the MINT request and
 * reply discriminator values that were produced.
 *
 * `interface_mint_table' is our table, `interface_mint_table_size' is the
 * number of elements in the table (i.e., the size of the array), and
 * `interface_mint_table_count' is the number of records that have been used.
 */
typedef struct {
	aoi_interface *interface;
	void *op_or_attr; /* `aoi_operation *' or `aoi_attribute *'. */
	mint_const request_discrim;
	mint_const reply_discrim;
} interface_mint_table_elem;

#define INTERFACE_MINT_TABLE_INCREMENT (128)

static interface_mint_table_elem *interface_mint_table;
static int interface_mint_table_size;
static int interface_mint_table_count;

void tam_interface_record_mint_discrims(aoi_interface *interface,
					void *op_or_attr,
					mint_const request_discrim,
					mint_const reply_discrim);
int lookup_interface_mint_discrims(aoi_interface *interface,
				   void *op_or_attr,
				   /* OUT */ mint_const *request_discrim,
				   /* OUT */ mint_const *reply_discrim);
static void init_interface_mint_table();

/*****/

static mint_ref tam_exception(aoi_exception *exception_def);
/* static mint_ref tam_const(aoi_const *const_def); */
static mint_ref tam_void();
static mint_ref tam_optional(aoi_optional *optional_def);
static mint_ref tam_any();
static mint_ref tam_type_tag();
static mint_ref tam_typed(aoi_typed *typed_def);
static int get_def();
static mint_ref get_union_def(int len);
static int expand_union_cases(mint_ref r);
#if 0  /* currently unused, but might be used later */
static int xl_int(int min, unsigned range);
#endif
static mint_ref get_union_def_given_discriminator_type(
	int len, mint_ref discriminator_type);
#if 0  /* currently unused, but might be used later */
static int xl_array_minlen(mint_ref type_d, int min, unsigned len);
#endif

/* function definitions */


/*
 * translate_aoi_to_mint() translates an AOI data structure to a MINT
 * structure.  This function is available for PFE components of the
 * MOM IDL Compiler.  It allocates memory as needed to build the MINT.
 * It also assigns values to aoi_to_mint_association and
 * aoi_to_mint_array_size.
 */

void translate_aoi_to_mint(void)
{
	int i, number_of_aoi_names;

	number_of_aoi_names = in_aoi.defs.defs_len;
	
	/*
	 * `out_mint' must already be initialized to contain the standard MINT
	 * types.  (We used to call `mint_add_standard_defs' here, but that
	 * made it more difficult to link the presentation generators.)
	 */
	if (out_mint.defs.defs_len == 0)
		panic("In `translate_aoi_to_mint', `out_mint' has not been "
		      "preloaded with the standard MINT types.");
	
	/*
	 * We try to be clever when reallocating the `out_mint.defs.def_val'
	 * array; see `get_def'.
	 */
	mint_max = out_mint.defs.defs_len;
	
	init_interface_mint_table();
	
	/*
	 * Create a MINT_UNION definition to serve as the top-level mom_msg
	 * union.  Initially, this union will have no cases, and
	 * get_union_def() assigns to it a 32-bit integer discriminator type.
	 */
	
	/*
	 * Assert that only the declaration-time initialization of mom_msg
	 * should have been done.
	 */
	assert(mom_msg == mint_ref_null);
	
	mom_msg = get_union_def(0);
	
	for (i = 0; i < number_of_aoi_names; i++)
		aoi_to_mint_association[i] = get_def();
	
	for (i = 0; i < number_of_aoi_names; i++) {
		aoi_type a_type;
		mint_ref temp;
		
		/* Select the AOI type to translate. */
		a_type = a(i).binding;
		a_name = a(i).name;
		/* Do the translation. */
		tam_aoi_type(a_type, &temp);
		m(aoi_to_mint_association[i]) = m(temp);
		if (a_type->kind == AOI_INDIRECT)
			aoi_to_mint_association[i]
				= aoi_to_mint_association[a_type->aoi_type_u_u.
							 indirect_ref];
		
		/* Similar to the above indirect case */
		if( (a_type->kind == AOI_FWD_INTRFC) &&
		    (a_type->aoi_type_u_u.fwd_intrfc_def != -1) ) {
			aoi_to_mint_association[i]
				= aoi_to_mint_association[a_type->aoi_type_u_u.
							 fwd_intrfc_def];
		}
		/*
		 * Check to see that the returned value is in the correct
		 * range.  It should always be that 0 <= new_mint_index <
		 * out_mint.defs.defs_len.
		 */
		/* assert(0 <= new_mint_index); */
		/*
		 * This has been changed.  Forward refs will be -1 until
		 * corrected...
		 */
		assert(aoi_to_mint_association[i]
		       < (signed int) out_mint.defs.defs_len);
		
		/*
		 * Make the aoi_name-to-mint_ref association, so that AOI_REF
		 * types can be translated.
		 */
		/* aoi_to_mint_association[i] = new_mint_index; */
	}
}


/*
 * `tam_aoi_type' translates an AOI type, and returns a new mint_ref in `*mr'.
 */
static void tam_aoi_type(aoi_type aoi_type, mint_ref *mr)
{
	mint_ref r;  /* return value for this function */
	mint_def *initial_defs_val = out_mint.defs.defs_val;
	int mr_inside_buffer;
	
	/*
	 * Many of the `tam_*' functions can potentially reallocate the
	 * `out_mint.defs.defs_val' array.  However, it's very likely that `mr'
	 * is a pointer into this buffer!  Therefore, we must be careful to fix
	 * `mr' if we discover that the buffer has changed.  What a mess.
	 */
	initial_defs_val = out_mint.defs.defs_val;
	mr_inside_buffer = (((char *) mr) >= ((char *) out_mint.defs.defs_val))
			   &&
			   (((char *) mr) < ((char *)
					     (out_mint.defs.defs_val +
					      out_mint.defs.defs_len)));
	
	switch (aoi_type->kind) {
	case AOI_INDIRECT:
		r = tam_ref(aoi_type->aoi_type_u_u.indirect_ref);
		break;
	case AOI_INTEGER:
		r = tam_integer(&(aoi_type->aoi_type_u_u.integer_def));
		break;
	case AOI_SCALAR:
		r = tam_scalar(&(aoi_type->aoi_type_u_u.scalar_def));
		break;
	case AOI_FLOAT:
		r = tam_float(&(aoi_type->aoi_type_u_u.float_def));
		break;
	case AOI_CHAR:
		r = tam_char(&(aoi_type->aoi_type_u_u.char_def));
		break;
	case AOI_ARRAY:
		r = tam_array(&(aoi_type->aoi_type_u_u.array_def));
		break;
	case AOI_STRUCT:
		r = tam_struct(&(aoi_type->aoi_type_u_u.struct_def));
		break;
	case AOI_UNION:
		r = tam_union(&(aoi_type->aoi_type_u_u.union_def));
		break;
	case AOI_INTERFACE:
		tam_interface(&(aoi_type->aoi_type_u_u.interface_def));
		/*
		 * We return a reference to the standard interface name def,
		 * and `tam_aoi_to_mint' copies that definition to produce a
		 * unique `mint_ref' for the interface.  This is important
		 * because the BE currently uses `mint_ref's to identify m/u
		 * stubs; see the function `pres_c_find_mu_stub'.  Without a
		 * unique `mint_ref' we couldn't distinguish the m/u stubs for
		 * different interfaces.
		 */
		r = out_mint.standard_refs.interface_name_ref;
		break;
	case AOI_FWD_INTRFC:
		/*
		 * The above comment applies here, too, although now we have a
		 * subtle problem: the forward interface and the real interface
		 * will have different `mint_ref's.  But this isn't a problem
		 * in practice because the m/u stubs will be identical.
		 */
		r = out_mint.standard_refs.interface_name_ref;
		break;		
	case AOI_EXCEPTION:
		r = tam_exception(&(aoi_type->aoi_type_u_u.exception_def));
		break;
	case AOI_CONST:
		r = tam_void();  /* XXX - ignore aoi_const, as test */
		break;
	case AOI_ENUM: {
		/*
		 * As discussed (by crum and baford) on 9-Aug-1995,
		 * aoi_enum values are "compiled out", i.e., not translated
		 * to any MINT definitions.  Also, mint_enum_def is obsolete.
		 * XXX - For now, return a MINT_VOID.
		 */
		/*
		 * CORBA needs something - you can't have a void parameter! 
		 * So, it's being moved to a u_int - Corba specifies that,
		 * and no one else uses enums, anyway! KBF - 7/22/96
		 */
		aoi_integer i = {0, ~0U};
		r = tam_integer(&i);
		break;
	}
	case AOI_NAMESPACE:
		/* we don't do anything - they're strictly mapping
		   instructions */
	case AOI_VOID:
		r = tam_void();
		break;
	case AOI_OPTIONAL:
		r = tam_optional(&(aoi_type->aoi_type_u_u.optional_def));
		break;
	case AOI_ANY:
		r = tam_any();
		break;
	case AOI_TYPE_TAG:
		r = tam_type_tag();
		break;
	case AOI_TYPED:
		r = tam_typed(&(aoi_type->aoi_type_u_u.typed_def));
		break;
	case AOI_ERROR:
		panic("In `tam_aoi_type', found a binding to an AOI_ERROR.  "
		      "AOI_ERRORs represent IDL parse errors and should never "
		      "appear in AOI files.");
		break;
	default:
		panic("Unexpected AOI binding %d in `tam_aoi_type'",
		      aoi_type->kind);
		break;
	}
	
	if (mr_inside_buffer && (initial_defs_val != out_mint.defs.defs_val)) {
		/*
		 * `mr' pointed to an address inside `out_mint.defs.defs_val',
		 * and that array has moved since this function was invoked.
		 * We must fix `mr'.
		 */
		mr = (mint_ref *)
		     (((char *) out_mint.defs.defs_val) +
		      (((char *) mr) - ((char *) initial_defs_val)));
	}
	
	*mr = r;
}


/* "tam_" stands for "Translate from AOI to MINT", and
   functions with the "tam_" prefix are used to translate
   particular definition types.  They are used in the
   translate_aoi_name_to_mint_def() function, which is defined
   later in this file.
*/


/* translate an AOI reference to a MINT reference */
static mint_ref tam_ref(aoi_ref ref)
{
	/*
	 * In this case, no new mint_def needs to be allocated.  We can simply
	 * use the `aoi_to_mint_association' array.
	 * Caveat: if ``ref'' refers to another AOI_INDIRECT (reference),
	 * we need to dig until we find the ``real'' AOI type.
	 */
	while (a(ref).binding->kind == AOI_INDIRECT)
		ref = a(ref).binding->aoi_type_u_u.indirect_ref;
	
	return aoi_to_mint_association[ref];
}


static mint_ref tam_integer(aoi_integer *integer_def)
{
	mint_ref r = get_def();

	m(r).kind = MINT_INTEGER;
	m(r).mint_def_u.integer_def.min = integer_def->min;
	m(r).mint_def_u.integer_def.range = integer_def->range;

	return(r);
}


static mint_ref tam_scalar(aoi_scalar *scalar_def) 
{
	mint_ref r = get_def();
	
	m(r).kind = MINT_SCALAR;
	m(r).mint_def_u.scalar_def.bits = scalar_def->bits;
	switch (scalar_def->flags) {
	case AOI_SCALAR_FLAG_NONE:
		m(r).mint_def_u.scalar_def.flags = MINT_SCALAR_FLAG_NONE;
		break;
	case AOI_SCALAR_FLAG_SIGNED:
		m(r).mint_def_u.scalar_def.flags = MINT_SCALAR_FLAG_SIGNED;
		break;
	case AOI_SCALAR_FLAG_UNSIGNED:
		m(r).mint_def_u.scalar_def.flags = MINT_SCALAR_FLAG_UNSIGNED;
		break;
	default:
		panic("Unrecognized value of aoi_scalar_flags: %d",
		      scalar_def->flags);
	}
	return(r);
}


static mint_ref tam_float(aoi_float *float_def)
{
	mint_ref r = get_def();

	m(r).kind = MINT_FLOAT;
	m(r).mint_def_u.float_def.bits = float_def->bits;

	return(r);
}


static mint_ref tam_char(aoi_char *char_def)
{
	aoi_char_flags flags = char_def->flags;
	mint_ref r = get_def();
	
	m(r).kind = MINT_CHAR;
	m(r).mint_def_u.char_def.bits = char_def->bits;
	switch (flags) {
	case AOI_CHAR_FLAG_NONE:
		m(r).mint_def_u.char_def.flags = MINT_CHAR_FLAG_NONE;
		break;
	case AOI_CHAR_FLAG_SIGNED:
		m(r).mint_def_u.char_def.flags = MINT_CHAR_FLAG_SIGNED;
		break;
	case AOI_CHAR_FLAG_UNSIGNED:
		m(r).mint_def_u.char_def.flags = MINT_CHAR_FLAG_UNSIGNED;
		break;
	default:
		panic("Unrecognized value of aoi_char_flags: %d", flags);
		break;
	}
	return(r);
}


static mint_ref tam_array(aoi_array *array_def)
{
	mint_ref r = get_def();

	m(r).kind = MINT_ARRAY;
	tam_aoi_type(array_def->element_type,
		     &(m(r).mint_def_u.array_def.element_type));
	tam_aoi_type(array_def->length_type,
		     &(m(r).mint_def_u.array_def.length_type));

	return(r);
}


static mint_ref tam_struct(aoi_struct *struct_def)
{
	mint_ref r = get_def();
	int slots_len = struct_def->slots.slots_len;
	int i;

	m(r).kind = MINT_STRUCT;
	m(r).mint_def_u.struct_def.slots.slots_len = slots_len;
	m(r).mint_def_u.struct_def.slots.slots_val =
		(mint_ref *)mustmalloc(sizeof(mint_ref)*slots_len);

	/*
	 * For each slot in the struct, translate the slot AOI type to a
	 * mint_ref.
	 */
	for (i = 0; i < slots_len; i++)
		tam_aoi_type(struct_def->slots.slots_val[i].type,
			     &(m(r).mint_def_u.struct_def.slots.slots_val[i]));
	
	return(r);
}


static mint_ref tam_union(aoi_union *union_def)
{
	mint_ref r = get_def();
	int cases_len = union_def->cases.cases_len;
	int i;
	
	m(r).kind = MINT_UNION;
	
	/* Translate the discriminator type. */
	tam_aoi_type(union_def->discriminator.type,
		     &(m(r).mint_def_u.union_def.discrim));
	
	/* Translate the union cases. */
	m(r).mint_def_u.union_def.cases.cases_len = cases_len;
	m(r).mint_def_u.union_def.cases.cases_val =
		(mint_union_case *) mustmalloc(sizeof(mint_union_case) *
					       cases_len);
	
	/* For each case in the union, translate the case field types (val and
	   var) into mint_refs. */
	for (i = 0; i < cases_len; i++)	{
		/* The mint_union_def `val' is a mint_const,
		   and the aoi_union `val' is an aoi_const. */
		m(r).mint_def_u.union_def.cases.cases_val[i].val =
			mint_new_const_from_aoi_const(union_def->
						      cases.cases_val[i].val);
		/* The mint_union_def `var' is a MINT type. */
		tam_aoi_type(union_def->cases.cases_val[i].var.type,
			     &(m(r).mint_def_u.union_def.cases.cases_val[i].
			       var));
	}
	
	/* Translate the default case, too. */
	if (union_def->dfault)
		tam_aoi_type(union_def->dfault->type,
			     &(m(r).mint_def_u.union_def.dfault));
	else
		/* `mint_ref_null' indicates that there is no default case. */
		m(r).mint_def_u.union_def.dfault = mint_ref_null;
	
	return r;
}


static void tam_interface(aoi_interface *a)
{
	mint_const idl_const, code_const;
	mint_ref l2_union, l3_union, returned_ref;
	mint_ref code_type, op_code_type;
	int l2_union_case, l3_union_case;
	
	/* level 1 (mom_msg union) */

	idl_const = mint_new_const_int((int)a->idl);
	/* XXX ---
	   We should use symbolic constants, now that we have them!  But until
	   then, assert that we have a literal.  Really, the whole `switch'
	   below should become obsolete when we fix the parts of AOI that are
	   IDL dependent. */
	assert(idl_const->mint_const_u_u.const_int.kind == MINT_CONST_LITERAL);
	
	switch (idl_const->mint_const_u_u.const_int.mint_const_int_u_u.value) {
	case AOI_IDL_SUN:
		/* With Sun RPC, AOI interfaces represent both programs and
		   versions.
		   We represent versions in the MINT;
		   however, programs add nothing to our MINT representation.
		   The AOI for a version already contains the number of the
		   program in which it is contained.
		   */
		if (a->parents.parents_len <= 0) {
			/* The interface `a' represents a program.  We're done!
			   It is important to punt here ---
			   the AOI_INTERFACE for a program has a different
			   code_type than an AOI_INTERFACE for a version.
			   If we didn't punt, then the second-level union would
			   be created with the wrong discriminator code type!
			   */
			return;
		}
		break;
		
	default:
		break;
	}
	
	/* If a case for this IDL does not already exist, then add it. */
	l2_union = mint_find_union_case(&out_mint, mom_msg, idl_const);
	if (l2_union == mint_ref_null) {
		tam_aoi_type(a->code_type, &code_type);
		/* If `a->code_type' were an AOI_INDIRECT, then it would be
		   possible for it to be a forward reference to another AOI
		   type.  If it were a forward reference, `code_type' would be
		   set to -1.  However, we can't deal with that --- because
		   `code_ref' is a local variable, and the forward-reference
		   tracking code will save the address of `code_type', and it
		   won't know that it really has to fix the reference to the
		   discriminator in the `l2_union' instead.  This difficulty
		   could be avoided by rewriting the code in this area, but
		   it's not really worth the effort right now. */
		assert(code_type != -1);
		l2_union =
			get_union_def_given_discriminator_type(0, code_type); 
		l2_union_case = expand_union_cases(mom_msg);  /* adds a case */
	   	m(mom_msg).mint_def_u.union_def.cases.
			cases_val[l2_union_case].val
		 = idl_const;
		m(mom_msg).mint_def_u.union_def.cases.
			cases_val[l2_union_case].var
		 = l2_union;
	}


	/* level 2 (interface code) */
	code_const = mint_new_const_from_aoi_const(a->code);
	tam_aoi_type(a->op_code_type, &op_code_type);
	/* Similar to `a->code_type', we can't allow `a->op_code_type' to be an
	   AOI_INDIRECT forward reference, because as this code now stands, the
	   forward-reference tracking code wouldn't be able to cope. */
	assert(op_code_type != -1);
	
	/* This interface should not already have a case value. */
	returned_ref = mint_find_union_case(&out_mint, l2_union, code_const);
	if (returned_ref != mint_ref_null)
	{
		panic("unexpected duplicate interface code value");
	}

	l3_union = get_union_def_given_discriminator_type(0, op_code_type);
	l3_union_case = expand_union_cases(l2_union);  /* adds a case */
	m(l2_union).mint_def_u.union_def.cases.cases_val[l3_union_case].val
	 = code_const;
	m(l2_union).mint_def_u.union_def.cases.cases_val[l3_union_case].var
	 = l3_union;

	/* Level 3 (operations, attributes and exceptions in the interface) */
	tam_interface_contents(a, a_name, a, a_name, l3_union);
}

static void tam_interface_contents(aoi_interface *this_interface,
				   char *this_interface_name,
				   aoi_interface *derived_interface,
				   char *derived_interface_name,
				   mint_ref interface_union)
{
	u_int ops_len = this_interface->ops.ops_len;
	u_int attribs_len = this_interface->attribs.attribs_len;
	u_int parents_len = this_interface->parents.parents_len;
	u_int i;
	
	mint_const request_discrim, reply_discrim;
	mint_ref request_type, reply_type;
	
	/* Add all of the operations that are part of this interface. */
	for (i = 0; i < ops_len; ++i) {
		aoi_operation *op_val = &(this_interface->ops.ops_val[i]);
		
		/*
		 * Determine the MINT discriminator that corresponds to the
		 * request code for this operation.
		 */
		if (mint_custom_op_const)
			request_discrim =
				(*mint_custom_op_const)(this_interface,
							this_interface_name,
							derived_interface,
							derived_interface_name,
							op_val,
							MAKE_OP_REQUEST);
		else
			request_discrim =
				mint_new_const_from_aoi_const(op_val->
							      request_code);
		
		/*
		 * Add this request to the MINT union that represents the
		 * interface.
		 */
		if (mint_find_union_case(&out_mint,
					 interface_union, request_discrim)
		    != mint_ref_null)
			/*
			 * This request is already defined in the MINT for the
			 * `derived_interface'.  This will occur if the
			 * operation was overridden by an interface derived
			 * from `this_interface', or if we are revisiting
			 * `this_interface' because there are multiple paths of
			 * inheritance between `derived_interface' and
			 * `this_interface'.
			 *
			 * In any case, the assumption is that we can ignore
			 * this redefinition and that we don't need to process
			 * the reply type.
			 */
			continue;
		
		request_type = tam_operation_request_struct(op_val);
		tam_interface_contents_add(interface_union,
					   request_discrim,
					   request_type,
					   derived_interface_name,
					   "operation request");
		
		/*
		 * Determine the MINT discriminator that corresponds to the
		 * reply code for this operation.
		 *
		 * XXX --- Yes, the request and reply codes must be distinct.
		 * This is a bug.
		 */
		if (mint_custom_op_const)
			reply_discrim =
				(*mint_custom_op_const)(this_interface,
							this_interface_name,
							derived_interface,
							derived_interface_name,
							op_val,
							MAKE_OP_REPLY);
		else
			reply_discrim =
				mint_new_const_from_aoi_const(op_val->
							      reply_code);
		
		/*
		 * Add this reply to the MINT union that represents the
		 * interface.
		 */
		if (mint_find_union_case(&out_mint,
					 interface_union, reply_discrim)
		    != mint_ref_null)
			/*
			 * This reply is already defined in the MINT for the
			 * `derived_interface'.
			 *
			 * As before, the assumption is that we can ignore
			 * this redefinition.  However, we can't `continue'
			 * here because we still need to record the connection
			 * between the `request_discrim' and `op_val'.
			 */
			/* Do nothing. */ ;
		else {
			reply_type = tam_operation_reply_struct(op_val,
								request_type);
			tam_interface_contents_add(interface_union,
						   reply_discrim,
						   reply_type,
						   derived_interface_name,
						   "operation reply");
		}
		
		/*
		 * Record the request and reply MINT discriminators so that the
		 * presentation generator can easily find them later.
		 */
		tam_interface_record_mint_discrims(derived_interface,
						   op_val,
						   request_discrim,
						   reply_discrim);
	}
	
	for (i = 0; i < attribs_len; ++i) {
		aoi_attribute *attrib_val = &(this_interface->
					      attribs.attribs_val[i]);
		
		/*
		 * Determine the MINT discriminator that corresponds to the
		 * read-request code for this attribute.
		 */
		if (mint_custom_attrib_const)
			request_discrim =
				(*mint_custom_attrib_const)(
					this_interface,
					this_interface_name,
					derived_interface,
					derived_interface_name,
					attrib_val,
					MAKE_ATTRIB_READ_REQUEST);
		else
			request_discrim =
				mint_new_const_from_aoi_const(
					attrib_val->read_request_code);
		
		/*
		 * Add this read-request to the MINT union that represents the
		 * interface.
		 */
		if (mint_find_union_case(&out_mint,
					 interface_union, request_discrim)
		    != mint_ref_null)
			/*
			 * This read-request is already defined in the MINT for
			 * the `derived_interface'.
			 *
			 * The assumption is that we can ignore this
			 * redefinition and that we don't need to process
			 * the read-reply type or any write-attribute stuff.
			 */
			continue;
		
		request_type = tam_attribute_read_request_struct(attrib_val);
		tam_interface_contents_add(interface_union,
					   request_discrim,
					   request_type,
					   derived_interface_name,
					   "attribute read request");
		
		/*
		 * Determine the MINT discriminator that corresponds to the
		 * read-reply code for this operation.
		 *
		 * XXX --- Yes, the request and reply codes must be distinct.
		 * This is a bug.
		 */
		if (mint_custom_attrib_const)
			reply_discrim =
				(*mint_custom_attrib_const)(
					this_interface,
					this_interface_name,
					derived_interface,
					derived_interface_name,
					attrib_val,
					MAKE_ATTRIB_READ_REPLY);
		else
			reply_discrim =
				mint_new_const_from_aoi_const(
					attrib_val->read_reply_code);
		
		/*
		 * Add this read-reply to the MINT union that represents the
		 * interface.
		 */
		if (mint_find_union_case(&out_mint,
					 interface_union, reply_discrim)
		    != mint_ref_null)
			/*
			 * This read-reply is already defined in the MINT for
			 * the `derived_interface'.
			 *
			 * As before, the assumption is that we can ignore
			 * this redefinition.  However, we can't `continue'
			 * here because we still need to record the connection
			 * between the `request_discrim' and `attrib_val'.
			 */
			/* Do nothing */ ;
		else {
			reply_type =
				tam_attribute_read_reply_struct(attrib_val);
			tam_interface_contents_add(interface_union,
						   reply_discrim,
						   reply_type,
						   derived_interface_name,
						   "attribute read reply");
		}
		
		/*
		 * Record the request and reply MINT discriminators so that the
		 * presentation generator can easily find them later.
		 */
		tam_interface_record_mint_discrims(derived_interface,
						   attrib_val,
						   request_discrim,
						   reply_discrim);
		
		/* If this attribute is read-only, skip the writing stuff. */
		if (attrib_val->readonly)
			continue;
		
		/*
		 * Determine the MINT discriminator that corresponds to the
		 * write-request code for this attribute.
		 */
		if (mint_custom_attrib_const)
			request_discrim =
				(*mint_custom_attrib_const)(
					this_interface,
					this_interface_name,
					derived_interface,
					derived_interface_name,
					attrib_val,
					MAKE_ATTRIB_WRITE_REQUEST);
		else
			request_discrim =
				mint_new_const_from_aoi_const(
					attrib_val->write_request_code);
		
		/*
		 * Add this write-request to the MINT union that represents the
		 * interface.
		 */
		if (mint_find_union_case(&out_mint,
					 interface_union, request_discrim)
		    != mint_ref_null)
			/*
			 * This write-request is already defined in the MINT
			 * for the `derived_interface'.
			 *
			 * The assumption is that we can ignore this
			 * redefinition and that we don't need to process
			 * the write-reply type.
			 */
			continue;
		
		request_type = tam_attribute_write_request_struct(attrib_val);
		tam_interface_contents_add(interface_union,
					   request_discrim,
					   request_type,
					   derived_interface_name,
					   "attribute write request");
		
		/*
		 * Determine the MINT discriminator that corresponds to the
		 * write-reply code for this operation.
		 *
		 * XXX --- Yes, the request and reply codes must be distinct.
		 * This is a bug.
		 */
		if (mint_custom_attrib_const)
			reply_discrim =
				(*mint_custom_attrib_const)(
					this_interface,
					this_interface_name,
					derived_interface,
					derived_interface_name,
					/* a_name */
					attrib_val,
					MAKE_ATTRIB_WRITE_REPLY);
		else
			reply_discrim =
				mint_new_const_from_aoi_const(
					attrib_val->write_reply_code);
		
		/*
		 * Add this write-reply to the MINT union that represents the
		 * interface.
		 */
		if (mint_find_union_case(&out_mint,
					 interface_union, reply_discrim)
		    != mint_ref_null)
			/*
			 * This write-reply is already defined in the MINT for
			 * the `derived_interface'.
			 *
			 * As before, the assumption is that we can ignore
			 * this redefinition.  However, we can't `continue'
			 * here because we still need to record the connection
			 * between the `request_discrim' and `attrib_val'.
			 */
			/* Do nothing */ ;
		else {
			reply_type =
				tam_attribute_write_reply_struct(attrib_val);
			tam_interface_contents_add(interface_union,
						   reply_discrim,
						   reply_type,
						   derived_interface_name,
						   "attribute write reply");
		}
		
		/*
		 * Record the request and reply MINT discriminators so that the
		 * presentation generator can easily find them later.
		 */
		tam_interface_record_mint_discrims(derived_interface,
						   attrib_val,
						   request_discrim,
						   reply_discrim);
		
	}
	
	/*
	 * Finally, we must process all of the operations and attributes
	 * defined by the inherited interfaces as well.
	 */
	for (i = 0; i < parents_len; ++i) {
		aoi_type parent_val = this_interface->parents.parents_val[i];
		aoi_ref  parent_ref;
		
		/*
		 * All parent references must be through indirects so that
		 * we can find the name to go with the parent interface!
		 */
		assert(parent_val->kind == AOI_INDIRECT);
		
		parent_ref = parent_val->aoi_type_u_u.indirect_ref;
		/*
		 * If `parent_ref' references a forward interface definition,
		 * find the real interface definition.
		 */
		parent_ref = aoi_deref_fwd(&in_aoi, parent_ref);
		
		tam_interface_contents(&(a(parent_ref).binding->
					 aoi_type_u_u.interface_def),
				       a(parent_ref).name,
				       derived_interface,
				       derived_interface_name,
				       interface_union);
	}
}

static void tam_interface_contents_add(mint_ref interface_union,
				       mint_const discrim,
				       mint_ref type,
				       char *interface_name,
				       char *type_name)
{
	int union_case;
	
	/*
	 * This check should *never* be true.  The callee is responsible for
	 * checking `mint_find_union_case' *before* calling this function.
	 */
	if (mint_find_union_case(&out_mint, interface_union, discrim)
	    != mint_ref_null)
		warn("Duplicate %s ignored for interface `%s'.",
		     type_name,
		     interface_name);
	else {
		union_case = expand_union_cases(interface_union);
		
		m(interface_union).mint_def_u.union_def.cases.
			cases_val[union_case].val
			= discrim;
		m(interface_union).mint_def_u.union_def.cases.
			cases_val[union_case].var
			= type;
	}
}

static mint_ref tam_operation_request_struct(aoi_operation *ops_val)
{
	mint_ref r = get_def();
	u_int slots_len = 0;
	u_int params_len = ops_val->params.params_len;
	u_int i;

	m(r).kind = MINT_STRUCT;
	m(r).mint_def_u.struct_def.slots.slots_len = slots_len;
	m(r).mint_def_u.struct_def.slots.slots_val =
		(mint_ref *)mustmalloc(sizeof(mint_ref)*params_len);

	/*
	 * For each parameter, add it to the structure if it should be in
	 * the request message.
	 */
	for (i = 0; i < params_len; i++)
	{
		aoi_parameter *params_val = &ops_val->params.params_val[i];
		if (params_val->direction == AOI_DIR_IN
		    || params_val->direction == AOI_DIR_INOUT)
		{
			/* Add the parameter to the structure. */
			tam_aoi_type(params_val->type,
				     &(m(r).mint_def_u.struct_def.slots.
				       slots_val[slots_len]));
			slots_len++;
			m(r).mint_def_u.struct_def.slots.slots_len = slots_len;
		}
	}

	return(r);
}

static mint_ref
tam_operation_reply_struct(aoi_operation *ops_val, mint_ref request_i)
{
	mint_ref u = get_def();
	u_int slots_len = 0;
	u_int params_len = ops_val->params.params_len;
	u_int i;
	u_int req_idx = 0;
	
	mint_ref r = get_def();
	
	m(r).kind = MINT_STRUCT;
	m(r).mint_def_u.struct_def.slots.slots_len = slots_len;
	/*
	 * Allocate params_len+1 mint refs, including one for the return value.
	 */
	m(r).mint_def_u.struct_def.slots.slots_val =
		(mint_ref *)mustmalloc(sizeof(mint_ref)*(params_len+1));
	
	/* provide the default for an exception discriminator */
	if (!mint_custom_exception_discrim_ref)
		mint_custom_exception_discrim_ref
			= out_mint.standard_refs.unsigned32_ref;
	
	/*
	 * For each parameter, add it to the structure if it should be in
         * the reply message.
	 */
	for (i = 0; i < params_len; i++)
	{
		aoi_parameter *params_val = &ops_val->params.params_val[i];
		if (params_val->direction == AOI_DIR_OUT
		    || params_val->direction == AOI_DIR_INOUT)
		{
			/* Add the parameter to the structure. */
			/*
			 * If it's an inout, don't retype it - just use the
			 * same type.
			 */
			if (params_val->direction == AOI_DIR_INOUT)
				m(r).mint_def_u.struct_def.slots.
					slots_val[slots_len] 
					= m(request_i).mint_def_u.
					struct_def.slots.slots_val[req_idx++];
			else
				tam_aoi_type(params_val->type,
					     &(m(r).mint_def_u.struct_def.
					       slots.slots_val[slots_len]));
			slots_len++;
			m(r).mint_def_u.struct_def.slots.slots_len = slots_len;
		} else
			req_idx++;
	}

	/* Add the return value of the operation. */
	tam_aoi_type(ops_val->return_type,
		     &(m(r).mint_def_u.struct_def.slots.
		       slots_val[slots_len]));
	slots_len++;
	m(r).mint_def_u.struct_def.slots.slots_len = slots_len;
	
	/*
	 * What we really want to return is the union of a normal response,
	 * system exception responses, & user exception responses
	 */
	m(u).kind = MINT_UNION;
	
	/*
	 * XXX --- It is not really correct to set the discriminator type to
	 * the unsigned32 type, although this is what the BE currently expects.
	 * Really, the back end should determine the actual type of exception
	 * discriminators.  We should just make ``symbolic'' discriminators
	 * here --- abstract MINT_EXCEPTIONs --- that are given on-the-wire
	 * encodings by the BE.
	 *
	 * The same comment applies to all of the other ``internal'' unions
	 * that we produce (i.e., those unions whose types are not explicitly
	 * defined by the source AOI).
	 */
	m(u).mint_def_u.union_def.discrim
		= out_mint.standard_refs.signed32_ref;
	m(u).mint_def_u.union_def.cases.cases_len
		= 2;
	m(u).mint_def_u.union_def.cases.cases_val
		= (mint_union_case *) mustmalloc(sizeof(mint_union_case) * 2);
	m(u).mint_def_u.union_def.cases.cases_val[0].val
		= mint_new_const_int(0);
	m(u).mint_def_u.union_def.cases.cases_val[0].var
		= r;
	m(u).mint_def_u.union_def.cases.cases_val[1].val
		= mint_new_const_int(-1);
	m(u).mint_def_u.union_def.cases.cases_val[1].var
		= out_mint.standard_refs.system_exception_ref;
	
	if (ops_val->exceps.exceps_len) {
		mint_ref uu;
		
		/* User-Union */
		uu = get_union_def_given_discriminator_type(
			ops_val->exceps.exceps_len,
			mint_custom_exception_discrim_ref);
			
		/*
		 * XXX --- See comment above about the types of ``internal''
		 * union discriminators.  It applies here too, except that
		 * here, a string discriminator is assumed, while each
		 * presentation could have different types.  XXX - This is
		 * currently a bug since the Fluke presentation expects an
		 * unsigned32 discriminator.
		 */
		for (i = 0; i < ops_val->exceps.exceps_len; i++) {
			mint_ref r;
			/*
			 * We must have an indirect ref to the exception's
			 * structure.
			 */
			assert(ops_val->exceps.exceps_val[i]->kind
			       == AOI_INDIRECT);
			r = aoi_to_mint_association[ops_val->exceps.
						   exceps_val[i]->aoi_type_u_u.
						   indirect_ref];
			/*
			 * Add the discriminator, then add the exception
			 * structure.
			 * XXX This should compute the RepositoryID for the
			 * discriminator.  Right now we just use the
			 * exception's unscoped name.
			 */
			if (mint_custom_exception_const) 
				m(uu).mint_def_u.union_def.cases.
					cases_val[i].val
				= (*mint_custom_exception_const)(
					(ops_val->exceps.exceps_val[i]->
					 aoi_type_u_u.indirect_ref),
					i);
			else
				m(uu).mint_def_u.union_def.cases.
					cases_val[i].val
					= mint_new_const_int(i);
			m(uu).mint_def_u.union_def.cases.cases_val[i].var
				= r;
		}
		/* No default value... */
		m(uu).mint_def_u.union_def.dfault = mint_ref_null;
		/* User exceptions are the default value of the return union */
		m(u).mint_def_u.union_def.dfault = uu;
	} else
		m(u).mint_def_u.union_def.dfault = mint_ref_null;
	
	return(u);
}

static mint_ref tam_attribute_read_request_struct(aoi_attribute *attribs_val)
{
	/* A read request is a request to "get" an attribute value.
	   So, the message has no parameters and "void" return value.
	   This function may simply return a MINT_VOID definition.  */
  /* That's WRONG - it needs to be a MINT_STRUCT */
  
  /*	return void_d;*/
	mint_ref r = get_def();

	m(r).kind = MINT_STRUCT;
	m(r).mint_def_u.struct_def.slots.slots_len = 0;
	m(r).mint_def_u.struct_def.slots.slots_val = 0 /* (mint_ref *)
							  mustmalloc(
							  sizeof(mint_ref));*/;
	
	/*	m(r).mint_def_u.struct_def.slots.slots_val[0] = void_d;*/

	return r;
}

static mint_ref tam_attribute_read_reply_struct(aoi_attribute *attribs_val)
{
	/* Create a MINT_STRUCT with the attribute type in it
	   (as the attribute value returned).  */

	mint_ref r = get_def();

	m(r).kind = MINT_STRUCT;
	m(r).mint_def_u.struct_def.slots.slots_len = 1;
	m(r).mint_def_u.struct_def.slots.slots_val =
		(mint_ref *)mustmalloc(sizeof(mint_ref));

	tam_aoi_type(attribs_val->type,
		     &(m(r).mint_def_u.struct_def.slots.slots_val[0]));

	return r;
}

static mint_ref tam_attribute_write_request_struct(aoi_attribute *attribs_val)
{
	/* Create a MINT_STRUCT with the attribute type in it
	   (as a parameter, which is the attribute to set). */

	mint_ref r = get_def();

	m(r).kind = MINT_STRUCT;
	m(r).mint_def_u.struct_def.slots.slots_len = 1;
	m(r).mint_def_u.struct_def.slots.slots_val =
		(mint_ref *)mustmalloc(sizeof(mint_ref));

	tam_aoi_type(attribs_val->type,
		     &(m(r).mint_def_u.struct_def.slots.slots_val[0]));

	return r;
}

static mint_ref tam_attribute_write_reply_struct(aoi_attribute *attribs_val)
{
	/* A write reply is a reply after an attribute value has been "set".
	   So, the message has no parameters and "void" return value.
	   This function may simply return a MINT_VOID definition.  */
  /* That's WRONG - it needs to be a MINT_STRUCT*/
  
  /*	return void_d;*/
	mint_ref r = get_def();

	m(r).kind = MINT_STRUCT;
	m(r).mint_def_u.struct_def.slots.slots_len = 0;
	m(r).mint_def_u.struct_def.slots.slots_val = 0 /* (mint_ref *)
							  mustmalloc(
							  sizeof(mint_ref));*/;
	/*	m(r).mint_def_u.struct_def.slots.slots_val[0] = void_d;*/

	return r;
}

void tam_interface_record_mint_discrims(aoi_interface *interface,
					void *op_or_attr,
					mint_const request_discrim,
					mint_const reply_discrim)
{
	mint_const dummy_request_discrim;
	mint_const dummy_reply_discrim;
	
	interface_mint_table_elem *this_elem;
	
	/*****/
	
	if (lookup_interface_mint_discrims(interface, op_or_attr,
					   &dummy_request_discrim,
					   &dummy_reply_discrim)) {
		/*
		 * We already have this (interface, operation/attribute) pair
		 * in the table.  We should never get here; however, we'll let
		 * it slide *if* we're not trying to change the associations.
		 */
		if ((request_discrim != dummy_request_discrim)
		    || (reply_discrim != dummy_reply_discrim))
			/* XXX --- Should panic. */
			warn("In `tam_record_operation_mint_discrims', "
			     "attempt to change previously-recorded MINT "
			     "discriminators.");
		return;
	}
	
	if (interface_mint_table_count >= interface_mint_table_size) {
		/*
		 * Grow the `interface_mint_table'.
		 */
		interface_mint_table_size = interface_mint_table_count
					    + INTERFACE_MINT_TABLE_INCREMENT;
		interface_mint_table =
			(interface_mint_table_elem *)
			mustrealloc(interface_mint_table,
				    (sizeof(interface_mint_table_elem)
				     * interface_mint_table_size));
	}
	
	this_elem = &(interface_mint_table[interface_mint_table_count]);
	
	this_elem->interface = interface;
	this_elem->op_or_attr = op_or_attr;
	this_elem->request_discrim = request_discrim;
	this_elem->reply_discrim = reply_discrim;
	
	++interface_mint_table_count;
}

int lookup_interface_mint_discrims(aoi_interface *interface,
				   void *op_or_attr,
				   /* OUT */ mint_const *request_discrim,
				   /* OUT */ mint_const *reply_discrim)
{
	int i;
	
	for (i = 0; i < interface_mint_table_count; ++i)
		if ((interface_mint_table[i].interface == interface)
		    && (interface_mint_table[i].op_or_attr == op_or_attr)) {
			/*
			 * We've found the right table element.
			 */
			*request_discrim = interface_mint_table[i].
					   request_discrim;
			*reply_discrim   = interface_mint_table[i].
					   reply_discrim;
			return 1;
		}
	
	*request_discrim = 0;
	*reply_discrim = 0;
	return 0;
}

static void init_interface_mint_table()
{
	/*
	 * Clear our associations for (interface, operation/attribute) to MINT
	 * discriminators.
	 */
	interface_mint_table_count = 0;
	interface_mint_table_size = INTERFACE_MINT_TABLE_INCREMENT;
	interface_mint_table =
		(interface_mint_table_elem *)
		mustmalloc(sizeof(interface_mint_table_elem)
			   * interface_mint_table_size);
}

/*****************************************************************************/

static mint_ref tam_exception(aoi_exception *exception_def)
{
	mint_ref r = get_def();
	int slots_len = exception_def->slots.slots_len;
	int i;

	m(r).kind = MINT_STRUCT;
	m(r).mint_def_u.struct_def.slots.slots_len = slots_len;
	m(r).mint_def_u.struct_def.slots.slots_val =
		(mint_ref *)mustmalloc(sizeof(mint_ref)*slots_len);

	/* For each slot in the exception, translate the slot AOI type to a
           mint_ref. */
	for (i = 0; i < slots_len; i++)
		tam_aoi_type(exception_def->slots.slots_val[i].type,
			     &(m(r).mint_def_u.struct_def.slots.slots_val[i]));
	
	return(r);
}


static mint_ref tam_void()
{
	mint_ref r = get_def();
	
	/* Translate the AOI_VOID type to MINT_VOID. */
	m(r).kind = MINT_VOID;
	
	return(r);
}


static mint_ref tam_optional(aoi_optional *optional_def)
{
	/* An AOI_OPTIONAL translates into a variable-length MINT array.
	   The array may contain zero elements or one element. */
	mint_ref r = get_def();
	
	m(r).kind = MINT_ARRAY;
	tam_aoi_type(optional_def->type,
		     &(m(r).mint_def_u.array_def.element_type));
	m(r).mint_def_u.array_def.length_type
		= out_mint.standard_refs.bool_ref;
	
	return r;
}


static mint_ref tam_any()
{
	mint_ref r = get_def();
	
	/* Translate the AOI_ANY type to MINT_ANY. */
	m(r).kind = MINT_ANY;
	
	return(r);
}


static mint_ref tam_type_tag()
{
	mint_ref r = get_def();
	
	/* Translate the AOI_TYPE_TAG type to MINT_TYPE_TAG. */
	m(r).kind = MINT_TYPE_TAG;
	
	return(r);
}


static mint_ref tam_typed(aoi_typed *typed_def)
{
	/* An `AOI_TYPED' translates into a corresponding `MINT_TYPED'. */
	mint_ref r = get_def();
	
	m(r).kind = MINT_TYPED;
	tam_aoi_type(typed_def->tag,
		     &(m(r).mint_def_u.typed_def.tag));
	tam_aoi_type(typed_def->type,
		     &(m(r).mint_def_u.typed_def.ref));
	
	return r;
}


/* auxiliary, helper routine as in mom/fe/sun/xlate.c */

/* Allocate a new (uninitialized) definition in the out_mint.  */
/* This is an auxiliary, helper routine as in mom/fe/sun/xlate.c */
static int get_def()
{
	int result;
	
	result = out_mint.defs.defs_len++;
	if (out_mint.defs.defs_len > (unsigned int) mint_max) {
		/* Reallocate our buffer. */
		mint_max = mint_max ? (mint_max *2) : 8;
		out_mint.defs.defs_val
			= ((mint_def *)
			   mustrealloc((void *) out_mint.defs.defs_val,
				       (size_t) (sizeof(mint_def)
						 * mint_max)));
	}
	
	return result;
}

static mint_ref get_union_def(int len)
{
	mint_ref r = get_def();
	m(r).kind = MINT_UNION;
	m(r).mint_def_u.union_def.discrim
		= out_mint.standard_refs.signed32_ref;
	m(r).mint_def_u.union_def.cases.cases_len
		= len;
	m(r).mint_def_u.union_def.cases.cases_val
		= mustmalloc(len ? (len * sizeof(mint_union_case)) : 1);
	m(r).mint_def_u.union_def.dfault
		= mint_ref_null;
	
	return r;
}

static int expand_union_cases(mint_ref r)
{
	int i = m(r).mint_def_u.union_def.cases.cases_len++;
	m(r).mint_def_u.union_def.cases.cases_val =
		(mint_union_case *)
		mustrealloc(m(r).mint_def_u.union_def.cases.cases_val,
			    sizeof(mint_union_case) * (i+1));
	return i;
}

#if 0  /* currently unused, but might be used later */
static int xl_int(int min, unsigned range)
{
	unsigned int i;

        /* See if there's already a matching int defined.  */
        for (i = 0; i < out_mint.defs.defs_len; i++)
        {
                if ((m(i).kind == MINT_INTEGER)
                    && (m(i).mint_def_u.integer_def.min == min)
                    && (m(i).mint_def_u.integer_def.range == range))
                {
                        fprintf(stderr, "Returning dup int %d.\n", i);
                        return i;
                }
        }

        /* If not, create one.  */
        i = get_def();
        m(i).kind = MINT_INTEGER;
        m(i).mint_def_u.integer_def.min = min;
        m(i).mint_def_u.integer_def.range = range;
        return i;
}
#endif /* 0 */

#if 0  /* currently unused, but might be used later */
/* translate array, taking min and length args */
static int xl_array_minlen(mint_ref type_d, int min, unsigned len)
{
	mint_ref length_d;
        int i;

	/* Find/produce a datatype representing the possible array lengths.  */
	length_d = xl_int(min, len);

        /* See if there's already a matching array defined.  */
        for (i = 0; i < out_mint.defs.defs_len; i++)
        {
                if ((m(i).kind == MINT_ARRAY)
                    && (m(i).mint_def_u.array_def.element_type == type_d)
                    && (m(i).mint_def_u.array_def.length_type == length_d))
                {
                        fprintf(stderr, "returning dup array %d.\n", i);
                        return i;
                }
        }

        /* If not, create one.  */
        i = get_def();
        m(i).kind = MINT_ARRAY;

        m(i).mint_def_u.array_def.element_type = type_d;
        m(i).mint_def_u.array_def.length_type = length_d;
        return i;
}
#endif


/* auxiliarly, helper routines (not taken from mom/fe/sun/xlate.c) */

/* Return a new union, as in get_union_def() but with a discriminator type
   given as an argument (rather than assuming a 32-bit integer type). */
static mint_ref
get_union_def_given_discriminator_type(int len, mint_ref discriminator_type)
{
	mint_ref r = get_def();
	m(r).kind = MINT_UNION;
	m(r).mint_def_u.union_def.discrim = discriminator_type;
	m(r).mint_def_u.union_def.cases.cases_len = len;
	m(r).mint_def_u.union_def.cases.cases_val =
		mustmalloc(len ? len*sizeof(mint_union_case) : 1);
	m(r).mint_def_u.union_def.dfault = mint_ref_null;
	return r;
}

/* End of file. */

