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

#include <mom/libmint.h>
#include <mom/c/libcast.h>
#include <mom/c/pbe.hh>

/* This function is an auxiliary for `mu_inline_struct_union_functor::func',
   below.  Given a MINT constant, this function creates and returns an
   equivalent CAST expression. */
cast_expr make_case_value_expr(mint_const mint_literal)
{
	cast_expr cast_literal;
	
	switch (mint_literal->kind) {
	case MINT_CONST_INT:
		switch (mint_literal->mint_const_u_u.const_int.kind) {
		case MINT_CONST_LITERAL:
			cast_literal =
				cast_new_expr_lit_int((mint_literal->
						       mint_const_u_u.
						       const_int.
						       mint_const_int_u_u.
						       value),
						      0);
			break;
		case MINT_CONST_SYMBOLIC:
			cast_literal =
				cast_new_expr_name(mint_literal->
						   mint_const_u_u.const_int.
						   mint_const_int_u_u.name);
			break;
		default:
			panic("Unknown MINT constant category.");
			break;
		}
		break;
		
	case MINT_CONST_CHAR:
		switch (mint_literal->mint_const_u_u.const_char.kind) {
		case MINT_CONST_LITERAL:
			cast_literal =
				cast_new_expr_lit_char((mint_literal->
							mint_const_u_u.
							const_char.
							mint_const_char_u_u.
							value),
						       0);
			break;
		case MINT_CONST_SYMBOLIC:
			cast_literal =
				cast_new_expr_name(mint_literal->
						   mint_const_u_u.const_char.
						   mint_const_char_u_u.name);
			break;
		default:
			panic("Unknown MINT constant category.");
			break;
		}
		break;
		
	default:
		panic("MINT constant type %d not supported as discrimintor.",
		      mint_literal->kind);
	}
	
	return cast_literal;
}

/*****************************************************************************/

/* This functor is called by `mu_union_case' to generate the body of a `case'
   within a `switch' that marshals/unmarshals a single variant of a "struct
   union."
   
   The marshaled/unmarshaled data is/will be stored in the location named by
   `slot_expr' (a CAST expression that names a member of the CAST union).  The
   type of that slot is described by `slot_ctype'.  The MINT type of the data
   is described by `slot_itype', and `slot_mapping' describes the association
   between the CAST and the MINT representations of the data to be marshaled or
   unmarshaled.
   */

struct mu_inline_struct_union_case_functor : public functor
{
	virtual void func(mu_state *must);
	
	/*****/
	
	mu_inline_struct_union_case_functor() {}
	mu_inline_struct_union_case_functor(cast_expr expr,
					    cast_type ctype,
					    mint_ref itype,
					    pres_c_mapping mapping) :
		slot_expr(expr),
		slot_ctype(ctype),
		slot_itype(itype),
		slot_mapping(mapping) {}
	
	/*****/
	
	cast_expr slot_expr;
	cast_type slot_ctype;
	mint_ref slot_itype;
	pres_c_mapping slot_mapping;
};

void mu_inline_struct_union_case_functor::func(mu_state *must)
{
	must->mu_mapping(slot_expr, slot_ctype, slot_itype, slot_mapping);
	/* Break any chunk that may be open at this point. */
	must->break_chunk();
}

/*****************************************************************************/

/* This functor is called by `mu_union_case' to generate the body of a `case'
   within a `switch' that marshals/unmarshals a single variant of a "struct
   union."  This functor is different the previous one, however, in that this
   functor *discards* the marshaled/unmarshaled data.  Void union variants are
   handled by this functor. */

struct mu_inline_struct_union_discard_functor : public functor
{
	virtual void func(mu_state *must);
	
	/*****/
	
	mu_inline_struct_union_discard_functor() {}
	mu_inline_struct_union_discard_functor(mint_ref itype,
					       pres_c_mapping mapping) :
		discard_itype(itype),
		discard_mapping(mapping) {}
	
	/*****/
	
	mint_ref discard_itype;
	pres_c_mapping discard_mapping;
};

void mu_inline_struct_union_discard_functor::func(mu_state *must)
{
	/* I suppose that in general, we should be able to discard any kind
	   of data.  But currently, when we have nowhere to store the data,
	   `mu_mapping' works only when we are discarding a `void' (MINT_VOID)
	   via a direct mapping (PRES_C_MAPPING_DIRECT). */
	mint_def *discard_def = &(must->
				  pres->mint.defs.defs_val[discard_itype]);
	
	if (discard_def->kind != MINT_VOID)
		panic("Cannot yet discard a non-void union variant.");
	
	/* Rather than risk our luck by calling `mu_mapping' with NULL
	   pointers, just do nothing.  When we can discard arbitrary data this
	   should be changed. */
	/* must->mu_mapping(0, 0, discard_itype, discard_mapping); */
	
	/* Break any chunk that may be open at this point. */
	must->break_chunk();
}

/*****************************************************************************/

/* This functor is called by `mu_union_case' to generate the body of an error-
   producing default `case' within a `switch' that marshals/unmarshals a
   "struct union." */

void mu_inline_struct_union_error_functor::func(mu_state *must)
{
	/* NOTE: Breaking a glob here would cause an excessive
	         number of globs in the common case.  It is not
		 necessary since the error macros have *no* 
		 assumptions about the marshal/unmarshal state. */
		
	must->add_stmt(must->make_error(FLICK_ERROR_STRUCT_UNION));
	
	/* Break any chunk that may be open at this point. */
	must->break_chunk();
}

/*****************************************************************************/

/* This functor is called by `mu_union' in order to generate the code that
   marshals/unmarshals the variants of a "struct union."  (The discriminator of
   the "struct union" is handled separately, before this functor is invoked.)
   */

struct mu_inline_struct_union_functor : public functor
{
	virtual void func(mu_state *must);
	
	mu_inline_struct_union_functor(inline_state *ist_in,
				       mint_ref union_itype_in,
				       pres_c_inline inl_in) :
		ist(ist_in),
		union_itype(union_itype_in),
		inl(inl_in) {}
	
	inline_state *ist;
	mint_ref union_itype;
	pres_c_inline inl;
};

/* This method handles a PRES_C_INLINE_STRUCT_UNION inline presentation node,
   which always corresponds to a MINT_UNION node on the interface (itype) side.
   
   This method produces the C `switch' statement that processes the variants of
   the union.  Each case within the switch contains code to marshal/unmarshal a
   particular variant.  Note that by the time we execute this function, the C
   code to marshal or unmarshal the union discriminator has already been output
   by `mu_inline_struct_union'.
   */
void mu_inline_struct_union_functor::func(mu_state *must)
{
	pres_c_inline_struct_union *inlsu;
	mint_union_def *udef;
	int i;
	
	mu_memory_allocator_state *enter_case_state;
	
	mint_const discrim_val;
	cast_expr discrim_expr, union_expr, slot_expr, case_value_expr;
	cast_type discrim_ctype, union_ctype, slot_ctype;
	cast_scoped_name slot_name;
	mint_ref slot_itype;
	int slot_union_ctype_index;
	
	mu_inline_struct_union_case_functor case_functor;
	mu_inline_struct_union_discard_functor discard_functor;
	mu_inline_struct_union_error_functor error_functor;
	
	cast_stmt saved_c_block, switch_body_c_block, case_c_block;
	
	mu_msg_span *union_span = 0, *parent_span = 0;
	
	/*****/
	
	assert(inl->kind == PRES_C_INLINE_STRUCT_UNION);
	inlsu = &(inl->pres_c_inline_u_u.struct_union);
	
	/* Find the itype and make sure it matches. */
	assert(must->pres->mint.defs.defs_val[union_itype].kind == MINT_UNION);
	udef = &(must->
		 pres->mint.defs.defs_val[union_itype].mint_def_u.union_def);
	
	/* At one time, we marshaled/unmarshaled the discriminator here.  But
	   now we process the discriminator in `mu_inline_struct_union' because
	   at that point we are *outside* of `mu_union'.  The union-generation
	   code analyzes memory usage in order to determine if any code path
	   must break a glob.  This analysis is effectively defeated when the
	   processing of the discriminator breaks the glob --- the result in
	   that case is we can't share a single glob for all arms of our
	   `switch'.  By handling the discriminator before we enter `mu_union',
	   `mu_union' can analyze memory usage across the `switch' alone.
	   
	   (How can the discriminator itself cause a glob break?  If it is
	   handled my a marshal/unmarshal stub.  Currently, we always break the
	   current lgob before calling a marshal or unmarshal stub, and the
	   stub always leaves us in a state without a current glob.)
	   
	   We still need to get the `discrim_expr', however, so that we can
	   reference it in our `switch' statement. */
	ist->slot_access(inlsu->discrim.index, &discrim_expr, &discrim_ctype);
	
	/* Break the current chunk and save the state of the glob/chunk memory
	   allocator.  Each arm of our `switch' statement will start in a state
	   that has no current chunk.  (In particular, each `switch' arm will
	   begin in the `enter_case_state'.  Each `switch' arm will result in a
	   state that has no current chunk; this is ensured by the functors we
	   give to `mu_union_case'.  Higher-level glob management is the domain
	   of `mu_union' and `mu_union_case'.) */
	must->break_chunk();
	enter_case_state = must->memory_allocator_state();

	/* If an abort were to occur after this union then we need to
	   try and rollback anything done in the union.  So we construct
	   an abort control block that does a switch and then for each
	   case we make a thread block and then add it to this
	   control block. */
	struct mu_abort_block *mab_par, *mab_con, *mab_thr;
	
	mab_con = new mu_abort_block();
	mab_par = must->abort_block;
	mab_con->set_kind(MABK_CONTROL_SWITCH);
	mab_con->set_expr(discrim_expr);
	mab_con->begin();
	
	/* After marshaling/unmarshaling the discriminator, we must produce a
	   `switch' statement that will take us to the marshal/unmarshal code
	   for the selected variant of the union.  Because many of our methods
	   use `add_stmt' to build code, we must manipulate `c_block' in order
	   to build different pieces of the code. */
	saved_c_block = must->c_block;
	must->c_block = cast_new_block(0, 0);
	
	/* Get `union_expr', which is the CAST expression that accesses the
	   slot containing our union, and `union_ctype', which is the CAST
	   type of that union.  We need these things in order to access the
	   members of the union. */
	ist->slot_access(inlsu->union_index, &union_expr, &union_ctype);
	
	if( must->current_span ) {
		union_span = new mu_msg_span;
		union_span->set_kind(MSK_UNION);
		union_span->set_block(saved_c_block);
		union_span->set_abort(must->abort_block);
		union_span->begin();
		parent_span = must->current_span;
	}
	/* Process the cases. */
	for (i = 0; i < (signed int)udef->cases.cases_len; ++i) {
		/* There is a variance in msg length needed so we
		   need to add a span for each case */
		if( must->current_span ) {
			must->current_span = new mu_msg_span;
			must->current_span->set_kind(MSK_SEQUENTIAL);
			must->current_span->set_block(must->c_block);
			must->current_span->set_abort(must->abort_block);
			must->current_span->begin();
		}
		/* Save the body of the `switch' statement we're generating,
		   and start a new block for this case. */
		switch_body_c_block = must->c_block;
		must->c_block = cast_new_block(0, 0);
		
		/* Reset the state of the glob/chunk management code to the
		   "enter case" state. */
		must->set_memory_allocator_state(enter_case_state);
		
		/* Create a CAST version of the selector for this case. */
		discrim_val = udef->cases.cases_val[i].val;
		case_value_expr = make_case_value_expr(discrim_val);
		
		/* Add the case for this child to the parent block
		   and then add a child to catch the code */
		mab_thr = new mu_abort_block();
		must->abort_block = mab_thr;
		mab_thr->set_kind(MABK_THREAD);
		mab_thr->set_expr(case_value_expr);
		mab_thr->begin();
		
		/* Now determine the things we need to know about the slot
		   of the union that corresponds to this case. */
		/* Determine `slot_itype'. */
		slot_itype = udef->cases.cases_val[i].var;
		
		/* The cases within `inlsu' tell us how the MINT cases in
		   `udef' correspond to the members of the `union_ctype'.
		   For each MINT case, the `inlsu' contains the index of the
		   slot in the `union_ctype' that we should reference.  This
		   index may be -1, which means that the MINT case doesn't map
		   to any member of the CAST union.  This is how void cases
		   are handled. */
		slot_union_ctype_index = inlsu->cases.cases_val[i].index;
		if (slot_union_ctype_index != -1) {
			/* Dig the `slot_name' and `slot_ctype' for this case
			   out of the `union_ctype'. */
			slot_name =
				union_ctype->cast_type_u_u.agg_type.
				scope.cast_scope_val[slot_union_ctype_index].
				name;
			slot_ctype = union_ctype->cast_type_u_u.agg_type.
				scope.cast_scope_val[slot_union_ctype_index].
				u.cast_def_u_u.var_def.type;
			/* Determine `slot_expr'. */
			slot_expr = cast_new_expr_sel(union_expr, slot_name);
			
			/* Next, marshal or unmarshal this case.  This will
			   start a new chunk automatically, since we haven't
			   marshaled/unmarshaled anything since calling
			   `break_chunk'. */
			case_functor.slot_expr = slot_expr;
			case_functor.slot_ctype = slot_ctype;
			case_functor.slot_itype = slot_itype;
			case_functor.slot_mapping = inlsu->
						    cases.cases_val[i].mapping;
			must->mu_union_case(&case_functor);
		} else {
			/* We must invoke `mu_union_case' even for cases that
			   don't map to anything in the CAST union.  Most
			   importantly, this lets `mu_union_case' insert any
			   glob/chunk management code that it needs to add.
			   Secondarily, it gives us a chance to discard junk
			   data in the message.  (Of course, if `slot_itype' is
			   MINT_VOID, there's nothing to discard.) */
			slot_expr = 0;
			discard_functor.discard_itype = slot_itype;
			discard_functor.discard_mapping = inlsu->
							  cases.cases_val[i].
							  mapping;
			must->mu_union_case(&discard_functor);
		}
		
		/* Finally, append a `break' to the block for this case, and
		   add this case to the body of our `switch' statement. */
		must->add_stmt(cast_new_break());
		mab_thr->end();
		mab_con->add_child(mab_thr, MABF_OUT_OF_LINE);
		must->add_stmt(mab_thr->get_block_label());
		case_c_block = must->c_block;
		must->c_block = switch_body_c_block;
		must->add_stmt(cast_new_case(case_value_expr, case_c_block));
		if( parent_span ) {
			must->current_span->end();
			union_span->add_child(must->current_span);
		}
	}
	
	/* Process the default case. */
	switch_body_c_block = must->c_block;
	must->c_block = cast_new_block(0, 0);
	mab_thr = new mu_abort_block();
	must->abort_block = mab_thr;
	mab_thr->set_kind(MABK_THREAD);
	mab_thr->begin();
	must->set_memory_allocator_state(enter_case_state);
	if (inlsu->dfault) {
		/* -1 is the magic value that indictates "no default." */
		assert(udef->dfault != -1);
		
		/* Determine `slot_itype'. */
		slot_itype = udef->dfault;
		
		slot_union_ctype_index = inlsu->dfault->index;
		if (slot_union_ctype_index != -1) {
			if( must->current_span ) {
				must->current_span = new mu_msg_span;
				must->current_span->set_kind(MSK_SEQUENTIAL);
				must->current_span->set_block(must->c_block);
				must->current_span->set_abort(must->
							      abort_block);
				must->current_span->begin();
			}
			/* Dig the `slot_name' and `slot_ctype' for this case
			   out of the `union_ctype'. */
			slot_name =
				union_ctype->cast_type_u_u.agg_type.
				scope.cast_scope_val[slot_union_ctype_index].
				name;
			slot_ctype = union_ctype->cast_type_u_u.agg_type.
				scope.cast_scope_val[slot_union_ctype_index].
				u.cast_def_u_u.var_def.type;
			/* Determine `slot_expr'. */
			slot_expr = cast_new_expr_sel(union_expr, slot_name);
			
			/* Marshal or unmarshal this case. */
			case_functor.slot_expr = slot_expr;
			case_functor.slot_ctype = slot_ctype;
			case_functor.slot_itype = slot_itype;
			case_functor.slot_mapping = inlsu->dfault->mapping;
			must->mu_union_case(&case_functor);
			if( parent_span ) {
				must->current_span->end();
				union_span->add_child(must->current_span);
			}
		} else {
			/* We must invoke `mu_union_case' even for cases that
			   don't map to anything in the CAST union. */
			discard_functor.discard_itype = slot_itype;
			discard_functor.discard_mapping = inlsu->
							  dfault->mapping;
			must->mu_union_case(&discard_functor);
		}
	} else {
		/* There is no default case of our union.  Emit a statement in
		   the default case of our `switch' statement that will handle
		   erroneous discriminator values. */
		must->mu_union_case(&error_functor);
	}
	/* Append a `break' to the block for this case, and add this case to
	   the body of our `switch' statement. */
	if( parent_span ) {
		union_span->end();
		parent_span->add_child(union_span);
		must->current_span = parent_span;
	}
	must->add_stmt(cast_new_break());
	mab_thr->end();
	mab_con->add_child(mab_thr, MABF_OUT_OF_LINE);
	must->add_stmt(mab_thr->get_block_label());
	
	case_c_block = must->c_block;
	must->c_block = switch_body_c_block;
	must->add_stmt(cast_new_default(case_c_block));
	
	mab_con->end();
	must->add_stmt(mab_con->get_block_label());
	mab_par->add_child(mab_con, MABF_OUT_OF_LINE);
	
	/* Produce the complete `switch' statement. */
	switch_body_c_block = must->c_block;
	must->c_block = saved_c_block;
	must->add_stmt(cast_new_switch(discrim_expr, switch_body_c_block));
	
	must->abort_block = mab_par;
	/* Finally, clean up. */
	delete enter_case_state;
}

/*****************************************************************************/

/* Finally, the `mu_state' method that sets the code generation in motion. */

void mu_state::mu_inline_struct_union(inline_state *ist,
				      mint_ref union_itype,
				      pres_c_inline inl)
{
	pres_c_inline_struct_union *inlsu;
	mint_union_def *udef;
	cast_expr discrim_expr;
	cast_type discrim_ctype;
	
	mu_inline_struct_union_functor f(ist, union_itype, inl);
	
	/*****/
	
	assert(inl->kind == PRES_C_INLINE_STRUCT_UNION);
	inlsu = &(inl->pres_c_inline_u_u.struct_union);
	
	/* Find the itype and make sure it matches. */
	assert(pres->mint.defs.defs_val[union_itype].kind == MINT_UNION);
	udef = &(pres->mint.defs.defs_val[union_itype].mint_def_u.union_def);
	
	/* Marshal or unmarshal the discriminator.  The code to do this is
	   similar to `mu_inline_atom', except that the index is determined
	   differently. */
	ist->slot_access(inlsu->discrim.index, &discrim_expr, &discrim_ctype);
	mu_mapping(discrim_expr, discrim_ctype, udef->discrim,
		   inlsu->discrim.mapping);
	
	/* Finally, let `mu_union' and our functor generate the big `switch'
	   statement. */
	mu_union(&f);
}

/* End of file. */

