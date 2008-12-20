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
%#ifndef _flick_mint_h
%#define _flick_mint_h
%
%#include <rpc/types.h>
%#include <rpc/xdr.h>
#endif
/*
 * Planned changes:
 *
 *   + Add pointers supporting full circularity.
 *   + Use pointers here instead of mint_ref's.
 *   + Change name_def.val to name_def.binding.
 *   + Change 'array' to 'sequence' - more appropriate term.
 *
 * Maybe changes:
 *   + Collapse some of the structs containing only one item.
 *   + Support arbitrary-size integers (bits=0?).
 *   + Support a time data type.
 */

/*
 * A user-defined data type can be used by many other types and RPC
 * procedures, and types can even be circularly defined in terms of
 * themselves (e.g. linked lists of structures).  Thus, we need
 * something equivalent to a pointer, even though we don't support
 * pointers (at least not yet).  So for this purpose we use integer
 * indexes into a mint_def_table.
 */
typedef int mint_ref;

/*
 * In some cases we need the equivalent of a "null pointer" here.
 * We define the value -1 as the mint_ref equivalent of a null pointer.
 */
const mint_ref_null = -1;

/*
 * We also sometimes need a way to refer to no slot of a MINT structure, no
 * case within a union, etc.
 */
const mint_slot_index_null = -1;

/*****************************************************************************/

typedef struct mint_const_u *mint_const;

enum mint_const_kind
{
	MINT_CONST_INT		= 1,
	MINT_CONST_CHAR		= 2,
	MINT_CONST_FLOAT	= 3,
	MINT_CONST_STRUCT	= 4,
	MINT_CONST_ARRAY	= 5
};

enum mint_const_category
{
	MINT_CONST_LITERAL	= 1,
	MINT_CONST_SYMBOLIC	= 2
};

union mint_const_int_u
switch (mint_const_category kind)
{
	case MINT_CONST_LITERAL:	long		value;
	case MINT_CONST_SYMBOLIC:	string		name<>;
};

union mint_const_char_u
switch (mint_const_category kind)
{
	case MINT_CONST_LITERAL:	char		value;
	case MINT_CONST_SYMBOLIC:	string		name<>;
};

union mint_const_float_u
switch (mint_const_category kind)
{
	case MINT_CONST_LITERAL:	double		value;
	case MINT_CONST_SYMBOLIC:	string		name<>;
};

typedef mint_const mint_const_struct<>;
typedef mint_const mint_const_array<>;

union mint_const_u
switch (mint_const_kind kind)
{
	case MINT_CONST_INT:	mint_const_int_u	const_int;
	case MINT_CONST_CHAR:	mint_const_char_u	const_char;
	case MINT_CONST_FLOAT:	mint_const_float_u	const_float;
	case MINT_CONST_STRUCT:	mint_const_struct	const_struct;
	case MINT_CONST_ARRAY:	mint_const_array	const_array;
};

/*****************************************************************************/

/* Enumeration of each of the data type categories supported.
   Each mint_def_kind is basically a type function.  */
enum mint_def_kind
{
	/* These types correspond to primitive C/C++ data types. */	
	MINT_VOID		= 0,
	MINT_BOOLEAN		= 1,
	MINT_INTEGER		= 2,
	MINT_SCALAR		= 3,
	MINT_FLOAT		= 4,
	MINT_CHAR		= 5,
	MINT_ARRAY		= 6,
	MINT_STRUCT		= 7,
	MINT_UNION		= 8,

	/* These types are for higher-level RPC-related types. */
	MINT_INTERFACE		= 9,
	MINT_SYSTEM_EXCEPTION	= 10,

	/* Generic kind that can refer to any type. */
	MINT_ANY		= 11,

	/*
	 * A MINT_TYPE_TAG is an opaque (to Flick) indicator of a type.  This
	 * useful, for instance, for communicating MIG polymorphic type info.
	 *
	 * A MINT_TYPED is a composite type, like a struct, that has two parts.
	 * One component of a MINT_TYPED describes the type of the other.
	 * Obviously, the first component is generally a MINT_TYPE_TAG.
	 */
	MINT_TYPE_TAG		= 12,
	MINT_TYPED		= 13
};

struct mint_integer_def
{
	/* Lowest possible value this integer can take.
	   If >= 0, it's an unsigned integer.  */
	int min;

	/* Number of possible values this integer can take in addition to `min'.
	   The maximum legal value is `min + range'.
	   If range is 0, the integer can take only one value -
	   i.e. it carries no information; useful for representing fixed-length arrays.
	   If range is 1, it's a boolean.  */
	unsigned range;
};

/* See aoi.x for an (brief) explanation of scalar vs integer */
typedef u_int mint_scalar_flags;
const MINT_SCALAR_FLAG_NONE     = 0;
const MINT_SCALAR_FLAG_SIGNED   = 1;
const MINT_SCALAR_FLAG_UNSIGNED = 2;

struct mint_scalar_def
{
	int bits;
	mint_scalar_flags	flags;
};

/* These are flags to modify mint_char's: signed, unsigned, or default. */
typedef u_int mint_char_flags;
const MINT_CHAR_FLAG_NONE     = 0;
const MINT_CHAR_FLAG_SIGNED   = 1;
const MINT_CHAR_FLAG_UNSIGNED = 2;

struct mint_char_def
{
	/* Currently, `bits' may be 8 (char) or 16 (wchar).  */
	int	bits;
	mint_char_flags flags;
};

struct mint_float_def
{
	/* Currently may be 32 or 64.  */
	int	bits;
};

struct mint_array_def
{
	/* Type of each of the elements in this array.  */
	mint_ref element_type;

	/* Type representing the possible lengths of this array.
	   Must be an integer type of some kind.
	   If the integer's `range' is zero, it's a fixed-length array.  */
	mint_ref length_type;
};

struct mint_struct_def
{
	/* Type of each structure slot, in order.  */
	mint_ref slots<>;
};

struct mint_union_case
{
	mint_const	val;
	mint_ref	var;
};

struct mint_union_def
{
	/* Union discriminator variable.
	   Type must be based on an int or enum prim_type.  */
	mint_ref discrim;

	/* Variable for each non-default case.
	   Any case can be void, meaning no additional data. */
	mint_union_case cases<>;

	/* Variable for the default case, `mint_ref_null' if no default.
	   (If there's a default case but it's void, then `dfault' should
	   point to a MINT_VOID. */
	mint_ref dfault;
};

/* used for type-tagged data, e.g. MIG's polymorphic type */ 
struct mint_typed_def
{
	mint_ref	tag;
	mint_ref	ref;
};


/*
 * The definitions above are only for simple, passive data types - types whose
 * entire object state can be trivially encapsulated into a simple canonical
 * byte stream format.  However, MINT also supports definitions of more active
 * entities, such as RPC interfaces and object references on which methods or
 * operations may be invoked, as defined below.
 */

enum mint_interface_right
{
	MINT_INTERFACE_NAME		= 0,    /* just names a interface   */
	MINT_INTERFACE_INVOKE		= 1,	/* can invoke an interface  */
	MINT_INTERFACE_INVOKE_ONCE	= 2,	/* can invoke only once     */
	MINT_INTERFACE_SERVICE		= 3	/* can service an interface */
};


struct mint_interface_def
{
	mint_interface_right	right;
};

/* Parameters for a particular primitive type function.  */
union mint_def
switch (mint_def_kind kind)
{
	case MINT_VOID:			void;
	case MINT_BOOLEAN:		void;
	case MINT_INTEGER:		mint_integer_def	integer_def;
	case MINT_SCALAR:		mint_scalar_def		scalar_def;
	case MINT_FLOAT:		mint_float_def		float_def;
	case MINT_CHAR:			mint_char_def		char_def;
	case MINT_ARRAY:		mint_array_def		array_def;
	case MINT_STRUCT:		mint_struct_def		struct_def;
	case MINT_UNION:		mint_union_def		union_def;
	
	case MINT_INTERFACE:		mint_interface_def	interface_def;
	case MINT_SYSTEM_EXCEPTION:	void;
	
	case MINT_ANY:			void;
	
	case MINT_TYPE_TAG:		void;
	case MINT_TYPED:		mint_typed_def		typed_def;
};

/*
 * A `mint_standard_refs' contains `mint_ref's for commonly-used MINT types.
 * It is useful to have immediate access to these basic building blocks.
 */
struct mint_standard_refs
{
	mint_ref void_ref;
	
	mint_ref bool_ref;
	mint_ref signed8_ref;
	mint_ref signed16_ref;
	mint_ref signed32_ref;
	mint_ref signed64_ref;
	mint_ref unsigned8_ref;
	mint_ref unsigned16_ref;
	mint_ref unsigned32_ref;
	mint_ref unsigned64_ref;
	
	mint_ref char8_ref;
	mint_ref float32_ref;
	mint_ref float64_ref;
	
	mint_ref interface_name_ref;
	mint_ref interface_invoke_ref;
	mint_ref interface_invoke_once_ref;
	mint_ref interface_service_ref;
	mint_ref system_exception_ref;
};

/*
 * `mint_1' defines an entire set of possibly self-referencing defs.
 * mint_ref's contained by these definitions refer to indexes into `defs'.
 *
 * `mint_1' used to be defined as:	typedef mint_def mint_1<>;
 *
 * But we discovered that we needed a place to keep pointers to well-known
 * MINT types, too.  So `standard_refs' was added.
 */
struct mint_1
{
	mint_def		defs<>;
	
	/* `standard_refs' are initialized by `mint_add_standard_defs'. */
	mint_standard_refs	standard_refs;
};

#ifdef RPC_HDR
%#endif /* _flick_mint_h */
#endif

/* End of file. */
