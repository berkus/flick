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
#include <mom/c/pbe.hh>

struct xlate_inline_state : public inline_state
{
	inline_state *orig;
	
	int changed_slot;
	cast_expr new_expr;
	cast_type new_type;
	
	virtual void slot_access(int slot,
				 cast_expr *out_expr, cast_type *out_type);
};

void xlate_inline_state::slot_access(int slot,
				     cast_expr *out_expr, cast_type *out_type)
{
	if (slot == changed_slot) {
		*out_expr = new_expr;
		*out_type = new_type;
	} else
		orig->slot_access(slot, out_expr, out_type);
}

void mu_state::mu_inline_xlate(inline_state *ist,
			       mint_ref itype, pres_c_inline inl)
{
	pres_c_inline_xlate *xinl = &(inl->pres_c_inline_u_u.xlate);
	
	assert(xinl->sub);
	assert(xinl->internal_ctype);
	assert(xinl->translator);
	
	/*
	 * The `&' and `~' translator strings are special translators used to
	 * signal that a fixed array or fixed struct translation is needed.
	 * Used by the MIG FE/PG.
	 */
	assert((xinl->translator[0] == 0)
	       || (xinl->translator[0] == '&')
	       || (xinl->translator[0] == '~')
		);
	
	/* Find the external type and expression. */
	cast_expr external_expr;
	cast_type external_type;
	
	cast_type int_ctype;
	
	ist->slot_access(xinl->index, &external_expr, &external_type);
	
	/*
	 * Create a new inline_state that represents the original one, but with
	 * the appropriate slot's ctype replaced with the internal type.
	 */
	xlate_inline_state xist;
	
	xist.orig = ist;
	xist.changed_slot = xinl->index;
	cast_expr internal_var_expr = 0;
	
	if ((xinl->translator[0] == '&') || (xinl->translator[0] == '~')) {
		int_ctype = xinl->internal_ctype;
		
		/*
		 * change any CAST_TYPE_ARRAY's to CAST_TYPE_POINTER's so
                 * we don't allocate space, when all we want is a
                 * pointer.
		 */
		if ((int_ctype->kind == CAST_TYPE_POINTER) &&
		    (int_ctype->cast_type_u_u.pointer_type.target->kind
		     == CAST_TYPE_ARRAY))
			int_ctype
				= cast_new_pointer_type(
					cast_new_pointer_type(
						int_ctype->cast_type_u_u.
						pointer_type.target->
						cast_type_u_u.array_type.
						element_type));
		else if (int_ctype->kind == CAST_TYPE_ARRAY)
			int_ctype
				= cast_new_pointer_type(
					int_ctype->cast_type_u_u.array_type.
					element_type);
		
		/*
		 * Create a temporary variable of the internal type, to
		 * marshal into or out of.
		 */
		internal_var_expr = add_temp_var("internal", int_ctype);
		
		xist.new_type = int_ctype;
		xist.new_expr = internal_var_expr;
		
	} else { /* ECP  These should be at least structurally equivalent */
/*		assert(!cast_cmp_type(external_type, xinl->internal_ctype)); */
		
		xist.new_type = int_ctype = external_type;
		xist.new_expr = external_expr;
	}
	
	if (xinl->translator[0] == '&') {
		/*
		 * The `&' translator signals that the address of the object
		 * should be cast into the internal C type.
		 */
		add_stmt(cast_new_stmt_expr(
			cast_new_expr_assign(
				internal_var_expr,
				cast_new_expr_cast(
					cast_new_unary_expr(CAST_UNARY_ADDR,
							    external_expr),
					int_ctype
					))));
		
		mu_inline(&xist, itype, xinl->sub);
		
#if 0
/*
 * The `memcpy' that was previously output below is entirely unnecessary.  We
 * already encoded or decoded to the target of the internal variable pointer.
 */
		
		mint_def *def = &pres->mint.mint_1_val[itype];
		
		assert(def->kind == MINT_ARRAY);
		
		mint_array_def *adef = &def->mint_def_u.array_def;
		
		mint_ref length_itype = adef->length_type;
		
		mint_def *length_def = &pres->mint.mint_1_val[length_itype];
		
		assert(length_def->kind == MINT_INTEGER);
		
		int array_len = (length_def->mint_def_u.integer_def.range ?
				 length_def->mint_def_u.integer_def.range :
				 length_def->mint_def_u.integer_def.min);
		
		/* XXX --- Will this work? */
		add_stmt(cast_new_stmt_expr(
			cast_new_expr_call_3(
				cast_new_expr_name("memcpy"),
				cast_new_expr_cast(
					cast_new_unary_expr(CAST_UNARY_ADDR,
							    external_expr),
					cast_new_pointer_type(
						cast_new_type(CAST_TYPE_VOID))
					),
				cast_new_expr_cast(
					internal_var_expr,
					cast_new_pointer_type(
						cast_new_type(CAST_TYPE_VOID))
					),
				cast_new_expr_lit_int(array_len, 0)
				)));
#endif
#if 0
		add_stmt(cast_new_stmt_expr(
			cast_new_expr_assign(
				external_expr,
				cast_new_expr_cast(cast_new_unary_expr(
					CAST_UNARY_DEREF,internal_var_expr),
						   external_type))));
#endif

	} else if (xinl->translator[0] == '~') {
		/*
		 * The `~' translator signals that we are dealing with a fixed
		 * size array.
		 */
		add_stmt(cast_new_stmt_expr(
			cast_new_expr_assign(
				internal_var_expr,
				cast_new_expr_cast(external_expr, int_ctype)
				)));
		
		mu_inline(&xist, itype, xinl->sub);
		
#if 0
/*
 * The `memcpy' that was previously output below is entirely unnecessary.  We
 * already encoded or decoded to the target of the internal variable pointer.
 */
		mint_def *def = &pres->mint.mint_1_val[itype];
		
		int array_len;
		
		if (def->kind == MINT_ARRAY) {
			mint_array_def *adef = &(def->mint_def_u.array_def);
			
			mint_ref length_itype = adef->length_type;
			
			mint_def *length_def
				= &(pres->mint.mint_1_val[length_itype]);
			
			assert(length_def->kind == MINT_INTEGER);
			
			array_len = (length_def->mint_def_u.integer_def.range ?
				     length_def->mint_def_u.integer_def.range :
				     length_def->mint_def_u.integer_def.min);
		} else {
			array_len = (def->mint_def_u.integer_def.range ?
				     def->mint_def_u.integer_def.range :
				     def->mint_def_u.integer_def.min);
		}
		
		/* XXX Will this work? */
		add_stmt(cast_new_stmt_expr(
			cast_new_expr_call_3(
				cast_new_expr_name("memcpy"),
				cast_new_expr_cast(
					external_expr,
					cast_new_pointer_type(
						cast_new_type(CAST_TYPE_VOID))
					),
				cast_new_expr_cast(
					internal_var_expr,
					cast_new_pointer_type(
						cast_new_type(CAST_TYPE_VOID))
					),
				cast_new_expr_lit_int(array_len, 0)
				)));
#endif
		
	} else {
		/*
		 * We have a simple cast between the presented C type and the
		 * C internal type.
		 */
#if 0
		if ((op & MUST_ENCODE)
			&& (int_ctype->kind == CAST_TYPE_POINTER)
		    )
			/*
			 * We are going to encode from the internal var, or we
			 * are going to decode to the address pointed to by our
			 * internal var.
			 */
			add_stmt(cast_new_stmt_expr(
				cast_new_expr_assign(
					internal_var_expr,
					cast_new_expr_cast(
						external_expr,
						int_ctype
						))));
		
#endif

		mu_inline(&xist, itype, xinl->sub);
		
#if 0
		if (op & MUST_DECODE)

			/*
			 * We decoded into the internal var itself, and not
			 * into the address pointed at by our internal var.
			 */
			add_stmt(cast_new_stmt_expr(
				cast_new_expr_assign(
					external_expr,
					cast_new_expr_cast(
						internal_var_expr,
						external_type
						))));
#endif
	}
}

/* End of file. */

