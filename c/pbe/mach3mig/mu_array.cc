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

#include <assert.h>

#include <mom/idl_id.h>
#include <mom/compiler.h>
#include <mom/libmint.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include "mach3.h"

struct ool_check_functor : public functor
{
	virtual void func(mu_state *must);
	ool_check_functor() {};

	ool_check_functor(cast_expr cond_expr,
			  int e_size,
			  cast_expr arr_expr,
			  cast_type arr_ctype,
			  cast_type el_ctype,
			  mint_ref el_itype,
			  pres_c_mapping el_map,
			  char *c_name) :
		condition_expr(cond_expr),
		elem_size(e_size),
		array_expr(arr_expr),
		array_ctype(arr_ctype),
		elem_ctype(el_ctype),
		elem_itype(el_itype),
		elem_map(el_map),
		cname(c_name)
		{};
	
	cast_expr condition_expr;
	int elem_size;
	cast_expr array_expr;
	cast_type array_ctype;
	cast_type elem_ctype;
	mint_ref elem_itype;
	pres_c_mapping elem_map;
	char *cname;
};

struct ool_case_functor : public functor
{
	virtual void func(mu_state *must);
	ool_case_functor(cast_expr arr_expr,
			 cast_type arr_ctype,
			 cast_type el_ctype,
			 mint_ref el_itype,
			 pres_c_mapping el_map,
			 char *c_name) :
		array_expr(arr_expr),
		array_ctype(arr_ctype),
		elem_ctype(el_ctype),
		elem_itype(el_itype),
		elem_map(el_map),
		cname(c_name)
		{};
	
	cast_expr array_expr;
	cast_type array_ctype;
	cast_type elem_ctype;
	mint_ref elem_itype;
	pres_c_mapping elem_map;
	char *cname;
};

struct inl_case_functor : public functor
{
	virtual void func(mu_state *must);
	inl_case_functor(cast_expr arr_expr,
			 cast_type arr_ctype,
			 cast_type el_ctype,
			 mint_ref el_itype,
			 pres_c_mapping el_map,
			 char *c_name) :
		array_expr(arr_expr),
		array_ctype(arr_ctype),
		elem_ctype(el_ctype),
		elem_itype(el_itype),
		elem_map(el_map),
		cname(c_name)
		{};
	
	cast_expr array_expr;
	cast_type array_ctype;
	cast_type elem_ctype;
	mint_ref elem_itype;
	pres_c_mapping elem_map;
	char *cname;
};

void ool_check_functor::func(mu_state *must)
{
	mu_memory_allocator_state *start_state;
	cast_stmt saved_block, then_block, else_block;
	struct mu_abort_block *mab_par, *mab_con, *mab_thr;
	char *skip_label = must->add_label();
	mu_msg_span *union_span = 0, *parent_span = 0;
	
	must->break_chunk();
	start_state = must->memory_allocator_state();
	
	/*
	 * Set the servercopy flag, if we have one (the ``mustcopy'' argument).
	 */ 
	cast_expr scopy;
	cast_type t;
	int gotarg = must->arglist->getargs(cname, "mustcopy", &scopy, &t);
	assert(gotarg);
	if (scopy)
		must->add_stmt(cast_new_stmt_expr(
			cast_new_expr_assign(scopy,
					     cast_new_unary_expr(
						     CAST_UNARY_LNOT,
						     condition_expr))));
	
	saved_block = must->c_block;
	
	mab_par = must->abort_block;
	mab_con = new mu_abort_block();
	mab_con->set_kind(MABK_CONTROL_IF_ELSE);
	mab_con->begin();
	if( must->current_span ) {
		union_span = new mu_msg_span;
		union_span->set_kind(MSK_UNION);
		union_span->set_block(must->c_block);
		union_span->set_abort(must->abort_block);
		union_span->begin();
		parent_span = must->current_span;
		must->current_span = new mu_msg_span;
		must->current_span->set_kind(MSK_SEQUENTIAL);
		must->current_span->begin();
	}
	mab_thr = new mu_abort_block();
	must->abort_block = mab_thr;
	mab_thr->set_kind(MABK_THREAD);
	mab_thr->set_expr(condition_expr);
	mab_thr->begin();
	
	// Generate the out-of-line branch
	must->c_block = cast_new_block(0, 0);
	ool_case_functor ooln(array_expr, array_ctype,
			      elem_ctype, elem_itype, elem_map, cname);
	must->mu_union_case(&ooln);
	if (must->op & MUST_ENCODE) {
		must->add_stmt(
			cast_new_stmt_expr(cast_new_expr_op_assign(
				CAST_BINARY_BOR,
				cast_new_expr_name(
					"_buf_start->Head.msgh_bits"),
				cast_new_expr_name(
					"MACH_MSGH_BITS_COMPLEX"))));
	}
	
	must->add_stmt(cast_new_goto(skip_label));
	mab_thr->end();
	mab_con->add_child(mab_thr, MABF_OUT_OF_LINE);
	must->add_stmt(mab_thr->get_block_label());
	if( parent_span ) {
		must->current_span->end();
		union_span->add_child(must->current_span);
		must->current_span = new mu_msg_span;
		must->current_span->set_kind(MSK_SEQUENTIAL);
		must->current_span->begin();
	}
	mab_thr = new mu_abort_block();
	must->abort_block = mab_thr;
	mab_thr->set_kind(MABK_THREAD);
	mab_thr->set_expr(0);
	mab_thr->begin();
	if( must->current_span ) {
		must->current_span->end();
		union_span->add_child(must->current_span);
		must->current_span = new mu_msg_span;
		must->current_span->set_kind(MSK_SEQUENTIAL);
		must->current_span->begin();
	}
	
	then_block = must->c_block;
	
	// Generate the in-line branch
	must->set_memory_allocator_state(start_state);
	must->c_block = cast_new_block(0, 0);
	inl_case_functor inln(array_expr, array_ctype,
			      elem_ctype, elem_itype, elem_map, cname);
	must->mu_union_case(&inln);
	
	must->add_stmt(cast_new_goto(skip_label));
	mab_thr->end();
	mab_con->add_child(mab_thr, MABF_OUT_OF_LINE);
	must->add_stmt(mab_thr->get_block_label());
	if( parent_span ) {
		must->current_span->end();
		union_span->add_child(must->current_span);
		union_span->end();
		parent_span->add_child(union_span);
		must->current_span = parent_span;
	}
	must->abort_block = mab_par;
	mab_con->end();
	mab_par->add_child(mab_con, MABF_OUT_OF_LINE);
	mab_par->add_stmt(mab_con->get_block_label());
	
	else_block = must->c_block;
	
	// Now make the if statement
	must->c_block = saved_block;
	must->add_stmt(cast_new_if(condition_expr,
				   then_block,
				   else_block));
	cast_stmt clabel
		= cast_new_label(skip_label, cast_new_stmt(CAST_STMT_EMPTY));
	clabel->cast_stmt_u_u.s_label.users = 2;
	must->add_stmt(clabel);
}

void ool_case_functor::func(mu_state *must)
{
	/*
	 * First, we have to ``fix'' the allocation semantics for this case.
	 * Out-of-line data is never allocated since the MACH kernel will do
	 * that for us.  To do this, we replace the allocation semantics in the
	 * associated allocation context with a tweaked one.
	 */
	mu_inline_alloc_context *iac = must->inline_alloc_context;
	while (iac) {
		if (strcmp(iac->name, cname) == 0)
			break;
		iac = iac->parent_context;
	}
	if (!iac)
		panic(("In `ool_case_functor::func', "
		       "allocation context `%s' not available."), cname);
	
	pres_c_allocation *orig_alloc = iac->alloc;
	pres_c_allocation aalloc = *orig_alloc;
	iac->alloc = &aalloc;
	
	/*
	 * Create a new allocation semantic for sending the data out-of-line.
	 * (The default allocation semantic assumes inline transmission.)
	 *
	 * XXX - This is a terrible hack, since the PG should be able
	 * to express the proper allocation semantic using the
	 * pres_c_allocation structure (including necessary runtime
	 * checks).  Until we have a new allocation representation,
	 * however, we have to munge the allocation here to force
	 * proper allocation.
	 */
	for (int i = 1; i < PRES_C_DIRECTIONS; i++) {
		/* Skip "invalid" allocation cases. */
		if (aalloc.cases[i].allow != PRES_C_ALLOCATION_ALLOW)
			continue;
		
		aalloc.cases[i].pres_c_allocation_u_u.val.allocator.kind
			= PRES_C_ALLOCATOR_OUTOFLINE;
		/*
		 * Never alloc out-of-line pointers; we might dealloc, though.
		 *
		 * XXX - We mark this to never be deallocated, but that is
		 * wrong if the work function isn't aware that this data came
		 * out-of-line (the reason we're in this functor is because it
		 * may be ``optimized'' at runtime to go out-of-line if the
		 * array is large).  The `servercopy' parameter, if it exists,
		 * informs the work function of this fact, and thus passes it
		 * the responsibility for deallocation.  However, if there is
		 * no `servercopy' parameter, we have to treat it like the rest
		 * of the `in' data and free it.  Unfortunatlely, we need to
		 * *EXPLICITLY* free it (the other `in' data is stack-allocated
		 * and automatically freed).
		 */
		aalloc.cases[i].pres_c_allocation_u_u.val.flags
			&= ~PRES_C_ALLOC_EVER;
		aalloc.cases[i].pres_c_allocation_u_u.val.allocator.
			pres_c_allocator_u.ool_name = ir_strlit("mach_vm");
	}
	
	/*
	 * We set our magic Out-of-line data flag, and marshal the data.
	 */
	((mach3_mu_state *)must)->mach3_outofline = 1;
	must->mu_array(array_expr, array_ctype, 
		       elem_ctype, elem_itype, elem_map, cname);
	((mach3_mu_state *)must)->mach3_outofline = 0;
	
	/* Restore the original allocation semantics. */
	iac->alloc = orig_alloc;
}

void inl_case_functor::func(mu_state *must)
{
	/* The inline case is just the normal thing. */
	must->mu_array(array_expr, array_ctype,
		       elem_ctype, elem_itype, elem_map, cname);
}


void mach3_mu_state::mu_array_type_descriptor(
	cast_expr len_expr, int fixed_elts,
	char *prim_macro_name, int longform, int out_of_line,
	cast_expr dealloc_expr, cast_expr port_type, cast_expr oolcheck)
{
	/* Produce a macro invocation to marshal the type descriptor. */
	char *macro_name = flick_asprintf("%s_type", prim_macro_name);
	cast_expr macro_expr = cast_new_expr_name(macro_name);
	cast_expr ofs_expr = cast_new_expr_lit_int(
		chunk_prim(2, longform ? 12 : 4), 0);
	cast_expr num_expr;
	if (fixed_elts > 1) {
		num_expr = cast_new_binary_expr(
			CAST_BINARY_MUL,
			len_expr,
			cast_new_expr_lit_int(fixed_elts, 0));
	} else 
		num_expr = len_expr;
	if (op & MUST_DECODE) {
		if (len_expr->kind == CAST_EXPR_LIT_PRIM) {
			/*
			 * if we are decoding an array, the length MUST be in
			 * a variable in order to decode the type!
			 */
			cast_expr lexpr =
				add_temp_var(
					"array_length",
					cast_new_prim_type(
						CAST_PRIM_INT,
						CAST_MOD_UNSIGNED));
			add_stmt(cast_new_stmt_expr(
				cast_new_expr_assign(lexpr, num_expr)));
			num_expr = lexpr;
		} else if (num_expr != len_expr) {
			add_stmt(cast_new_stmt_expr(
				cast_new_expr_assign(len_expr, num_expr)));
			num_expr = len_expr;
		}
	}
	cast_expr inl_expr;
	if (oolcheck)
		inl_expr = cast_new_unary_expr(CAST_UNARY_LNOT,
					       oolcheck);
	else
		inl_expr = cast_new_expr_lit_int(!out_of_line, 0);
	cast_expr long_expr = cast_new_expr_name(
		longform ? "_long_type" : "_type");
	
	cast_expr cex;
	/* For ports, we need to add an extra param to the macro call
	   -- the port type */
	if (port_type) {
		cex = cast_new_expr_call(macro_expr, 6);
		cast_set_expr_array(
			&cex->cast_expr_u_u.call.params,
			ofs_expr,
			port_type,
			num_expr,
			inl_expr,
			dealloc_expr,
			long_expr,
			NULL);
		is_complex = (cast_expr) 1;
	} else {
		cex = cast_new_expr_call_5(macro_expr,
					   ofs_expr,
					   num_expr,
					   inl_expr,
					   dealloc_expr,
					   long_expr);
	}
	
	add_stmt(cast_new_stmt_expr(cex));
}

void mach3_mu_state::mu_aggregated_array(
	cast_expr array_expr, cast_type array_ctype,
	pres_c_allocation *array_alloc,
	cast_type elem_ctype, mint_ref elem_itype,
	pres_c_mapping elem_map,
	cast_expr len_expr, cast_type /*len_ctype*/,
	unsigned long /*len_min*/, unsigned long len_max,
	mint_ref prim_itype, int fixed_elts,
	int out_of_line, char *cname)
{
	mach3_array = 1;
	assert(prim_itype >= 0);
	assert(prim_itype < (mint_ref) pres->mint.defs.defs_len);
	mint_def *pidef = &pres->mint.defs.defs_val[prim_itype];
	
	/* Decide whether to use a shortform or longform type descriptor.  */
	int longform = (len_max >= 4096);

	/* Find the vital parameters of the primitive array elements.  */
	int prim_size = -1, prim_align_bits = -1;
	char *prim_macro_name = 0;
	if (pidef->kind != MINT_VOID) {
		get_prim_params(prim_itype, &prim_size, &prim_align_bits,
				&prim_macro_name);
		if (!prim_macro_name) {
			panic(("In `mach3_mu_state::mu_aggregated_array', "
			       "invalid MINT type (%d) encountered."),
			      prim_itype);
		}
	}
	
	/* See if we have a servercopy argument available. */
	cast_expr scopy;
	cast_type scopyt;
	int gotarg = arglist->getargs(cname, "mustcopy", &scopy, &scopyt);
	assert(gotarg);
	
	/*
	 * We need a runtime out-of-line check for unbounded `inline' arrays.
	 * But we don't need it if we are just deallocating the array and have
	 * a servercopy argument (since the work function is then responsible
	 * for deallocation, and we don't have to do a thing).
	*/
	cast_expr oolcheck = 0;
	if (len_max == 0xffffffff /* unbounded array */
	    && !out_of_line
	    && !((op & MUST_DEALLOCATE) && pidef->kind == MINT_VOID
		 && scopy)) {
		/* send inline if <= 2K, OOL if > 2K */
		oolcheck = add_temp_var(
			"ool_check",
			cast_new_prim_type(CAST_PRIM_INT, 0));
	}

	cast_expr limit2m;
	if (prim_size > 0) {
		limit2m = cast_new_expr_lit_int(
			2048 / (prim_size * fixed_elts),
			0);
	} else {
		limit2m =
			cast_new_binary_expr(CAST_BINARY_DIV,
					     cast_new_expr_lit_int(2048, 0),
					     cast_new_expr_sizeof_type(
						     elem_ctype));
	}
	
	if (oolcheck && (op & MUST_ENCODE)) {
		add_stmt(
			cast_new_stmt_expr(
				cast_new_expr_assign(
					oolcheck,
					cast_new_binary_expr(
						CAST_BINARY_GT,
						len_expr,
						limit2m))));
	}
	
	/* Output the proper tag for strings (esp. fixed-length strings). */
	if (mu_array_is_string(cname)) {
		prim_macro_name = flick_asprintf("flick_%s_%s_string_c",
						 get_be_name(),
						 get_buf_name());
	}
	
	if (pidef->kind != MINT_VOID) {
		/* Find/make the deallocation expression. */
		cast_expr dealloc_expr = 0;
		cast_type t;
		int dealloc_always = ((get_allocator_flags(array_alloc)
				       & PRES_C_DEALLOC_EVER)
				      == PRES_C_DEALLOC_ALWAYS);
		if (dealloc_always) {
			gotarg = arglist->getargs(cname, "release",
						  &dealloc_expr, &t);
			assert(gotarg);
		}
		if (!dealloc_expr)
			dealloc_expr
				= cast_new_expr_lit_int(
					(out_of_line && dealloc_always), 0);
		
		/* For ports, we need to add an extra param to the macro call
		   -- the port type */
		cast_expr port_type = 0;
		if (pres->mint.defs.defs_val[prim_itype].kind
		    == MINT_INTERFACE) {
			pres_c_mapping temp_map;
			/*
			 * XXX -- Dig through any stub that may be in the way.
			 */
			if (elem_map->kind == PRES_C_MAPPING_STUB) {
				int stub_num = pres_c_find_mu_stub(
					pres, elem_itype, elem_ctype,
					elem_map, PRES_C_MARSHAL_STUB);
				temp_map = pres->stubs.stubs_val[stub_num].
					   pres_c_stub_u.mstub.seethru_map;
			} else
				temp_map = elem_map;
			
			assert(temp_map->kind == PRES_C_MAPPING_REFERENCE);
			port_type = cast_new_expr_name(
				get_mach_port_type(
					&temp_map->pres_c_mapping_u_u.ref,
					prim_itype));
			is_complex = (cast_expr) 1;
		}
		
		mu_array_type_descriptor(len_expr, fixed_elts,
					 prim_macro_name, longform,
					 out_of_line, dealloc_expr,
					 port_type, oolcheck);
	}
	
	/* Now descend into the array and marshal the individual elements. */
	
	if (oolcheck && (op & MUST_DECODE)) {
		add_stmt(
			cast_new_stmt_expr(
				cast_new_expr_assign(
					oolcheck,
					cast_new_binary_expr(
						CAST_BINARY_GT,
						len_expr,
						limit2m))));
	}
	
	if (oolcheck && pidef->kind != MINT_VOID) {
		/*
		 * Add a check to see if this should go inline or out-of-line.
		 */
		ool_check_functor  ocf(oolcheck, prim_size * fixed_elts,
				       array_expr, array_ctype, elem_ctype,
				       elem_itype, elem_map, cname);
		mu_union(&ocf);
		if (is_complex != (void *) 1) {
			is_complex = oolcheck;
		}
		last_ool_check = oolcheck;
	} else if (oolcheck && pidef->kind == MINT_VOID) {
		/*
		 * Add a check to see if we need to deallocate this out-of-line
		 * `in' data.  (It may have been sent out-of-line, so we must
		 * preserve the ``in-line'' semantics of the parameter in that
		 * it is destroyed after the server work function has finished.
		 * This happens automatically for in-line data by recycling the
		 * message buffer, but out-of-line data normally isn't freed.)
		 * If a servercopy argument exists, then we don't have to worry
		 * about it, since the responsibility for deallocation then
		 * rests on the work function.
		 */
		
		/* set up a new c_block to capture the deallocation. */
		cast_stmt orig_cblock = c_block;
		c_block = 0;
		
		/*
		 * Create a new allocation semantic for deallocating the data.
		 * (The default allocation semantic assumes inline
		 * transmission, and thus no deallocation.)
		 *
		 * XXX - This is a terrible hack, since the PG should be able
		 * to express the proper allocation semantic using the
		 * pres_c_allocation structure (including necessary runtime
		 * checks).  Until we have a new allocation representation,
		 * however, we have to munge the allocation here to force
		 * proper allocation.
		 */
		pres_c_allocation aalloc;
		for (int i = 0; i < PRES_C_DIRECTIONS; i++) {
			/* The `in' direction is the only valid one here. */
			if (i != PRES_C_DIRECTION_IN) {
				aalloc.cases[i].allow
					= PRES_C_ALLOCATION_INVALID;
				continue;
			}
			
			aalloc.cases[i].allow = PRES_C_ALLOCATION_ALLOW;
			/*
			 * The allocator must be a PRES_C_ALLOCATOR_NAME
			 * since PRES_C_ALLOCATOR_OUTOFLINE assumes the
			 * deallocation is taken care of by the kernel.
			 */
			aalloc.cases[i].pres_c_allocation_u_u.val.allocator.
				kind = PRES_C_ALLOCATOR_NAME;
			aalloc.cases[i].pres_c_allocation_u_u.val.allocator.
				pres_c_allocator_u.name = ir_strlit("mach_vm");
			aalloc.cases[i].pres_c_allocation_u_u.val.flags
				= PRES_C_DEALLOC_ALWAYS;
		}
		pres_c_allocation old_alloc = *array_alloc;
		*array_alloc = aalloc;
		mem_mu_state::mu_array(array_expr, array_ctype, elem_ctype,
				       elem_itype, elem_map, cname);
		*array_alloc = old_alloc;
		
		cast_stmt dealloc_if
			= cast_new_if(oolcheck, c_block, 0);
		
		/* Restore the c_block and add the dealloc `if'. */
		c_block = orig_cblock;
		add_stmt(dealloc_if);
	} else if (out_of_line && pidef->kind != MINT_VOID) {
		/* Allocate space */
		if ((op & MUST_ALLOCATE) &&
		    (array_ctype->kind == CAST_TYPE_POINTER))
			mu_pointer_alloc(array_expr, elem_ctype, cname);
		
		/*
		 * Bounds check the array, but only for *real* arrays.
		 * Normally, this is taken care of by the standard mu_array(),
		 * but we aren't running that here.
		 */
		if (array_data.is_valid)
			mu_array_check_bounds(cname);
		
		/*
		 * Here we m/u the pointer itself, literally, into the
		 * standard data stream (as an unsigned32 value).  The Mach
		 * kernel handles the rest.
		 */
		mu_mapping_simple(array_expr,
				  cast_new_prim_type(CAST_PRIM_INT,
						     CAST_MOD_UNSIGNED),
				  pres->mint.standard_refs.unsigned32_ref);
		
		/*
		 * Terminate the array, but only for *real* arrays.
		 * Normally, this is taken care of by the standard mu_array(),
		 * but we aren't running that here.
		 */
		if (array_data.is_valid)
			mu_array_terminate(array_expr, elem_ctype, cname);
		
		/* Deallocate space */
		if ((op & MUST_DEALLOCATE) &&
		    (array_ctype->kind == CAST_TYPE_POINTER))
			mu_pointer_free(array_expr, elem_ctype, cname);
	} else if (!((op & MUST_DEALLOCATE) && pidef->kind == MINT_VOID
		     && scopy)) {
		mem_mu_state::mu_array(array_expr, array_ctype, elem_ctype,
				       elem_itype, elem_map, cname);
	}
	
	mach3_array = 0;
	
	/* we need to align ourselves to a 4-byte boundary */
	if ((align_bits < 2) || (align_ofs & 3))
		chunk_prim(2, 0);
}

void mach3_mu_state::mu_array(cast_expr array_expr, cast_type array_ctype,
			      cast_type elem_ctype, mint_ref elem_itype,
			      pres_c_mapping elem_map, char *cname)
{
	/* Decide whether to transmit it in-line or out-of-line. */
	mu_inline_alloc_context *iac = inline_alloc_context;
	while (iac) {
		if (strcmp(iac->name, cname) == 0)
			break;
		iac = iac->parent_context;
	}
	assert(iac);
	int out_of_line = 0;
	if (get_allocator_kind(iac->alloc).kind == PRES_C_ALLOCATOR_OUTOFLINE
	    || mach3_outofline) {
		out_of_line = 1;
		/*
		 * If `mach3_outofline' is set, then we're forcing data
		 * out-of-line because it's > 2K.  The `is_complex' flag
		 * should only be set here if we have data ALWAYS sent
		 * out-of-line (rather than conditionally).
		 */
		if (!mach3_outofline) {
			is_complex = (cast_expr) 1;
		}
	}
	
	/* Now make sure we have a valid length expression in the arglist. */
	cast_expr len_expr;
	cast_type len_ctype;
	int gotarg = arglist->getargs(cname, "length", &len_expr, &len_ctype);
	assert(gotarg);
	assert(len_expr);
	assert(len_ctype);
	
	/* See if the whole array can be lumped under one MIG type descriptor.
	   This is possible iff this is an array of [fixed arrays of...]
	   primitive types.  At the same time, compute the total number of
	   primitive elements contained in each element of this (top-level,
	   possibly variable-length) array.  */
	mint_ref prim_itype = elem_itype;
	int fixed_elts = 1;
	
	unsigned len_min, len_max;
	mu_array_get_encoded_bounds(&len_min, &len_max, cname);
	
	if (out_of_line
	    || (pres->mint.defs.defs_val[elem_itype].kind == MINT_VOID)
	    || (array_data.is_valid && len_max > 1)) while (1) {
		assert(prim_itype >= 0);
		assert(prim_itype < (signed int)pres->mint.defs.defs_len);
		
		mint_def *def = &pres->mint.defs.defs_val[prim_itype];
		
		/*
		 * Don't try to aggregate again if we're already aggregating.
		 */
		if (mach3_array)
			break;
		
		if ((def->kind == MINT_INTEGER) ||
		    (def->kind == MINT_FLOAT) ||
		    (def->kind == MINT_CHAR) ||
		    (def->kind == MINT_VOID) ||
		    (def->kind == MINT_INTERFACE)) {
			/* We reached the primitive type at the bottom,
			   so aggregation is possible - do it.  */
			mu_aggregated_array(array_expr, array_ctype,
					    iac->alloc,
					    elem_ctype, elem_itype, elem_map,
					    len_expr, len_ctype,
					    len_min, len_max,
					    prim_itype, fixed_elts,
					    out_of_line, cname);
			return;
			
		} else if (def->kind == MINT_ARRAY) {
			unsigned array_min, array_max;
			mint_get_array_len(&pres->mint, prim_itype,
					   &array_min, &array_max);
			if (array_min != array_max) {
				/* Can't aggregate arrays of variable-length
                                   arrays.*/
				break;
			}
			fixed_elts *= array_min;
			/* simple overflow check */
			assert(fixed_elts % array_min == 0);
			prim_itype = def->mint_def_u.array_def.element_type;
		} else {
			/* Saw something else - no aggregation possible.
			 *
			 * XXX - presentations other than MIG may cause this
			 * case to occur, so we can't panic here... But for
			 * MIG arrays, we have to know what type to send for
			 * the type tag.  If we get here, we don't know, and
			 * thus can't make a correct tag.  It would be wrong
			 * to handle it "the normal way", because no tag would
			 * be associated with this array, and the message
			 * would not be correctly interpreted by the kernel.
			 */
#if 0
			panic("Can't determine type of array data!");
#else
			static int is_warned = 0;
			if (!is_warned++)
				warn("Can't determine type of array data!");
#endif
			break;
		}
	}
	
	/* No aggregation possible - deal with the array the normal way.  */
	if (out_of_line
	    && pres->mint.defs.defs_val[prim_itype].kind != MINT_VOID) {
		/* Allocate space */
		if ((op & MUST_ALLOCATE) &&
		    (array_ctype->kind == CAST_TYPE_POINTER))
			mu_pointer_alloc(array_expr, elem_ctype, cname);
		
		/*
		 * Here we m/u the pointer itself, literally, into the
		 * standard data stream (as an unsigned32 value).  The Mach
		 * kernel handles the rest.
		 */
		mu_mapping_simple(array_expr,
				  cast_new_prim_type(CAST_PRIM_INT,
						     CAST_MOD_UNSIGNED),
				  pres->mint.standard_refs.unsigned32_ref);
		
		/* Deallocate space */
		if ((op & MUST_DEALLOCATE) &&
		    (array_ctype->kind == CAST_TYPE_POINTER))
			mu_pointer_free(array_expr, elem_ctype, cname);
	} else {
		mem_mu_state::mu_array(array_expr, array_ctype, elem_ctype,
				       elem_itype, elem_map, cname);
	}
}

/*
 * For Mach3MIG strings, we encode the terminator as part of the string.
 */
int mach3_mu_state::mu_array_encode_terminator(char *cname)
{
	return mu_array_is_string(cname);
}

/* End of file. */
