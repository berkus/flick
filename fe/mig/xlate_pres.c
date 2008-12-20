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

/* General Notes: The presentation structure creates linkages between
 * the MINT and CAST structures. Presentations specify both mapping and 
 * linkage types. 
 *
 * Note, some presentations are merely used to walk down a structure
 * For example, a PRES_C_INLINE_COLLAPSED_UNION presentation moves down
 * to a specific case in a union.
 */

#include <stdio.h>
#include <rpc/types.h>

/* #include "cpu.h" */
#include "message.h"
#include "type.h"
#include "routine.h"
#include "statement.h"
#include "mom_routines.h"
#include "global.h"
#include "assert.h"
#include "boolean.h"
#include "xlate_util.h"

#include <mom/types.h>
#include <mom/compiler.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>
#include <mom/idl_id.h>

#define NONE 0x0
#define COUNT_ARR 0x1
#define NOIN_OUT 0x2

/* Macro to define cast scope */
#define c_scope out_pres_c.cast

/* Macro to define cast scope */
#define c_stubscope out_pres_c.stubs_cast

/* Macro to define stubs location */
#define c_stubs out_pres_c.stubs

/* Quickie lookup macro into the mint structure. */
#define m(n) (out_pres_c.mint.defs.defs_val[n])

/* Lookup to get at func def */
#define c(n) (c_scope.cast_scope_val[n].u.cast_def_u_u.func_type)

/* Lookup into stubs array */
#define s(n) (c_stubs.stubs_val[n])

/* Useful macro in generating routine presentations. */
#define sl(n) (pres_in_2->pres_c_inline_u_u.struct_i.slots.slots_val[n])


/* Useful macros for deciphering an Argument */

#define CurTypeNameUsed(CurType, which_stub, which_mess)		\
	(((which_mess == akIn && which_stub == PRES_C_CLIENT_STUB)	\
	  || (which_mess == akOut && which_stub == PRES_C_SERVER_SKEL))	\
	 ? CurType->itInName : CurType->itOutName)
	
	
#define IsArg(arg, which_stub)					\
	(akCheckAll(arg->argKind,				\
		    which_stub == PRES_C_CLIENT_STUB		\
		    ? akbUserArg				\
		    : akbServerArg))
	
#define IsArgIn(arg, which_stub)					      \
	((akCheckAll(arg->argKind,					      \
		     akAddFeature(akInTest,				      \
				  which_stub == PRES_C_CLIENT_STUB	      \
				  ? akAddFeature(akbUserArg, akbSendSnd)      \
				  : akAddFeature(akbServerArg, akbSendRcv)))) \
	 || (((akIdent(arg->argKind) == akeMsgSeqno)			      \
	      || (akIdent(arg->argKind) == akeWaitTime)			      \
	      || (akIdent(arg->argKind) == akeMsgOption)		      \
	      || (akIdent(arg->argKind) == akeServerCopy)		      \
	      || (akIdent(arg->argKind) == akeRequestPort)		      \
	      || (akIdent(arg->argKind) == akeCountInOut))		      \
	     && (IsArg(arg, which_stub))))
	
#define IsArgOut(arg, which_stub)					     \
	(akCheckAll(arg->argKind,					     \
		    akAddFeature(akOutTest,				     \
				 which_stub == PRES_C_CLIENT_STUB	     \
				 ? akAddFeature(akbUserArg, akbReturnRcv)    \
				 : akAddFeature(akbServerArg, akbReturnSnd))))
	
#define IsArgInOut(arg, which_stub)					      \
	((akCheckAll(arg->argKind,					      \
		     akAddFeature(akAddFeature(akInTest, akOutTest),	      \
				  which_stub == PRES_C_CLIENT_STUB	      \
				  ? akAddFeature(akbUserArg,		      \
						 akAddFeature(akbReturnRcv,   \
							      akbSendSnd))    \
				  : akAddFeature(akbServerArg,		      \
						 akAddFeature(akbReturnSnd,   \
							      akbSendRcv))))) \
	 || ((akIdent(arg->argKind) == akeCountInOut)			      \
	     && (IsArg(arg, which_stub))))
	

/* Declarations for useful functions. */
void make_arg_server_dealloc_in(cast_type param_ctype,
				mint_ref param_itype,
				pres_c_mapping param_mapping,
				pres_c_mapping *dealloc_mapping);
void make_arg_server_alloc_out(cast_type param_ctype,
			       mint_ref param_itype,
			       pres_c_mapping param_mapping,
			       pres_c_mapping *alloc_mapping,
			       int initialize,
			       int dig_thru_pointers);


/* Variable to store presentation stuff */
extern pres_c_1 out_pres_c;

static const char * get_ctype_name(ipc_type_t *Type, int which_stub)
{
	const char *ret;
	if (which_stub == PRES_C_CLIENT_STUB)
		ret = Type->itUserType;
	else
	        if (Type->itTransType) 
			ret = Type->itTransType;
		else
			ret = Type->itServerType;
	if (ret == NULL) return "";
	return ret;
}

/*
 * get_allocation() returns a pres_c_allocation structure describing this
 * presentation's allocation semantics for various parameter roles.
 *
 * This code is adapted from the pfe library function p_get_allocation()
 * in `pfe/lib/p_get_allocation.cc'.
 *
 * XXX - See comment in c/pfe/lib/p_get_allocation.cc.
 */
pres_c_allocation get_allocation(int which_stub)
{
	pres_c_allocation alloc;
	
	/* Unknown direction shouldn't happen */
	alloc.cases[PRES_C_DIRECTION_UNKNOWN].allow
		= PRES_C_ALLOCATION_INVALID;
	/* Mach doesn't allow for user-specified return values. */
	alloc.cases[PRES_C_DIRECTION_RETURN].allow
		= PRES_C_ALLOCATION_INVALID;
	
	/* Set up the default allocation flags and allocators */
	if (which_stub == PRES_C_CLIENT_STUB) {
		
		pres_c_allocation_u dfault;
		dfault.allow = PRES_C_ALLOCATION_ALLOW;
		dfault.pres_c_allocation_u_u.val.flags
			= PRES_C_ALLOC_ALWAYS | PRES_C_DEALLOC_NEVER;
		dfault.pres_c_allocation_u_u.val.allocator.kind
			= PRES_C_ALLOCATOR_NAME;
		dfault.pres_c_allocation_u_u.val.allocator.pres_c_allocator_u.
			name = "mach_vm";
		dfault.pres_c_allocation_u_u.val.alloc_init = 0;

		alloc.cases[PRES_C_DIRECTION_IN].allow
			= PRES_C_ALLOCATION_ALLOW;
		alloc.cases[PRES_C_DIRECTION_IN].pres_c_allocation_u_u.val.
			flags
			= PRES_C_ALLOC_NEVER | PRES_C_DEALLOC_NEVER;
		alloc.cases[PRES_C_DIRECTION_IN].pres_c_allocation_u_u.val.
			allocator.kind
			= PRES_C_ALLOCATOR_DONTCARE;
		alloc.cases[PRES_C_DIRECTION_IN].pres_c_allocation_u_u.val.
			alloc_init = 0;
		
		alloc.cases[PRES_C_DIRECTION_INOUT] = dfault;
		alloc.cases[PRES_C_DIRECTION_OUT] = dfault;
		
	} else if (which_stub == PRES_C_SERVER_SKEL) {
		
		pres_c_allocation_u dfault;
		dfault.allow = PRES_C_ALLOCATION_ALLOW;
		dfault.pres_c_allocation_u_u.val.flags
			= PRES_C_ALLOC_ALWAYS | PRES_C_DEALLOC_ALWAYS;
		dfault.pres_c_allocation_u_u.val.allocator.kind
			= PRES_C_ALLOCATOR_DONTCARE;
		dfault.pres_c_allocation_u_u.val.alloc_init = 0;
		
		alloc.cases[PRES_C_DIRECTION_IN] = dfault;
		alloc.cases[PRES_C_DIRECTION_INOUT] = dfault;
		alloc.cases[PRES_C_DIRECTION_OUT] = dfault;
		
	} else {
		panic("In `get_allocation': "
		      "Generating neither client nor server!");
	}
	
	return alloc;
}

/*
 * get_fixed_array_allocation() returns a pres_c_allocation structure
 * describing the allocation semantics for fixed arrays.
 *
 * See get_allocation() above.
 */
pres_c_allocation get_fixed_array_allocation(int which_stub)
{
	pres_c_allocation alloc = get_allocation(which_stub);
	
	/*
	 * Fixed arrays are entirely allocated by the caller (user or
	 * server dispatch), thus never allocated/deallocated by the
	 * client, but always allocated/deallocated by the server.
	 */
	if (which_stub == PRES_C_CLIENT_STUB) {
		
		alloc.cases[PRES_C_DIRECTION_IN].
			pres_c_allocation_u_u.val.flags
			= PRES_C_ALLOC_NEVER | PRES_C_DEALLOC_NEVER;
		alloc.cases[PRES_C_DIRECTION_INOUT].
			pres_c_allocation_u_u.val.flags
			= PRES_C_ALLOC_NEVER | PRES_C_DEALLOC_NEVER;
		alloc.cases[PRES_C_DIRECTION_OUT].
			pres_c_allocation_u_u.val.flags
			= PRES_C_ALLOC_NEVER | PRES_C_DEALLOC_NEVER;
		
	} else if (which_stub == PRES_C_SERVER_SKEL) {
		
		alloc.cases[PRES_C_DIRECTION_IN].
			pres_c_allocation_u_u.val.flags
			= PRES_C_ALLOC_ALWAYS | PRES_C_DEALLOC_ALWAYS;
		alloc.cases[PRES_C_DIRECTION_INOUT].
			pres_c_allocation_u_u.val.flags
			= PRES_C_ALLOC_ALWAYS | PRES_C_DEALLOC_ALWAYS;
		alloc.cases[PRES_C_DIRECTION_OUT].
			pres_c_allocation_u_u.val.flags
			= PRES_C_ALLOC_ALWAYS | PRES_C_DEALLOC_ALWAYS;
		
		/*
		 * Further, fixed arrays are not reallocated by the callee
		 * (client stub or server work function).  Thus, we don't
		 * have to use the default presentation-supplied allocator.
		 */
		alloc.cases[PRES_C_DIRECTION_IN].
			pres_c_allocation_u_u.val.allocator.kind
			= PRES_C_ALLOCATOR_DONTCARE;
		alloc.cases[PRES_C_DIRECTION_INOUT].
			pres_c_allocation_u_u.val.allocator.kind
			= PRES_C_ALLOCATOR_DONTCARE;
		alloc.cases[PRES_C_DIRECTION_OUT].
			pres_c_allocation_u_u.val.allocator.kind
			= PRES_C_ALLOCATOR_DONTCARE;
		
	}
	
	return alloc;
}

/*
 * get_ool_allocation() returns a pres_c_allocation structure
 * describing the allocation semantics for fixed arrays.
 *
 * See get_allocation() above.
 */
pres_c_allocation get_ool_allocation(argument_t *CurArg,
				     int which_stub, int which_mess)
{
	pres_c_allocation_u dfault;
	pres_c_allocation alloc;
	
	dfault.allow = PRES_C_ALLOCATION_ALLOW;
	/*
	 * Allocation for out-of-line pointers is handled almost entirely by
	 * the user.
	 * Usually out-of-line data is never deallocated, either, but it may
	 * be specified by the user in the input definition file.
	 */
	dfault.pres_c_allocation_u_u.val.flags
		= ((CurArg->argDeallocate == d_YES
		    && ((which_stub == PRES_C_CLIENT_STUB
			 && which_mess == akIn)
			|| (which_stub == PRES_C_SERVER_SKEL
			    && which_mess == akOut)))?
		   PRES_C_ALLOC_NEVER|PRES_C_DEALLOC_ALWAYS:
		   PRES_C_ALLOC_NEVER|PRES_C_DEALLOC_NEVER);
	dfault.pres_c_allocation_u_u.val.allocator.kind
		= PRES_C_ALLOCATOR_OUTOFLINE;
	dfault.pres_c_allocation_u_u.val.allocator.
		pres_c_allocator_u.ool_name
		= "mach_vm";
	dfault.pres_c_allocation_u_u.val.alloc_init = 0;
	
	/* Specify the out-of-line allocation semantics */
	alloc.cases[PRES_C_DIRECTION_UNKNOWN].allow
		= PRES_C_ALLOCATION_INVALID;
	
	alloc.cases[PRES_C_DIRECTION_IN] = dfault;
	alloc.cases[PRES_C_DIRECTION_INOUT] = dfault;
	alloc.cases[PRES_C_DIRECTION_OUT] = dfault;
	
	alloc.cases[PRES_C_DIRECTION_RETURN].allow
		= PRES_C_ALLOCATION_INVALID;
	
	return alloc;
}

static void interpose_direction_mapping(pres_c_mapping *map,
					argument_t *arg,
					int which_stub)
{
	/*
	 * Wrap the given mapping within a special ``direction'' mapping that
	 * communicates the argument direction to the back end.  This is a bit
	 * of hackery that should be eliminated someday.
	 * These tests should be done in a specific order for identification
	 */
	if      (IsArgInOut(arg, which_stub))
		pres_c_interpose_direction(map, AOI_DIR_INOUT);
	else if (akCheckAll(arg->argKind, akReturn))
		pres_c_interpose_direction(map, AOI_DIR_RET);
	else if (IsArgIn(arg, which_stub))
		pres_c_interpose_direction(map, AOI_DIR_IN);
	else if (IsArgOut(arg, which_stub))
		pres_c_interpose_direction(map, AOI_DIR_OUT);
}

/* add the levels of pointer indirection that may be necessary,
   and convert it to an inline. */
static pres_c_inline interpose_inline_indirect(cast_type *ctypep,
					       pres_c_mapping *mapp,
					       argument_t *CurArg,
					       int cparam, int extern_indir,
					       int which_stub)
{
        /* If the external ctype will be pointer-indirected,
           then pointer-indirect the internal ctype correspondingly.  */

        if (extern_indir) {
		/*
		 * Just an indirection pointer --
		 *   never allocated/deallocated on the client,
		 *   always allocated/deallocated on the server
		 */
		pres_c_allocation alloc = get_allocation(which_stub);
		if (which_stub == PRES_C_CLIENT_STUB) {
			alloc.cases[PRES_C_DIRECTION_IN].
				pres_c_allocation_u_u.val.flags
				= PRES_C_ALLOC_NEVER | PRES_C_DEALLOC_NEVER;
			alloc.cases[PRES_C_DIRECTION_INOUT].
				pres_c_allocation_u_u.val.flags
				= PRES_C_ALLOC_NEVER | PRES_C_DEALLOC_NEVER;
			alloc.cases[PRES_C_DIRECTION_OUT].
				pres_c_allocation_u_u.val.flags
				= PRES_C_ALLOC_NEVER | PRES_C_DEALLOC_NEVER;
		} else if (which_stub == PRES_C_SERVER_SKEL) {
			alloc.cases[PRES_C_DIRECTION_IN].
				pres_c_allocation_u_u.val.flags
				= PRES_C_ALLOC_ALWAYS | PRES_C_DEALLOC_ALWAYS;
			alloc.cases[PRES_C_DIRECTION_INOUT].
				pres_c_allocation_u_u.val.flags
				= PRES_C_ALLOC_ALWAYS | PRES_C_DEALLOC_ALWAYS;
			alloc.cases[PRES_C_DIRECTION_OUT].
				pres_c_allocation_u_u.val.flags
				= PRES_C_ALLOC_ALWAYS | PRES_C_DEALLOC_ALWAYS;
		} else
			panic("In interpose_inline_indirect: "
			      "Generating neither client nor server!");
		
		pres_c_interpose_indirection_pointer(ctypep, mapp, alloc);
	}

	/* Tell the back end that this is the ``root'' of the parameter. */
	pres_c_interpose_param_root(mapp, 0, 0);
	
	/* Wrap the `map' in a direction mapping. */
	interpose_direction_mapping(mapp, CurArg, which_stub);
	
	/* Map this argument to a function parameter with a
	   pres_c_inline_atom.  */
	return pres_c_new_inline_atom(cparam, *mapp);
}

/* Make a direct, simple atom argument. Makes the MINT, CAST, and mapping all
   at once. */
static void make_simple_arg(argument_t *CurArg, int which_stub, int which_mess,
			    char *arglist_name, mint_ref *out_itype,
			    cast_type *out_ctype, pres_c_mapping *out_map)
{
	ipc_type_t *CurType = CurArg->argType; 
	ipc_type_t *ct;
	
	mint_ref itype;
	cast_type ctype;
	pres_c_mapping map = 0;
	
	int ctype_flags = CAST_PRIM_INT; /* What C type */
	int cmod_flags = 0; /* signed, unsigned, long, short... */
	
	if (akIdent(CurArg->argKind) == akeMsgSeqno)
		itype = out_pres_c.mint.standard_refs.signed32_ref;
	else {
		switch (CurTypeNameUsed(CurType, which_stub, which_mess)) {
		case MACH_MSG_TYPE_BOOLEAN:
			itype = out_pres_c.mint.standard_refs.bool_ref;
			break;
		case MACH_MSG_TYPE_INTEGER_64:
			itype = out_pres_c.mint.standard_refs.signed64_ref;
			cmod_flags = CAST_MOD_LONG_LONG;
			break;
		case MACH_MSG_TYPE_INTEGER_32:
			itype = out_pres_c.mint.standard_refs.signed32_ref;
			break;
		case MACH_MSG_TYPE_INTEGER_16:
			itype = out_pres_c.mint.standard_refs.signed16_ref;
			cmod_flags = CAST_MOD_SHORT;
			break;
		case MACH_MSG_TYPE_INTEGER_8:
			itype = out_pres_c.mint.standard_refs.signed8_ref;
			ctype_flags = CAST_PRIM_CHAR;
			break;
		case MACH_MSG_TYPE_REAL:
			switch (CurType->itSize) {
			case 32:
				itype = (out_pres_c.mint.standard_refs.
					 float32_ref);
				break;
			case 64:
				itype = (out_pres_c.mint.standard_refs.
					 float64_ref);
				break;
			default:
				panic("make_simple_arg: unsupported floating-"
				      "point size (%d-bit)", CurType->itSize);
			}
			ctype_flags = CAST_PRIM_FLOAT;
			break;
		case MACH_MSG_TYPE_CHAR:
			itype = out_pres_c.mint.standard_refs.char8_ref;
			ctype_flags = CAST_PRIM_CHAR;
			break;
		default:
			panic("make_simple_arg: unknown primitive type %d", 
			      CurTypeNameUsed(CurType,
					      which_stub, which_mess));
		}
	}
	
	/* Make the internal ctype. */
	
	/* find the base element type */
	for (ct = CurType; ct->itElement; ct = ct->itElement);
	
	/* make the ctype with the given name */
	ctype = cast_new_prim_alias(ctype_flags, cmod_flags,
				    cast_new_scoped_name(
					    get_ctype_name(ct, which_stub),
					    NULL));
	
	/* Make the internal mapping. We will impose the translation
	   mapping later, as we may not always use a translation. */
	
	switch (akIdent(CurArg->argKind)) {
	case akeWaitTime:
		map = pres_c_new_mapping(PRES_C_MAPPING_MESSAGE_ATTRIBUTE);
		map->pres_c_mapping_u_u.message_attribute.kind
			= PRES_C_MESSAGE_ATTRIBUTE_TIMEOUT;
		itype = out_pres_c.mint.standard_refs.void_ref; /* Not Used */
		break;
		
	case akeMsgOption:
		map = pres_c_new_mapping(PRES_C_MAPPING_MESSAGE_ATTRIBUTE);
		map->pres_c_mapping_u_u.message_attribute.kind
			= PRES_C_MESSAGE_ATTRIBUTE_FLAGS;
		itype = out_pres_c.mint.standard_refs.void_ref; /* Not Used */
		break;
		
	case akeMsgSeqno:
		map = pres_c_new_mapping(PRES_C_MAPPING_MESSAGE_ATTRIBUTE);
		map->pres_c_mapping_u_u.message_attribute.kind
			= PRES_C_MESSAGE_ATTRIBUTE_SEQUENCE_RECEIVED;
		itype = out_pres_c.mint.standard_refs.void_ref; /* Not Used */
		break;
		
	case akeServerCopy:
		pres_c_interpose_argument(&map, arglist_name, "mustcopy");
		itype = out_pres_c.mint.standard_refs.void_ref; /* Not Used */
		break;
		
	case akeDealloc:
		pres_c_interpose_argument(&map, arglist_name, "release");
		itype = out_pres_c.mint.standard_refs.void_ref; /* Not Used */
		break;
		
	default:
		/* Regular argument -- make a direct mapping. */
		map = pres_c_new_mapping(PRES_C_MAPPING_DIRECT);
		break;
	}
	
	*out_itype = itype;
	*out_ctype = ctype;
	*out_map = map;
}

/* adds arg_inline to the MINT and to the PRES_C trees.
   If the itype is mint_ref_null, no MINT is added.
 */
static void add_arg_to_top_inlines(mint_ref itype, pres_c_inline arg_inline,
				   mint_ref struct_m, pres_c_inline struct_i)
{
	int struct_m_index, struct_i_index;
	
	/* Add the itype to the struct_m */
	if (itype != mint_ref_null)
	{
		struct_m_index = mint_add_struct_slot(&out_pres_c.mint,
						      struct_m);
		m(struct_m).mint_def_u.struct_def.
			slots.slots_val[struct_m_index]
			= itype;
	}
	else
		struct_m_index = mint_slot_index_null;
	
        /* add the arg_inline to the struct_i.  */
	struct_i_index = pres_c_add_inline_struct_slot(struct_i);

	switch (struct_i->kind) {
	case PRES_C_INLINE_STRUCT:
		/*
		 * Place the new slot just before the last one in the list.
		 * This effectively puts the deallocation slots first
		 * (in order), and pushes the virtual union to the end.
		 */
		assert(struct_i_index > 0);
		struct_i->pres_c_inline_u_u.struct_i.slots.
			slots_val[struct_i_index] =
			(struct_i->pres_c_inline_u_u.struct_i.slots.
			 slots_val[struct_i_index - 1]);
		
		struct_i->pres_c_inline_u_u.struct_i.slots.
			slots_val[struct_i_index-1].mint_struct_slot_index
			= struct_m_index;
		struct_i->pres_c_inline_u_u.struct_i.slots.
			slots_val[struct_i_index-1].inl
			= arg_inline;
		break;
		
	case PRES_C_INLINE_FUNC_PARAMS_STRUCT:
		struct_i->pres_c_inline_u_u.func_params_i.slots.
			slots_val[struct_i_index].mint_struct_slot_index
			= struct_m_index;
		struct_i->pres_c_inline_u_u.func_params_i.slots.
			slots_val[struct_i_index].inl
			= arg_inline;
		break;

	default:
		assert(struct_i->kind == PRES_C_INLINE_FUNC_PARAMS_STRUCT
		       || struct_i->kind == PRES_C_INLINE_STRUCT);
	}
}

static void make_ref_arg(argument_t *CurArg, int which_stub, int which_mess,
		         mint_ref *out_itype, cast_type *out_ctype, 
			 pres_c_mapping *out_map)
{
    ipc_type_t *CurType = CurArg->argType;

    mint_ref itype;
    cast_type ctype;
    pres_c_mapping map;
    
    /* This is the internal type for ports. The user will see something 
       different. */

    ctype = cast_new_type_name("mach_port_t");

    map = pres_c_new_mapping(PRES_C_MAPPING_REFERENCE);
    
    /* Okay, find out the port type. Note. It can be different between 
       the client and the server. (This is transmission types at work.) */
    
    switch (CurTypeNameUsed(CurType, which_stub, which_mess))
    {
    case MACH_MSG_TYPE_POLYMORPHIC:
	    /* assign dummy values which won't be used */
    case MACH_MSG_TYPE_PORT_NAME:
	    map->pres_c_mapping_u_u.ref.kind = PRES_C_REFERENCE_COPY;
	    itype = out_pres_c.mint.standard_refs.interface_name_ref;
	    break;
    case MACH_MSG_TYPE_COPY_SEND:
	    map->pres_c_mapping_u_u.ref.kind = PRES_C_REFERENCE_COPY;
	    itype = out_pres_c.mint.standard_refs.interface_invoke_ref;
	    break;
    case MACH_MSG_TYPE_MAKE_SEND:	
	    map->pres_c_mapping_u_u.ref.kind
		    = PRES_C_REFERENCE_COPY_AND_CONVERT;
	    itype = out_pres_c.mint.standard_refs.interface_invoke_ref;
	    break;
    case MACH_MSG_TYPE_MAKE_SEND_ONCE:
	    map->pres_c_mapping_u_u.ref.kind
		    = PRES_C_REFERENCE_COPY_AND_CONVERT;
	    itype = out_pres_c.mint.standard_refs.interface_invoke_once_ref;
	    break;
    case MACH_MSG_TYPE_MOVE_RECEIVE:
	    map->pres_c_mapping_u_u.ref.kind
		    = PRES_C_REFERENCE_MOVE;
	    itype = out_pres_c.mint.standard_refs.interface_service_ref;
	    break;
    case MACH_MSG_TYPE_MOVE_SEND:
	    map->pres_c_mapping_u_u.ref.kind
		    = PRES_C_REFERENCE_MOVE;
	    itype = out_pres_c.mint.standard_refs.interface_invoke_ref;
	    break;
    case MACH_MSG_TYPE_MOVE_SEND_ONCE:
	    map->pres_c_mapping_u_u.ref.kind
		    = PRES_C_REFERENCE_MOVE;
	    itype = out_pres_c.mint.standard_refs.interface_invoke_once_ref;
	    break;
    default:
	    panic("make_ref: unknown reference type %d", 
		  CurTypeNameUsed(CurType, which_stub, which_mess));
    }
    
    /* ref_count means how many references we're sending/receiving,
       which must be 1 or more. */
    map->pres_c_mapping_u_u.ref.ref_count = 1;
    map->pres_c_mapping_u_u.ref.arglist_name = ir_strlit("");
    
    /*
     * Add an appropriate argument mapping so that we can identify this
     * argument's purpose in the back end.
     */
    if (akIdent(CurArg->argKind) == akeRequestPort)
	    pres_c_interpose_argument(&map, "params", "object");
    if (akCheckAll(CurArg->argKind, akReplyPort)
	|| akCheckAll(CurArg->argKind,akUReplyPort)
	|| akCheckAll(CurArg->argKind,akSReplyPort))
	    pres_c_interpose_argument(&map, "params", "client");
    
    *out_itype = itype;
    *out_ctype = ctype;
    *out_map = map;
}
    
/* 
 * Makes a string argument. This mapping is a terminated array mapping;
 * an array whose end is marked by a fencepost.
 */
static void make_string_arg(argument_t *CurArg, ipc_type_t *CurType,
			    mint_ref *out_itype, cast_type *out_ctype,
			    pres_c_mapping *out_map, int which_stub)
{
	mint_ref itype;
	cast_type ctype;
	
	cast_type lctype = mint_to_ctype(
		&out_pres_c.mint,
		out_pres_c.mint.standard_refs.unsigned32_ref);
	
	pres_c_mapping map, element_map;
	pres_c_inline inl;
	pres_c_inline_allocation_context *ac;
	
	unsigned len_min = CurType->itVarArray? 0:CurType->itNumber;
	unsigned len_max = CurType->itNumber;
	
	itype = xl_array(out_pres_c.mint.standard_refs.char8_ref,
			 len_min, len_max);
	
	ctype = cast_new_array_type(
		cast_new_expr_lit_int(CurType->itNumber, 0),
		cast_new_prim_type(CAST_PRIM_CHAR, 0));
	
	/* Create the allocation context node.  */
	inl = pres_c_new_inline(PRES_C_INLINE_ALLOCATION_CONTEXT);
	ac = &inl->pres_c_inline_u_u.acontext;
	
	ac->arglist_name = pres_c_make_arglist_name("vararray");
	
	/* MIG's strings are presented as fixed-size arrays of char. */
	ac->alloc = get_fixed_array_allocation(which_stub);
	
	/* Create the array inline/mapping. */
	element_map = pres_c_new_mapping(PRES_C_MAPPING_INTERNAL_ARRAY);
	element_map->pres_c_mapping_u_u.internal_array.element_mapping
		= pres_c_new_mapping(PRES_C_MAPPING_DIRECT);
	element_map->pres_c_mapping_u_u.internal_array.arglist_name
		= ac->arglist_name;
	ac->ptr = pres_c_new_inline_atom(0, element_map);
	
	/*
	 * Set the minimum and maximum presented lengths (note that normally,
	 * the *real* presented array has one more byte than what the IDL/MINT
	 * will show, for the terminator; however, string bounds in MIG's IDL
	 * *include* this terminator, so we don't have to make any
	 * modifications here).
	 */
	if (len_min != 0) {
		ac->min_len = PRES_C_I_TEMPORARY,
			        PIA_Name, "array_min",
			        PIA_CType, lctype,
			        PIA_Value, cast_new_expr_lit_int(len_min, 0),
			        PIA_IsConst, 1,
			        PIA_Mapping, PRES_C_M_ARGUMENT,
			          PMA_ArgList, ac->arglist_name,
			          PMA_Name, "min_len",
			          PMA_Mapping, NULL,
			          END_PRES_C,
			        END_PRES_C;
	}
	if (len_max != ~0U) {
		ac->max_len = PRES_C_I_TEMPORARY,
			        PIA_Name, "array_max",
			        PIA_CType, lctype,
			        PIA_Value, cast_new_expr_lit_int(len_max, 0),
			        PIA_IsConst, 1,
			        PIA_Mapping, PRES_C_M_ARGUMENT,
			          PMA_ArgList, ac->arglist_name,
			          PMA_Name, "max_len",
			          PMA_Mapping, NULL,
			          END_PRES_C,
			        END_PRES_C;
		ac->min_alloc_len = PRES_C_I_TEMPORARY,
				      PIA_Name, "array_max",
				      PIA_CType, lctype, 
				      PIA_Value, cast_new_expr_lit_int(
					      len_max, 0),
				      PIA_IsConst, 1,
				      PIA_Mapping, PRES_C_M_ARGUMENT,
				        PMA_ArgList, ac->arglist_name,
				        PMA_Name, "min_alloc_len",
				        PMA_Mapping, NULL,
				        END_PRES_C,
				      END_PRES_C;
	}
	
	/* Create the length. */
	if (len_min == len_max) {
		/* Treat as a fixed array of char. */
		ac->length = PRES_C_I_TEMPORARY,
			       PIA_Name, "string_len",
			       PIA_CType, lctype,
			       PIA_Value, cast_new_expr_lit_int(len_max, 0),
			       PIA_IsConst, 1,
			       PIA_TempType, TEMP_TYPE_ENCODED,
			       PIA_Mapping, PRES_C_M_ARGUMENT,
			         PMA_ArgList, ac->arglist_name,
			         PMA_Name, "length",
			         PMA_Mapping, NULL,
			         END_PRES_C,
			       END_PRES_C;
	} else {
		/* Treat as a regular string. */
		ac->length = PRES_C_I_ATOM,
			       PIA_Index, 0,
			       PIA_Mapping, PRES_C_M_TEMPORARY,
			         PMA_Name, "string_len",
			         PMA_CType, lctype,
			         PMA_PreHandler, "stringlen",
 			         PMA_TempType, TEMP_TYPE_ENCODED,
			         PMA_Target, PRES_C_M_ARGUMENT,
			           PMA_ArgList, ac->arglist_name,
			           PMA_Name, "length",
			           PMA_Mapping, PRES_C_M_DIRECT, END_PRES_C,
			           END_PRES_C,
			         END_PRES_C,
			       END_PRES_C;
	}
	
	/* Create the terminator. */
	ac->terminator = PRES_C_I_TEMPORARY,
		           PIA_Name, "string_term",
		           PIA_CType, cast_new_prim_type(CAST_PRIM_CHAR, 0),
			   PIA_Value, cast_new_expr_lit_char(0, 0),
			   PIA_IsConst, 1,
			   PIA_Mapping, PRES_C_M_ARGUMENT,
			     PMA_ArgList, ac->arglist_name,
			     PMA_Name, "terminator",
			     PMA_Mapping, NULL,
			     END_PRES_C,
			   END_PRES_C;
	
	/*
	 * Finally, create the map.  We have to get into inline mode
	 * for the allocation context, so we create a singleton node.
	 */
	map = pres_c_new_mapping(PRES_C_MAPPING_SINGLETON);
	map->pres_c_mapping_u_u.singleton.inl = inl;
	
	*out_itype = itype;
	*out_ctype = ctype;
	*out_map = map;
}

static void make_top_inlines(int is_reply,
			     int mig_code,
			     mint_ref procs_union,
			     mint_ref *out_mint_struct,
			     pres_c_inline *out_top_inline,
			     pres_c_inline *out_struct_inline,
			     pres_c_inline *out_dealloc_inline)
{
	pres_c_inline top_i, intf_i, proc_i, union_i = 0, struct_i;
	mint_ref mint_union_ref, mint_struct_ref;
	int procs_case;
	
	pres_c_inline proc_variant_i;
	mint_ref proc_variant_ref;
	
	/*
	 * Create a MINT_STRUCT to hold the parameters for this routine.
	 * The slots array will be built on demand.
	 */
	mint_struct_ref = get_def();
	m(mint_struct_ref).kind = MINT_STRUCT;
	m(mint_struct_ref).mint_def_u.struct_def.slots.slots_val = 0;
	m(mint_struct_ref).mint_def_u.struct_def.slots.slots_len = 0;
	
	if (!is_reply)
		/*
		 * For requests, the MINT_STRUCT is the variant of the union
		 * of operations (`procs_union').
		 */
		proc_variant_ref = mint_struct_ref;
	else {
		/*
		 * For replies, the MINT_STRUCT we just created is a variant of
		 * a MINT_UNION, representing the union of normal replies and
		 * exceptional replies (i.e., errors).
		 */
		mint_union_def *union_def;

		mint_union_ref
			= mint_add_union_def(
				&out_pres_c.mint,
				out_pres_c.mint.standard_refs.unsigned32_ref,
				1 /* one discrimintated case */);
		union_def = &(m(mint_union_ref).mint_def_u.union_def);
		
		union_def->cases.cases_val[0].val
			= mint_new_symbolic_const(MINT_CONST_INT,
						  "KERN_SUCCESS");
		union_def->cases.cases_val[0].var = mint_struct_ref;
		
		/* The exceptional case: a system exception. */
		union_def->dfault = out_pres_c.mint.standard_refs.
				    system_exception_ref;
		
		proc_variant_ref = mint_union_ref;
	}
	
	/* Add `mint_variant_ref' to the union of operations `procs_union'. */
	procs_case = mint_add_union_case(&out_pres_c.mint, procs_union);
	
	m(procs_union).mint_def_u.union_def.cases.cases_val[procs_case].val
		= mint_new_const_int(mig_code);
	m(procs_union).mint_def_u.union_def.cases.cases_val[procs_case].var
		= proc_variant_ref;
	
	/*
	 * Now create the necessary PRES_C inlines, starting with the inline
	 * for `mint_struct_ref' and going up to the one for the union of all
	 * IDLs.
	 */
	struct_i = pres_c_new_inline_func_params_struct(0);
	
	if (is_reply)
		/* In the reply, the `return_slot' is unused. */
		struct_i->pres_c_inline_u_u.func_params_i.return_slot = 0;
	else {
		/*
		 * In the request, the `return_slot' is used to cause the BE
		 * to allocate a storage location for the return value.
		 */
		pres_c_mapping return_mapping;
		
		return_mapping = pres_c_new_mapping(PRES_C_MAPPING_IGNORE);
		pres_c_interpose_param_root(&return_mapping, 0, 0);
		pres_c_interpose_direction(&return_mapping, AOI_DIR_RET);
		
		struct_i->pres_c_inline_u_u.func_params_i.return_slot
			= ((pres_c_inline_struct_slot *)
			   mustmalloc(sizeof(pres_c_inline_struct_slot)));
		
		struct_i->pres_c_inline_u_u.func_params_i.return_slot->
			mint_struct_slot_index
			= mint_slot_index_null;
		
		struct_i->pres_c_inline_u_u.func_params_i.return_slot->
			inl
			= pres_c_new_inline_atom(pres_c_func_return_index,
						 return_mapping);
	}
	
	if (!is_reply)
		/*
		 * For requests, the `struct_i' inline will be a variant of
		 * the `proc_i' below.
		 */
		proc_variant_i = struct_i;
	else {
		/*
		 * For replies, we need to create a PRES_C_INLINE_VIRTUAL_UNION
		 * corresponding to the MINT_UNION we created above.  The
		 * virtual union inline will be contained within `proc_i'.
		 */
		pres_c_inline_virtual_union *union_def;
		pres_c_mapping map;
		
		union_i = pres_c_new_inline_virtual_union(1);
		union_def = &(union_i->pres_c_inline_u_u.virtual_union);
		
		/* Setup a name for the arglist the virtual union will use. */
		union_def->arglist_name = pres_c_make_arglist_name("vunion");
		
		/*
		 * The discriminator is a `kern_return_t' and is the return
		 * value of our stub.
		 */
		map = pres_c_new_mapping(PRES_C_MAPPING_DIRECT);
		pres_c_interpose_argument(&map, union_def->arglist_name,
					  "discrim");
		union_def->discrim = pres_c_new_inline_atom(
			-1 /* return slot */,
			map);
		
		/*
		 * This is the normal reply case;
		 * see `pfe/lib/p_do_return_union.cc'.
		 */
		union_def->cases.cases_val[0] = struct_i;
		
		/*
		 * This is the system exception case;
		 * see `pfe/sun/p_exceptions.cc'.
		 */
		union_def->dfault = pres_c_new_inline_atom(
			-1 /* return slot */,
			pres_c_new_mapping(PRES_C_MAPPING_SYSTEM_EXCEPTION));
		
		proc_variant_i = union_i;
	}
	
	/* Now create some ``magic'' to allow deallocation of in parameters
	   in the reply */
	if (is_reply) {
		/*
		 * Create a PRES_C_INLINE_FUNC_PARAMS_STRUCT to hold
		 * the virtual union created above, as well as the `in'
		 * parameters that (possibly) need to be deallocated.
		 */
		pres_c_inline params_i;
		pres_c_inline_struct *params_def;
		mint_ref params_ref;
		
		params_i = pres_c_new_inline_struct(1);
		params_def = &(params_i->pres_c_inline_u_u.struct_i);
		
		/* The `return_slot' is never used for this */
		params_def->slots.slots_val[0].mint_struct_slot_index = 0;
		params_def->slots.slots_val[0].inl = union_i;

		proc_variant_i = params_i;
		
		/*
		 * Create a MINT_STRUCT to hold the parameters & virtual union.
		 */
		params_ref = get_def();
		m(params_ref).kind = MINT_STRUCT;
		m(params_ref).mint_def_u.struct_def.slots.slots_len = 1;
		m(params_ref).mint_def_u.struct_def.slots.slots_val =
			mustcalloc(sizeof(mint_ref));
		m(params_ref).mint_def_u.struct_def.slots.slots_val[0] =
			proc_variant_ref;
		
		m(procs_union).mint_def_u.union_def.cases.cases_val[procs_case].var
			= params_ref;

		assert(out_dealloc_inline);
		*out_dealloc_inline = params_i;
	}
	
	/* Now create the inline that traverses the union of procedures. */
	proc_i = pres_c_new_inline(PRES_C_INLINE_COLLAPSED_UNION);
	
	proc_i->pres_c_inline_u_u.collapsed_union.discrim_val
		= mint_new_const_int(mig_code);
	proc_i->pres_c_inline_u_u.collapsed_union.selected_case
		= proc_variant_i;
	
	/* Then create the inline that traverses the union of interfaces. */
	intf_i = pres_c_new_inline(PRES_C_INLINE_COLLAPSED_UNION);
	
	intf_i->pres_c_inline_u_u.collapsed_union.discrim_val
		= mint_new_const_int(SubsystemBase);
	intf_i->pres_c_inline_u_u.collapsed_union.selected_case = proc_i;
	
	/* Finally, create the inline that traverses the union of IDLs. */
	top_i = pres_c_new_inline(PRES_C_INLINE_COLLAPSED_UNION);
	
	top_i->pres_c_inline_u_u.collapsed_union.discrim_val
		= mint_new_const_int(IDL_MIG); /* MIG magic number */
	top_i->pres_c_inline_u_u.collapsed_union.selected_case = intf_i;
	
	*out_mint_struct = mint_struct_ref;
	*out_struct_inline = struct_i;
	*out_top_inline = top_i;
}

/* Adds a fixed array mapping onto a element mapping, which of course may be
   another fixed array mapping. Currently, we limit the nesting of 
   fixed_array mappings to two levels, even though MIG doesn't require it. 
   The element mapping is replaced by the fixed array map. */

static void interpose_fixed_array_arg(argument_t *CurArg, mint_ref *itypep,
				      cast_type *ctypep, pres_c_mapping *mapp,
				      maxint array_len, int which_stub)
{
	pres_c_mapping map, element_map;
	pres_c_inline inl;
	pres_c_inline_allocation_context *ac;
	
	cast_type lctype = mint_to_ctype(
		&out_pres_c.mint,
		out_pres_c.mint.standard_refs.unsigned32_ref);
	
	/* It must be fixed length. */
	*itypep = xl_array(*itypep, array_len, array_len);
	
	*ctypep = cast_new_array_type(cast_new_expr_lit_int(array_len, 0), 
				      *ctypep);
	
	/* Create the allocation context node.  */
	inl = pres_c_new_inline(PRES_C_INLINE_ALLOCATION_CONTEXT);
	ac = &inl->pres_c_inline_u_u.acontext;
	
	ac->arglist_name = pres_c_make_arglist_name("fixedarray");
	
	ac->alloc = get_fixed_array_allocation(which_stub);
	
	/* Create the array inline/mapping. */
	element_map = pres_c_new_mapping(PRES_C_MAPPING_INTERNAL_ARRAY);
	element_map->pres_c_mapping_u_u.internal_array.element_mapping
		= pres_c_new_mapping(PRES_C_MAPPING_DIRECT);
	element_map->pres_c_mapping_u_u.internal_array.arglist_name
		= ac->arglist_name;
	ac->ptr = pres_c_new_inline_atom(0, element_map);
	
	ac->min_len = PRES_C_I_TEMPORARY,
		        PIA_Name, "array_min",
		        PIA_CType, lctype,
		        PIA_Value, cast_new_expr_lit_int(array_len, 0),
		        PIA_IsConst, 1,
		        PIA_Mapping, PRES_C_M_ARGUMENT,
		          PMA_ArgList, ac->arglist_name,
		          PMA_Name, "min_len",
		          PMA_Mapping, NULL,
		          END_PRES_C,
		        END_PRES_C;
	
	ac->max_len = PRES_C_I_TEMPORARY,
		        PIA_Name, "array_max",
		        PIA_CType, lctype,
		        PIA_Value, cast_new_expr_lit_int(array_len, 0),
		        PIA_IsConst, 1,
		        PIA_Mapping, PRES_C_M_ARGUMENT,
		          PMA_ArgList, ac->arglist_name,
		          PMA_Name, "max_len",
		          PMA_Mapping, NULL,
		          END_PRES_C,
		        END_PRES_C;
	
	ac->min_alloc_len = PRES_C_I_TEMPORARY,
			      PIA_Name, "array_max",
			      PIA_CType, lctype, 
			      PIA_Value, cast_new_expr_lit_int(
				      array_len, 0),
			      PIA_IsConst, 1,
			      PIA_Mapping, PRES_C_M_ARGUMENT,
			        PMA_ArgList, ac->arglist_name,
			        PMA_Name, "min_alloc_len",
			        PMA_Mapping, NULL,
			        END_PRES_C,
			      END_PRES_C;
	
	/* Create the length. */
	ac->length = PRES_C_I_TEMPORARY,
		       PIA_Name, "array_len",
		       PIA_CType, lctype,
		       PIA_Value, cast_new_expr_lit_int(array_len, 0),
		       PIA_IsConst, 1,
		       PIA_TempType, TEMP_TYPE_ENCODED,
		       PIA_Mapping, PRES_C_M_ARGUMENT,
		         PMA_ArgList, ac->arglist_name,
		         PMA_Name, "length",
		         PMA_Mapping, NULL,
		         END_PRES_C,
		       END_PRES_C;
	
	/*
	 * Finally, create the map.  We have to get into inline mode
	 * for the allocation context, so we create a singleton node.
	 */
	map = pres_c_new_mapping(PRES_C_MAPPING_SINGLETON);
	map->pres_c_mapping_u_u.singleton.inl = inl;
	
	*mapp = map;
}

/* If type translation information was given, make the necessary 
   mapping. */
static void interpose_mapping_xlate(argument_t *CurArg, int which_stub,
				    cast_type *ctypep, pres_c_mapping *mapp)
{
	cast_type ctype;
	pres_c_mapping map;
	const char *presented_ctype_name;
	
	/*
	 * Create the external (presented) C type.  Be careful to recognize C
	 * keywords and create corresponding CAST_PRIM_* types; if the internal
	 * and presented types are the same, we can avoid the translation node
	 * entirely.
	 */
	presented_ctype_name = get_ctype_name(CurArg->argType, which_stub);
	if      (!strcmp(presented_ctype_name, "int"))
		ctype = cast_new_prim_type(CAST_PRIM_INT, 0);
	else if (!strcmp(presented_ctype_name, "long"))
		ctype = cast_new_prim_type(CAST_PRIM_INT, CAST_MOD_LONG);
	else if (!strcmp(presented_ctype_name, "short"))
		ctype = cast_new_prim_type(CAST_PRIM_INT, CAST_MOD_SHORT);
	else if (!strcmp(presented_ctype_name, "signed"))
		ctype = cast_new_prim_type(CAST_PRIM_INT, CAST_MOD_SIGNED);
	else if (!strcmp(presented_ctype_name, "unsigned"))
		ctype = cast_new_prim_type(CAST_PRIM_INT, CAST_MOD_UNSIGNED);
	else if (!strcmp(presented_ctype_name, "char"))
		ctype = cast_new_prim_type(CAST_PRIM_CHAR, 0);
	else if (!strcmp(presented_ctype_name, "float"))
		ctype = cast_new_prim_type(CAST_PRIM_FLOAT, 0);
	else if (!strcmp(presented_ctype_name, "double"))
		ctype = cast_new_prim_type(CAST_PRIM_DOUBLE, 0);
	else
		ctype = cast_new_type_name((char *) presented_ctype_name);
	
	map = pres_c_new_mapping(PRES_C_MAPPING_XLATE);
	map->pres_c_mapping_u_u.xlate.internal_mapping = *mapp;
	map->pres_c_mapping_u_u.xlate.internal_ctype = *ctypep;

	if (!CurArg->argType->itVarArray &&
	    !CurArg->argType->itIndefinite &&
	    CurArg->argType->itNumber > 1) {
		if (CurArg->argType->itPassByValue)
			map->pres_c_mapping_u_u.xlate.translator = "&";
		else
			map->pres_c_mapping_u_u.xlate.translator = "~";
	} else
		map->pres_c_mapping_u_u.xlate.translator = "";

	map->pres_c_mapping_u_u.xlate.destructor = "";

	*ctypep = ctype;
	*mapp = map;
}
	       

/* If type translation information was given, make the necessary 
   mapping. */

static void interpose_type_translation(argument_t *CurArg, 
				       cast_type *ctypep, pres_c_mapping *mapp,
				       int which_stub, int which_mess,
				       int *used_trans)
{
	pres_c_mapping server_map; 
	pres_c_mapping trans_map;

	ipc_type_t *CurType = CurArg->argType;
	
	if (CurType->itInTrans && which_mess == akIn)
	{
	    *used_trans = 1;
	    
	    trans_map = pres_c_new_mapping(PRES_C_MAPPING_XLATE);
	    
	    trans_map->pres_c_mapping_u_u.xlate.internal_mapping = *mapp;
	    
	    trans_map->pres_c_mapping_u_u.xlate.internal_ctype = 	   
		    cast_new_type_name((char *)CurType->itServerType);

	    trans_map->pres_c_mapping_u_u.xlate.translator = 
		    (char *) CurType->itInTrans;
	    
	    if (CurType->itDestructor)
	    {
		    trans_map->pres_c_mapping_u_u.xlate.destructor = 
			    (char *) CurType->itDestructor;
	    }
	    else
		    trans_map->pres_c_mapping_u_u.xlate.destructor = "";

	    *mapp = trans_map;
	}

	else if (CurType->itOutTrans && which_mess == akOut)
	{	    
	    *used_trans = 1;
	    trans_map = pres_c_new_mapping(PRES_C_MAPPING_XLATE);
	    server_map = pres_c_new_mapping(PRES_C_MAPPING_XLATE);

	    server_map->pres_c_mapping_u_u.xlate.internal_mapping = *mapp;

	    server_map->pres_c_mapping_u_u.xlate.internal_ctype = *ctypep;

	    server_map->pres_c_mapping_u_u.xlate.translator = "";
		    
	    server_map->pres_c_mapping_u_u.xlate.destructor = "";
	    
	    trans_map->pres_c_mapping_u_u.xlate.internal_mapping = server_map;
	    
	    trans_map->pres_c_mapping_u_u.xlate.internal_ctype = 
		    cast_new_type_name((char *)CurType->itServerType);

	    trans_map->pres_c_mapping_u_u.xlate.translator = 
		    (char *) CurType->itOutTrans;
	    
	    trans_map->pres_c_mapping_u_u.xlate.destructor = "";

	    *mapp = trans_map;
	}
}

static void add_typedef(const char * presented_ctype_name, cast_type ctype,
			cast_type savetype)
{
	int new_def;
	
	/* Save the old ctype as an implied typedef for later */
	if (cast_find_typedef_type(&(out_pres_c.cast), ctype) == NULL) {
		new_def = cast_add_def(&(out_pres_c.cast),
				       cast_new_scoped_name(
					       strmake(presented_ctype_name),
					       NULL),
				       CAST_SC_EXTERN,
				       CAST_TYPEDEF,
				       channel_maps[MIG_CHANNEL_CLIENT_DECL]
				       [builtin_file],
				       CAST_PROT_NONE);
		out_pres_c.cast.cast_scope_val[new_def].u.cast_def_u_u.
			typedef_type = savetype;
	}
}

/* Impose a type translation at the inline level.
   This is used for _every_ MIG argument,
   so that the user-visible function prototype will use the user's 
   specified type instead of the actual internal type used for marshaling 
   or unmarshaling. This kludge is necessary only because, unlike 
   sensible IDL compilers, MIG doesn't produce typedefs for data types 
   declared in the IDL file; it only produces the function prototypes,
   and expects the user to have declared the appropriate typedefs.
   Since Flick can't know exactly what the user will declare a 
   particular C type to be, we just have to spit out a null translation node
   and hope a simple typecast does the trick... */

static void interpose_inline_xlate(argument_t *CurArg, int which_stub,
				   cast_type *ctypep, pres_c_inline *inlp,
				   int index, int extern_indir, 
				   int make_null)
{
	ipc_type_t *CurType = CurArg->argType;
	
	identifier_t presented_ctype_name;
	cast_type ctype;
	pres_c_inline xlate_inline;
	
	/*
	 * Create the external (presented) C type.  Be careful to recognize C
	 * keywords and create corresponding CAST_PRIM_* types; if the internal
	 * and presented types are the same, we can avoid the translation node
	 * entirely.
	 */
	presented_ctype_name = get_ctype_name(CurArg->argType, which_stub);
	if      (!strcmp(presented_ctype_name, "int"))
		ctype = cast_new_prim_type(CAST_PRIM_INT, 0);
	else if (!strcmp(presented_ctype_name, "long"))
		ctype = cast_new_prim_type(CAST_PRIM_INT, CAST_MOD_LONG);
	else if (!strcmp(presented_ctype_name, "short"))
		ctype = cast_new_prim_type(CAST_PRIM_INT, CAST_MOD_SHORT);
	else if (!strcmp(presented_ctype_name, "signed"))
		ctype = cast_new_prim_type(CAST_PRIM_INT, CAST_MOD_SIGNED);
	else if (!strcmp(presented_ctype_name, "unsigned"))
		ctype = cast_new_prim_type(CAST_PRIM_INT, CAST_MOD_UNSIGNED);
	else if (!strcmp(presented_ctype_name, "char"))
		ctype = cast_new_prim_type(CAST_PRIM_CHAR, 0);
	else if (!strcmp(presented_ctype_name, "float"))
		ctype = cast_new_prim_type(CAST_PRIM_FLOAT, 0);
	else if (!strcmp(presented_ctype_name, "double"))
		ctype = cast_new_prim_type(CAST_PRIM_DOUBLE, 0);
	else
		ctype = cast_new_type_name((char *) presented_ctype_name);
	
	/*
	 * Interpose a pointer on the presented C type if required (e.g., for
	 * an `out' parameter).
	 */
	if (extern_indir) {
		assert((*ctypep)->kind == CAST_TYPE_POINTER);
		add_typedef(presented_ctype_name, ctype,
			    (*ctypep)->cast_type_u_u.pointer_type.target);
		ctype = cast_new_pointer_type(ctype);
	} else {
		add_typedef(presented_ctype_name, ctype, *ctypep);
	}
	
	/*
	 * Decide if we need a translation inline at all.
	 */
	if (!cast_cmp_type(ctype, (*ctypep)))
		/*
		 * The presented and internal C types are the same, so no xlate
		 * node is necessary.
		 */
		;
	else {
		xlate_inline = pres_c_new_inline(PRES_C_INLINE_XLATE);
		
		xlate_inline->pres_c_inline_u_u.xlate.sub = *inlp;
		xlate_inline->pres_c_inline_u_u.xlate.index = index;
		
		if (make_null) {
			xlate_inline->pres_c_inline_u_u.xlate.internal_ctype
#if 0
				= ctype;
#else
			        = *ctypep;  /*ECP: I don't know why it
					      wouldn't be this*/
#endif
			
			xlate_inline->pres_c_inline_u_u.xlate.translator = "";
			xlate_inline->pres_c_inline_u_u.xlate.destructor = "";
			
		} else if (CurType->itVarArray || CurType->itIndefinite
			   || (CurType->itNumber <= 1) || (!CurType->itInLine)) {
			xlate_inline->pres_c_inline_u_u.xlate.internal_ctype
				= *ctypep;
			
			/*
			 * The translation for the xlate will always be a cast.
			 */
			xlate_inline->pres_c_inline_u_u.xlate.translator = "";
			xlate_inline->pres_c_inline_u_u.xlate.destructor = "";
		} else {
			/*
			 * If we have a fixed-size array, make the internal
			 * type a pointer to the element type.  This pointer
			 * will be cast into the appropriate type.  If we have
			 * a struct, the address of the variable must be cast.
			 */
			xlate_inline->pres_c_inline_u_u.xlate.internal_ctype
				= *ctypep;
			
			if (CurType->itPassByValue) /*Struct*/
				xlate_inline->pres_c_inline_u_u.xlate.
					translator = "&";
			else {
				if (which_stub == PRES_C_SERVER_SKEL) {
					/*
					 * no translation necessary, since the
					 * server creates pointer-to-element
					 * types instead of array types.
					 */
					xlate_inline->pres_c_inline_u_u.xlate.
						translator = "";
				} else
					xlate_inline->pres_c_inline_u_u.xlate.
						translator = "~";
			}
			
			xlate_inline->pres_c_inline_u_u.xlate.destructor = "";
		}
		
		/* Interpose the new inline and ctype on the old ones. */
		*ctypep = ctype;
		*inlp = xlate_inline;
	}
}

/* Interpose an out-of-line pointer */
static void interpose_ool_pointer(argument_t *CurArg,
				  cast_type *ctypep,
				  pres_c_mapping *mapp,
				  int which_stub,
				  int which_mess)
{
	pres_c_allocation alloc = get_ool_allocation(CurArg,
						     which_stub, which_mess);
	pres_c_mapping singleton;
	pres_c_inline alloc_inl;
	pres_c_inline_allocation_context *ac;
	
	cast_type lctype = mint_to_ctype(
		&out_pres_c.mint,
		out_pres_c.mint.standard_refs.unsigned32_ref);
	
	assert(mapp);
	assert(ctypep);
	
	/*
	 * In order to interpose this pointer, we also need to interpose an
	 * allocation context inline and temporary mappings to hold imporant
	 * contextual information.
	 */
	singleton = pres_c_new_mapping(PRES_C_MAPPING_SINGLETON);
	singleton->pres_c_mapping_u_u.singleton.inl
		= alloc_inl
		= pres_c_new_inline(PRES_C_INLINE_ALLOCATION_CONTEXT);
	ac = &alloc_inl->pres_c_inline_u_u.acontext;
	
	ac->arglist_name = pres_c_make_arglist_name("oolptr");
	
	ac->length
		= PRES_C_I_TEMPORARY,
		    PIA_Name, "array_len",
		    PIA_CType, lctype,
		    PIA_Value, cast_new_expr_lit_int(1, 0),
		    PIA_IsConst, 1,
		    PIA_TempType, TEMP_TYPE_ENCODED,
		    PIA_Mapping, PRES_C_M_ARGUMENT,
		      PMA_ArgList, ac->arglist_name,
		      PMA_Name, "length",
		      PMA_Mapping, NULL,
		      END_PRES_C,
		    END_PRES_C;
	
	ac->min_len
		= PRES_C_I_TEMPORARY,
		    PIA_Name, "array_len",
		    PIA_CType, lctype,
		    PIA_Value, cast_new_expr_lit_int(1, 0),
		    PIA_IsConst, 1,
		    PIA_Mapping, PRES_C_M_ARGUMENT,
		      PMA_ArgList, ac->arglist_name,
		      PMA_Name, "min_len",
		      PMA_Mapping, NULL,
		      END_PRES_C,
		    END_PRES_C;
	
	ac->max_len
		= PRES_C_I_TEMPORARY,
		    PIA_Name, "array_len",
		    PIA_CType, lctype,
		    PIA_Value, cast_new_expr_lit_int(1, 0),
		    PIA_IsConst, 1,
		    PIA_Mapping, PRES_C_M_ARGUMENT,
		      PMA_ArgList, ac->arglist_name,
		      PMA_Name, "max_len",
		      PMA_Mapping, NULL,
		      END_PRES_C,
		    END_PRES_C;
	
	ac->min_alloc_len = PRES_C_I_TEMPORARY,
			      PIA_Name, "array_len",
			      PIA_CType, lctype, 
			      PIA_Value, cast_new_expr_lit_int(1, 0),
			      PIA_IsConst, 1,
			      PIA_Mapping, PRES_C_M_ARGUMENT,
			        PMA_ArgList, ac->arglist_name,
			        PMA_Name, "min_alloc_len",
			        PMA_Mapping, NULL,
			        END_PRES_C,
			      END_PRES_C;
	
	ac->alloc = alloc;
	
	pres_c_interpose_pointer(ctypep, mapp, ac->arglist_name);
	
	ac->ptr = pres_c_new_inline_atom(0, *mapp);
	
	*mapp = singleton;
}

/*
 * `interpose_counted_array_arg()' makes the mapping for the array argument
 * itself.  PLEASE NOTE that it DOES NOT create everything required for
 * properly handling the array, (e.g. its length and other allocation aspects).
 * This is done at a higher level.
 */
static void interpose_counted_array_arg(mint_ref *itypep,
					cast_type *ctypep,
					pres_c_mapping *mapp,
					argument_t *CurArg, 
					int *needs_extern_indir,
					int which_stub,
					int which_mess,
					char *arglist_name)
{
        maxuint array_max;
	
	ipc_type_t *CurType = CurArg->argType;
	
        /* MIG supports variable-length arrays of fixed-length arrays.  */

	assert(CurType->itElement);
	
    	if (CurType->itElement->itNumber > 1)
	{
		/* take care of the array of array case */
		interpose_fixed_array_arg(CurArg, itypep, ctypep, mapp, 
					  CurType->itElement->itNumber,
					  which_stub);
	}
	
	/* Create the array *itypep, *ctypep, and pointer mapping
	   interposition.  */
	assert(arglist_name);
	if (CurType->itIndefinite || !CurType->itInLine) {
		array_max = (CurType->itLongForm) ? 0xffffffff : 0xffff;
		pres_c_interpose_internal_array(ctypep, mapp, arglist_name);
	} else {
		pres_c_mapping map;
		
		array_max = CurType->itNumber;
		*ctypep = cast_new_array_type(cast_new_expr_lit_int(array_max,
								    0), 
					      *ctypep);
		map = pres_c_new_mapping(PRES_C_MAPPING_INTERNAL_ARRAY);
		
		map->pres_c_mapping_u_u.internal_array.element_mapping = *mapp;
		map->pres_c_mapping_u_u.internal_array.arglist_name
			= arglist_name;
		*needs_extern_indir = 0;
		*mapp = map;
	}
	
	*itypep = xl_array(*itypep, 0, array_max);
}

/*
 * This is similar to make_arg_server_alloc_out(), below.
 *
 * Decide if this is an `in' parameter for which the server must deallocate
 * storage beyond the root.  If so, create a `dealloc_mapping' for doing this.
 */
void make_arg_server_dealloc_in(cast_type param_ctype,
				mint_ref param_itype,
				pres_c_mapping param_mapping,
				pres_c_mapping *dealloc_mapping)
{
	cast_type new_ctype = 0;
	pres_c_mapping new_map = 0;
	
	/*****/
	
	*dealloc_mapping = 0;
	
	if (!param_mapping) return;
	
	switch (param_mapping->kind) {
	default:
		/*
		 * By default, we assume that the server dispatch function is
		 * not required to do anything special beyond deallocating the
		 * ``root'' of the in parameter.
		 */
		fprintf(stderr,
			"Warning: In `make_arg_server_dealloc_in', "
			"unexpected mapping type %d.\n",
			param_mapping->kind);
		break;
		
	case PRES_C_MAPPING_DIRECT:
	case PRES_C_MAPPING_REFERENCE:
	case PRES_C_MAPPING_MESSAGE_ATTRIBUTE:
#if 0	/* XXX --- These cases are never encountered, at least not now. */
	case PRES_C_MAPPING_STRUCT:
	case PRES_C_MAPPING_SID:
#endif
		/*
		 * In these cases, there is nothing for the server dispatch
		 * function to do but deallocate the parameter root.
		 * Thus, we change the mapping into a MAPPING_IGNORE.
		 */
		*dealloc_mapping = pres_c_new_mapping(PRES_C_MAPPING_IGNORE);
		break;
		
	case PRES_C_MAPPING_POINTER: {
		/*
		 * Descend the target mapping.
		 */
		make_arg_server_dealloc_in(param_ctype, param_itype,
					   (param_mapping->pres_c_mapping_u_u.
					    pointer.target),
					   &new_map);
		
		/* Reinsert the pointer. */
		assert(new_map);
		
		*dealloc_mapping = pres_c_new_mapping(PRES_C_MAPPING_POINTER);
		
		(*dealloc_mapping)->pres_c_mapping_u_u.pointer
			= param_mapping->pres_c_mapping_u_u.pointer;
		(*dealloc_mapping)->pres_c_mapping_u_u.pointer.target
			= new_map;
		break;
	}
	
	case PRES_C_MAPPING_INTERNAL_ARRAY: {
		/*
		 * Descend the element mapping.
		 */
		make_arg_server_dealloc_in(param_ctype, param_itype,
					   (param_mapping->pres_c_mapping_u_u.
					    internal_array.element_mapping),
					   &new_map);
		
		/* Reinsert the internal array. */
		assert(new_map);
		
		*dealloc_mapping
			= pres_c_new_mapping(PRES_C_MAPPING_INTERNAL_ARRAY);
		(*dealloc_mapping)->pres_c_mapping_u_u.internal_array
			= param_mapping->pres_c_mapping_u_u.internal_array;
		(*dealloc_mapping)->pres_c_mapping_u_u.internal_array.
			element_mapping
			= new_map;
		break;
	}
	
	case PRES_C_MAPPING_VAR_REFERENCE: {
		/*
		 * Descend the referent mapping.
		 */
		make_arg_server_dealloc_in(param_ctype, param_itype,
					   (param_mapping->pres_c_mapping_u_u.
					    var_ref.target),
					   &new_map);
		
		/* Reinsert the var reference. */
		assert(new_map);
		
		*dealloc_mapping
			= pres_c_new_mapping(PRES_C_MAPPING_VAR_REFERENCE);
		(*dealloc_mapping)->pres_c_mapping_u_u.var_ref
			= param_mapping->pres_c_mapping_u_u.var_ref;
		(*dealloc_mapping)->pres_c_mapping_u_u.var_ref.target
			= new_map;
		break;
	}
	
	case PRES_C_MAPPING_STUB:
		/*
		 * Descend into the stub's mapping.
		 */
		new_ctype = param_ctype;
		new_map = param_mapping;
		
		pres_descend_mapping_stub(&out_pres_c, param_itype,
					  &new_ctype, &new_map);
		make_arg_server_dealloc_in(new_ctype, param_itype, new_map,
					   dealloc_mapping);
		break;
		
	case PRES_C_MAPPING_XLATE:
		/*
		 * Descend into the translation.
		 */
		new_ctype = param_mapping->pres_c_mapping_u_u.xlate.
			    internal_ctype;
		new_map = param_mapping->pres_c_mapping_u_u.xlate.
			  internal_mapping;
		
		make_arg_server_dealloc_in(new_ctype, param_itype, new_map,
					   dealloc_mapping);
		/* Reinsert the translation node. */
		assert(*dealloc_mapping);
		new_map = pres_c_new_mapping(PRES_C_MAPPING_XLATE);
		*new_map = *param_mapping;
		
		new_map->pres_c_mapping_u_u.xlate.internal_mapping
			= *dealloc_mapping;
		*dealloc_mapping = new_map;
		break;
		
	case PRES_C_MAPPING_DIRECTION:
		/*
		 * Skip past the direction mapping.
		 */
		make_arg_server_dealloc_in(param_ctype, param_itype,
					   (param_mapping->pres_c_mapping_u_u.
					    direction.mapping),
					   dealloc_mapping);
		/* Reinsert the direction node. */
		assert(*dealloc_mapping);
		pres_c_interpose_direction(dealloc_mapping,
					   AOI_DIR_IN);
		break;
		
	case PRES_C_MAPPING_ARGUMENT:
		/*
		 * Skip past the argument mapping.
		 */
		make_arg_server_dealloc_in(param_ctype, param_itype,
					   (param_mapping->pres_c_mapping_u_u.
					    argument.map),
					   dealloc_mapping);
		/* Reinsert the argument node. */
		/* Normally, we would assert that `dealloc_mapping' actually
                   contains something, but in the case of arguments, their
                   mappings may be (and sometimes are) NULL. */
		pres_c_interpose_argument(dealloc_mapping,
					  (param_mapping->
					   pres_c_mapping_u_u.argument.
					   arglist_name),
					  (param_mapping->
					   pres_c_mapping_u_u.argument.
					   arg_name));
		break;
		
	case PRES_C_MAPPING_PARAM_ROOT:
		/*
		 * Skip past the ``parameter root'' mapping.
		 */
		make_arg_server_dealloc_in(param_ctype, param_itype,
					   (param_mapping->pres_c_mapping_u_u.
					    param_root.map),
					   dealloc_mapping);
		/*
		 * Reinsert the ``parameter root'' node.  This node must always
		 * be inserted, or else the back end won't notice the parameter
		 * root at all!
		 */
		assert(*dealloc_mapping);
		pres_c_interpose_param_root(dealloc_mapping,
					    (param_mapping->pres_c_mapping_u_u.
					     param_root.ctype),
					    (param_mapping->pres_c_mapping_u_u.
					     param_root.init));
		break;
		
	case PRES_C_MAPPING_SINGLETON: {
		/*
		 * Skip past the singleton mapping.  Unfortunately, this
		 * returns us to inline mode, and we really aren't prepared to
		 * handle inlines at this point.  The only case that we should
		 * encounter is a PRES_C_INLINE_ALLOCATION_CONTEXT node, which
		 * we need to pluck off also, then a PRES_C_INLINE_ATOM in the
		 * ptr slot that should refer to slot 0 of the singleton, and
		 * then we can continue looking for the mapping.  Phew!
		 */
		pres_c_inline_allocation_context *ac;
		pres_c_inline inl;
		
		assert(param_mapping->pres_c_mapping_u_u.singleton.inl->kind
		       == PRES_C_INLINE_ALLOCATION_CONTEXT);
		ac = &param_mapping->pres_c_mapping_u_u.singleton.inl->
		     pres_c_inline_u_u.acontext;
		assert(ac->ptr->kind == PRES_C_INLINE_ATOM);
		assert(ac->ptr->pres_c_inline_u_u.atom.index == 0);
		
		/* Go for the ptr mapping. */
		make_arg_server_dealloc_in(param_ctype, param_itype,
					   (ac->ptr->pres_c_inline_u_u.atom.
					    mapping),
					   dealloc_mapping);
		
		/* Reinsert a copy of all the junk we just stripped off. */
		assert (*dealloc_mapping);
		inl = pres_c_new_inline(PRES_C_INLINE_ALLOCATION_CONTEXT);
		inl->pres_c_inline_u_u.acontext = *ac;
		inl->pres_c_inline_u_u.acontext.ptr
			= pres_c_new_inline_atom(0, *dealloc_mapping);

		/* We may also have to do the length mapping. */
		if (ac->length->kind == PRES_C_INLINE_ATOM) {
			assert(ac->ptr->pres_c_inline_u_u.atom.index == 0);
			make_arg_server_dealloc_in(param_ctype, param_itype,
						   (ac->length->
						    pres_c_inline_u_u.atom.
						    mapping),
						   dealloc_mapping);
			assert(*dealloc_mapping);
			inl->pres_c_inline_u_u.acontext.length
				= pres_c_new_inline_atom(0, *dealloc_mapping);
		}
		
		*dealloc_mapping
			= pres_c_new_mapping(PRES_C_MAPPING_SINGLETON);
		(*dealloc_mapping)->pres_c_mapping_u_u.singleton.inl = inl;
		break;
	}
	
	case PRES_C_MAPPING_TEMPORARY: {
		/*
		 * Descend the temporary mapping.
		 */
		make_arg_server_dealloc_in(param_ctype, param_itype,
					   (param_mapping->pres_c_mapping_u_u.
					    temp.map),
					   &new_map);
		
		/* Reinsert the internal array. */
		assert(new_map);
		
		*dealloc_mapping
			= pres_c_new_mapping(PRES_C_MAPPING_TEMPORARY);
		(*dealloc_mapping)->pres_c_mapping_u_u.temp
			= param_mapping->pres_c_mapping_u_u.temp;
		(*dealloc_mapping)->pres_c_mapping_u_u.temp.map
			= new_map;
		break;
	}
	}
}

/*
 * THIS IS AN ALMOST DIRECT COPY FROM `c/pfe/lib/p_param.cc's
 * p_param_server_alloc_out().
 *
 * Decide if this is an `out' parameter for which the server must allocate
 * storage beyond the root.  If so, create an `alloc_mapping' for doing this.
 */
void make_arg_server_alloc_out(cast_type param_ctype,
			       mint_ref param_itype,
			       pres_c_mapping param_mapping,
			       pres_c_mapping *alloc_mapping,
			       int initialize,
			       int dig_thru_pointers)
{
	cast_type new_ctype = 0;
	pres_c_mapping new_map = 0;
	
	/*****/
	
	*alloc_mapping = 0;
	
	if (!param_mapping) return;
	
	switch (param_mapping->kind) {
	default:
		/*
		 * By default, we assume that the server dispatch function is
		 * not required to do anything special beyond allocating the
		 * ``root'' of the out parameter.
		 */
		fprintf(stderr,
			"Warning: In `make_arg_server_alloc_out', "
			"unexpected mapping type %d.\n",
			param_mapping->kind);
		break;
		
	case PRES_C_MAPPING_IGNORE:
	case PRES_C_MAPPING_DIRECT:
	case PRES_C_MAPPING_REFERENCE:
#if 0 /* XXX -- These cases are never encountered, at least not now. */
	case PRES_C_MAPPING_MESSAGE_ATTRIBUTE:
	case PRES_C_MAPPING_STRUCT:
	case PRES_C_MAPPING_SID:
#endif
		/*
		 * Handle mappings for values presumably passed by reference.
		 *
		 * In these cases, there is nothing for the server dispatch
		 * function to do but allocate the parameter root.
		 */
		break;
		
	case PRES_C_MAPPING_POINTER: {
		/*
		 * Descend the target mapping.
		 */
		if (param_ctype->kind == CAST_TYPE_NAME) {
			param_ctype = cast_find_typedef_type(&out_pres_c.cast,
							     param_ctype);
			if (!param_ctype)
				panic(("In `p_param_server_alloc_out(), can't "
				       "locate `typedef' for a named type."));
		}
		assert(param_ctype->kind == CAST_TYPE_POINTER);
		new_ctype = param_ctype->cast_type_u_u.pointer_type.target;
		
		if (dig_thru_pointers) {
			make_arg_server_alloc_out(new_ctype, param_itype,
						  (param_mapping->
						   pres_c_mapping_u_u.
						   pointer.target),
						  &new_map, initialize,
						  dig_thru_pointers);
		}
		
		/* Reinsert the pointer. */
		if (!new_map) {
			if (initialize) {
				new_map = pres_c_new_mapping(
					PRES_C_MAPPING_INITIALIZE);
				new_map->pres_c_mapping_u_u.initialize.value
					= cast_new_expr_lit_int(0, 0);
			} else {
				new_map = pres_c_new_mapping(
					PRES_C_MAPPING_IGNORE);
			}
		}
		
		*alloc_mapping = pres_c_new_mapping(PRES_C_MAPPING_POINTER);
		(*alloc_mapping)->pres_c_mapping_u_u.pointer
			= param_mapping->pres_c_mapping_u_u.pointer;
		(*alloc_mapping)->pres_c_mapping_u_u.pointer.target
			= new_map;
		
		break;
	}
	
	case PRES_C_MAPPING_INTERNAL_ARRAY: {
		/*
		 * Create a new internal array mapping.
		 */
		if (!new_map) {
			new_map = pres_c_new_mapping(PRES_C_MAPPING_IGNORE);
		}
		
		if (initialize) {
			*alloc_mapping = pres_c_new_mapping(
				PRES_C_MAPPING_INITIALIZE);
			(*alloc_mapping)->pres_c_mapping_u_u.initialize.value
				= cast_new_expr_lit_int(0, 0);
		} else {
			*alloc_mapping = pres_c_new_mapping(
				PRES_C_MAPPING_INTERNAL_ARRAY);
			(*alloc_mapping)->pres_c_mapping_u_u.internal_array
				= (param_mapping->pres_c_mapping_u_u.
				   internal_array);
			(*alloc_mapping)->pres_c_mapping_u_u.internal_array.
				element_mapping
				= new_map;
		}
		break;
	}
	
	case PRES_C_MAPPING_VAR_REFERENCE: {
		/*
		 * Create a new reference mapping.
		 */
		if (initialize) {
			new_map = pres_c_new_mapping(
				PRES_C_MAPPING_INITIALIZE);
			new_map->pres_c_mapping_u_u.initialize.value
				= cast_new_expr_lit_int(0, 0);
		} else {
			new_map = pres_c_new_mapping(PRES_C_MAPPING_IGNORE);
		}
		
		*alloc_mapping
			= pres_c_new_mapping(PRES_C_MAPPING_VAR_REFERENCE);
		(*alloc_mapping)->pres_c_mapping_u_u.var_ref
			= param_mapping->pres_c_mapping_u_u.var_ref;
		(*alloc_mapping)->pres_c_mapping_u_u.var_ref.target
			= new_map;
		break;
	}
	
	case PRES_C_MAPPING_STUB:
		/*
		 * Descend into the stub's mapping.
		 */
		new_ctype = param_ctype;
		new_map = param_mapping;
		
		pres_descend_mapping_stub(&out_pres_c, param_itype,
					  &new_ctype, &new_map);
		make_arg_server_alloc_out(new_ctype, param_itype, new_map,
					  alloc_mapping, initialize,
					  dig_thru_pointers);
		break;
		
	case PRES_C_MAPPING_XLATE:
		/*
		 * Descend into the translation.
		 */
		new_ctype = param_mapping->pres_c_mapping_u_u.xlate.
			    internal_ctype;
		new_map = param_mapping->pres_c_mapping_u_u.xlate.
			  internal_mapping;
		
		make_arg_server_alloc_out(new_ctype, param_itype, new_map,
					  alloc_mapping, initialize,
					  dig_thru_pointers);
		/* Reinsert the translation node. */
		if (!*alloc_mapping) {
			if (initialize) {
				*alloc_mapping = pres_c_new_mapping(
					PRES_C_MAPPING_INITIALIZE);
				(*alloc_mapping)->pres_c_mapping_u_u.
					initialize.value
					= cast_new_expr_lit_int(0, 0);
			} else {
				*alloc_mapping = pres_c_new_mapping(
					PRES_C_MAPPING_IGNORE);
			}
		}
		
		new_map = pres_c_new_mapping(PRES_C_MAPPING_XLATE);
		*new_map = *param_mapping;
		
		new_map->pres_c_mapping_u_u.xlate.internal_mapping
			= *alloc_mapping;
		*alloc_mapping = new_map;
		break;
		
	case PRES_C_MAPPING_DIRECTION:
		/*
		 * Skip past the direction mapping.
		 */
		make_arg_server_alloc_out(param_ctype, param_itype,
					  (param_mapping->pres_c_mapping_u_u.
					   direction.mapping),
					  alloc_mapping, initialize,
					  dig_thru_pointers);
		/* Reinsert the direction node. */
		if (!*alloc_mapping) {
			if (initialize) {
				*alloc_mapping = pres_c_new_mapping(
					PRES_C_MAPPING_INITIALIZE);
				(*alloc_mapping)->pres_c_mapping_u_u.
					initialize.value
					= cast_new_expr_lit_int(0, 0);
			} else {
				*alloc_mapping = pres_c_new_mapping(
					PRES_C_MAPPING_IGNORE);
			}
		}
		
		new_map = pres_c_new_mapping(PRES_C_MAPPING_DIRECTION);
		new_map->pres_c_mapping_u_u.direction
			= param_mapping->pres_c_mapping_u_u.direction;
		new_map->pres_c_mapping_u_u.direction.mapping
			= *alloc_mapping;
		*alloc_mapping = new_map;
		break;
		
	case PRES_C_MAPPING_ARGUMENT:
		/*
		 * Skip past the argument mapping.
		 */
		make_arg_server_alloc_out(param_ctype, param_itype,
					  (param_mapping->pres_c_mapping_u_u.
					   argument.map),
					  alloc_mapping, initialize,
					  dig_thru_pointers);
		/* Reinsert the argument node. */
		if (!*alloc_mapping) {
			if (initialize) {
				*alloc_mapping = pres_c_new_mapping(
					PRES_C_MAPPING_INITIALIZE);
				(*alloc_mapping)->pres_c_mapping_u_u.
					initialize.value
					= cast_new_expr_lit_int(0, 0);
			} else {
				*alloc_mapping = pres_c_new_mapping(
					PRES_C_MAPPING_IGNORE);
			}
		}
		
		pres_c_interpose_argument(alloc_mapping,
					  (param_mapping->
					   pres_c_mapping_u_u.argument.
					   arglist_name),
					  (param_mapping->
					   pres_c_mapping_u_u.argument.
					   arg_name));
		break;
		
	case PRES_C_MAPPING_PARAM_ROOT:
		/*
		 * Skip past the ``parameter root'' mapping.
		 */
		make_arg_server_alloc_out(param_ctype, param_itype,
					  (param_mapping->pres_c_mapping_u_u.
					   param_root.map),
					  alloc_mapping, initialize,
					  dig_thru_pointers);
		/*
		 * Reinsert the ``parameter root'' node.  This node must always
		 * be inserted, or else the back end won't notice the parameter
		 * root at all!
		 */
		if (!*alloc_mapping) {
			if (initialize) {
				*alloc_mapping = pres_c_new_mapping(
					PRES_C_MAPPING_INITIALIZE);
				(*alloc_mapping)->pres_c_mapping_u_u.
					initialize.value
					= cast_new_expr_lit_int(0, 0);
			} else {
				*alloc_mapping = pres_c_new_mapping(
					PRES_C_MAPPING_IGNORE);
			}
		}
		
		pres_c_interpose_param_root(alloc_mapping,
					    (param_mapping->pres_c_mapping_u_u.
					     param_root.ctype),
					    (param_mapping->pres_c_mapping_u_u.
					     param_root.init));
		break;
		
	case PRES_C_MAPPING_SINGLETON: {
		/*
		 * Skip past the singleton mapping.  Unfortunately, this
		 * returns us to inline mode, and we really aren't prepared to
		 * handle inlines at this point.  The only case that we should
		 * encounter is a PRES_C_INLINE_ALLOCATION_CONTEXT node, which
		 * we need to pluck off also, then a PRES_C_INLINE_ATOM in the
		 * ptr slot that should refer to slot 0 of the singleton, and
		 * then we can continue looking for the mapping.  Phew!
		 */
		pres_c_inline_allocation_context *ac;
		pres_c_inline inl;
		
		assert(param_mapping->pres_c_mapping_u_u.singleton.inl->kind
		       == PRES_C_INLINE_ALLOCATION_CONTEXT);
		ac = &param_mapping->pres_c_mapping_u_u.singleton.inl->
		     pres_c_inline_u_u.acontext;
		assert(ac->ptr->kind == PRES_C_INLINE_ATOM);
		assert(ac->ptr->pres_c_inline_u_u.atom.index == 0);
		
		/* Go for the *real* mapping. */
		make_arg_server_alloc_out(param_ctype, param_itype,
					  (ac->ptr->pres_c_inline_u_u.atom.
					   mapping),
					  alloc_mapping, initialize,
					  dig_thru_pointers);
		
		/* Reinsert a copy of all the junk we just stripped off. */
		assert(*alloc_mapping);
		inl = pres_c_new_inline(PRES_C_INLINE_ALLOCATION_CONTEXT);
		inl->pres_c_inline_u_u.acontext = *ac;
		inl->pres_c_inline_u_u.acontext.ptr
			= pres_c_new_inline_atom(0, *alloc_mapping);
		
		/* We may also have to do the length mapping. */
		if (ac->length->kind == PRES_C_INLINE_ATOM) {
			assert(ac->length->pres_c_inline_u_u.atom.index == 0);
			/*
			 * To preserve the argument mapping, we always dig
			 * through any pointers that might be in the way.
			 */
			make_arg_server_alloc_out(param_ctype, param_itype,
						  (ac->length->
						   pres_c_inline_u_u.atom.
						   mapping),
						  alloc_mapping, initialize,
						  1 /* dig_thru_pointers */);
			assert(*alloc_mapping);
			inl->pres_c_inline_u_u.acontext.length
				= pres_c_new_inline_atom(0, *alloc_mapping);
		}
		
		*alloc_mapping
			= pres_c_new_mapping(PRES_C_MAPPING_SINGLETON);
		(*alloc_mapping)->pres_c_mapping_u_u.singleton.inl = inl;
		break;
	}
	
	case PRES_C_MAPPING_TEMPORARY: {
		/*
		 * Descend the temporary mapping.
		 */
		new_ctype = param_mapping->pres_c_mapping_u_u.temp.ctype;
		
		/*
		 * We do NOT pass down our initialize flag, since we can now
		 * perform that initialization here by making the temporary a
		 * constant 0.  If we did pass it down, our *constant* would be
		 * initialized to 0 ("0 = 0;"), which is bad.
		 */
		make_arg_server_alloc_out(new_ctype, param_itype,
					  (param_mapping->pres_c_mapping_u_u.
					   temp.map),
					  &new_map, 0 /* never initialize */,
					  dig_thru_pointers);
		
		/* Reinsert the temporary. */
		assert(new_map);
		*alloc_mapping
			= pres_c_new_mapping(PRES_C_MAPPING_TEMPORARY);
		(*alloc_mapping)->pres_c_mapping_u_u.temp
			= param_mapping->pres_c_mapping_u_u.temp;
		/*
		 * We make the temporary constant and remove any handlers to
		 * prevent a temporary from being declared.  It makes no sense
		 * to have a temporary around when allocating, since it
		 * couldn't be properly initialized anyway.  It shouldn't even
		 * be used for the allocation.
		 */
		(*alloc_mapping)->pres_c_mapping_u_u.temp.init
			= cast_new_expr_lit_int(0, 0);
		(*alloc_mapping)->pres_c_mapping_u_u.temp.is_const = 1;
		(*alloc_mapping)->pres_c_mapping_u_u.temp.prehandler = "";
		(*alloc_mapping)->pres_c_mapping_u_u.temp.posthandler = "";
		(*alloc_mapping)->pres_c_mapping_u_u.temp.map = new_map;
		
		break;
	}
	}
}

/* Determine the type of a argument, make it. */
/* make_arg returns a non-null alloc_mapp if it is an Out parameter and
   it needs allocation information put into the Request */

void make_arg(argument_t *CurArg, int which_stub, int which_mess, int cparam, 
	      cast_ref cfunc, cast_type *ctypep, mint_ref *itypep,
	      pres_c_inline *arg_ip, pres_c_inline *alloc_inlp,
	      char *arglist_name)
{
	pres_c_mapping map = 0;
	pres_c_mapping alloc_map = 0;
	
	ipc_type_t *CurType = CurArg->argType;
	
	int made_trans = 0;
	
	int is_struct = 0;
	int add_ool = 0;
	
	/* This is set to true if the external type needs a level of 	
	   pointer indirection.  */
	
	int extern_indir = IsArgOut(CurArg, which_stub);
	
	/* First produce a ctype, itype, and mapping for the 
	   basic "singleton" type.  */
	
	if (akIdent(CurArg->argKind) == akePoly)
	{
		*itypep = get_def();
		m(*itypep).kind = MINT_TYPE_TAG;
		
		*ctypep = cast_new_prim_alias(
			CAST_PRIM_INT, CAST_MOD_UNSIGNED,
			cast_new_scoped_name("mach_msg_type_name_t", NULL));
		map = pres_c_new_mapping(PRES_C_MAPPING_TYPE_TAG);
	}
	else if ((akIdent(CurArg->argKind) == akeCount)
		 || (akIdent(CurArg->argKind) == akeCountInOut))
	{
		/* no MINT needed here: all is in MINT_ARRAY */
		*itypep = mint_ref_null;
		
		*ctypep = cast_new_prim_alias(
			CAST_PRIM_INT, CAST_MOD_UNSIGNED,
			cast_new_scoped_name("mach_msg_type_number_t", NULL));

		/* Make the length mapping an argument mapping */
		assert(arglist_name);
		map = pres_c_new_mapping(PRES_C_MAPPING_DIRECT);
		pres_c_interpose_argument(&map, arglist_name, "length");
	}
	else {
		switch (CurTypeNameUsed(CurType, which_stub, which_mess))
		{
		case MACH_MSG_TYPE_PORT_NAME:
		case MACH_MSG_TYPE_MOVE_RECEIVE:
		case MACH_MSG_TYPE_MOVE_SEND:	
		case MACH_MSG_TYPE_MOVE_SEND_ONCE:
		case MACH_MSG_TYPE_COPY_SEND:
		case MACH_MSG_TYPE_MAKE_SEND:
		case MACH_MSG_TYPE_MAKE_SEND_ONCE:
		case MACH_MSG_TYPE_POLYMORPHIC:
			
			make_ref_arg(CurArg, which_stub, which_mess,
				     itypep, ctypep, &map);
			break;
			
		case MACH_MSG_TYPE_BOOLEAN:
		case MACH_MSG_TYPE_INTEGER_64: 
		case MACH_MSG_TYPE_INTEGER_32:    
		case MACH_MSG_TYPE_INTEGER_16:
		case MACH_MSG_TYPE_INTEGER_8:
		case MACH_MSG_TYPE_REAL:
		case MACH_MSG_TYPE_CHAR:
			
			make_simple_arg(CurArg, which_stub, which_mess,
					arglist_name, itypep, ctypep, &map);
			break;
			
		case MACH_MSG_TYPE_STRING:
			
			/* this is handled below */
			
			break;
			
			
		default:
			panic("Unsupported message type %d",
			      CurTypeNameUsed(CurType, which_stub,
					      which_mess));
		}
	}
	
	/* Handle fixed and variable-length strings and arrays. */
	
	if (CurTypeNameUsed(CurType, which_stub, which_mess)
	    == MACH_MSG_TYPE_STRING) {
		/*
		 * Strings are special cases:
		 *   They never need external pointer indirection
		 *     (even for out or inout parameters)
		 *   Variable length strings never have a count
		 *     parameter associated with them; their length
		 *     is implicitly determined by the terminating nul
		 */

		make_string_arg(CurArg, CurType, itypep, ctypep, &map,
				which_stub);
		/* inline strings never require extra indirection */
		if (CurType->itInLine)
			extern_indir = 0;
		else
			add_ool = 1;
	} else if (CurType->itIndefinite || CurType->itVarArray) {
                /* Counted array */
		interpose_counted_array_arg(itypep, ctypep, &map, CurArg,
					    &extern_indir, which_stub,
					    which_mess, arglist_name);
	} else if (CurType->itNumber != 1) {
		/*
		 * If this fixed-length array really represents a structure,
		 * then it is pass-by-value and needs indirection for 
		 * out and inout parameters.
		 * If it's really a fixed-length array, no external type
		 * indirection is needed.
		 * If it's OOL data, this doesn't apply.
		 */

		if (CurType->itInLine) {
			if (!CurType->itPassByValue)
				extern_indir = 0;
			else
				/* Inline structs needs special handling */
				is_struct = 1;
		}
		
		/* Handle plain fixed-length arrays.  */
		interpose_fixed_array_arg(CurArg, itypep, ctypep, &map,
					  CurType->itNumber, which_stub);
		
		/* Out-of-line data */
		if (!CurType->itInLine)
			add_ool = 1;
	} else if (!CurType->itInLine) {
		add_ool = 1;
	}
	
	if (add_ool) {
		/* Out-of-line data */
		interpose_ool_pointer(CurArg, ctypep, &map,
				      which_stub, which_mess);
	}
	
	/* Check and make translation mapping, if needed. */
	if (which_stub == PRES_C_SERVER_SKEL) {
		interpose_type_translation(CurArg, ctypep,
					   &map, which_stub,
					   which_mess, &made_trans);
	}

	/* Interpose a mapping translation for inline structs. */
	if (is_struct)
		interpose_mapping_xlate(CurArg, which_stub, ctypep,
					&map);
	
        /* add the levels of pointer indirection that may be necessary,
	   and convert it to an inline. */
	*arg_ip = interpose_inline_indirect(ctypep, &map, CurArg,
					    cparam, extern_indir, which_stub);
	
	/* Now that we have the complete internal type worked out,
	   encapsulate it in a pres_c_inline_xlate
	   so the external type can be the name the user specified.
	   (we've already translated inline structs, so exclude them here)
	*/
	if (!is_struct)
		interpose_inline_xlate(CurArg, which_stub, ctypep, arg_ip,
				       cparam, extern_indir, made_trans);
	
	/* Load up the function parameter with the
	   final external name and *ctypep.*/
	c(cfunc).params.params_val[cparam].name = (char*)CurArg->argName;
	c(cfunc).params.params_val[cparam].type = *ctypep;
	
	if (!IsArgInOut(CurArg, which_stub)
	    && which_stub == PRES_C_SERVER_SKEL) {
		/* Make an allocation mapping for `out' parameters for the
		   request inline. */
		if (IsArgOut(CurArg, which_stub)) {
			/*
			 * Should we initialize it?
			 *   Yes for count parameters, unbounded variable
			 *     length arrays, and out-of-line data.
			 *   No for all other parameters.     
			 */
			int init
				= (akIdent(CurArg->argKind) == akeCount)
				|| (akIdent(CurArg->argKind) == akeCountInOut)
				|| CurType->itIndefinite
				/*|| CurType->itVarArray*/
				|| add_ool;
			/*
			 * Should we dig through pointers (to perserve argument
			 * mappings)?  Yes for all count parameters, and
			 * dealloc and servercopy flags.
			 */
			int dig_thru_pointers
				= (akIdent(CurArg->argKind) == akeCount)
				|| (akIdent(CurArg->argKind) == akeCountInOut)
				|| (akIdent(CurArg->argKind) == akeDealloc)
				|| (akIdent(CurArg->argKind) == akeServerCopy);
			make_arg_server_alloc_out(*ctypep, *itypep,
						  map, &alloc_map, init,
						  dig_thru_pointers);
		}
		
		/*
		 * Make a deallocation mapping for `in' parameters for the
		 * reply inline.
		 */
		if (IsArgIn(CurArg, which_stub))
			make_arg_server_dealloc_in(*ctypep, *itypep,
						   map, &alloc_map);
	}
	
	if (alloc_map)
		*alloc_inlp = pres_c_new_inline_atom(cparam, alloc_map);
	else
		*alloc_inlp = 0;
}

void interpose_inline_typed(argument_t *TypeArg, int which_stub,
			    cast_ref cfunc,
			    mint_ref *arg_mp, pres_c_inline *arg_ip,
			    int *type_cparam,
			    pres_c_inline *type_alloc_inlp)
{
	/* the definitions for the tag type arg */
	mint_ref type_arg_m, typed_m;
	pres_c_inline type_arg_i, typed_i;
	cast_type type_arg_ctype;
	
	assert(TypeArg->argParent);
	
	/* process the type arg */
	*type_cparam = cast_func_add_param(&c(cfunc));
	make_arg(TypeArg, which_stub, akIn, *type_cparam,
		 cfunc, &type_arg_ctype, &type_arg_m,
		 &type_arg_i, type_alloc_inlp, NULL /* no arglist */);
	
	/* build the PRES_C_INLINE_TYPED */
	typed_i = pres_c_new_inline(PRES_C_INLINE_TYPED);
	typed_i->pres_c_inline_u_u.typed.inl
		= *arg_ip;
	typed_i->pres_c_inline_u_u.typed.tag
		= type_arg_i;
	
	/* build the MINT_TYPED */
	typed_m = get_def();
	m(typed_m).kind = MINT_TYPED;
	m(typed_m).mint_def_u.typed_def.ref = *arg_mp;
	m(typed_m).mint_def_u.typed_def.tag = type_arg_m;
	
	/* now return the TYPED structure as the focus */
	*arg_mp = typed_m;
	*arg_ip = typed_i;
}

void make_arg_inlines(argument_t *arg, int which_stub, cast_ref c_func,
		      mint_ref request_m, pres_c_inline request_struct_i,
		      mint_ref reply_m, pres_c_inline reply_struct_i)
{
	pres_c_inline req_i = 0, rep_i = 0;
	pres_c_inline alloc_i = 0, dealloc_i = 0;
	mint_ref req_m = mint_ref_null, rep_m = mint_ref_null;
	cast_type ctype = 0;
	cast_ref cparam;
	char *cname = 0;
	
	/*
	 * This flag is set if we need to create an inline array (the array,
	 * it's count, dealloc, and servercopy flags need to be contained in a
	 * single inline), rather than just a normal parameter that will end up
	 * an inline atom.
	 */
	int is_inline_array = ((arg->argCount != NULL)
			       && (CurTypeNameUsed(arg->argType, which_stub,
						   akIn)
				   != MACH_MSG_TYPE_STRING));
				  
	/* Create an arglist/allocation context name if we have an array. */
	if (is_inline_array) {
		cname = pres_c_make_arglist_name("vararray");
	}
	
	/* Allocate a C function parameter for this MIG
	   parameter.  Additional C parameters may also
	   be allocated later.  */
	cparam = cast_func_add_param(&c(c_func));

	if (IsArgIn(arg, which_stub)) {
		make_arg(arg, which_stub, akIn, cparam, c_func,
			 &ctype, &req_m, &req_i, &dealloc_i, cname);
		/* Only arrays will ever need a deallocation inline. */
		if (!is_inline_array) dealloc_i = 0;
	}
	if (IsArgOut(arg, which_stub)) {
		make_arg(arg, which_stub, akOut, cparam, c_func,
			 &ctype, &rep_m, &rep_i, &alloc_i, cname);
	}
	
	if (is_inline_array) {
		pres_c_inline req_inl, rep_inl;
		pres_c_inline inl_alloc_only, inl_dealloc_only;
		pres_c_inline_allocation_context *req_ac, *rep_ac, *aoc, *doc;
		mint_ref mr;
		
		/*
		 * We have a set of parameters that belong together
		 * (probably a variable-length array).  We need to
		 * construct the proper inline to hold them all.
		 */
		assert(arg->argType->itIndefinite || arg->argType->itVarArray);
		
		/*
		 * We create both request and reply inlines, regardless of what
		 * we really need.  Later, we'll only add the ones that are
		 * really valid.
		 */
		/* Create the regular inline for the request. */
		req_inl = pres_c_new_inline(PRES_C_INLINE_ALLOCATION_CONTEXT);
		req_ac = &req_inl->pres_c_inline_u_u.acontext;
		req_ac->ptr = req_i;
		
		/* Create the regular inline for the reply. */
		rep_inl = pres_c_new_inline(PRES_C_INLINE_ALLOCATION_CONTEXT);
		rep_ac = &rep_inl->pres_c_inline_u_u.acontext;
		rep_ac->ptr = rep_i;
		
		/* Create the allocation-only inline. */
		inl_alloc_only
			= pres_c_new_inline(PRES_C_INLINE_ALLOCATION_CONTEXT);
		aoc = &inl_alloc_only->pres_c_inline_u_u.acontext;
		aoc->ptr = alloc_i;

		/* Create the deallocation-only inline. */
		inl_dealloc_only
			= pres_c_new_inline(PRES_C_INLINE_ALLOCATION_CONTEXT);
		doc = &inl_dealloc_only->pres_c_inline_u_u.acontext;
		doc->ptr = dealloc_i;
		
		/* Get the right allocation semantics. */
		if (!arg->argType->itInLine) {
			req_ac->alloc = doc->alloc
				      = get_ool_allocation(arg,
							   which_stub, akIn);
			rep_ac->alloc = aoc->alloc
				      = get_ool_allocation(arg,
							   which_stub, akOut);
		} else {
			req_ac->alloc = doc->alloc
				      = rep_ac->alloc
				      = aoc->alloc
				      = get_allocation(which_stub);
		}
		
		req_ac->arglist_name = rep_ac->arglist_name
				     = aoc->arglist_name
				     = doc->arglist_name
				     = cname;
		
		/* Now, do the length (count). */
		cparam = cast_func_add_param(&c(c_func));
		make_arg(arg->argCount, which_stub, akIn, cparam, c_func,
			 &ctype, &mr, &req_ac->length, &doc->length, cname);
		make_arg(arg->argCount, which_stub, akOut, cparam, c_func,
			 &ctype, &mr, &rep_ac->length, &aoc->length, cname);
		
		if (IsArgOut(arg, which_stub) && arg->argCInOut)
			/*make_arg(arg->argCInOut, which_stub, akOut, cparam,
			  c_func, &ctype, &mr, &req_i, 0, cname);*/
			req_i = rep_ac->length;
		
		/* If the array is bounded, set the max_len inline. */
		if (!arg->argType->itIndefinite) {
			cast_type lctype = mint_to_ctype(
				&out_pres_c.mint,
				out_pres_c.mint.standard_refs.unsigned32_ref);

			/* We only set the request and reply max_len's.
                           Alloc/dealloc do not require these specified. */
			req_ac->max_len
				= rep_ac->max_len
				= aoc->max_len
				= doc->max_len
				= PRES_C_I_TEMPORARY,
				    PIA_Name, "array_max",
				    PIA_CType, lctype,
				    PIA_Value, cast_new_expr_lit_int(
					    arg->argType->itNumber, 0),
				    PIA_IsConst, 1,
				    PIA_Mapping, PRES_C_M_ARGUMENT,
				      PMA_ArgList, cname,
				      PMA_Name, "max_len",
				      PMA_Mapping, NULL,
				      END_PRES_C,
				    END_PRES_C;

			/* The minimum allocated length for bounded arrays is
                           the specified bound. */
			req_ac->min_alloc_len
				= rep_ac->min_alloc_len
				= aoc->min_alloc_len
				= doc->min_alloc_len
				= PRES_C_I_TEMPORARY,
				    PIA_Name, "array_max",
				    PIA_CType, lctype, 
				    PIA_Value, cast_new_expr_lit_int(
					    arg->argType->itNumber, 0),
				    PIA_IsConst, 1,
				    PIA_Mapping, PRES_C_M_ARGUMENT,
			              PMA_ArgList, cname,
			              PMA_Name, "min_alloc_len",
			              PMA_Mapping, NULL,
			              END_PRES_C,
				    END_PRES_C;
		}
		
		/* Now, do the dealloc flag, if it exists. */
		if (arg->argDealloc &&
		    ((which_stub == PRES_C_SERVER_SKEL &&
		      IsArgOut(arg->argDealloc, which_stub)) ||
		     (which_stub == PRES_C_CLIENT_STUB &&
		      IsArgIn(arg->argDealloc, which_stub)))) {
			pres_c_allocation alloc;
			int i;
			
			cparam = cast_func_add_param(&c(c_func));
			make_arg(arg->argDealloc, which_stub, akIn, cparam,
				 c_func, &ctype, &mr, &req_ac->release,
				 &doc->release, cname);
			make_arg(arg->argDealloc, which_stub, akOut, cparam,
				 c_func, &ctype, &mr, &rep_ac->release,
				 &aoc->release, cname);
			/*
			 * Since we might dealloc (particularly in a place
			 * where we normally would never dealloc), we have to
			 * munge the default allocation flags to reflect this.
			 */
			alloc = req_ac->alloc;
			for (i = 0; i < PRES_C_DIRECTIONS; i++) {
				if (alloc.cases[i].allow
				    != PRES_C_ALLOCATION_ALLOW)
					continue;
				alloc.cases[i].pres_c_allocation_u_u.val.flags
					|= PRES_C_DEALLOC_ALWAYS;
			}
			req_ac->alloc = doc->alloc = alloc;
			alloc = rep_ac->alloc;
			for (i = 0; i < PRES_C_DIRECTIONS; i++) {
				if (alloc.cases[i].allow
				    != PRES_C_ALLOCATION_ALLOW)
					continue;
				alloc.cases[i].pres_c_allocation_u_u.val.flags
					|= PRES_C_DEALLOC_ALWAYS;
			}
			rep_ac->alloc = aoc->alloc = alloc;
		}
		
		/* Now, the servercopy flag (server side), if it exists. */
		if (arg->argSCopy && which_stub == PRES_C_SERVER_SKEL) {
			cparam = cast_func_add_param(&c(c_func));
			make_arg(arg->argSCopy, which_stub, akIn, cparam,
				 c_func, &ctype, &mr, &req_ac->mustcopy,
				 &doc->mustcopy, cname);
			/* We should never have an ``out'' servercopy param. */
		}
		
		if (IsArgIn(arg, which_stub)) {
			req_i = req_inl;
			/*
			 * Only add the dealloc inline if this is an ``inline''
			 * (but possibly out-of-line) unbounded array that has
			 * no servercopy argument.  This is the only case that
			 * ever needs deallocated (to preserve the inline
			 * semantics of the call).
			 */
			if (which_stub == PRES_C_SERVER_SKEL
			    && arg->argType->itIndefinite
			    && arg->argType->itInLine
			    && !arg->argSCopy)
				dealloc_i = inl_dealloc_only;
			else
				dealloc_i = 0;
		}
		if (IsArgOut(arg, which_stub)) {
			rep_i = rep_inl;
			if (which_stub == PRES_C_SERVER_SKEL)
				alloc_i = inl_alloc_only;
			else
				alloc_i = 0;
		}
		
	} else if (arg->argPoly != NULL) {
		if (CurTypeNameUsed(arg->argType, which_stub, akIn)
		    == MACH_MSG_TYPE_POLYMORPHIC) {
			interpose_inline_typed(
				(arg->argPoly), which_stub, c_func,
				&req_m, &req_i, &cparam, &dealloc_i);
		}
		if (CurTypeNameUsed(arg->argType, which_stub, akOut)
		    == MACH_MSG_TYPE_POLYMORPHIC) {
			interpose_inline_typed(
				(arg->argPoly), which_stub, c_func,
				&rep_m, &rep_i, &cparam, &alloc_i);
		}

	}
	
	if (!IsArgInOut(arg, which_stub)) {
		/* Add the deallocation to the reply. */
		if (dealloc_i)
			add_arg_to_top_inlines(mint_ref_null,
					       dealloc_i,
					       reply_m,
					       reply_struct_i);
		
		/* Add the allocation to the request. */
		if (alloc_i)
			add_arg_to_top_inlines(mint_ref_null,
					       alloc_i,
					       request_m,
					       request_struct_i);
	}
	
	if (IsArgIn(arg, which_stub)) {
		/* Add the argument mint/inline to the request mint/inline. */
		add_arg_to_top_inlines(req_m, req_i,
				       request_m, request_struct_i);
	}
	if (IsArgOut(arg, which_stub)) {
		/* Add the argument mint/inline to the reply mint/inline. */
		add_arg_to_top_inlines(rep_m, rep_i,
				       reply_m, reply_struct_i);
	}
}

void make_routine(routine_t *rt, int which_stub, mint_ref procs_union,
		  cast_ref *out_c_func,
		  mint_ref *out_target_m, pres_c_inline *out_target_i,
		  mint_ref *out_client_m, pres_c_inline *out_client_i,
		  pres_c_inline *out_request_i, pres_c_inline *out_reply_i)
{
    cast_ref c_func, c_def;
    pres_c_inline alloc_inl;
    mint_ref arg_m, request_m, reply_m, target_m, client_m;
    pres_c_inline target_i, client_i, request_i, reply_i;
    pres_c_inline request_struct_i, reply_struct_i, reply_dealloc_i;
    argument_t *arg;
    int cparam;
    char *name;
    
    /* Create a function declaration.  */
    {
	const char *prefix = which_stub == PRES_C_CLIENT_STUB ? UserPrefix 
		: ServerPrefix;
	name = flick_asprintf("%s%s", prefix, rt->rtName);
	
	c_func = cast_add_def(&c_scope,
			      cast_new_scoped_name(name, NULL),
			      CAST_SC_NONE,
			      CAST_FUNC_DECL,
			      PASSTHRU_DATA_CHANNEL,
			      CAST_PROT_NONE);
	cast_init_function_type(&c(c_func), 0);
	c(c_func).return_type = cast_new_type_name("kern_return_t");
    }
    
    make_top_inlines(0, /* request */
		     SubsystemBase+rt->rtNumber, procs_union,
    		     &request_m, &request_i, &request_struct_i,
		     &reply_dealloc_i);
    make_top_inlines(1, /* reply */
		     SubsystemBase+rt->rtNumber+100, procs_union,
    		     &reply_m, &reply_i, &reply_struct_i,
		     &reply_dealloc_i);
    assert(reply_dealloc_i);
    
    /* These will be set as necessary as we process the arguments. */
    target_i = 0;
    client_i = 0;
    target_m = mint_ref_null;
    client_m = mint_ref_null;
    
    /* Make the error mapping to send error codes to the return value
     */
        
    /*
     * Walk through the arguments for this routine.
     * index is the index into the presentation struct.
     * count is the index of the argument in the CAST function decl.
     */

    for (arg = rt->rtArgs; arg != NULL; arg=arg->argNext)
    {
	    /* Skip these types of arguments (for now) */
	    /* akePoly, akeCount, and akeCountInOut are processed by their
	       Parent directly. */
	    if (arg->argType == NULL
		|| akIdent(arg->argKind) == akeReturn
		|| akIdent(arg->argKind) == akeCount 
		|| akIdent(arg->argKind) == akeCountInOut
		|| akIdent(arg->argKind) == akeDealloc
		|| akIdent(arg->argKind) == akeServerCopy
		|| akIdent(arg->argKind) == akePoly)
	    {
		    continue;
	    }
	    
	    if (akIdent(arg->argKind) == akeRequestPort)	
		{
		    cast_type ctype; pres_c_mapping map;

		    make_ref_arg(arg, which_stub, akIn, &arg_m, &ctype, &map);
		    
		    /* Tell the BE that this is the ``root'' of a parameter. */
		    pres_c_interpose_param_root(&map, 0, 0);
		    
		    cparam = cast_func_add_param(&c(c_func));

		    target_i = pres_c_new_inline_atom(cparam, map);
#if 0
		    /*
		     * XXX --- No translation is required; either the
		     * target is a port or it's not.  We can't ``fix''
		     * it by casting it.  Moreover, the code produced
		     * by an inline translation sometimes gets *lost*
		     * because it is output when the BE is invoking
		     * methods on an auxiliary `mu_target_...' object.
		     * Some BE's throw away the `mu_target_...' code,
		     * and keep just the side effect.  (See the Fluke
		     * BE, for example.)
		     *
		     * So even if we did try to translate the target
		     * object reference, we would sometimes end up with
		     * bogus output code.
		     */
		    interpose_inline_xlate(arg, which_stub, &ctype, 
					   &target_i, cparam, 0, 0);
#endif
		    
		    c(c_func).params.params_val[cparam].name
			    = (char*)arg->argName;
		    c(c_func).params.params_val[cparam].type = ctype;

		    if ((arg->argPoly != NULL)
			&& (CurTypeNameUsed(arg->argType, which_stub, akIn)
			    == MACH_MSG_TYPE_POLYMORPHIC))
			    interpose_inline_typed(
				    (arg->argPoly), which_stub,
				    c_func, &arg_m, &target_i,
				    &cparam, &alloc_inl);
		    
		    target_m = arg_m;
		    
		    continue;
		}
	    
	    if (akCheckAll(arg->argKind, akReplyPort) ||
		(akCheckAll(arg->argKind,akUReplyPort) && 
			which_stub == PRES_C_CLIENT_STUB) ||
		(akCheckAll(arg->argKind,akSReplyPort) && 
  		        which_stub == PRES_C_SERVER_SKEL))
	    {
		    cast_type ctype; pres_c_mapping map;

		    make_ref_arg(arg, which_stub, akIn, &arg_m, &ctype, &map);

		    /* Tell the BE that this is the ``root'' of a parameter. */
		    pres_c_interpose_param_root(&map, 0, 0);
		    
		    cparam = cast_func_add_param(&c(c_func));

		    client_i = pres_c_new_inline_atom(cparam, map);
#if 0
		    /*
		     * XXX --- No translation is required; either the
		     * target is a port or it's not.  We can't ``fix''
		     * it by casting it.  Moreover, the code produced
		     * by an inline translation sometimes gets *lost*
		     * because it is output when the BE is invoking
		     * methods on an auxiliary `mu_target_...' object.
		     * Some BE's throw away the `mu_target_...' code,
		     * and keep just the side effect.  (See the Fluke
		     * BE, for example.)
		     *
		     * So even if we did try to translate the target
		     * object reference, we would sometimes end up with
		     * bogus output code.
		     */
		    interpose_inline_xlate(arg, which_stub, &ctype, 
					   &client_i, cparam, 0, 0);
#endif
		    
		    c(c_func).params.params_val[cparam].name
			    = (char*)arg->argName;
		    c(c_func).params.params_val[cparam].type = ctype;

		    if ((arg->argPoly != NULL)
			&& (CurTypeNameUsed(arg->argType, which_stub, akIn)
			    == MACH_MSG_TYPE_POLYMORPHIC))
			    interpose_inline_typed(
				    (arg->argPoly), which_stub,
				    c_func, &arg_m, &client_i,
				    &cparam, &alloc_inl);
		    
		    client_m = arg_m;
		    
		    continue;
	    }

	    if (akCheckAll(arg->argKind,akUReplyPort) ||
		akCheckAll(arg->argKind,akSReplyPort))
		continue;
	    
	    /* All arguments should be In and/or Out
	       or not be a User/Server Arg at all (like dummy args) */
	    if (IsArg(arg, which_stub)
		&& !IsArgIn(arg, which_stub)
		&& !IsArgOut(arg, which_stub))
		    fprintf(stderr,"In make_routine(): warning: "
			  "Skipping `%s' which is a valid argument!\n",
			  arg->argName);
	    
	    /* For normal arguments,
	       handle them for the request and/or reply
	       depending on whether they're in, out, or inout.  */
	    if (IsArgIn(arg, which_stub) || IsArgOut(arg, which_stub))
		    make_arg_inlines(arg, which_stub, c_func,
				     request_m, request_struct_i,
				     reply_m, reply_struct_i);
    }

    /*
     * Copy our function declaration into the stubs-only CAST scope.
     * This is the copy that really matters for generating the stubs.
     */
    c_def = cast_add_def(&c_stubscope,
			 cast_new_scoped_name(name, NULL),
			 CAST_SC_NONE,
			 CAST_FUNC_DECL,
			 PASSTHRU_DATA_CHANNEL,
			 CAST_PROT_NONE);
    c_stubscope.cast_scope_val[c_def] = c_scope.cast_scope_val[c_func];
    
    /* Set the out parameters.  */
    *out_c_func = c_def;  /* NOT `c_func'; see comment above. */
    *out_target_i = target_i;
    *out_client_i = client_i;
    *out_target_m = target_m;
    *out_client_m = client_m;
    *out_request_i = request_i;
    *out_reply_i = reply_i;
}

/* End of file. */

