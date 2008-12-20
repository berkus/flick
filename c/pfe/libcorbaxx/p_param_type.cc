/*
 * Copyright (c) 1998, 1999 The University of Utah and
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

#include <mom/libaoi.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include <mom/c/pg_corbaxx.hh>

void pg_corbaxx::p_param_type(aoi_type at, mint_ref /*mr*/,
			      aoi_direction dir,
			      cast_type *out_ctype,
			      pres_c_mapping *out_mapping)
{
	/*
	 * These variables determine how allocation/deallocation occurs for
	 * parameters passed by reference.  The flags are different between
	 * client and server.
	 *
	 * `in' parameters are passed by reference when they are non-atomic.
	 *
	 * `inout' and `out' parameters are passed by reference so that the
	 * callee can modify the parameter's value.  When an `out' parameter
	 * has a variable size, the callee cannot know in advance how large the
	 * returned object will be.  Therefore, in these cases, the data passed
	 * to the callee is actually a pointer to a pointer to the variable-
	 * sized type.  See `varobj_pointer_flags' below.
	 *
	 * `return' parameters are passed by reference only when they are
	 * variable-sized.  Because `varobj_pointer_flags' already describes
	 * how these references should be managed (for both `out' and `return'
	 * parameters), we don't need a separate `return_pointer_flags'.
	 */
	pres_c_allocation in_pointer_alloc;
	pres_c_allocation inout_pointer_alloc;
	pres_c_allocation out_pointer_alloc;
	/* pres_c_allocation return_pointer_alloc; */
	
	/*
	 * `varobj_pointer_alloc' is the set of allocation flags used when a
	 * variable size object is to be passed by reference so that the callee
	 * can allocate/reallocate the memory for the object.
	 */
	pres_c_allocation varobj_pointer_alloc;
	
	/*
	 * `t' is the ``concrete'' AOI type referenced by `at'.  If `at' is an
	 * `AOI_INDIRECT', `t' is the type referenced through the indirection.
	 */
	aoi_type t;
	
	/*
	 * `min' and `max' are the subscript limits of an array type.
	 *
	 * `is_string', `is_sequence', `is_array', and `is_object' are flags
	 * indicating that the AOI type at hand is being presented as a CORBA
	 * string, sequence, array, or object reference, respectively.
	 *
	 * `is_variable' is a flag indicating that the AOI type at hand has a
	 * variable-size C presentation.  See Section 14.8 of the CORBA 2.0
	 * spec.
	 */
	unsigned int min, max;
	int is_string, is_sequence, is_array, is_object, is_typecode;
	int is_variable;
	
	/*
	 * `param_root_ctype' is the ``internal'' type of this parameter.  In
	 * some cases, the presented type is too complicated for the BE to sort
	 * out by itself.
	 *
	 * `param_root_init' is the initialization for the variable, if any.
	 */
	cast_type param_root_ctype;
	cast_init param_root_init;
	
	/*********************************************************************/
	
	/*
	 * Here we define the (specific) allocation semantics for `in', `out',
	 * `inout', and variable-sized pointer parameters.
	 *
	 * XXX - This should be much simpler; see comment in
	 * c/pfe/lib/p_get_allocation.cc.
	 */
	
	/* Invalid allocation case (ie, should never be reached). */
	pres_c_allocation_u alloc_invalid;
	alloc_invalid.allow = PRES_C_ALLOCATION_INVALID;
	
	/* Never alloc, never dealloc. */
	pres_c_allocation_u corba_ad_never;
	corba_ad_never.allow = PRES_C_ALLOCATION_ALLOW;
	corba_ad_never.pres_c_allocation_u_u.val.flags
		= PRES_C_ALLOC_NEVER | PRES_C_DEALLOC_NEVER;
	corba_ad_never.pres_c_allocation_u_u.val.allocator
		= p_get_allocator();
	corba_ad_never.pres_c_allocation_u_u.val.alloc_init = 0;
	
	/* Always alloc, never dealloc;
	   use presenation-defined [de]allocator function. */
	pres_c_allocation_u corba_alloc_always = corba_ad_never;
	corba_alloc_always.pres_c_allocation_u_u.val.flags
		= PRES_C_ALLOC_ALWAYS | PRES_C_DEALLOC_NEVER |
		PRES_C_RUN_CTOR;
	
	/* Always alloc, always dealloc;
	   use presentation-defined [de]allocator function. */
	pres_c_allocation_u corba_ad_always = corba_ad_never;
	corba_ad_always.pres_c_allocation_u_u.val.flags
		= PRES_C_ALLOC_ALWAYS | PRES_C_DEALLOC_ALWAYS |
		PRES_C_RUN_CTOR | PRES_C_RUN_DTOR;
	
	/* Never alloc, always dealloc;
	   use presentation-defined [de]allocator function. */
	pres_c_allocation_u corba_dealloc_always = corba_ad_never;
	corba_dealloc_always.pres_c_allocation_u_u.val.flags
		= PRES_C_ALLOC_NEVER | PRES_C_DEALLOC_ALWAYS | PRES_C_RUN_DTOR;
	
	/* Always alloc, always dealloc;
	   use ``optimal'' [de]allocator defined by back end. */
	pres_c_allocation_u ad_always;
	ad_always.allow = PRES_C_ALLOCATION_ALLOW;
	ad_always.pres_c_allocation_u_u.val.flags
		= PRES_C_ALLOC_ALWAYS | PRES_C_DEALLOC_ALWAYS |
		PRES_C_RUN_CTOR | PRES_C_RUN_DTOR;
	ad_always.pres_c_allocation_u_u.val.allocator.kind
		= PRES_C_ALLOCATOR_DONTCARE;
	ad_always.pres_c_allocation_u_u.val.alloc_init = 0;
	
	/*
	 * All UNKNOWN cases are invalid; even inside m/u stubs we now have
	 * proper direction information.
	 */
	in_pointer_alloc.cases[PRES_C_DIRECTION_UNKNOWN] = alloc_invalid;
	inout_pointer_alloc.cases[PRES_C_DIRECTION_UNKNOWN] = alloc_invalid;
	out_pointer_alloc.cases[PRES_C_DIRECTION_UNKNOWN] = alloc_invalid;
	varobj_pointer_alloc.cases[PRES_C_DIRECTION_UNKNOWN] = alloc_invalid;
	
	if (gen_client) {
		
		in_pointer_alloc.cases[PRES_C_DIRECTION_IN]
			= corba_ad_never;
		in_pointer_alloc.cases[PRES_C_DIRECTION_INOUT]
			= alloc_invalid;
		in_pointer_alloc.cases[PRES_C_DIRECTION_OUT]
			= alloc_invalid;
		in_pointer_alloc.cases[PRES_C_DIRECTION_RETURN]
			= alloc_invalid;
		
		inout_pointer_alloc.cases[PRES_C_DIRECTION_IN]
			= alloc_invalid;
		inout_pointer_alloc.cases[PRES_C_DIRECTION_INOUT]
			= corba_ad_never;
		inout_pointer_alloc.cases[PRES_C_DIRECTION_OUT]
			= alloc_invalid;
		inout_pointer_alloc.cases[PRES_C_DIRECTION_RETURN]
			= alloc_invalid;
		
		out_pointer_alloc.cases[PRES_C_DIRECTION_IN]
			= alloc_invalid;
		out_pointer_alloc.cases[PRES_C_DIRECTION_INOUT]
			= alloc_invalid;
		out_pointer_alloc.cases[PRES_C_DIRECTION_OUT]
			= corba_ad_never;
		out_pointer_alloc.cases[PRES_C_DIRECTION_RETURN]
			= alloc_invalid;
		
		varobj_pointer_alloc.cases[PRES_C_DIRECTION_IN]
			= alloc_invalid;
		varobj_pointer_alloc.cases[PRES_C_DIRECTION_INOUT]
			= corba_ad_always;
		varobj_pointer_alloc.cases[PRES_C_DIRECTION_OUT]
			= corba_alloc_always;
		varobj_pointer_alloc.cases[PRES_C_DIRECTION_RETURN]
			= corba_alloc_always;
		
	} else if (gen_server) {
		
		in_pointer_alloc.cases[PRES_C_DIRECTION_IN]
			= ad_always;
		in_pointer_alloc.cases[PRES_C_DIRECTION_INOUT]
			= alloc_invalid;
		in_pointer_alloc.cases[PRES_C_DIRECTION_OUT]
			= alloc_invalid;
		in_pointer_alloc.cases[PRES_C_DIRECTION_RETURN]
			= alloc_invalid;
		
		/* This inout allocation is NEVER associated with variable
		   sized data, so we are safe using a "DONTCARE" allocator. */
		inout_pointer_alloc.cases[PRES_C_DIRECTION_IN]
			= alloc_invalid;
		inout_pointer_alloc.cases[PRES_C_DIRECTION_INOUT]
			= ad_always;
		inout_pointer_alloc.cases[PRES_C_DIRECTION_OUT]
			= alloc_invalid;
		inout_pointer_alloc.cases[PRES_C_DIRECTION_RETURN]
			= alloc_invalid;
		
		out_pointer_alloc.cases[PRES_C_DIRECTION_IN]
			= alloc_invalid;
		out_pointer_alloc.cases[PRES_C_DIRECTION_INOUT]
			= alloc_invalid;
		out_pointer_alloc.cases[PRES_C_DIRECTION_OUT]
			= ad_always;
		out_pointer_alloc.cases[PRES_C_DIRECTION_RETURN]
			= alloc_invalid;
		
		/* Variable sized data should never be "DONTCARE",
		   since that most likely leads to stack allocation.
		   InOut var-sized data could be reallocated, and we
		   have to preserve the presented allocator. */
		varobj_pointer_alloc.cases[PRES_C_DIRECTION_IN]
			= alloc_invalid;
		varobj_pointer_alloc.cases[PRES_C_DIRECTION_INOUT]
			= corba_ad_always;
		varobj_pointer_alloc.cases[PRES_C_DIRECTION_OUT]
			= corba_dealloc_always;
		varobj_pointer_alloc.cases[PRES_C_DIRECTION_RETURN]
			= corba_dealloc_always;
		
	} else
		panic("In `pg_corbaxx::p_param_type', "
		      "generating neither client nor server!");
	
	/*
	 * Until recently there was a great deal of code here for handling
	 * sequence type declarations within operation parameter lists.  That
	 * syntax was allowed by Sun's CORBA front end but isn't actually
	 * allowed by the CORBA 2.0 IDL grammar.
	 *
	 * Because the sequence-declaration-within-parameter-list code is long
	 * and now very out-of-date (and therefore potentially confusing), I
	 * decided to remove the code rather than leave it in as ``dead code.''
	 * If you need to review the code that was here, check out an old
	 * (pre-1997) copy of this file.
	 */
	if (at->kind == AOI_ARRAY)
		/*
		 * Just to be safe, if we have a direct AOI reference to an
		 * array type, assert that it must be a string.  Now that we
		 * don't allow sequence-declaration-within-parameter-list
		 * syntax, all references to non-string arrays should be
		 * through `AOI_INDIRECT's (to typedef'ed types).
		 *
		 * Direct references to string types are possible because
		 * `string<N> foo' is legal syntax for a CORBA operation
		 * parameter.
		 */
		assert(at->aoi_type_u_u.array_def.flgs
		       & AOI_ARRAY_FLAG_NULL_TERMINATED_STRING);
	
	/*
	 * Get the basic presentation for this type: the `out_ctype' and the
	 * `out_mapping'.
	 */
	p_type_collection *ptc = 0;
	p_type_node *ptn;
	
	p_type(at, &ptc);
	param_root_ctype = 0;
	param_root_init = 0;
	
	/*
	 * Get the actual AOI type (not an AOI_INDIRECT type).  If the actual
	 * AOI type is an AOI_ARRAY, determine if it corresponds to a CORBA
	 * string, sequence, or array.
	 */
	t = at;
	while (t->kind == AOI_INDIRECT)
		t = in_aoi->defs.
			defs_val[t->aoi_type_u_u.indirect_ref].binding;
	
	if (t->kind == AOI_ARRAY) {
		aoi_get_array_len(in_aoi, &(t->aoi_type_u_u.array_def),
				  &min, &max);
		
		is_string   = (t->aoi_type_u_u.array_def.flgs
			       == AOI_ARRAY_FLAG_NULL_TERMINATED_STRING);
		is_sequence = ((min != max) && !is_string);
		is_array    = ((min == max) && !is_string);
	} else {
		min = 0;
		max = 0;
		is_string = 0;
		is_sequence = 0;
		is_array = 0;
	}
	if ((t->kind == AOI_INTERFACE) || (t->kind == AOI_FWD_INTRFC))
		is_object = 1;
	else
		is_object = 0;
	
	if( t->kind == AOI_TYPE_TAG )
		is_typecode = 1;
	else
		is_typecode = 0;
	
	is_variable = isVariable(at);
	
	if( is_object || is_typecode )
		ptn = ptc->find_type("pointer");
	else
		ptn = ptc->find_type("definition");
	*out_ctype = ptn->get_type();
	*out_mapping = ptn->get_mapping();
	switch( t->kind ) {
	case AOI_STRUCT:
	case AOI_UNION:
	case AOI_ARRAY:
	case AOI_TYPED:
		if( is_string ) {
			cast_type type;
			
			type = cast_new_prim_type(CAST_PRIM_CHAR, 0);
			if( dir == AOI_DIR_IN )
				type = cast_new_qualified_type(type,
							       CAST_TQ_CONST);
			type = cast_new_pointer_type(type);
			*out_ctype = type;
		} else if( (dir == AOI_DIR_IN) ) {
			*out_ctype = cast_new_qualified_type(*out_ctype,
							    CAST_TQ_CONST);
		}
		break;
	default:
		break;
	}
	
#if 0
	/*
	 * XXX - For now, we aren't going to do this.  This isn't the complete
	 * fix, since sequences that are NOT parameters (inside of a struct,
	 * union, etc.) will NOT get the special constructor.  This makes it
	 * impossible to ever do the right thing.  The alternative to this is
	 * to never allocate bounded sequences, and just use the preallocated
	 * buffer they were constructed with.  We lose the msgptr optimization,
	 * but we don't leak memory.
	 */
	
	/*
	 * For sequences, we need to call the special constructor so we DON'T
	 * automatically allocate a data buffer.
	 */
	if (is_sequence) {
		cast_expr_array ctor_params = cast_set_expr_array(
			0,
			cast_new_expr_lit_int(0, 0),
			cast_new_expr_lit_int(0, 0),
			cast_new_expr_lit_int(0, 0),
			((max == ~0U)? cast_new_expr_lit_int(0, 0) : 0),
			NULL);
		
		param_root_init = cast_new_init_construct(ctor_params);
		
		for (int i = 0; i < PRES_C_DIRECTIONS; i++) {
			if (varobj_pointer_alloc.cases[i].allow
			    != PRES_C_ALLOCATION_ALLOW)
				continue;
			varobj_pointer_alloc.cases[i].pres_c_allocation_u_u.
				val.alloc_init = param_root_init;
		}
	}
#endif
	
	/*
	 * Modify the presentation of a few specific, special types:
	 * arrays, and variable-size data.
	 */
	if (is_array) {
		/*
		 * CORBA arrays are generally presented as C arrays, and C
		 * arrays are automatically passed by reference when they are
		 * passed to functions.  They are therefore like strings in
		 * that they are ``semi-atomic'': the address of the array is
		 * always passed.
		 *
		 * The principal difference is that, since the length of an
		 * array cannot change, the presentation of `inout' and `out'
		 * can be somewhat simpler.  (See section 14.19 of the CORBA
		 * 2.0 spec.  An `inout' string is a `char **' so that the
		 * string can be resized.  An `inout' array is simply `array':
		 * automatically passed by reference, and no extra level of
		 * indirection is necessary.)
		 */
		switch (dir) {
		case AOI_DIR_IN:
			break;
		case AOI_DIR_INOUT:
			break;
		case AOI_DIR_OUT:
			if (!is_variable)
				break;
			/* FALLTHROUGH */
		case AOI_DIR_RET:
			*out_ctype = ptc->find_type("array_slice")->get_type();
			*out_ctype = cast_new_pointer_type(*out_ctype);
			break;
		}
		
	} else if (is_variable && !is_object && !is_string && !is_typecode) {
		/*
		 * When a variable-size type is used as an `out' or `return'
		 * value, the callee is responsible for allocating the storage
		 * for the returned data (because the caller cannot do so).
		 * The address of that storage will be communicated to the
		 * caller --- either as a `return' value, or as the value of
		 * an `out' parameter.
		 *
		 * Object references are an exception to this rule.  Although
		 * they are listed as variable-length types in Section 14.8 of
		 * the CORBA 2.0 spec, they are presented as if they were
		 * fixed-length types when used as parameters (according to the
		 * rules shown in Section 14.19).  This is because object
		 * references are essentially pointers, and can therefore be
		 * returned ``by value.''  Or, in other words, we don't need to
		 * *add* a pointer because an object reference already *is* a
		 * pointer.
		 *
		 * We also don't have to handle the case of CORBA strings.
		 * Strings are presented as `char *'s by the CORBA version of
		 * `p_variable_array_type'.  Essentially, this means that
		 * strings are presented as atomic data: the pointer fits in a
		 * word.  The CORBA rules for string parameters (in section
		 * 14.19 of the CORBA 2.0 spec) make sense if you think of
		 * `char *' as an atomic object like an `int'.
		 *
		 * The code here handles variable-size structs, variable-size
		 * unions, sequences, and anys.  The special presentations for
		 * (variable-size) arrays are handled previously.
		 */
		switch (dir) {
		case AOI_DIR_IN:
			break;
		case AOI_DIR_INOUT:
			/*
			 * When a variable-sized datum is `inout', the caller
			 * allocates the ``root'' of the variable-size object
			 * (e.g., the C structure that represents a sequence).
			 * The callee can then change the variable-size objects
			 * contained *within* the `inout' parameter (e.g.,
			 * change the buffer pointer wihin the sequence
			 * structure) but cannot reallocate the root `inout'
			 * parameter.  This is why `inout' variable-sized
			 * structs, variable-size unions, sequences, and CORBA
			 * anys are presented with one fewer levels of
			 * indirection that their `out' counterparts.
			 *
			 * Since the caller, not the callee, allocates storage
			 * for the `inout' parameter, we don't need to insert
			 * any levels of indirection here.  (Later on, we will
			 * add the one level of indirection that is required
			 * for all `inout' parameters.)
			 */
			break;
		case AOI_DIR_OUT:
			pres_c_interpose_indirection_pointer(
				out_ctype, out_mapping, varobj_pointer_alloc);
			param_root_init = 0;
			break;
		case AOI_DIR_RET:
			pres_c_interpose_indirection_pointer(
				out_ctype, out_mapping, varobj_pointer_alloc);
			param_root_init = 0;
			break;
		}
	}
	
	/*
	 * Now, modify the presentation of the parameter based (principally) on
	 * its direction: `in', `inout', `out', or `return'.
	 */
	switch (dir) {
	case AOI_DIR_IN:
		/*
		 * Nonatomic data (including most variable-length data) are
		 * passed by reference; the data itself will be held in
		 * auto-allocated (stack) storage.  Atomic data are simply
		 * passed by value.
		 *
		 * This distiction has three caveats:
		 *
		 * First, strings are considered ``atomic'' because they are
		 * basically presented as `char *'s, which may be passed by
		 * value.  (The CORBA rules for strings make sense if you think
		 * of `char *' as an atomic data type.)
		 *
		 * Second, arrays are considered atomic for the same reason:
		 * they are fundamentally presented as C pointers, which may be
		 * passed by value.
		 *
		 * CORBA sequences are *not* atomic because they are presented
		 * as C structures.
		 *
		 * Finally, object references are considered to be atomic,
		 * because they are essentially (opaque) pointers.
		 */
		if ((t->kind == AOI_STRUCT)
		    || (t->kind == AOI_UNION)
		    || is_sequence
		    || (t->kind == AOI_TYPED)
			)
			pres_c_interpose_var_reference(out_ctype, out_mapping);
		break;
		
	case AOI_DIR_INOUT:
		/*
		 * All `inout' data (except CORBA arrays) are passed by
		 * reference; the data itself is contained in auto-allocated
		 * storage.  The contents of that storage may be modified by
		 * the callee.
		 *
		 * This level of indirection is on top of any levels provided
		 * for the purpose of allowing the callee to reallocate storage
		 * for the `inout' parameter.
		 *
		 * CORBA arrays are handled specially.  Because CORBA arrays
		 * are presented as C arrays, they are automatically passed by
		 * reference and we therefore don't need any extra level of
		 * indirection.
		 */
		if (is_array)
			/* Do nothing. */
			;
		else
			pres_c_interpose_var_reference(out_ctype, out_mapping);
		break;
		
	case AOI_DIR_OUT:
		/*
		 * All `out' data (except CORBA arrays of fixed-size elements)
		 * are passed by reference to auto-allocated storage.  The
		 * contents of that storage is expected to be set by the
		 * callee.
		 *
		 * Note that if the object is variable-length, then the auto
		 * allocated storage will consist of an uninit'ed pointer which
		 * the callee will set to point to the returned value.  This
		 * presentation was taken care of previously.
		 */
		if (is_array && !is_variable) {
			param_root_ctype = *out_ctype;
			
			ptn = ptc->find_type("out_pointer");
			*out_ctype = ptn->get_type();
		} else {
			pres_c_interpose_var_reference(out_ctype, out_mapping);
			
			/*
			 * Save the internal reference type, so that the BE
			 * knows how to treat the ``out_pointer'' type that
			 * holds the reference.  That type may be too hard for
			 * the BE to decipher for itself (e.g., it may be a
			 * class with a constructor).
			 *
			 * XXX --- In the future, the BE should be smarter.
			 */
			param_root_ctype = *out_ctype;
			
			ptn = ptc->find_type("out_pointer");
			*out_ctype = ptn->get_type();
		}
		break;
		
	case AOI_DIR_RET:
		/*
		 * Nothing additional is required.  Most fixed-size data are
		 * returned by value.  Variable-size data are returned by
		 * reference, and the required level of indirection was added
		 * previously.
		 */
		break;
	}
	
	/*
	 * Tell the back end that this is the ``root'' of the parameter.
	 */
	pres_c_interpose_param_root(out_mapping, param_root_ctype,
				    param_root_init);
	
	/*
	 * Finally, wrap the mapping in a `hint' that tells the back end what
	 * kind of parameter this is: `in', `out', etc.
	 */
	pres_c_interpose_direction(out_mapping, dir);
}

/* End of file. */

