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

#ifdef RPC_HDR
%#ifndef _flick_aoi_h
%#define _flick_aoi_h
%
%#include <rpc/types.h>
%#include <rpc/xdr.h>
%#include <mom/meta.h>
#endif

/*
 * AOI builds on MINT, adding higher-level more-abstract interface information.
 * This information is not necessary to produce marshal/unmarshal stubs, and
 * thus it is not used by PBE's.  However, it is useful earlier in the pipeline
 * for other things:
 *
 *   + Producing convenient default presentations of a MINT
 *     based on constructs not directly representable in the MINT,
 *     such as procedures, objects, interfaces, subtyping, etc.
 *   + Analyzing and type-checking interfaces at an abstract level.
 *
 * What is the criteria for determining what goes in and what stays out?
 * Basically, the "bottom" boundary is provided by the definition of MINT;
 * the "top" boundary is the set of features in commonly used existing RPC
 * and object systems - CORBA and DCE in particular.
 *
 * Specific interesting features possibly to be included at some point:
 *   + CORBA-style interfaces with interface inheritance
 *   + CORBA-style "attributes" as wrappers for set/query RPCs.
 *   + CORBA-style exceptions
 *   + Exceptions specifiable for entire interfaces?
 *   + Aliased pointers, as in DCE.
 *   +  Arbitrary pass-by-value objects, as in Spring.
 *   + Explicit interface constraints
 *   + Struct and union inheritance
 *
 * We can assume that a MINT will always be used with something else -
 * an AOI, or a PRES_C, or whatever.
 *
 * One way to state the dividing line between MINT and AOI: with only a MINT,
 * you can completely "parse" a canonical data stream that conforms to that
 * message interface definition - the entire message can be resolved into a
 * data tree with no additional information.  AOI only adds meaning to the
 * data.
 */

typedef int aoi_ref;
const aoi_ref_null = -1;

typedef struct aoi_type_u *aoi_type;
typedef struct aoi_const_u *aoi_const;

typedef long aoi_const_int;
typedef char aoi_const_char;
typedef double aoi_const_float;
typedef aoi_const aoi_const_struct<>;
typedef aoi_const aoi_const_array<>;

enum aoi_const_kind
{
	AOI_CONST_INT		= 1,
	AOI_CONST_CHAR		= 2,
	AOI_CONST_FLOAT		= 3,
	AOI_CONST_STRUCT	= 4,
	AOI_CONST_ARRAY		= 5
};

union aoi_const_u
switch (aoi_const_kind kind)
{
	case AOI_CONST_INT:	aoi_const_int		const_int;
	case AOI_CONST_CHAR:	aoi_const_char		const_char;
	case AOI_CONST_FLOAT:	aoi_const_float		const_float;
	case AOI_CONST_STRUCT:	aoi_const_struct	const_struct;
	case AOI_CONST_ARRAY:	aoi_const_array		const_array;
};

struct aoi_field
{
	string		name<>; 
	aoi_type	type;
};

/*****************************************************************************/

/*
 * An `aoi_const_def' represents a typed constant value.  This is used when a
 * named constant is defined.  Knowing the AOI type of a constant (e.g., the
 * signedness of the type) is important to some presentation generators.
 */
struct aoi_const_def
{
	aoi_type	type;
	aoi_const	value;
};

/*
 * An `aoi_integer' describes an integer type as a range of values, starting at
 * some specific minimum value.  This is the preferred representation for
 * integer types and many compile-time integer constants.  (A constant is
 * described as a type with a range of zero.)
 */
struct aoi_integer
{
	/*
	 * Lowest possible value this integer can take.
	 * If >= 0, it's an unsigned integer.
	 */
	int min;
	
	/*
	 * Number of possible values this integer can take in addition to
	 * `min'.  The maximum legal value is `min + range'.
	 *
	 * If range is 0, then the integer can take only one value --- i.e.,
	 * it carries no information; useful for representing fixed-length
	 * arrays.
	 *
	 * If range is 1, it's a boolean.
	 */
	unsigned range;
};

/*
 * An `aoi_scalar' describes an integer type by the number of bits in its
 * range.  The minimum value is implicit and what one would expect.  For signed
 * scalar types, the minimum is the smallest representable two's-complement
 * value; e.g., -(2^31) for a 32-bit scalar.  For unsigned scalars, the minimum
 * value is zero.
 *
 * `aoi_scalar's are only used to describe integer types that `aoi_integer'
 * cannot represent (e.g., 64-bit and 128-bit integers).
 */
typedef u_int aoi_scalar_flags;
const AOI_SCALAR_FLAG_NONE	= 0;
const AOI_SCALAR_FLAG_SIGNED	= 1;
const AOI_SCALAR_FLAG_UNSIGNED	= 2;

struct aoi_scalar 
{
	int bits;
	aoi_scalar_flags flags;
};

struct aoi_float
{
	int bits;
};

/* These are flags to modify aoi_char's: signed, unsigned, or default. */
typedef u_int aoi_char_flags;
const AOI_CHAR_FLAG_NONE     = 0;
const AOI_CHAR_FLAG_SIGNED   = 1;
const AOI_CHAR_FLAG_UNSIGNED = 2;

struct aoi_char
{
	int bits;
	aoi_char_flags flags;
};

/* These are flags to modify aoi_array types, such as to indicate
   that an array is a terminated array. */
typedef u_int aoi_array_flags;
const AOI_ARRAY_FLAG_NONE			= 0x00000000;
const AOI_ARRAY_FLAG_NULL_TERMINATED_STRING	= 0x00000001;
const AOI_ARRAY_FLAG_OPAQUE			= 0x00000002;
const AOI_ARRAY_FLAG_ALL			= 0x00000003;

struct aoi_array
{
	/* Type of the elements in this array. */
	aoi_type element_type;	  /* YYY should be a ref */

	/*
	 * Type representing the possible lengths of this array.
	 * Must be an integer type of some kind.
	 * If the integer's `range' is zero, it's a fixed-length array.
	 */
	aoi_type length_type;

	aoi_array_flags		flgs;
};

typedef aoi_field aoi_struct_slot;
struct aoi_struct
{
	/* Type of each structure slot, in order.  */
	aoi_struct_slot slots<>;
};


struct aoi_union_case
{
	aoi_const	val;
	aoi_field	var;
};

struct aoi_union
{
	/* Union discriminator variable type & name
	   Can be anything, even strings and other constructed types.  */
	aoi_field discriminator;
	/* the label for the C union */
	string union_label<>;
	
	/* Variable for each non-default case. */
	aoi_union_case cases<>;
	
	/*
	 * Variable for the default case, null if no default.
	 * (If there's a default case but it's void,
	 * then this field is non-null but its type is AOI_VOID.)
	 */
	aoi_field *dfault;
};

enum aoi_direction
{
	AOI_DIR_IN	= 1,
	AOI_DIR_OUT	= 2,
	AOI_DIR_INOUT	= 3,
	AOI_DIR_RET	= 4
};

struct aoi_parameter
{
	string	name<>;
	aoi_direction	direction;
	aoi_type	type;
};

typedef u_int aoi_op_flags;
/*
 * These are flag bits that may be and'ed and or'ed together.
 */
const AOI_OP_FLAG_NONE		= 0x00000000;
const AOI_OP_FLAG_ONEWAY	= 0x00000001;
const AOI_OP_FLAG_IDEMPOTENT	= 0x00000002;
const AOI_OP_FLAG_SETTER	= 0x00000004;
const AOI_OP_FLAG_GETTER	= 0x00000008;

struct aoi_operation
{
	string	name<>;

	/*
	 * Code uniquely identifying this operation within this interface;
	 * works the same way as the code_type and code in aoi_interface.
	 */
	aoi_const	request_code;
	aoi_const	reply_code;

	aoi_op_flags	flags;

	/* Parameters to/from this operation.  */
	aoi_parameter params<>;

	/* Return type, if any. */
	aoi_type return_type;

	/*
	 * Exceptions that can be raised by this method.  These must be
	 * AOI_INDIRECTS so that we can access the exceptions' names.
	 */
	aoi_type exceps<>;
};

struct aoi_attribute
{
	string	name<>;

	/* Attribute code/identifier.  */
	aoi_const read_request_code;
	aoi_const read_reply_code;
	aoi_const write_request_code;
	aoi_const write_reply_code;

	/* Type of this attribute.  */
	aoi_type type;

	/* Is the attribute readonly */
	bool readonly;
};

typedef aoi_field aoi_exception_slot;
struct aoi_exception
{
	aoi_exception_slot slots<>;
};

enum aoi_idl_id
{
	AOI_IDL_CORBA		= 1,
	AOI_IDL_MIG		= 2,
	AOI_IDL_SUN		= 3,
	AOI_IDL_DCE		= 4
};


struct aoi_interface
{
	/* IDL this interface was defined in - "top-level" interface ID.  */
	aoi_idl_id	idl;

	/*
	 * `code_type' is the type of the identifying interface, e.g. a
	 * char-array (string) for CORBA, a number for MIG, a struct for DCE
	 * uuid's, etc.  `code' is the actual code identifying this interface.
	 */
	aoi_type	code_type;
	aoi_const	code;

	/* Other interfaces this inherits from. */
	aoi_type	parents<>;

	/* Type of the identifying code of each attribute/operation.  */
	aoi_type	op_code_type;

	/* Operations and attributes accessible through this interface. */
	aoi_operation ops<>;
	aoi_attribute attribs<>;

	/* Exceptions that can be raised by any method in this interface.  */
	aoi_type	excepts<>;
};


struct aoi_enum
{
	/* the string from the `enum' label. */
	string enum_label<>;
	
	/* These must all be AOI_INTEGER defs. */
	/*
	 * Actually, you can override them to be whatever you want In the
	 * current implementation, they should be integers OR aoi_consts [Sun
	 * XDR allows them to be values].
	 */
	struct aoi_field defs<>;
};

/*
 * An `aoi_optional' represents a denoted optional data element.  ``Denoted''
 * means that the item was specified as optional using the IDL's standard
 * notation for an optional data element.
 *
 * Why is `aoi_optional' required?  Optional data can be described in many
 * different ways in an IDL file: as a variable-length array with zero or one
 * elements, as a discriminated union with a void variant, or as a denoted
 * optional element.  Although these different techniques are logically
 * equivalent, each technique generally results in a different presentation ---
 * that is, a different set of RPC stub interfaces.
 *
 * No technique for describing optionality is redundant because each may result
 * in a different presentation.  Therefore, AOI needs ways to describe every
 * different IDL technique for representing optional data.
 */
struct aoi_optional
{
	aoi_type type;
};

/*
 * An `aoi_typed' represents a pair containing a type tag and a value.  The
 * type tag describes the type of the given value data.  (In other words, an
 * `aoi_typed' is similar to a structure, with the additional semantic that the
 * data in the type tag slot describes the type of the data in the value slot.)
 */
struct aoi_typed
{
	/* XXX --- Should `tag' and `type' be `aoi_field's? */
	
	aoi_type	tag;
	aoi_type	type;
};

enum aoi_kind
{
	AOI_INDIRECT	= 1,
	AOI_INTEGER	= 2,
	AOI_SCALAR	= 3, 
	AOI_FLOAT	= 4, 
	AOI_CHAR	= 5, 
	AOI_ARRAY	= 6, 
	AOI_STRUCT	= 7, 
	AOI_UNION	= 8, 
	AOI_INTERFACE	= 9, 
	AOI_EXCEPTION	= 10,
	AOI_ENUM	= 11,
	AOI_VOID	= 12,
	AOI_CONST	= 13,
	AOI_NAMESPACE	= 14,
	AOI_OPTIONAL	= 15,
	AOI_FWD_INTRFC	= 16,
	
	/*
	 * An `AOI_ANY' represents ``any type.''  Note that an `AOI_ANY' does
	 * *not* implicitly carry a type tag.  To represent ``tagged'' types,
	 * use `AOI_TYPED' instead.
	 */
	AOI_ANY		= 17,
	
	/*
	 * An `AOI_TYPE_TAG' is an opaque (to Flick) indicator of a type.  This
	 * useful, for instance, for representing CORBA TypeCodes or for
	 * communicating MIG polymorphic type information.
	 */
	AOI_TYPE_TAG	= 18,
	AOI_TYPED	= 19,
	
	/*
	 * `AOI_ERROR's are used only by the front ends, and only in order to
	 * deal with parsing errors.  An `AOI_ERROR' should *never* appear in
	 * an actual output `.aoi' file.
	 */
	AOI_ERROR	= 20
};

union aoi_type_u
switch (aoi_kind kind)
{
	case AOI_INDIRECT:	aoi_ref			indirect_ref;
	case AOI_INTEGER:	aoi_integer		integer_def;
	case AOI_SCALAR:	aoi_scalar		scalar_def;
	case AOI_FLOAT:		aoi_float		float_def;
	case AOI_CHAR:		aoi_char		char_def;
	case AOI_ARRAY:		aoi_array		array_def;
	case AOI_STRUCT:	aoi_struct		struct_def;
	case AOI_UNION:		aoi_union		union_def;
	case AOI_INTERFACE:	aoi_interface		interface_def;
	case AOI_EXCEPTION:	aoi_exception		exception_def;
	case AOI_ENUM:		aoi_enum		enum_def;
	case AOI_VOID:		void;
	case AOI_CONST:		aoi_const_def		const_def;
	case AOI_NAMESPACE:	void;
	case AOI_OPTIONAL:	aoi_optional		optional_def;
	case AOI_FWD_INTRFC:	aoi_ref			fwd_intrfc_def;
	case AOI_ANY:		void;
	case AOI_TYPE_TAG:	void;
	case AOI_TYPED:		aoi_typed		typed_def;
	case AOI_ERROR:		void;
};

struct aoi_def
{
	/* defined types */
	string	name<>;
	/* this is a monotonically increasing scope index */
	int scope;
	/* reference to the idl file that this def came from */
	io_file_index idl_file;
	/* Binding of this name, 0 if unknown. */
	aoi_type  binding;
};

struct aoi {
	aoi_def		defs<>;
	meta		meta_data;
};

#ifdef RPC_HDR
%#endif /* _flick_aoi_h */
#endif

/* End of file. */
