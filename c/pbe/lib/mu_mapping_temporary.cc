/*
 * Copyright (c) 1999 The University of Utah and
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
#include <mom/c/libcast.h>
#include <mom/c/pbe.hh>
#include <mom/c/libpres_c.h>

/*
 * When we encounter one of these nodes, we declare a temporary variable in the
 * current scope, and initialize it as described by the pres_c_temporary node.
 * If the initialization expression is flagged as constant, then we bypass
 * introducing a temporary variable, and pass down the initialization
 * expression itself as the cexpr (this is very useful for maintaining literal
 * CAST values, say for a fixed array's length).  If we are encoding then we
 * call a macro handler before continuing with the mapping.  If we are decoding
 * then we call a macro handler after whatever happened in the mapping.
 *
 * The significant difference between this and it's inline version, is that
 * here we already have a CAST expr/type that we may need to use to initialize
 * the temporary.
 */
void mu_state::mu_mapping_temporary(cast_expr cexpr,
				    cast_type ctype,
				    mint_ref itype,
				    pres_c_temporary *temp)
{
	cast_expr temp_expr = 0;
	
	assert(temp->map);
	assert(cexpr);
	assert(ctype);
	
	const char *temptype = 0;
	switch(temp->type) {
	case TEMP_TYPE_PRESENTED:
		temptype = pres->pres_context;
		break;
	case TEMP_TYPE_ENCODED:
		temptype = get_encode_name();
		break;
	default:
		panic(("In mu_state::mu_mapping_tempoaray(): "
		       "Unknown temporary type %d"),
		      temp->type);
	}
	assert(temptype);
	
	if (temp->is_const) {
		assert(temp->init);
		assert(!temp->prehandler || (strlen(temp->prehandler) == 0));
		temp_expr = temp->init;
	} else {
		temp_expr = add_temp_var(flick_asprintf("temp_%s", temp->name),
					 temp->ctype);
		
		/* If there is an initializing expression,
		   make the assignment. */
		if (temp->init) {
			add_initial_stmt(
				cast_new_stmt_expr(
					cast_new_expr_assign(temp_expr,
							     temp->init)));
		}
		
		/* If there is a pre-encode handler, issue its macro call. */
		if ((op & MUST_ENCODE)
		    && temp->prehandler && (strlen(temp->prehandler) > 0)) {
			char *macro = flick_asprintf("flick_%s_%s",
						     temptype,
						     temp->prehandler);
			add_initial_stmt(cast_new_stmt_expr(
				cast_new_expr_call_4(
					cast_new_expr_name(macro),
					cast_new_expr_type(ctype),
					cexpr,
					cast_new_expr_type(temp->ctype),
					temp_expr)));
		}
	}
	assert(temp_expr);
	
	mu_mapping(temp_expr, temp->ctype, itype, temp->map);
	
	if ((op & MUST_DECODE)
	    && temp->posthandler && (strlen(temp->posthandler) > 0)) {
		char *macro = flick_asprintf("flick_%s_%s",
					     temptype,
					     temp->posthandler);
		
		add_stmt(cast_new_stmt_expr(
			cast_new_expr_call_4(
				cast_new_expr_name(macro),
				cast_new_expr_type(ctype),
				cexpr,
				cast_new_expr_type(temp->ctype),
				temp_expr)));
	}
}

/* End of file. */
