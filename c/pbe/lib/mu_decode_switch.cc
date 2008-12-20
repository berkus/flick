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

#include <assert.h>
#include <string.h>
#include <mom/compiler.h>
#include <mom/libmint.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>
#include <mom/c/pbe.hh>

/* First, declare all the functors that we need in order to generate our
   discrimination code.  Higher-order programming in C++ is a pain. */

/*****************************************************************************/

/* This functor is used by `mu_decode_switch' when it must generate the body of
   a `case' that leads to only one possible server function.  When invoked,
   this functor calls `mu_server_func' to generate the decode-request, invoke,
   and encode-reply code for a single server work function.
   
   Note that `mu_server_func' may still have to descened through several levels
   of collapsed unions --- `mu_decode_switch' doesn't necessarily have to
   descend though all the collapsed unions in order to narrow the set of
   possible server functions down to one.  Any leftover collapsed unions will
   be handled later on by `mu_inline_collapsed_union'. */
   
struct decode_switch_unary_functor : public functor
{
	virtual void func(mu_state *must);
	
	decode_switch_unary_functor(decode_switch_case *dc, mint_ref mr) :
		decode_case(dc), mint_r(mr) {}
	
	decode_switch_case *decode_case;
	mint_ref mint_r;
};

void decode_switch_unary_functor::func(mu_state *must)
{
	switch(decode_case->func->kind) {
	case PRES_C_SERVER_FUNC:
		must->mu_server_func(decode_case->inl,
				     mint_r,
				     &decode_case->func->pres_c_func_u.sfunc,
				     decode_case->skel);
		break;
	case PRES_C_RECEIVE_FUNC:
		must->mu_receive_func(decode_case->inl,
				      mint_r,
				      &decode_case->func->pres_c_func_u.rfunc,
				      decode_case->skel);
		break;
	default:
		panic("In `decode_switch_unary_functor::func()', "
		      "unknown pres_c_func_kind %d.",
		      decode_case->func->kind);
	}
}

/*****************************************************************************/

/* This functor is used by `mu_decode_switch' when it must generate the body of
   a `case' that leads to more than one server function.  Not surprisingly,
   this functor makes a recursive call to `mu_decode_switch'. */

struct decode_switch_nary_functor : public functor
{
	virtual void func(mu_state *must);
	
	decode_switch_nary_functor(decode_switch_case *dc, int dcl,
				   mint_ref mr) :
		decode_cases(dc), decode_cases_len(dcl), mint_r(mr) {}
	
	decode_switch_case *decode_cases;
	int decode_cases_len;
	mint_ref mint_r;
};

void decode_switch_nary_functor::func (mu_state *must)
{
	must->mu_decode_switch(decode_cases, decode_cases_len, mint_r);
}

/*****************************************************************************/

/* This functor is used by `decode_switch' when it must generate the body of a
   default `case' within a `switch'.  The default case may lead to a (single)
   default server function, or it may lead to an error. */

struct decode_switch_default_functor : public functor
{
	virtual void func(mu_state *must);
	
	decode_switch_default_functor(decode_switch_case *dc, mint_ref mr) :
		decode_case(dc), mint_r(mr) {}
	
	decode_switch_case *decode_case;
	mint_ref mint_r;
};

void decode_switch_default_functor::func(mu_state *must)
{
	if (decode_case != 0)
		switch(decode_case->func->kind) {
		case PRES_C_SERVER_FUNC:
			must->mu_server_func(decode_case->inl,
					     mint_r,
					     &(decode_case->func->
					       pres_c_func_u.sfunc),
					     decode_case->skel);
			break;
		case PRES_C_RECEIVE_FUNC:
			must->mu_receive_func(decode_case->inl,
					      mint_r,
					      &(decode_case->func->
						pres_c_func_u.rfunc),
					      decode_case->skel);
			break;
		default:
			panic("In `decode_switch_default_functor::func()', "
			      "unknown pres_c_func_kind %d.",
			      decode_case->func->kind);
		}
	else {
		/* NOTE: Breaking a glob here would cause an excessive
		         number of globs in the common case.  It is not
			 necessary since the error macros have *no* 
			 assumptions about the marshal/unmarshal state. */
		
		must->add_stmt(must->make_error(FLICK_ERROR_DECODE_SWITCH));
	}       
}

/*****************************************************************************/

/* This functor is used to "wrap" another functor within a call to
   `mu_union_case'.  The `mu_union_case' method must be invoked before each
   `case' of a union-decoding `switch' is generated.  `mu_union_case' may do
   alginment, bookkeepping, or whatever.  It then invokes a functor to generate
   the actual body of the current case.
   
   In the code below, we wrap all `unary', `nary', and `default' functors
   (defined above) within these `case' functors.  A case functor simply invokes
   `mu_union_case', which in turn invokes the wrapped functor. */

struct decode_switch_case_functor : public functor
{
	virtual void func(mu_state *must);
	
	decode_switch_case_functor(functor *f, mint_const dv) :
		fun(f),
		discrim_val(dv) {};
	
	functor *fun;
	mint_const discrim_val;
};

void decode_switch_case_functor::func(mu_state *must)
{
	must->mu_union_case(fun);
}

/*****************************************************************************/

/* Finally, the "top level" functor.  This functor is passed to `mu_union'.
   `mu_union' does alignment, bookeeping, or whatever, and then invokes this
   functor in order to generate the code that will decode a discriminated
   union.  This functor simply calls `mu_discriminate' to do the job. */

struct decode_switch_discriminate_functor : public functor
{
	virtual void func(mu_state *must);
	
	decode_switch_discriminate_functor(mint_const *gd, mint_ref gdi,
					   int gdl,
					   functor **gf, functor *df) :
		group_discrims(gd),
		group_discrims_itype(gdi),
		group_discrims_len(gdl),
		group_functors(gf),
		default_functor(df) {}
	
	mint_const *group_discrims;
	mint_ref group_discrims_itype;
	int group_discrims_len;
	functor **group_functors;
	functor *default_functor;
};

void decode_switch_discriminate_functor::func(mu_state *must)
{
	must->mu_discriminate(group_discrims, group_discrims_itype,
			      group_discrims_len,
			      group_functors, default_functor,
			      0, 0 /* unmarshal the discriminator */);
}

/*****************************************************************************/

/* This is primarily used for creating server stubs which have to decode
   incoming request messages and dispatch to the appropriate server work
   function.  However, it could also be used in other situations, such as (on
   the client side) decoding exception messages returned from the server.

   The basic operation of this routine is as follows.
   
   The caller of this method creates an array of `decode_switch_case' structs,
   one for each case that we want to decode to.  For example, when generating
   decoding code for a server stub, there will initially be one
   `decode_switch_case' for each server work function.

   The on-the-network type of the message being decoded, which is the same for
   all cases, is provided to this routine in the parameter `union_mint_ref'.
   However, the presented (C) type is different for each case.  Therefore, each
   `decode_switch_case' contains a different `pres_c_inline', one for each
   particular way to unmarshal the message into C data.  Generally, only one of
   these cases can apply to a given message, because the `pres_c_inline' for
   each case starts with one or more PRES_C_INLINE_COLLAPSED_UNION nodes which
   narrow the applicability of that case to a particular subtree of the MINT
   message type graph in question.

   The "topmost" message type node indicated by `union_mint_ref' must be a
   MINT_UNION type.  For each possible discriminator value in that union, this
   routine searches through the `decode_cases' array for cases with
   PRES_C_INLINE_COLLAPSED_UNION nodes matching that discriminator value.
   There may be more than one, because further decoding may still be needed for
   sub-unions.  All of the matching `decode_cases' are collected into a group
   (`group_decode_cases'), and this group is associated with a functor that
   will be invoked to produce the C code to handle the members of the group.
   
   For groups of size one, the functor will be a `unary' functor.  When
   invoked, this functor leads to `mu_server_func', which produces all of the
   decode-request, invoke work function, and encode-reply for the single server
   work function in the group.
   
   For groups of more than one, the functor will be an `nary' functor.  When
   invoked, an `nary' functor makes a recursive call to `mu_decode_switch',
   which will further discriminate the members of the group.
   
   This function preserves the contents of the passed `decode_cases' array.
   Thus, the function can be called more than once on the same array if
   necessary.
   */

void mu_state::mu_decode_switch(decode_switch_case *decode_cases,
				int decode_cases_len,
				mint_ref union_mint_ref)
{
	/* Stuff about the union discriminator. */
	mint_ref discrim_itype;
	
	/* Stuff about the division of all cases into groups. */
	const int NO_GROUP = -1;
	int number_of_groups = 0;
	int *decode_case_groups = new int[decode_cases_len];
	
	mint_const *group_discrims = new mint_const[decode_cases_len];
	functor **group_functors = new functor *[decode_cases_len];
	
	/* Stuff about the group under consideration. */
	int this_group;
	int group_size;
	int found_count;
	decode_switch_case *group_decode_cases;
	mint_ref group_union_mint_ref;
	
	/* Stuff about the default case. */
	decode_switch_case *default_case;
	functor *default_functor;
	
	/* Loop variables. */
	int i, j;
	
	/*****/
	
	assert(op & MUST_DECODE);
	
	/* If there is nothing to decode, stop here. */
	if (decode_cases_len == 0)
		return;
	
	/*
	 * Initialize `discrim_itype'.  Actually, `discrim_itype' isn't really
	 * used, because `mu_discriminate' only needs the discriminator's MINT
	 * type when (op & MUST_ENCODE).  Still, this might change, so we
	 * compute the correct `discrim_itype'.
	 */
	assert(pres->mint.defs.defs_val[union_mint_ref].kind == MINT_UNION);
	discrim_itype = (pres->
			 mint.defs.defs_val[union_mint_ref].
			 mint_def_u.union_def.discrim);
	
	/* Initialize the `decode_case_groups' array. */
	for (i = 0; i < decode_cases_len; ++i)
		decode_case_groups[i] = NO_GROUP;
	
	/* Divide the `decode_cases' array elements into groups based on their
	   PRES_C_INLINE_COLLAPSED_UNION discriminator values. */
	for (i = 0; i < decode_cases_len; i++) {
		/* If this `decode_case' is already in a group, skip it. */
		if (decode_case_groups[i] != NO_GROUP)
			continue;
		/* If this `decode_case' does not have a collapsed union
		   presentation, skip it. */
		if (decode_cases[i].inl->kind != PRES_C_INLINE_COLLAPSED_UNION)
			continue;
		
		/* Otherwise, we've found a PRES_C_INLINE_COLLAPSED_UNION that
		   isn't yet in any group.  Make a new group and mark the
		   current case as a member of that group. */
		this_group = number_of_groups;
		++number_of_groups;
		decode_case_groups[i] = this_group;
		
		/* Remember the discriminator value that is associated with
		   this group.  All members of this group will have the same
		   discriminator value. */
		group_discrims[this_group] =
			decode_cases[i].inl->pres_c_inline_u_u.collapsed_union.
			discrim_val;
		
		/* Search through the remaining cases to find the
		   PRES_C_INLINE_COLLAPSED_UNIONs that share the current
		   case's discriminator value.  Mark those cases as members of
		   the current group. */
		group_size = 1;
		for (j = i + 1; j < decode_cases_len; ++j) {
			if (decode_case_groups[j] != NO_GROUP)
				continue;
			if (decode_cases[j].inl->kind !=
			    PRES_C_INLINE_COLLAPSED_UNION)
				continue;
			
			if (mint_const_cmp(group_discrims[this_group],
					   (decode_cases[j].inl->
					    pres_c_inline_u_u.collapsed_union.
					    discrim_val))
			    == 0) {
				/* `decode_cases[j]' has the same discriminator
				   value, so add it to the current group. */
				decode_case_groups[j] = this_group;
				++group_size;
			}
		}
		
		/* Find the things that we will need in order to generate the
		   body of the `case' statement that will correspond to this
		   group.  These things include (1) an array containing the
		   members of this group, and (2) a reference to the MINT type
		   that describes the variant of the union corresponding to
		   this group. */
		group_decode_cases = new decode_switch_case[group_size];
		for (j = i, found_count = 0;
		     ((found_count < group_size) && (j < decode_cases_len));
		     ++j)
			if (decode_case_groups[j] == this_group) {
				/* Copy `decode_cases[j]'... */
				group_decode_cases[found_count] =
					decode_cases[j];
				/* ...but change the inline to be the inline
				   that describes the selected case. */
				group_decode_cases[found_count].inl =
					decode_cases[j].inl->
					pres_c_inline_u_u.
					collapsed_union.selected_case;
				++found_count;
			}
		if (found_count != group_size)
			panic("Didn't find all the group members!");
		
		group_union_mint_ref =
			mint_find_union_case(&(pres->mint),
					     union_mint_ref,
					     group_discrims[this_group]);
		assert(group_union_mint_ref != mint_ref_null);
		
		/* Now we can associate a functor with this group.  If this
		   group has only one member, a `decode_switch_unary_functor'
		   will build the body of the `case' statement for this group.
		   Otherwise, the group has more than one member, and we need
		   an `decode_switch_nary_functor' to produce the next level of
		   discrimination code. */
		if (group_size == 1)
			group_functors[this_group] =
				new decode_switch_unary_functor(
					group_decode_cases,
					group_union_mint_ref);
		else
			group_functors[this_group] =
				new decode_switch_nary_functor(
					group_decode_cases,
					group_size,
					group_union_mint_ref);
		/* We must "wrap" the functor that we just created inside a
		   functor that calls `mu_union_case' first. */
		group_functors[this_group] =
			new decode_switch_case_functor(
				group_functors[this_group],
				group_discrims[this_group]);
	}
	
	/* Now determine the default case for the `switch' statement that will
	   be generated.  After filtering out all of the collapsed union cases,
	   there may at most one leftover case in `decode_cases'.  This will be
	   our default case --- it will be used if the discriminator doesn't
	   match any of the values specified by the collapsed unions. */
	default_case = 0;
	for (i = 0; i < decode_cases_len; ++i) {
		if (decode_case_groups[i] != NO_GROUP)
			continue;
		if (default_case != 0)
			panic("More than one default case was found.");
		
		default_case = new decode_switch_case;
		/* Copy `decode_cases[i]'. */
		*default_case = decode_cases[i];
		/* XXX --- Do we need to change its inline? */
	};
	default_functor =
		new decode_switch_default_functor(default_case,
						  (pres->
						   mint.defs.defs_val[
							   union_mint_ref].
						   mint_def_u.union_def.dfault)
			);
	/* Again, we must "wrap" our default functor inside a functor that
           calls `mu_union_case'. */
	default_functor =
		new decode_switch_case_functor(
			default_functor,
			/* XXX --- A default case doesn't have a constant
			   discriminator value, but the Mach3MIG PBE insists on
			   checking the type of the discriminator in its custom
			   version of `mu_union_case'!  Nothing cares about the
			   actual *value*, however, so we cheat. */
			group_discrims[0]);
	
	/* Finally, generate the big `switch' statement.
	   
	   `mu_union' will do any required pre-processing and then invoke our
	   `decode_switch_discriminate_functor' to generate the actual code.
	   That functor calls `mu_discriminate', which in turn calls the
	   functors for each case to be generated (the `group_functors' and the
	   `default_functor').  These functors all lead to `mu_union_case', and
	   from there to the `unary' and `nary' functors created above.
	   
	   It's really quite straightforward, once you can follow the functors.
	   
	   For a longer synopsis of this code-generation process, read the
	   comments in `mu_inline_collapsed_union'.  That method implements a
	   process like the one implemented here and uses similar functors.
	   Moreover, it contains a big comment describing the whole scheme.
	   */
	mu_union(new decode_switch_discriminate_functor(group_discrims,
							discrim_itype,
							number_of_groups,
							group_functors,
							default_functor));
	
	/* XXX --- Cleanup: Free memory, etc. */
}

/* End of file. */

