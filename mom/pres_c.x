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
%#ifndef _flick_pres_c_h
%#define _flick_pres_c_h
%
%#include <rpc/types.h>
%#include <rpc/xdr.h>
%#include <mom/mint.h>
%#include <mom/cast.h>
%#include <mom/aoi.h>
%#include <mom/meta.h>
#endif

/*
 * itype: interface type.
 * ptype: presentation type.
 *
 * There is not always a one-to-one mapping between IDL constructs and PRES_C
 * or CAST constructs.  We may (and often do) require a tree of PRES_C and CAST
 * nodes to describe the presentation of a single IDL-defined entity.  For
 * example, an IDL-defined sequence generally corresponds to a `pres_c_inline_
 * allocation_context', which is the root of a PRES_C tree that describes
 * associations between aspects of the sequence (e.g., maximum length, actual
 * length, and the data vector) and separate CAST elements (e.g., the slots of
 * a C structure).
 *
 * It is not generally necessary or desirable for presentation mappings to
 * indicate which interface type they map.  Only the "top-level" presentation
 * declarations that directly cause the PBE to produce something need this.  In
 * all other cases, the interface type can easily be found based on context.
 * Conveniently, this allows a single ptype to map multiple itypes - e.g. a C
 * 'int' ptype that maps multiple integerish itypes.  However, this "feature"
 * is very limited, since the kind and format of the presentation mapping must
 * simultaneously match all interface types it is supposed to map.
 *
 * Possible ways to deal with exceptions on the client side:
 *   1 Invoke a non-returning function.
 *   2 Invoke a returning function, and return its result as an error/status
 *     code. (May be void.)
 *   3 Throw an exception using the MOM runtime library's exception mechanism.
 *     Really just #1 with some sugar coating.
 *     But decoding is done on the way down through the stack, not initially?
 *   4 Invoke a returning function, and retry the call, possibly with different
 *     arguments?
 *   5 Fill in an 'exception' out parameter and return a status code.
 *   6 Just return a status code.
 *
 * As far as client-side stub generation is concerned, the only thing it ever
 * has to do is call a particular routine if an encoding/decoding error occurs
 * or if an unexpected reply is received.
 *
 * Possible ways to deal with exceptions on the server side:
 *   1 Call a generic/specific routine to set the exception status before
 *     returning.
 *   2 Call a generic/specific non-returning "throw" routine.
 *   3 Fill an 'exception' out parameter and return a status code.
 *   4 Just return a status code.
 *
 * Things to add maybe:
 *   1 Way to throwaway values upon receipt, producing constants upon send.
 *     Not sure of the usefulness.
 *    (Or maybe check values against constants upon receipt.)
 */

/* Define what a cast referenc is. */
typedef int cast_ref;

/* These are kludges to get around the limitations of current C language
   mappings.  */
typedef struct pres_c_inline_u *pres_c_inline;
typedef struct pres_c_mapping_u *pres_c_mapping;


/***** Allocation *****/

/*
 * The following structures combine to form the pres_c_allocation structure.
 * At the top level, pres_c_allocation is a vector of pres_c_allocation_u, one
 * for each possible parameter direction (unknown, in, inout, out, and return).
 * A pres_c_allocation_u is a union describing whether the allocation is valid
 * (pres_c_allocation_case) or invalid (void).  A pres_c_allocation_case
 * contains a pres_c_alloc_flags, pres_c_allocator, and a cast_init for
 * initializing the allocated space, if necessary (for C++ only).  The
 * pres_c_alloc_flags come from the defined constants, and the pres_c_allocator
 * is a union of all allocator types and the information necessary for each
 * (usually just its name for now, if anything).
 */

typedef u_int pres_c_alloc_flags;

/* These are applied just before the RPC system "produces" some data
   into the memory pointed to by a C pointer.  */
const PRES_C_ALLOC_NEVER		= 0x00000000;
const PRES_C_ALLOC_IF_NULL		= 0x00000001;
const PRES_C_ALLOC_IF_TOO_SMALL		= 0x00000002;
const PRES_C_ALLOC_IF_TOO_LARGE		= 0x00000004;
const PRES_C_ALLOC_ALWAYS		= 0x00000008;
const PRES_C_ALLOC_EVER			= 0x0000000f;
const PRES_C_ALLOC_CLEAR		= 0x00000010;
const PRES_C_REALLOC_IF_TOO_SMALL	= 0x00000200;
const PRES_C_REALLOC_IF_TOO_LARGE	= 0x00000400;
const PRES_C_REALLOC_ALWAYS		= 0x00000800;
const PRES_C_REALLOC_EVER		= 0x00000f00;
const PRES_C_REALLOC_CLEAR		= 0x00001000;
const PRES_C_FAIL_IF_TOO_SMALL		= 0x00002000;

/* These are applied just after the RPC system has "consumed" some C data
   from the memory pointed to by the C pointer.  */
const PRES_C_DEALLOC_NEVER		= 0x00000000;
const PRES_C_DEALLOC_ALWAYS		= 0x00080000;
const PRES_C_DEALLOC_EVER		= 0x000f0000;
const PRES_C_DEALLOC_NULLIFY		= 0x00100000;
const PRES_C_DEALLOC_ON_FAIL		= 0x00200000;
const PRES_C_RUN_CTOR			= 0x01000000;
const PRES_C_RUN_DTOR			= 0x02000000;

/* PRES_C_DIRECTIONS is the number of enumerations in pres_c_direction. */
const PRES_C_DIRECTIONS = 5;

/* 
 * Allocator union
 *   Describes applicable allocator function.
 */
enum pres_c_allocator_kind
{
	/*
	 * DONTCARE indicates that it doesn't really matter what allocator
	 * is used.  This allows the back end to choose something optimal
	 * (e.g., stack allocation).
	 */
	PRES_C_ALLOCATOR_DONTCARE,
	
	/* Static allocation */
	PRES_C_ALLOCATOR_STATIC,
	
	/* Out-of-line allocation (with a named allocator) */
	PRES_C_ALLOCATOR_OUTOFLINE,
	
	/* Named allocator */
	PRES_C_ALLOCATOR_NAME
};

union pres_c_allocator
switch (pres_c_allocator_kind kind)
{
	case PRES_C_ALLOCATOR_DONTCARE:		void;
	case PRES_C_ALLOCATOR_STATIC:		void;
	case PRES_C_ALLOCATOR_OUTOFLINE:	string ool_name<>;
	case PRES_C_ALLOCATOR_NAME:		string name<>;
};

/*
 * Allocation case
 *   Indicates the allocation/deallocation flags and allocator to use.
 */
struct pres_c_allocation_case
{
	/* Flags specifying the allocation/deallocation behavior for a
	   pointer. */
	pres_c_alloc_flags	flags;

	/* Name of allocator to use when allocating or deallocating instances
	   of this ptype.
	   Could be made more flexible (e.g. refer to a function prototype),
	   but that's probably not needed.  */
	pres_c_allocator	allocator;
	
	/* C++ only: parameters to the constructor. */
	cast_init		alloc_init;
};

/*
 * Allocation allowance union
 *   Indicates if an allocation is allowed or invalid.
 */
enum pres_c_allocation_allow
{
	PRES_C_ALLOCATION_INVALID = 0,
	PRES_C_ALLOCATION_ALLOW = 1
};

union pres_c_allocation_u
switch (pres_c_allocation_allow allow)
{
	case PRES_C_ALLOCATION_ALLOW:	pres_c_allocation_case	val;
	case PRES_C_ALLOCATION_INVALID:	void;
};

/*
 * Top-level allocation structure
 *   A vector of allocation semantics -- one for each pres_c_direction.
 */
struct pres_c_allocation
{
	/* Each PRES_C_DIRECTION has its own allocation case */
	pres_c_allocation_u cases[PRES_C_DIRECTIONS];
};

/*
 * Allocation owners
 * This is a list of possible entities that can own allocated data buffers.
 * XXX - This may be a partial or partailly redundant list (or both).  This
 * is not currently used by the allocation context, although could be in the
 * future to profide more semantic information regarding alloc/dealloc.
 * This is given as a sample list, from which a final version may be derived.
 */
enum allocation_owner
{
	ALLOCATION_OWNER_FLICK,
	ALLOCATION_OWNER_USER,
	ALLOCATION_OWNER_CLIENT,
	ALLOCATION_OWNER_SERVER,
	ALLOCATION_OWNER_STUB,
	ALLOCATION_OWNER_DISPATCH,
	ALLOCATION_OWNER_WORKFUNC
};	


/***** Temporaries *****/

/*
 * *** These can represent both a PRES_C_INLINE and a PRES_C_MAPPING. ***
 *
 * A `pres_c_temporary' is used to resolve mismatches between data to be
 * encoded/decoded and variables available in the function.  For example, a C++
 * client might need to unmarshal an exceptional reply separately because there
 * is no environment variable or the variable doesn't have fields for message
 * data to be marshalled/unmarshalled.
 *
 * This can be either an inline node or a mapping node.  In the inline case, it
 * serves much the same purpose as an inline_atom, however nothing is selected
 * from the inline state; rather, a temporary variable is created.  In the case
 * of a mapping, a temporary is still created, but the incoming CAST expr/type
 * may be used to initialize the temporary.  In either case, the CAST expr/type
 * of the temporary is passed to the embedded mapping node.  The MINT type here
 * is only for the inline case; mappings inherit their original MINT type.
 */
enum pres_c_temporary_type
{
	TEMP_TYPE_PRESENTED,
	TEMP_TYPE_ENCODED
};
struct pres_c_temporary
{
	pres_c_mapping		map;	/* The mapping for the data */
	
	string			name<>;	/* A hint used in naming the temp */
	cast_type		ctype;	/* The type of the temporary */
	cast_expr		init;	/* Initialization expression */
	pres_c_temporary_type	type;	/* How this temporary is used */
	
	/*
	 * `is_const' is TRUE (non-zero) if `init' evaluates to some constant
	 * expression.  This is used to avoid creating a temporary variable for
	 * constant values.  The initialization expr is simply passed down
	 * instead of a temporary variable name.  If this flag is set, you are
	 * asserting that the resulting expr will NEVER be used as an lvalue.
	 */
	int		is_const;
	
	/*
	 * Handlers indicate to the back end what this temporary is used for so
	 * it can resolve the mismatch.  For example, it may simply be mapped
	 * to a macro that has parameters for the original variable and the
	 * temporary.  We define both pre- and post-handlers, so that different
	 * processing can be done both before encode and after decode.
	 */
	string		prehandler<>;	
	string		posthandler<>;	
};


/***** Inlines *****/

/*
 * C structure slot/function parameter index to which a particular inlined atom
 * is mapped.  Note that there is no restriction on how many times a slot is
 * referenced.  This allows us to ignore some slots, while using others
 * multiple times.
 */
typedef int pres_c_inline_index;

struct pres_c_inline_atom
{
	/* Which slot/parameter maps this itype.  */
	pres_c_inline_index	index;

	/* How that slot/parameter's ptype maps to this itype.  */
	pres_c_mapping		mapping;
};

struct pres_c_inline_struct_slot
{
	/*
	 * `mint_struct_slot_index' indicates which slot in the MINT structure
	 * corresponds to the `pres_c_inline' contained in this PRES_C node.
	 * If the value of this field is `mint_slot_index_null' (-1), then
	 * there is no underlying MINT.
	 *
	 * Previously, when `pres_c_inline_structs' were used to handle stub
	 * parameters, there was an implicit assocation between `slot[i]' in
	 * the `pres_c_inline_struct' and `slot[i]' in the MINT structure type.
	 * But this proved to be a problem: If we wanted to handle a parameter
	 * via PRES_C, we *had* to have an underlying MINT slot for the param.
	 *
	 * Why was this a problem?  Because some parameters, such as SIDs, are
	 * presentation-only notions, and should not be represented by anything
	 * in the MINT.  MINT should describe *only* IDL-specified interfaces
	 * (as represented by AOI), and should *not* need to be modified in
	 * order for a presentation generator to insert special parameters such
	 * as SIDs.
	 *
	 * So, previously, in order to add SID parameters to a stub signature,
	 * the presentation generator was forced to modify the underlying MINT
	 * structure and insert a slot, so that `mu_state::mu_inline_struct'
	 * could make the correct PRES_C node <--> MINT node associations.
	 *
	 * The addition of `mint_struct_slot_index' solves this problem.  We
	 * can now create arbitrary PRES_C slot <--> MINT slot associations,
	 * and create PRES_C nodes to handle stub parameters for which there is
	 * no underlying MINT.
	 */
	int			mint_struct_slot_index;
	pres_c_inline		inl;
};

/*
 * One might be tempted to do this:
 *     typedef pres_c_inline_struct_slot pres_c_inline_struct_slots<>;
 * until one remembers that the names of the structure slots would be:
 *     pres_c_inline_struct_slots_{len,val}
 * and you think about your poor fingers.  On the whole, it's easier to just
 * declare the sequence types inline in `pres_c_inline_*_struct', below.
 */

struct pres_c_inline_struct
{
	/*
	 * One for each slot in the interface-defined structure.
	 */
	pres_c_inline_struct_slot	slots<>;
};

/*
 * A `pres_c_inline_func_params_struct' connects a CAST function declaration
 * (the parameter list and return value) with a MINT_STRUCT representing the
 * set of parameter and return values contained within a request or reply
 * message.
 *
 * Previously, Flick just used a `pres_c_inline_struct' to connect the CAST and
 * MINT for a function signature.  But this made it difficult to distinguish
 * the return value from the other function parameters, and different message
 * formats may position a return value at different places in the message (i.e.
 * before or after the `out' and `inout' values).  So now, the PG creates a
 * `pres_c_inline_func_params_struct' instead, so that the return value can be
 * easily distinguished.
 */
struct pres_c_inline_func_params_struct
{
	/*
	 * One element for each parameter contained in this message.
	 */
	pres_c_inline_struct_slot	slots<>;
	
	/*
	 * And in replies, a slot for the return value as well.
	 */
	pres_c_inline_struct_slot	*return_slot;
};

/*
 * A `pres_c_inline_handler_func' is used to handle an incoming message in a
 * decomposed presentation style.  This node indicates that Flick's processing
 * of the message should stop here, and pass a nicely packaged message to the
 * client or server work function.
 */
struct pres_c_inline_handler_func
{
	/*
	 * One element for each parameter to the handler function.
	 * These are all ``special'' presentation-only parameters.
	 */
	pres_c_inline_struct_slot slots<>;
};

/* This is used for struct-unions and void*-unions.  */
struct pres_c_inline_struct_union_case
{
	/* `index' indicates which member of the union CAST type we should
	   use in this case.  In other words, it tells us which slot of the
	   CAST union we should make reference to.  Note that `index' is an
	   `int' and not a `pres_c_inline_index' because conceptually, we are
	   *mapping*, not *inlining*, the union case.  As a practical matter,
	   however, it doesn't really make a difference.
	   */
	int			index;
	
	/* `mapping' tells us how this case's ptype and itype are related. */
	pres_c_mapping		mapping;
};

struct pres_c_inline_struct_union
{
	pres_c_inline_atom			discrim;
	pres_c_inline_index			union_index;
	pres_c_inline_struct_union_case		cases<>;
	pres_c_inline_struct_union_case		*dfault;
};

struct pres_c_inline_void_union_case
{
	/* 
	 * `case_value' is the CAST literal value of the discriminator in this
	 * branch.  It is needed when the discriminated value is used in the
	 * presentation and not just when deciphering the message.  This is
	 * only used when the discriminator uses PRES_C_MAPPING_IGNORE.
	 * Otherwise, it is nil.
         */ 
	cast_expr case_value;

	/* The type is the cast_type to cast the void* to */
	cast_type type;
	pres_c_mapping mapping;
};

struct pres_c_inline_void_union
{
	pres_c_inline_atom			discrim;
	pres_c_inline_index			void_index;
	pres_c_inline_void_union_case		cases<>;
	pres_c_inline_void_union_case		*dfault;
};

struct pres_c_inline_expanded_union
{
	pres_c_inline_atom	discrim;
	pres_c_inline		cases<>;
	pres_c_inline		dfault;
};

/*
 * A collapsed union presentation is valid only for one specific, constant
 * union branch.  A union sent from this presentation always has a particular
 * discriminator; receiving a union into a presentation of this kind causes
 * unions with any other discriminator value to be rejected.  This is commonly
 * used to differentiate between different "remote procedures" that might be
 * called through a single message data type.  Thus, there is no restriction
 * that the MINT has only one union case; rather, this particular presentation
 * of the union has only one valid union case.
 */
struct pres_c_inline_collapsed_union
{
	mint_const		discrim_val;
	pres_c_inline		selected_case;
};

/*
 * A virtual union (I prefer arbitrary, but it's harder to type than virtual)
 * specifies a union that is essentially arbitrary.  It's sole reason for being
 * (at this writing) is to allow mapping of exceptions instead of the 'normal'
 * return result.  Currently, the discriminator is pitched after it has been
 * switched on, and it is an integer.  The back end should deal with it...
 */
typedef pres_c_inline pres_c_inline_virtual_union_case;

struct pres_c_inline_virtual_union
{
	string arglist_name<>; /* Name of associated arglist. */
	pres_c_inline_virtual_union_case discrim;
	pres_c_inline_virtual_union_case cases<>;
	pres_c_inline_virtual_union_case dfault;
};

struct pres_c_inline_typed
{
	pres_c_inline		tag;
	pres_c_inline		inl;
};

/*
 * The `pres_c_inline_allocation context' establishes context for pointer/array
 * allocation that will happen in the PRES_C subtree (``ptr''), by either a
 * pres_c_mapping_pointer or a pres_c_mapping_internal_array.
 */
struct pres_c_inline_allocation_context
{
	/* Allocation context: */
	string arglist_name<>;		/* Name of the context's arglist. */
	
	/*
	 * XXX - The arguments below marked with `XXX' are NOT IMPLEMENTED.
	 * There is minimal support in place for handling them, but they are
	 * not currently being used to support any particular allocation
	 * semantics.
	 */
/*XXX*/	pres_c_inline	  offset;	/* First element to m/u */
	pres_c_inline	  length;	/* # of elements to m/u */
	pres_c_inline	  min_len;	/* [hard] minimum (usu. IDL const) */
	pres_c_inline	  max_len;	/* [hard] maximum (usu. IDL const) */
	pres_c_inline	  alloc_len;	/* allocated # of elements */
	pres_c_inline	  min_alloc_len;/* minimum length of allocation */
/*XXX*/	pres_c_inline	  max_alloc_len;/* maximum length of allocation */
	pres_c_inline	  release;	/* release of struct-owned buffer */
	pres_c_inline	  terminator;	/* buffer termination */
	pres_c_inline	  mustcopy;	/* must copy data to keep it */
/*XXX*/	int		  overwrite;	/* `out' buffer is preallocated */
/*XXX*/	allocation_owner  owner;	/* entity ownership */

	pres_c_allocation alloc;	/* the allocation semantics */
	
	/*
	 * The real inline to be handled:
	 * `ptr' must eventually lead to a PRES_C_MAPPING_INTERNAL_ARRAY node,
	 * a PRES_C_MAPPING_POINTER node, or a PRES_C_MAPPING_OPTIONAL_POINTER
	 * node, which is responsible for actually instantiating the
	 * allocation.
	 */
	pres_c_inline	  ptr;
};

struct pres_c_inline_xlate
{
	/* the _real_ pres_c_inline... */
	pres_c_inline		sub;
	
	pres_c_inline_index	index;
	cast_type		internal_ctype;
	
	string			translator<>;
	string			destructor<>;
};

struct pres_c_inline_assign
{
	/* the _real_ pres_c_inline... */
	pres_c_inline		sub;

	pres_c_inline_index     index;
	cast_expr		value;
};

struct pres_c_inline_cond
{
	/* This index must refer to a boolean C variable
	   (or at least an integerish type that can be used as a boolean).
	   At runtime, if the value of that C variable(/parameter/slot)
	   is true, marshal using true_inl; otherwise use false_inl.  */
	pres_c_inline_index     index;
	pres_c_inline		true_inl;
	pres_c_inline		false_inl;
};

/*
 * A `pres_c_inline_message_attribute' tells us that the corresponding data
 * will access/change an attribute of the message being sent or received.
 * The difference between the inline and mapping versions of this is
 * that the inline has no corresponding CAST.  The inline node is used
 * simply to encode/decode a known message attribute into/from the message.
 * The mapping node is used to [un]marshal a parameter as a message attribute.
 * Currently this is only implemented for MIG.
 */
enum pres_c_message_attribute_kind
{
	/* sets some attribute flags */
	PRES_C_MESSAGE_ATTRIBUTE_FLAGS = 0,

	/* how long to wait for reply */
	PRES_C_MESSAGE_ATTRIBUTE_TIMEOUT = 1,

	/* index of when received */
	PRES_C_MESSAGE_ATTRIBUTE_SEQUENCE_RECEIVED = 2,

	/* client reference */
	PRES_C_MESSAGE_ATTRIBUTE_CLIENT_REFERENCE = 3,

	/* flag indicating work function should make its own copy of
	   the data if it wants to keep it */
	PRES_C_MESSAGE_ATTRIBUTE_SERVERCOPY = 4

	/* maybe in the future, SID will be moved here */
};

struct pres_c_inline_message_attribute
{
	pres_c_message_attribute_kind	kind;
};

enum pres_c_inline_kind
{
	/* Non-inlined instance of an arbitrary ptype.  */
	PRES_C_INLINE_ATOM			= 1,

	/* Inlined struct.  Corresponding itype must be MINT_STRUCT.  */
	PRES_C_INLINE_STRUCT			= 2,
	PRES_C_INLINE_FUNC_PARAMS_STRUCT	= 3,
	PRES_C_INLINE_HANDLER_FUNC		= 4,
	
	/* Union presentations.  Corresponding itype must be MINT_UNION.  */
	PRES_C_INLINE_STRUCT_UNION		= 5,
	PRES_C_INLINE_EXPANDED_UNION		= 6,
	PRES_C_INLINE_VOID_UNION		= 7,
	PRES_C_INLINE_COLLAPSED_UNION		= 8,

	/* Typed presentation.  Corresponding itype must be MINT_TYPED.  */
	PRES_C_INLINE_TYPED			= 9,

	/* Only valid for receive-only mappings: throw away the
	   received value. */
	PRES_C_INLINE_THROWAWAY			= 10,

	PRES_C_INLINE_XLATE			= 11,
	PRES_C_INLINE_ASSIGN			= 12,
	PRES_C_INLINE_COND			= 13,
	PRES_C_INLINE_VIRTUAL_UNION		= 14,
	PRES_C_INLINE_MESSAGE_ATTRIBUTE		= 15,
	PRES_C_INLINE_ALLOCATION_CONTEXT	= 16,
	PRES_C_INLINE_TEMPORARY			= 17,
	
	/* Illegal inline (i.e., a union slot that must exist, but is not
	   a valid union case). */
	PRES_C_INLINE_ILLEGAL			= 18
};

/* Defines the inlined mapping of a particular interface type
   onto the members of a C struct.  */
union pres_c_inline_u
switch (pres_c_inline_kind kind)
{
	case PRES_C_INLINE_ATOM:
		pres_c_inline_atom			atom;

	case PRES_C_INLINE_STRUCT:
		pres_c_inline_struct			struct_i;
	case PRES_C_INLINE_FUNC_PARAMS_STRUCT:
		pres_c_inline_func_params_struct	func_params_i;
	case PRES_C_INLINE_HANDLER_FUNC:
		pres_c_inline_handler_func		handler_i;

	case PRES_C_INLINE_STRUCT_UNION:
		pres_c_inline_struct_union		struct_union;
	case PRES_C_INLINE_EXPANDED_UNION:
		pres_c_inline_expanded_union		expanded_union;
	case PRES_C_INLINE_VOID_UNION:
		pres_c_inline_void_union		void_union;
	case PRES_C_INLINE_COLLAPSED_UNION:
		pres_c_inline_collapsed_union		collapsed_union;
	case PRES_C_INLINE_VIRTUAL_UNION:
		pres_c_inline_virtual_union		virtual_union;

	case PRES_C_INLINE_TYPED:
		pres_c_inline_typed			typed;

	case PRES_C_INLINE_THROWAWAY:
		void;

	case PRES_C_INLINE_XLATE:
		pres_c_inline_xlate			xlate;
	case PRES_C_INLINE_ASSIGN:
		pres_c_inline_assign			assign;
	case PRES_C_INLINE_COND:
		pres_c_inline_cond			cond;
	case PRES_C_INLINE_MESSAGE_ATTRIBUTE:
		pres_c_inline_message_attribute		msg_attr;
	case PRES_C_INLINE_ALLOCATION_CONTEXT:
		pres_c_inline_allocation_context	acontext;
	case PRES_C_INLINE_TEMPORARY:
		pres_c_temporary			temp;

	case PRES_C_INLINE_ILLEGAL:
		void;
};


/***** Mappings *****/

/* These define how a particular ptype is mapped to a particular itype.  */

/*
 * For PRES_C_MAPPING_DIRECT, the itype and ptype must be primitive types that
 * have an "obvious" mapping.
 */

/*
 * For PRES_C_MAPPING_STUB, the ptype must be a CAST_TYPE_NAME; The mapping_
 * stub_index value is an index into the stubs<> sequence field in a pres_c_1
 * type.  It is for the index to the corresponding marshal/unmarshal stub.
 */
struct pres_c_mapping_stub
{
	int	mapping_stub_index;
};

/*
 * A `pres_c_mapping_direction' node contains information for the back end: is
 * the data below this node `in', `inout', `out', `return', or unknown?  The
 * back end sometimes needs this information in order to optimize allocation,
 * etc.  (E.g., `in' parameters can be allocated on the runtime stack.)
 *
 * Fundamentally, the need for `direction' nodes points to a shortcoming in
 * the presentation generator: the PG phase is driven almost exclusively by
 * types, not by how the data is being used.  The role of the data, however,
 * determines certain things such as allocation semantics.  If the presentation
 * generator was more careful to consider the roles of data when generating
 * PRES_C, it could generate PRES_C without resorting to `direction' nodes
 * which need to be interpreted by the back end.
 *
 * NOTE: These numbers should be similar to those for the aoi_direction enum.
 */
enum pres_c_direction
{
	PRES_C_DIRECTION_UNKNOWN = 0,
	PRES_C_DIRECTION_IN = 1,
	PRES_C_DIRECTION_OUT = 2,
	PRES_C_DIRECTION_INOUT = 3,
	PRES_C_DIRECTION_RETURN = 4
};

struct pres_c_mapping_direction
{
	pres_c_direction dir;
	pres_c_mapping   mapping;
};

/* One of these serves to "eat up" one level of C pointer indirection in the
   ptype, without descending any further in the itype.  */
struct pres_c_mapping_pointer
{
	/* Name of associated allocation context's arglist. */
	string		arglist_name<>;
	/* Mapping for the itype and the ptype the pointer points to.  */
	pres_c_mapping	target;
};

struct pres_c_mapping_var_reference
{
	pres_c_mapping		target;
};

/*
 * A `pres_c_mapping_optional_pointer' is unlike a `pres_c_mapping_pointer' in
 * that an optional pointer doesn't just "eat up" one level of C pointer
 * indirection.  An optional pointer is a presentation mechanism that embodies
 * semantics --- the value of the pointer (NULL or non-NULL) is determined by
 * the presence of the optional datum.  A simple `pres_c_mapping_pointer', on
 * the other hand, doesn't have any semantics to speak of --- it's simply a
 * way of presenting data.
 */
struct pres_c_mapping_optional_pointer
{
	/* Name of associated allocation context's arglist. */
	string		arglist_name<>;
	/* Mapping for the itype and the ptype the pointer points to.  */
	pres_c_mapping	target;
};

/*
 * A `pres_c_mapping_struct' says that the current MINT type, whatever it is
 * (not necessarily a struct), maps onto the elements of the current struct
 * CAST type, as defined by the `pres_c_inline' tree that begins here.
 */
typedef pres_c_inline pres_c_mapping_struct;

/*
 * A `pres_c_mapping_internal_array' is associated with the data buffer that is
 * part of an array.  There must be a parent `pres_c_inline_allocation_context'
 * node that has set up the appropriate context, e.g. length, maximum, etc.
 *
 * A `pres_c_mapping_internal_array' is similar to a `pres_c_mapping_pointer':
 * each must have an allocation context for a parent, each must be paired with
 * CAST pointer (or array type for a mapping array), and each contains a
 * target/element mapping.  The difference comes in how the node types are
 * processed.  A `pointer' mapping describes a pointer-to-single-element.  An
 * `internal_array', however, describes an address-to-array.  Also, here the
 * associated MINT type must be an array, whereas a `pointer' mapping does not
 * descend on the MINT.
 *
 * Also note that, although the MINT type must be an array, it need not be
 * identical to the given presentation established by the parent allocation
 * context node.  The MINT may describe a variable array while the presentation
 * might be that of a fixed array, or vice versa.
 */
struct pres_c_mapping_internal_array
{
	/* Name of associated allocation context's arglist. */
	string		arglist_name<>;
	pres_c_mapping	element_mapping;
};

/*
 * The current ptype presents a union itype all by itself.  The first case must
 * be an integerish variable containing the discriminator, and all the other
 * cases must be structs with that variable in the first slot.  The default
 * case, if any, must be last in the C union.
 */
struct pres_c_mapping_flat_union
{
	pres_c_mapping	discrim;
	pres_c_mapping	cases<>;
	pres_c_mapping	dfault;
};

struct pres_c_mapping_special
{
	string marshaler_name<>;
};

struct pres_c_mapping_xlate
{
	cast_type		internal_ctype;
	pres_c_mapping		internal_mapping;
	
	string			translator<>;
	string			destructor<>;
};

enum pres_c_reference_kind
{
	PRES_C_REFERENCE_COPY	          = 0,	/* send a copy      */
	PRES_C_REFERENCE_MOVE	          = 1,	/* send my instance */
	PRES_C_REFERENCE_COPY_AND_CONVERT = 2	/* send a converted copy.
				                 * This corresponds to 
						 * MIG's MAKE, since the OS
						 * converts a RECV right
						 * to a SEND right
						 */
};

/* This is the type of mapping typically used for object references.  */
struct pres_c_mapping_reference
{
	/* 
	 * `kind' refers to the semantic: either copy, move, or MIG's
	 * copy_and_convert (make).  `ref_count' is how many references we
	 * are working with (currently it's always 1).
	 */		
 	pres_c_reference_kind kind; 
	int ref_count;
	string arglist_name<>;
};

/*
 * A `pres_c_mapping_sid' tells us that the corresponding data is a security
 * identifier (SID).  Like object references, the implementation of a SID is
 * inherently tied to the services used to implement communication.
 */
enum pres_c_sid_kind
{
	PRES_C_SID_CLIENT	= 0,	/* effective SID of the client */
	PRES_C_SID_SERVER	= 1	/* required SID of the server  */
};

struct pres_c_mapping_sid
{
	pres_c_sid_kind kind;
};


/*
 * A `pres_c_mapping_message_attribute' tells us that the corresponding data
 * will access/change an attribute of the message being sent or received.
 * See comment for `pres_c_inline_message_attribute'.
 * Currently this is only implemented for MIG.
 */
struct pres_c_mapping_message_attribute
{
	pres_c_message_attribute_kind kind;
};

/*
 * A `pres_c_mapping_argument' tells the back end to save the information about
 * the current CAST storage location (a `cast_expr' that names the location,
 * and a `cast_type' describing its type).  This is how certain PRES_C nodes
 * collect information from their children.
 *
 * `arg_name' is the ``key'' under which the CAST information is stored (in the
 * list named by `arglist_name').  This key generally described the argument's
 * purpose, e.g., "length".
 *
 * `map' is the internal mapping for this node; it allows the back end to
 * continue generating code after the corresponding CAST information has been
 * saved off.
 */
struct pres_c_mapping_argument
{
	string		arglist_name<>;	/* Name of the arglist itself. */
	string		arg_name<>;	/* Name of argument in the list. */
	pres_c_mapping	map;
};

/*
 * A `pres_c_mapping_initialize' tells the back end to initialize the current
 * CAST-described storage location with a specified `cast_expr'.
 */
struct pres_c_mapping_initialize
{
	cast_expr	value;
};

/*
 * A `pres_c_mapping_selector' selects a member from a CAST entity without
 * assuming anything about corresponding MINT.  This is a way to get a
 * ``presented struct'', i.e. where you can map `foo.bar' instead of just
 * `foo'.  This is particularly useful for handling the `CORBA_Exception'
 * environment type, directly (un)marshaling the `_ev->_major' discriminator.
 *
 * NOTE: This may look a lot like a PRES_C_INLINE_ATOM, but has a much
 * different semantic meaning.  It selects a member from a CAST scope instead
 * of an inline state.
 */
struct pres_c_mapping_selector
{
	int		index;
	pres_c_mapping	mapping;
};

/*
 * A `pres_c_mapping_param_root' node indicates that the current CAST
 * expression and CAST type describe a formal parameter to a client stub or
 * server work function: i.e., the name and type of a parameter in the function
 * type signature.
 *
 * On the server side, the task of a `pres_c_mapping_param_root' node is to
 * determine the type and name of the actual parameter that will be passed to
 * the server work function.  Typically, this means declaring a local variable
 * within the server dispatch function, and then using that variable in the
 * invocation of the work function.  In other cases, the actual parameter can
 * be handled differently, e.g., by constructing a local C++ object, or by
 * passing a literal expression (`0') as the actual parameter.
 *
 * On the client side, the task of a `pres_c_mapping_param_root' node is more
 * modest.  The `param_root' node may specify an internal type transformation
 * (see below), but otherwise, the `param_root' node has little effect.
 */
struct pres_c_mapping_param_root
{
	/*
	 * `ctype' is an optional CAST type (i.e., it may be null).
	 * `init' is an optional CAST initializer (i.e., it may be null).
	 *
	 * When generating a server dispatch function, `ctype' is the type of
	 * the actual parameter that will be given to the server work function.
	 * If `ctype' is null, the actual type will be the same as the formal
	 * type.  In either case, we may transform the actual type further,
	 * e.g., by changing array types into pointer-to-element types, or by
	 * mucking with the type qualifiers (removing `const'), or by diddling
	 * with C++-style references as necessary.
	 *
	 * When generating a client stub, `ctype' is the type through which we
	 * access the formal parameter.  Again, if `ctype' is null, we simply
	 * rely on the formal parameter type.
	 *
	 * In either case, `init' is used to initialize the variable (e.g. call
	 * a particular constructor).
	 */
	cast_type		ctype;
	cast_init		init;
	
	/*
	 * The mapping for marshaling/unmarshaling the parameter data.
	 */
	pres_c_mapping		map;
};

/*
 * A `pres_c_mapping_singleton' returns the mapping to an inline state (the
 * mapping's CAST expr/type becomes the only slot of the new inline state).
 * There is no descent on either CAST or MINT.
 */
struct pres_c_mapping_singleton
{
	pres_c_inline inl;
};

enum pres_c_mapping_kind
{
	PRES_C_MAPPING_DIRECT			= 0,
	PRES_C_MAPPING_STUB			= 1,
	PRES_C_MAPPING_POINTER			= 2,
	PRES_C_MAPPING_INTERNAL_ARRAY		= 3,
	PRES_C_MAPPING_STRUCT			= 4,
	PRES_C_MAPPING_FLAT_UNION		= 5,
	PRES_C_MAPPING_SPECIAL			= 6,
	PRES_C_MAPPING_XLATE			= 7,
	PRES_C_MAPPING_REFERENCE		= 8,
	PRES_C_MAPPING_TYPE_TAG			= 9,
	PRES_C_MAPPING_TYPED			= 10,
	PRES_C_MAPPING_OPTIONAL_POINTER		= 11,
	/*
	 * `PRES_C_MAPPING_IGNORE' is for tossing stuff on the CAST side (i.e.,
	 * no corresponding MINT stuff to decode).
	 */
	PRES_C_MAPPING_IGNORE			= 12,
	PRES_C_MAPPING_SYSTEM_EXCEPTION		= 13,
	PRES_C_MAPPING_DIRECTION		= 14,
	PRES_C_MAPPING_SID			= 15,
	PRES_C_MAPPING_ARGUMENT			= 16,
	PRES_C_MAPPING_MESSAGE_ATTRIBUTE	= 17,
	PRES_C_MAPPING_INITIALIZE		= 18,
	PRES_C_MAPPING_ILLEGAL			= 19,
	PRES_C_MAPPING_VAR_REFERENCE		= 20,
	PRES_C_MAPPING_PARAM_ROOT		= 21,
	/*
	 * `PRES_C_MAPPING_ELSEWHERE' is for asserting that any associated
	 * CAST and MINT are handled elsewhere, or by some other means than
	 * a regular mapping.
	 */
	PRES_C_MAPPING_ELSEWHERE		= 22,
	PRES_C_MAPPING_SELECTOR			= 23,
	PRES_C_MAPPING_TEMPORARY		= 24,
	PRES_C_MAPPING_SINGLETON		= 25
};

union pres_c_mapping_u
switch (pres_c_mapping_kind kind)
{
	case PRES_C_MAPPING_DIRECT:
		void;
	case PRES_C_MAPPING_STUB:
		pres_c_mapping_stub			mapping_stub;
	case PRES_C_MAPPING_POINTER:
		pres_c_mapping_pointer			pointer;
	case PRES_C_MAPPING_INTERNAL_ARRAY:
		pres_c_mapping_internal_array		internal_array;
	case PRES_C_MAPPING_STRUCT:
		pres_c_mapping_struct			struct_i;
	case PRES_C_MAPPING_FLAT_UNION:
		pres_c_mapping_flat_union		flat_union;
	case PRES_C_MAPPING_SPECIAL:
		pres_c_mapping_special			special;
	case PRES_C_MAPPING_XLATE:
		pres_c_mapping_xlate			xlate;
	case PRES_C_MAPPING_REFERENCE:
		pres_c_mapping_reference		ref;
	case PRES_C_MAPPING_TYPE_TAG:
		void;
	case PRES_C_MAPPING_TYPED:
		void;
	case PRES_C_MAPPING_OPTIONAL_POINTER:
		pres_c_mapping_optional_pointer		optional_pointer;
	case PRES_C_MAPPING_IGNORE:
		void;
	case PRES_C_MAPPING_SYSTEM_EXCEPTION:
		void;
	case PRES_C_MAPPING_DIRECTION:
		pres_c_mapping_direction		direction;
	case PRES_C_MAPPING_SID:
		pres_c_mapping_sid			sid;
	case PRES_C_MAPPING_ARGUMENT:
		pres_c_mapping_argument			argument;
	case PRES_C_MAPPING_MESSAGE_ATTRIBUTE:
		pres_c_mapping_message_attribute	message_attribute;
	case PRES_C_MAPPING_INITIALIZE:
		pres_c_mapping_initialize		initialize;
	case PRES_C_MAPPING_ILLEGAL:
		void;
	case PRES_C_MAPPING_VAR_REFERENCE:	
		pres_c_mapping_var_reference		var_ref;
	case PRES_C_MAPPING_PARAM_ROOT:
		pres_c_mapping_param_root		param_root;
	case PRES_C_MAPPING_ELSEWHERE:
		void;
	case PRES_C_MAPPING_SELECTOR:
		pres_c_mapping_selector			selector;
	case PRES_C_MAPPING_TEMPORARY:
		pres_c_temporary			temp;
	case PRES_C_MAPPING_SINGLETON:
		pres_c_mapping_singleton		singleton;
};


/***** Stubs *****/

/* Stub functions are treated much like structs for mapping purposes.
   Index numbers >= 0 indicate parameters;
   index -1 indicates the return value.  */
const pres_c_func_return_index = -1;

struct pres_c_marshal_stub
{
	/* This is an index into the cast's top-level scope,
	   indicating which C function declaration we're talking about.  */
	cast_ref	c_func;

	/* Interface data type this stub is supposed to marshal.  */
	mint_ref	itype;

	/* How the parameters of the stub map to the interface data.  */
	pres_c_inline	i;

	/* How the connection to marshal to is specified.  */
	pres_c_inline	target_i;

	/* The PRES_C mapping that allows us to "see through" or "see into"
	   this stub.  We need this maping when we want to inline the body of
	   this marshal stub into a client stub, a server function, etc. */
	pres_c_mapping seethru_map;
};

typedef u_int pres_c_stub_op_flags;
/*
 * These are flag bits that may be and'ed and or'ed together.
 */
const PRES_C_STUB_OP_FLAG_NONE		= 0x00000000;
const PRES_C_STUB_OP_FLAG_ONEWAY	= 0x00000001;
const PRES_C_STUB_OP_FLAG_IDEMPOTENT	= 0x00000002;

struct pres_c_client_stub
{
	/*
	 * The function arguments are the data sources, and the return value is
	 * the only data sink.  Memory can be allocated from the stack for data
	 * that only has to stick around until the reply is received; this
	 * generally means message buffers and such, but not returned data.
	 */
	
	/*
	 * This is an index into the cast's top-level scope, indicating which C
	 * function declaration we're talking about.
	 */
	cast_ref	c_func;
	
        /*
	 * op_flags is used to indicate if a function is oneway.  If so, there
	 * are no inout parameters, out parameters, or return value, and no
	 * Reply is received.
	 */
	pres_c_stub_op_flags 	op_flags;
	
	/*
	 * Interface data types for request and reply messages.  Normally these
	 * point to the "mom_request" and "mom_reply" union types,
	 * respectively.
	 */
	mint_ref	request_itype;
	mint_ref	reply_itype;
	
	/* How the parameters of the stub map to request and reply data.  */
	pres_c_inline	request_i;
	pres_c_inline	reply_i;
	
	/* How the target object to call is specified.  */
	mint_ref	target_itype;
	pres_c_inline	target_i;
	
	/* How the client "object" is specified.  */
	mint_ref	client_itype;
	pres_c_inline	client_i;
	
	/* How errors will be reported to the caller.  */
	mint_ref	error_itype;
	pres_c_inline	error_i;
};

struct pres_c_server_func
{
	/*
	 * For each server function that can be dispatched to, the function
	 * arguments are the data sinks, and the return value is the only data
	 * source.  Memory can be allocated from the stack for data that only
	 * has to stick around until the reply is sent; this generally means
	 * incoming and outgoing unpacked data, but not message buffers.
	 * (XXX - maybe message buffers, in some cases?)
	 */
	
	/*
	 * This is an index into the cast's top-level scope, indicating the C
	 * function declaration for the server function.
	 */
	cast_ref	c_func;
	
         /*
	  * op_flags is used to indicate if a function is oneway.  If so, there
	  * are no inout parameters, out parameters, or return value, and no
	  * Reply is sent.
	  */
	pres_c_stub_op_flags 	op_flags;
	
	/*
	 * How the parameters of the stub map to request and reply data.
	 * reply_i corresponds to the the "expected" reply message type; the
	 * server can return a reply that doesn't conform to the constraints of
	 * reply_i by throwing an exception.
	 */
	pres_c_inline	request_i;
	pres_c_inline	reply_i;
	
	/* How the target object the request was directed to is indicated to
           the server.  */
	mint_ref	target_itype;
	pres_c_inline	target_i;
	
	/* How the client "object" is indicated.  */
	mint_ref	client_itype;
	pres_c_inline	client_i;
	
	/* How the work function is expected to report errors.  */
	mint_ref	error_itype;
	pres_c_inline	error_i;
};

/*
 * Similar to `pres_c_server_func', `pres_c_receive_func' is a work
 * function that can be dispatched from the server skeleton.  However, 
 * this function represents a `decomposed' presentation of the work
 * function: it receives the pickled (encoded) message, rather than
 * each individual (decoded) parameter.  Further, the work function is 
 * responsible for issuing a reply if one is necessary.
 */
struct pres_c_receive_func
{
	/*
	 * This is an index into the cast's top-level scope, indicating the C
	 * function declaration for the server function.
	 */
	cast_ref	c_func;
	
        /* op_flags is used to indicate if a function is oneway. */
	pres_c_stub_op_flags 	op_flags;
	
	/* How the parameters of the function map to incoming message data. */
	mint_ref	simple_msg_itype;
	pres_c_inline	msg_i;
	
	/*
	 * How the target object the request was directed to is indicated to
	 * the server.
	 */
	mint_ref	target_itype;
	pres_c_inline	target_i;
	
	/* How the client "object" is indicated.  */
	mint_ref	client_itype;
	pres_c_inline	client_i;
	
	/* How the work function is expected to report errors.  */
	mint_ref	error_itype;
	pres_c_inline	error_i;
};

enum pres_c_func_kind
{
	/* A standard RPC server work function */
	PRES_C_SERVER_FUNC	= 1,
	
	/*
	 * Like PRES_C_SERVER_FUNC, but does not produce a reply after the
	 * function returns.  Instead, (if the interface specifies that a reply
	 * is in order) the work function must call (or arrange to later call)
	 * a SEND_STUB to send the reply back to the client.
	 */
	PRES_C_RECEIVE_FUNC	= 2
};

union pres_c_func
switch (pres_c_func_kind kind)
{
	case PRES_C_SERVER_FUNC:	pres_c_server_func	sfunc;
	case PRES_C_RECEIVE_FUNC:	pres_c_receive_func	rfunc;
};

/*
 * A "server stub" is really a static data structure produced by a MOM PBE that
 * represents a server implementation that can receive MOM object invocations.
 *
 * The local C code can call `mob_create' or an equivalent function
 * (e.g. `BOA_create' in a CORBA Basic-Object-Adaptor environment) with the
 * server stub as the ImplementationDef parameter; that will allow the server
 * to start receiving object invocations.
 */
struct pres_c_skel
{
	/*
	 * This is an index into the cast's top-level scope, indicating which C
	 * declaration we're talking about.
	 */
	cast_ref	c_def;
	
	/*
	 * Interface data types for request and reply messages.  Normally these
	 * point to the "mom_request" and "mom_reply" union types,
	 * respectively.
	 */
	mint_ref	request_itype;
	mint_ref	reply_itype;
	
	/*
	 * List of C functions the server stub can dispatch to.  If multiple
	 * functions match, the most specialized one will be chosen.
	 */
	pres_c_func	funcs<>;
};

struct pres_c_msg_stub
{
	/*
	 * The function arguments are the data sources, and the return value is
	 * the only data sink.  Memory can be allocated from the stack for data
	 * that only has to stick around until the reply is received; this
	 * generally means message buffers and such, but not returned data.
	 */
	
	/*
	 * This is an index into the cast's top-level scope, indicating which C
	 * function declaration we're talking about.
	 */
	cast_ref	c_func;
	
	/*
	 * Interface data types for request and reply messages.  Normally these
	 * point to the "mom_request" and "mom_reply" union types,
	 * respectively.
	 */
	mint_ref	msg_itype;
	
	/* How the parameters of the stub map to request and reply data.  */
	pres_c_inline	msg_i;
	
	/* How the target object to call is specified.  */
	mint_ref	target_itype;
	pres_c_inline	target_i;
	
	/* Flag to indicate request or reply */
	int request;
};

struct pres_c_msg_marshal_stub
{
	/*
	 * This is an index into the cast's top-level scope, indicating which C
	 * function declaration we're talking about.
	 */
	cast_ref	c_func;
	
        /*
	 * op_flags is used to indicate if a function is oneway.  If so, there
	 * are no inout parameters, out parameters, or return value, and no
	 * Reply is received.
	 */
	pres_c_stub_op_flags 	op_flags;
	
	/* Interface data type this stub is supposed to marshal.  */
	mint_ref	itype;
	
	/* How the parameters of the stub map to the interface data.  */
	pres_c_inline	i;
	
	/* Flag to indicate client or server */
	int client;
	
	/* Flag to indicate request or reply */
	int request;
	
	/* Name of the reply handler function */
	string reply_handler_name<>;
};

struct pres_c_continue_stub
{
	/*
	 * This is an index into the cast's top-level scope, indicating which C
	 * function declaration we're talking about.
	 */
	cast_ref	c_func;
	
	/* Interface data type this stub is supposed to marshal.  */
	mint_ref	itype;
	
	/* How the parameters of the stub map to the interface data.  */
	pres_c_inline	i;
	
	/* Flag to indicate request or reply */
	int request;
};

enum pres_c_stub_kind
{
	/* A stub that marshals a data structure.  */
	PRES_C_MARSHAL_STUB	= 1,
	
	/* Same, but unmarshals a data structure instead.  */
	PRES_C_UNMARSHAL_STUB	= 2,
	
	/*
	 * This function represents a client stub that packs a message, sends
	 * it off, waits for a response, and unpacks the response.
	 */
	PRES_C_CLIENT_STUB	= 3,
	
	/*
	 * These represent client and server stubs that wait for a
	 * message, unpack it, dispatch to a work function, and possibly
	 * package up a reply and send it off.
	 */
	PRES_C_CLIENT_SKEL	= 5,
	PRES_C_SERVER_SKEL	= 6,
	
	/*
	 * Continuation stub.  This allows a client stub or server
	 * function to continue its processing at a later time.
	 */
	PRES_C_CONTINUE_STUB	= 8,
	
	/* A stub that marshals a complete message.  */
	PRES_C_MESSAGE_MARSHAL_STUB	= 10,
	
	/* Same, but unmarshals a complete message instead.  */
	PRES_C_MESSAGE_UNMARSHAL_STUB	= 11,
	
	/*
	 * These represent stubs that pack or unpack one-way messages and send
	 * them off or receive them, possibly asynchronously.
	 */
	PRES_C_SEND_STUB	= 15,
	PRES_C_RECV_STUB	= 16
	
	/* XXX exception handler functions, e.g. for authentication?  */
};

union pres_c_stub
switch (pres_c_stub_kind kind)
{
	case PRES_C_MARSHAL_STUB:	pres_c_marshal_stub	mstub;
	case PRES_C_UNMARSHAL_STUB:	pres_c_marshal_stub	ustub;
	case PRES_C_CLIENT_STUB:	pres_c_client_stub	cstub;
	case PRES_C_CLIENT_SKEL:	pres_c_skel		cskel;
	case PRES_C_SERVER_SKEL:	pres_c_skel		sskel;
	case PRES_C_SEND_STUB:		pres_c_msg_stub		send_stub;
	case PRES_C_RECV_STUB:		pres_c_msg_stub		recv_stub;
	case PRES_C_MESSAGE_MARSHAL_STUB:   pres_c_msg_marshal_stub	mmstub;
	case PRES_C_MESSAGE_UNMARSHAL_STUB: pres_c_msg_marshal_stub	mustub;
	case PRES_C_CONTINUE_STUB:	pres_c_continue_stub	continue_stub;
};


/***** Tags *****/

/* Tags are used for storing generic data which
   have some properties specified by the tag */
enum tag_data_kind
{
	TAG_NONE			= 0x0001,
	TAG_ANY				= 0x0002,
	TAG_TAG_LIST			= 0x0003,
	TAG_BOOL			= 0x0004,
	TAG_STRING			= 0x0005,
	TAG_INTEGER			= 0x0006,
	TAG_FLOAT			= 0x0007,
	TAG_REF				= 0x0008,
	TAG_OBJECT			= 0x0009,
	TAG_CAST_SCOPED_NAME		= 0x000a,
	TAG_CAST_DEF			= 0x000b,
	TAG_CAST_TYPE			= 0x000c,
	TAG_CAST_EXPR			= 0x000d,
	TAG_CAST_STMT			= 0x000e,
	TAG_CAST_INIT			= 0x000f,
	
	TAG_ANY_ARRAY			= 0x8002,
	TAG_TAG_LIST_ARRAY		= 0x8003,
	TAG_BOOL_ARRAY			= 0x8004,
	TAG_STRING_ARRAY		= 0x8005,
	TAG_INTEGER_ARRAY		= 0x8006,
	TAG_FLOAT_ARRAY			= 0x8007,
	TAG_REF_ARRAY			= 0x8008,
	TAG_OBJECT_ARRAY		= 0x8009,
	TAG_CAST_SCOPED_NAME_ARRAY	= 0x800a,
	TAG_CAST_DEF_ARRAY		= 0x800b,
	TAG_CAST_TYPE_ARRAY		= 0x800c,
	TAG_CAST_EXPR_ARRAY		= 0x800d,
	TAG_CAST_STMT_ARRAY		= 0x800e,
	TAG_CAST_INIT_ARRAY		= 0x800f
};
const TAG_ARRAY				= 0x8000;

typedef struct tag_list *tag_list_ptr;
typedef string tag_string_array<>;
typedef char   tag_object_array<>;

union tag_data
switch (tag_data_kind kind)
{
	case TAG_NONE:				void;
	case TAG_ANY:				void;
	case TAG_TAG_LIST:			tag_list_ptr		tl;
	case TAG_BOOL:				char			b;
	case TAG_STRING:			string			str<>;
	case TAG_INTEGER:			int			i;
	case TAG_FLOAT:				float			f;
	case TAG_REF:				string			ref<>;
	case TAG_OBJECT:			tag_object_array	obj;
	case TAG_CAST_SCOPED_NAME:		cast_scoped_name	scname;
	case TAG_CAST_DEF:			cast_def_t		cdef;
	case TAG_CAST_TYPE:			cast_type		ctype;
	case TAG_CAST_EXPR:			cast_expr		cexpr;
	case TAG_CAST_STMT:			cast_stmt		cstmt;
	case TAG_CAST_INIT:			cast_init		cinit;
	
	case TAG_TAG_LIST_ARRAY:
		tag_list_ptr		tl_a<>;
	case TAG_BOOL_ARRAY:
		char			b_a<>;
	case TAG_STRING_ARRAY:
		tag_string_array	str_a<>;
	case TAG_INTEGER_ARRAY:
		int			i_a<>;
	case TAG_FLOAT_ARRAY:
		float			f_a<>;
	case TAG_REF_ARRAY:
		tag_string_array	ref_a<>;
	case TAG_OBJECT_ARRAY:
		tag_object_array	obj_a<>;
	case TAG_CAST_SCOPED_NAME_ARRAY:
		cast_scoped_name	scname_a<>;
	case TAG_CAST_DEF_ARRAY:
		cast_def_t		cdef_a<>;
	case TAG_CAST_TYPE_ARRAY:	
		cast_type		ctype_a<>;
	case TAG_CAST_EXPR_ARRAY:
		cast_expr		cexpr_a<>;
	case TAG_CAST_STMT_ARRAY:
		cast_stmt		cstmt_a<>;
	case TAG_CAST_INIT_ARRAY:
		cast_init		cinit_a<>;
};

struct tag_item
{
	string		tag<>;
	tag_data	data;
};

struct tag_list
{
	struct tag_list	*parent;
	tag_item	items<>;
};


/***** The PRES_C structure *****/

struct pres_c_1
{
	mint_1		mint;
	cast_1		cast;
	cast_1		stubs_cast;
	cast_1		pres_cast;
	/*
	 * XXX --- `a' is here for the sole benefit of the Sun BE, so that it
	 * can generate program and version numbers for non-ONC RPC IDL derived
	 * interfaces.  See `c/pbe/sun/misc.cc' and `c/pbe/sun/server_main.cc'.
	 *
	 * The proper solution is to change MINT so that the BE has enough
	 * context from which to generate the interface/object keys that are
	 * required by the message encoding format.  But until we change MINT,
	 * we have AOI...
	 */
	aoi		a;
	pres_c_stub	stubs<>;
	string		pres_context<>;
	cast_expr	error_mappings<>;
	data_channel_mask	unpresented_channels<>;
	tag_list	*pres_attrs;
	meta		meta_data;
	int		cast_language;
};

#ifdef RPC_HDR
%#endif /* _flick_pres_c_h_ */
#endif

/* End of file. */
