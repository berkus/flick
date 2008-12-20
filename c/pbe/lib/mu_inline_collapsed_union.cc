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
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>
#include <mom/c/pbe.hh>

/* This routine handles PRES_C_INLINE_COLLAPSED_UNION nodes,
   which "present" a MINT_UNION by simply "collapsing" it
   to a particular (constant) branch.
   In this case ihe fact that we're descending through a MINT_UNION
   is completely invisible to the C (programmer) side of things,
   so the inline_state `ist' is not touched at all,
   but merely passed on to the next level.

   This routine is primarily used
   when encoding request messages in client stubs,
   and for encoding reply messages in server stubs.
   In this case, the set of collapsed union nodes
   at the top of the presentation tree
   represents the "request code" or "operation ID"
   for the message sent or returned in that particular case.
   For decoding, this routine just marshals
   the appropriate constant into the message stream
   and recursively descends into the appropriate union case.

   However, this routine may also be invoked
   when writing decoding (unmarshaling) code;
   in that case, it means that the message being decoded
   "had better" select a particular constant union case;
   if it doesn't, we can't handle the message.
   For example, if a server returns
   an unexpected ("exceptional") reply to a call,
   (XXX not implemented yet)
   the client-side decoding code will fail in one of these checks
   because the reply message's ID codes will not match its expectations,
   and a special exception decoding routine will be called.
*/

/* The class of the `functor' that is invoked in order to generate the code
   that appears within the `case' corresponding to a successful decode of the
   value contained in the collapsed union.  Basically, it simply resumes
   inlining. */
   
struct cuinl_success_functor : public functor
{
	virtual void func(mu_state *must);

	inline_state *ist;
	mint_ref case_itype;
	pres_c_inline case_inl;
};

void cuinl_success_functor::func(mu_state *must)
{
	must->mu_inline(ist, case_itype, case_inl);
}

/***/

/* The class of the `functor' that is invoked in order to generate the code
   that appears within the `case' corresponding to a successful decode of the
   value contained in the collapsed union.  A failure functor emits a statement
   that indicates a decoding error. */
   
struct cuinl_failure_functor : public functor
{
	virtual void func(mu_state *must);
};

void cuinl_failure_functor::func(mu_state *must)
{
	/* NOTE: Breaking a glob here would cause an excessive
	         number of globs in the common case.  It is not
	         necessary since the error macros have *no* 
	         assumptions about the marshal/unmarshal state. */
		
	must->add_stmt(must->make_error(FLICK_ERROR_COLLAPSED_UNION));
}

/***/

/* A `cuinl_case_functor' is a "wrapper" around another functor.  It invokes
   `mu_union_case' (so that it may do alignment, bookkeepping, or whatever),
   and `mu_union_case' subsequently invokes the "wrapped" functor.
   
   This class is used to "wrap" `cuinl_{success,faliure}_functor's.
   */
   
struct cuinl_case_functor : public functor
{
	virtual void func(mu_state *must);

	functor *fun;
	mint_const discrim_val;
};

void cuinl_case_functor::func(mu_state *must)
{
	must->mu_union_case(fun);
}

/***/

/* The class of the `functor' that is invoked by `mu_union' in order to
   generate discriminating code. */
   
struct cuinl_discriminate_functor : public functor
{
	virtual void func(mu_state *must);
	
	mint_const discrim_val;
	mint_ref discrim_itype;
	cuinl_success_functor success;
};

void cuinl_discriminate_functor::func(mu_state *must)
{
	cuinl_case_functor success_case;
	functor *success_array[1];
	cuinl_case_functor failure_case;
	cuinl_failure_functor failure;
	
	/* Create the `case functors' for success and failure. */
	success_case.fun = &success;
	success_case.discrim_val = discrim_val;
	
	success_array[0] = &success_case;
	
	failure_case.fun = &failure;
	failure_case.discrim_val = discrim_val;
	
	/* `mu_discriminate' will generate the `switch', call the functors,
	   and thereby generate code. */
	must->mu_discriminate(&discrim_val, discrim_itype, 1,
			      success_array, &failure_case, 0, 0);
}

/*****************************************************************************/

void mu_state::mu_inline_collapsed_union(inline_state *ist,
					 mint_ref itype, pres_c_inline inl)
{
	assert(itype >= 0);
	assert(itype < (signed int) pres->mint.defs.defs_len);
	
	mint_def *def = &pres->mint.defs.defs_val[itype];
	assert(def->kind == MINT_UNION);
	mint_union_def *udef = &def->mint_def_u.union_def;
	
	mint_ref case_itype = itype;
	pres_c_inline case_inl = inl;
	/* Find next level in the collapsed union */
	descend_collapsed_union(pres, &case_itype, &case_inl);
	
	mint_const discrim_val = inl->pres_c_inline_u_u.
				 collapsed_union.discrim_val;
	
	/* Fill out our functor and call `mu_union' --- simple! */
	cuinl_discriminate_functor discrim_functor;
	
	discrim_functor.discrim_val = discrim_val;
	discrim_functor.discrim_itype = udef->discrim;
	discrim_functor.success.ist = ist;
	discrim_functor.success.case_itype = case_itype;
	discrim_functor.success.case_inl = case_inl;
	
	mu_union(&discrim_functor);
		
	/* Below is a synopsis of how our `mu_union' statement works.
	   First, consider what happens for MUST_DECODE:
	   
	   `mu_union' does whatever it needs to in order to prepare us to
	   generate union-discriminating code.  It then invokes the
	   `discrim_functor', which will call `mu_discriminate'.
	   
	   `mu_discriminate' calls code that unmarshals the union discriminator
	   and then generates a `switch' statement.  For us, it will generate a
	   `switch' with two cases: One matching `discrim_val' and one default
	   case.
	   
	   For the case matching `discrim_val', a `cuinl_case_functor' will be
	   invoked to generate the code to appear within that case.  That
	   functor takes us to `mu_union_case', which prepares us to generate
	   the body of a case for a variant of a discriminated union.
	   `mu_union_case' in turn invokes the functor given to it --- in this
	   case, `discrim_functor.success'.  Finally, that functor gets back to
	   producing the code that will be executed when `discrim_val' is
	   unmarshaled and recognized.
	   
	   For the default case of our `switch', a different
	   `cuinl_case_functor' will be invoked.  That functor will take us
	   through `mu_union_case' again (this time, for the default case), and
	   `mu_union_case' will invoke the functor that was given to it: a
	   `cuinl_failure_functor'.  That functor simply outputs a statement to
	   handle the decoding error.
	   
	   That's what happens when we're decoding.  When we're encoding (op &
	   MUST_ENCODE), things are a little different.
	   
	   `mu_union' invokes `mu_discriminate', but `mu_discriminate' does not
	   generate a `switch' statement.  Rather, it calls `mu_encode_const'
	   to encode the `discrim_val', and then `mu_discriminate' calls the
	   `cuinl_case_functor' for that case.
	   
	   The `cuinl_case_functor' calls `mu_union_case', which in turn
	   invokes the functor it is given: namely, `discrim_functor.success'.
	   That functor will start inlining code as explained above.  This
	   time, however, the code won't appear within a `case' of a `switch':
	   It will simply appear after the code that marshals `discrim_val'.
	   
	   Because we're encoding, we can't fail to encode the value we expect
	   (`discrim_val').  Therefore, we don't need to invoke a
	   `cuinl_failure_functor' in order to generate error handling code.
	   */
}

