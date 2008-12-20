/*
 * Copyright (c) 1995, 1996, 1999 The University of Utah and
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
#include <mom/c/libcast.h>
#include <mom/c/pbe.hh>

int mu_type_cast_mapping_handler(mu_state *must, cast_expr cexpr,
				 cast_type ctype, mint_ref itype,
				 pres_c_mapping_xlate *xmap)
{
	if( must->op & MUST_ENCODE ) {
		cexpr = cast_new_expr_cast(cexpr, ctype);
		must->mu_mapping(cexpr,
				 xmap->internal_ctype,
				 itype,
				 xmap->internal_mapping);
	}
	if( must->op & MUST_DECODE ) {
		/* Create a temporary variable of the internal
		   type, to marshal into/out of.  */
		cast_expr internal_var_expr =
			must->add_temp_var("internal", xmap->internal_ctype);
		must->mu_mapping(internal_var_expr,
				 xmap->internal_ctype,
				 itype,
				 xmap->internal_mapping);
		must->add_stmt(cast_new_stmt_expr(
			cast_new_expr_assign(
				cexpr,
				xmap->translator[0]
				? cast_new_expr_call_1(
					cast_new_expr_name(xmap->translator),
					internal_var_expr)
				: cast_new_expr_cast(internal_var_expr,
						     ctype))));
	}
	return( 1 );
}

int mu_ampersand_mapping_handler(mu_state *must, cast_expr cexpr,
				 cast_type /* ctype */, mint_ref itype,
				 pres_c_mapping_xlate *xmap)
{
	cast_type int_ctype = xmap->internal_ctype;
	
	/*
	 * change any CAST_TYPE_ARRAY's to CAST_TYPE_POINTER's so
	 * we don't allocate space, when all we want is a
	 * pointer.
	 */
	if ((int_ctype->kind == CAST_TYPE_POINTER) &&
	    (int_ctype->cast_type_u_u.pointer_type.target->kind
	     == CAST_TYPE_ARRAY))
		int_ctype = cast_new_pointer_type(
			cast_new_pointer_type(int_ctype->cast_type_u_u.
					      pointer_type.target->
					      cast_type_u_u.array_type.
					      element_type));
	else if (int_ctype->kind == CAST_TYPE_ARRAY)
		int_ctype = cast_new_pointer_type(
			int_ctype->cast_type_u_u.array_type.element_type);
	
	/*
	 * Create a temporary variable of the internal type, to
	 * marshal into or out of.
	 */
	cast_expr internal_var_expr =
		must->add_temp_var("internal", int_ctype);
	
	must->add_stmt(cast_new_stmt_expr(
		cast_new_expr_assign(
			internal_var_expr,
			cast_new_expr_cast(
				cast_new_expr_call_1(
					cast_new_expr_name(xmap->translator),
					cexpr),
				int_ctype))));
	must->mu_mapping(internal_var_expr,
			 xmap->internal_ctype,
			 itype,
			 xmap->internal_mapping);
	return( 1 );
}

void mu_state::mu_mapping_xlate(cast_expr expr, cast_type ctype,
				mint_ref itype, pres_c_mapping_xlate *xmap)
/* unmarshaling: temp = unmarshal(itype);
                 expr = (ctype) xmap(temp);
   marshaling:   marshal((itype)xmap'((ctype)expr)); */
{
	struct translation_handler_entry *the;
	
	assert(xmap->internal_ctype);
	assert(xmap->internal_mapping);
	assert(xmap->translator);
	
	if (op & MUST_ENCODE) {
		the = (struct translation_handler_entry *)
			find_entry(translation_handlers, xmap->translator);
		if( the )
			the->handler(this, expr, ctype, itype, xmap);
		else {
			if( xmap->translator )
				expr = cast_new_expr_call_1(
					cast_new_expr_name(xmap->translator),
					expr);
			mu_mapping(expr,
				   xmap->internal_ctype,
				   itype,
				   xmap->internal_mapping);
		}
	}
	
	if (op & MUST_DECODE) {
		the = (struct translation_handler_entry *)
			find_entry(translation_handlers, xmap->translator);
		if( the ) {
			the->handler(this, expr, ctype, itype, xmap);
		} else {
			mu_mapping(expr,
				   xmap->internal_ctype,
				   itype,
				   xmap->internal_mapping);
		}
	}
}

