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

#include <mom/compiler.h>
#include <mom/libmint.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>
#include <mom/c/pbe.hh>


/* This class is called from within a case branch
 * of the top level switch statement.
 * It should deal with the branch of the union that it receives
 */
struct void_u_inl_union_case_functor : public functor
{
	void_u_inl_union_case_functor(functor *func):
		func_to_call(func) {}
	
	virtual void func(mu_state *must) {
		must->mu_union_case(func_to_call);
	} ;
	
	functor *func_to_call;
};
  
/*
 * This class is called from within a case branch to set the discriminator
 * to a literal value.  This functor is only used when the discriminator
 * uses a PRES_C_MAPPING_ELSEWHERE and needs to be set statically.
 */
struct void_u_mu_discriminator_functor : public functor
{
	void_u_mu_discriminator_functor(
		cast_expr discrim_expr,
		cast_expr cast_case_value,
		mint_const mint_case_value,
		mint_ref discrim_itype):
		cexpr(discrim_expr), cvalue(cast_case_value),
		mvalue(mint_case_value), itype(discrim_itype) {}
	
	virtual void func(mu_state *must) {
		if (must->op & MUST_DECODE) {
			/* Assign the discriminator the value
			   of the given branch */
			assert(cexpr);
			assert(cvalue);
			must->add_stmt(
				cast_new_stmt_expr(
					cast_new_expr_assign(cexpr, cvalue)));
		} else {
			must->mu_encode_const(mvalue, itype);
		}
	}
	
	cast_expr cexpr, cvalue;
	mint_const mvalue;
	mint_ref itype;
}; 

/*
 * This class is called from within a case branch from inside mu_union_case
 * of the top level switch statement.
 */
struct void_u_inl_success_functor : public functor
{
	void_u_inl_success_functor(inline_state *ist_in,
				   mint_ref case_itype_in,
				   pres_c_inline_void_union_case case_inl,
				   cast_expr void_expr,
				   functor *optional_func):
		ist(ist_in), itype(case_itype_in), inl(case_inl),
		vexpr(void_expr), opt_func(optional_func) {}
	
	virtual void func(mu_state *must) {
		// Call the optional functor if it exists.
		if (opt_func)
			opt_func->func(must);
		
		// We need to type cast the mapping to the correct type.
		cast_expr typed = cast_new_expr(CAST_EXPR_CAST);
		typed->cast_expr_u_u.cast.expr = vexpr;
		typed->cast_expr_u_u.cast.type = inl.type;
		must->mu_mapping(typed, inl.type, itype, inl.mapping);
	}

	inline_state *ist;
	mint_ref itype;
	pres_c_inline_void_union_case inl;
	cast_expr vexpr;
	functor *opt_func;
};

/***/

/* The class of the `functor' that is invoked in order to generate the code
   that appears within the `case' corresponding to a successful decode of the
   value contained in the collapsed union.  A failure functor emits a statement
   that indicates a decoding error. */
   
struct void_u_inl_failure_functor : public functor {
	virtual void func(mu_state *must) {
		/* NOTE: Breaking a glob here would cause an excessive
		         number of globs in the common case.  It is not
			 necessary since the error macros have *no* 
			 assumptions about the marshal/unmarshal state. */
		
		must->add_stmt(must->make_error(FLICK_ERROR_VOID_UNION));
	};
};


/***/

/* The class of the `functor' that is invoked by `mu_union' in order to
   generate discriminating code. */
   
struct void_u_inl_discriminate_functor : public functor
{
	void_u_inl_discriminate_functor(inline_state *istate,
					mint_ref type,
					pres_c_inline _inline) {
		ist = istate;
		itype = type;
		inl = _inline;
	}
	virtual void func(mu_state *must);
	
	inline_state *ist;
	mint_ref itype;
	pres_c_inline inl;
};

void void_u_inl_discriminate_functor::func(mu_state *must)
{
	functor **success_array;
	void_u_inl_failure_functor failure;
	mint_const *discrim_vals;
	cast_type discrim_ctype;
	cast_expr discrim_cexpr, void_expr;
	int count;
	// used to set the discriminator's value in each branch.
	functor *opt_func = 0; 
	
	// See how many cases we have
	mint_union_def *udef = &(must->pres->mint.defs.defs_val[itype].
				 mint_def_u.union_def);
	count = udef->cases.cases_len;
	
	// Allocate space for the functors & the union values
	discrim_vals = (mint_const *)mustcalloc(sizeof(mint_const) * count);
	success_array = (functor **)mustcalloc(sizeof(functor *) * count);
	
	// Get the expression (we don't need the type) of the void * element
	ist->slot_access(inl->pres_c_inline_u_u.void_union.void_index, &void_expr, &discrim_ctype);
	// We could assert that the type is a void *, but who cares? - XXX
	
	// Get the C type and expression to get to the discriminator
	ist->slot_access(inl->pres_c_inline_u_u.void_union.discrim.index, &discrim_cexpr, &discrim_ctype);
	
	/* mu_discriminate and hash_const() take care of memory management */
	
	// Build the arrays of functors and values
	for (int i = 0; i < count; i++) {
		discrim_vals[i] = udef->cases.cases_val[i].val;
		
		/* assign the discriminator's value if not assigned yet */
		if (inl->pres_c_inline_u_u.void_union.discrim.mapping->kind
		    == PRES_C_MAPPING_ELSEWHERE) {
			if (inl->pres_c_inline_u_u.void_union.
			    cases.cases_val[i].case_value) {
				opt_func = new void_u_mu_discriminator_functor(
					discrim_cexpr,
					inl->pres_c_inline_u_u.void_union.
					cases.cases_val[i].case_value,
				        discrim_vals[i],
					udef->discrim);
				assert(opt_func);
			} else {
				/*
				 * XXX - discriminator is never initialized
				 * upon decoding
				 */
#if 1
				assert(opt_func == 0); /* inconsistencies? */
#else
				opt_func = 0;
#endif
			}
		}
		
		success_array[i] = new void_u_inl_union_case_functor(
  			new void_u_inl_success_functor(
  				ist, udef->cases.cases_val[i].var,
  				inl->pres_c_inline_u_u.void_union.cases.
  				cases_val[i],
				void_expr, opt_func));
		assert(success_array[i]);
	}
	
	if ((opt_func) && (must->op & MUST_DECODE)) {
		/*
		 * The discriminator hasn't been
		 * unmarshaled yet, so make mu_discriminate()
		 * discriminate piece by piece.
		 */
		discrim_ctype = 0;
		discrim_cexpr = 0;
	}
		
	/* `mu_discriminate' will generate the `switch', 
	   call the functors, and thereby generate code.
	   NOTE: This code used to call mu_var_discriminate. */
	must->mu_discriminate(discrim_vals,
			      must->pres->mint.defs.defs_val[itype].
			      mint_def_u.union_def.discrim,
			      count, success_array,
			      new void_u_inl_union_case_functor(&failure),
			      discrim_ctype, discrim_cexpr);
}

/*****************************************************************************/

void mu_state::mu_inline_void_union(inline_state *ist, mint_ref itype,
				    pres_c_inline inl)
{
	cast_expr discrim_expr;
	cast_type discrim_ctype;
	assert(itype >= 0);
	assert(itype < (signed int)pres->mint.defs.defs_len);
	
	mint_def *def = &pres->mint.defs.defs_val[itype];
	mint_union_def *udef = &def->mint_def_u.union_def;
	assert(def->kind == MINT_UNION);
	pres_c_inline_void_union *inlvu;
	
	// Build the functor that we're using
	void_u_inl_discriminate_functor discrim_functor(ist, itype, inl);
	
	assert(inl->kind == PRES_C_INLINE_VOID_UNION);
	inlvu = &(inl->pres_c_inline_u_u.void_union);
	
	// marshal or unmarshal the discriminator
	ist->slot_access(inlvu->discrim.index, &discrim_expr, &discrim_ctype);
	mu_mapping(discrim_expr, discrim_ctype,
		   udef->discrim, inlvu->discrim.mapping);
	
	// Now deal with the union
	mu_union(&discrim_functor);
}

/* End of file. */

