/*
 * Copyright (c) 1995, 1996, 1997, 1999 The University of Utah and
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


/*
  This functor is used to build the switch statement.
  mu_inline_virtual_union has already read/written the discriminator.
*/
struct mu_inline_virtual_union_functor : public functor
{
	virtual void func(mu_state *must);
	mu_inline_virtual_union_functor(inline_state *ist_in,
					mint_ref union_itype_in,
					pres_c_inline inl) :
		ist(ist_in),
		union_itype(union_itype_in)
	{
		assert (inl->kind == PRES_C_INLINE_VIRTUAL_UNION);
		vuinl = &(inl->pres_c_inline_u_u.virtual_union);
		discrim_expr = 0;
		discrim_type = 0;
	}
	
	inline_state *ist;
	mint_ref union_itype;
	pres_c_inline_virtual_union *vuinl;
	cast_expr discrim_expr;
	cast_type discrim_type;
};

// This functor is used to build the contents of a case statement,
struct vu_case_functor : public functor
{
	virtual void func(mu_state *must);
	vu_case_functor() {}
	
	mint_ref slot_itype;
	pres_c_inline slot_inl;
	inline_state *ist;
};

void vu_case_functor::func(mu_state *must)
{
	must->mu_inline(ist, slot_itype, slot_inl);
	// Break the open chunk
	must->break_chunk();
}

struct mu_inline_virtual_union_error_functor : public functor
{
	virtual void func(mu_state *must);
};

void mu_inline_virtual_union_error_functor::func(mu_state *must)
{
	/* NOTE: Breaking a glob here would cause an excessive
	         number of globs in the common case.  It is not
		 necessary since the error macros have *no* 
		 assumptions about the marshal/unmarshal state. */
		
	must->add_stmt(must->make_error(FLICK_ERROR_VIRTUAL_UNION));
	/* Break any chunk that may be open at this point. */
	must->break_chunk();
}

static void do_vu_case(pres_c_inline_virtual_union_case c,
		       mint_ref itype,
		       inline_state *ist,
		       mu_state *must)
{
	vu_case_functor case_functor;
	case_functor.slot_itype = itype;
	case_functor.slot_inl = c;
	case_functor.ist = ist;
	
	must->mu_union_case(&case_functor);
}
		

void
mu_inline_virtual_union_functor::func(mu_state *must) 
{
	int i;
	mint_ref val_itype;
	mint_const discrim_val;
	mint_union_def *udef;
	mu_memory_allocator_state *enter_case_state;
	cast_stmt saved_c_block, switch_body_c_block, case_c_block;
	
	/* Since there is no other code that can execute alongside a
	   virtual union we don't need to rollback through each of the
	   cases, but we do need to make separate blocks for each case... */
	struct mu_abort_block *mab_par, *mab_con, *mab_thr;
	
	assert(must->pres->mint.defs.defs_val[union_itype].kind == MINT_UNION);
	udef = &(must->
		 pres->mint.defs.defs_val[union_itype].mint_def_u.union_def);
	
	// Break the chunk, we'll be dealing with data of different sizes
	// Save the state so we can restore for each case possible
	must->break_chunk();
	enter_case_state = must->memory_allocator_state();
	
	mab_par = must->abort_block;
	mab_con = new mu_abort_block;
	mab_con->set_kind(MABK_CONTROL);
	mab_con->begin();
	
	mu_msg_span *union_span = 0, *parent_span = 0;
	// Save the current block of code
	// we'll add a 'switch' statement after we've built our cases
	saved_c_block = must->c_block;
	must->c_block = cast_new_block(0,0);
	if( must->current_span ) {
		union_span = new mu_msg_span;
		union_span->set_kind(MSK_UNION);
		union_span->set_block(must->c_block);
		union_span->set_abort(must->abort_block);
		union_span->begin();
		parent_span = must->current_span;
	}
	for (i = 0; i < (signed int)vuinl->cases.cases_len; i++) {
		mab_thr = new mu_abort_block();
		must->abort_block = mab_thr;
		mab_thr->set_kind(MABK_THREAD);
		mab_thr->begin();
		
		/* Add a span for each case */
		if( must->current_span ) {
			must->current_span = new mu_msg_span;
			must->current_span->set_kind(MSK_SEQUENTIAL);
			must->current_span->set_block(must->c_block);
			must->current_span->set_abort(must->abort_block);
			must->current_span->begin();
		}
		// Save the body of the 'switch' statment we've built so far
		switch_body_c_block = must->c_block;
		
		// Start a new block for this case
		must->c_block = cast_new_block(0,0);
		
		// Reset the memory state to
		// the one we saved before we starte the cases
		must->set_memory_allocator_state(enter_case_state);
		
		// Get the itype for the value selected
		val_itype = udef->cases.cases_val[i].var;
		
		// Create a CAST version of the selector for this case.
		discrim_val = udef->cases.cases_val[i].val;
		cast_expr case_value_expr = make_case_value_expr(discrim_val);
		
		// now build the individual 'case' statement for this case
		do_vu_case(vuinl->cases.cases_val[i], val_itype, ist, must);
		
		// Append a break & add it to the switch statement body
		must->add_stmt(cast_new_break());
		mab_thr->end();
		mab_con->add_child(mab_thr, 0);
		must->add_stmt(mab_thr->get_block_label());
		
		if( parent_span ) {
			must->current_span->end();
			union_span->add_child(must->current_span);
		}
		case_c_block = must->c_block;
		must->c_block = switch_body_c_block;
		
		must->add_stmt(cast_new_case(case_value_expr, case_c_block));
	}
	
	// Handle the default case
	if( must->current_span ) {
		must->current_span = new mu_msg_span;
		must->current_span->set_kind(MSK_SEQUENTIAL);
		must->current_span->set_block(must->c_block);
		must->current_span->set_abort(must->abort_block);
		must->current_span->begin();
	}
	switch_body_c_block = must->c_block;
	must->c_block = cast_new_block(0,0);
	mab_thr = new mu_abort_block();
	must->abort_block = mab_thr;
	mab_thr->set_kind(MABK_THREAD);
	mab_thr->begin();
	must->set_memory_allocator_state(enter_case_state);
	
	if (vuinl->dfault) {
		assert(udef->dfault != -1);
		
		// Get the itype for the value selected
		val_itype = udef->dfault;
		
		// now build the individual 'case' statement for this case
		// - XXX - notice the borrowing of val
		do_vu_case(vuinl->dfault, val_itype, ist, must);
	} else {
		// No default case - it's an error -
		// XXX - notice the borrowing of val
		mu_inline_virtual_union_error_functor error_functor;
		must->mu_union_case(&error_functor);
	}
		
	// add the default to the switch statement body
	must->add_stmt(cast_new_break());
	if( parent_span ) {
		must->current_span->end();
		union_span->add_child(must->current_span);
		union_span->end();
		parent_span->add_child(union_span);
		must->current_span = parent_span;
	}
	
	mab_thr->end();
	mab_con->add_child(mab_thr, 0);
	must->add_stmt(mab_thr->get_block_label());
	
	mab_con->end();
	mab_par->add_child(mab_con, 0);
	must->abort_block = mab_par;
	case_c_block = must->c_block;
	must->c_block = switch_body_c_block;
	must->add_stmt(cast_new_default(case_c_block));
	
	// Finally produce the completed switch statement
	switch_body_c_block = must->c_block;
	must->c_block = saved_c_block;
	must->add_stmt(cast_new_switch(discrim_expr, switch_body_c_block));
	
	delete enter_case_state;
}

/***************************************************************************/
/* Finally, the `mu_state' method that sets the code generation in motion. */

void mu_state::mu_inline_virtual_union(inline_state *ist,
				       mint_ref union_itype,
				       pres_c_inline inl)
{
	pres_c_inline_virtual_union *vuinl;
	mint_union_def *udef;
	
	mu_inline_virtual_union_functor f(ist, union_itype, inl);
	
	/*****/
	
	assert(inl->kind == PRES_C_INLINE_VIRTUAL_UNION);
	vuinl = &(inl->pres_c_inline_u_u.virtual_union);
	
	/* Find the itype and make sure it matches. */
	assert(pres->mint.defs.defs_val[union_itype].kind == MINT_UNION);
	udef = &(pres->mint.defs.defs_val[union_itype].mint_def_u.union_def);
	
	/* Marshal or unmarshal the discriminator.
	   We also have to find and save the discriminator expression. */
	mu_state_arglist *oldlist = arglist;
	arglist = new mu_state_arglist(vuinl->arglist_name, oldlist);
	arglist->add(vuinl->arglist_name, "discrim");
	mu_inline(ist, udef->discrim, vuinl->discrim);
	int gotarg = arglist->getargs(vuinl->arglist_name, "discrim",
				      &f.discrim_expr, &f.discrim_type);
	assert(gotarg);
	assert(f.discrim_expr);
	assert(f.discrim_type);
	delete arglist;
	arglist = oldlist;
	
	/* Finally, let `mu_union' and our functor generate the big `switch'
	   statement. */
	mu_union(&f);
}

/* End of file. */

