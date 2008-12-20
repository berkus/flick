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

#include <mom/libmint.h>
#include <mom/c/libcast.h>
#include <mom/c/pbe.hh>
#include <mom/compiler.h>

/*
 * This routine handles inline temporary nodes.  An inline temporary differs
 * from a mapping temporary in that there is no inline_state slot to access.
 * Generally, this is a constant expression, such as the length of a fixed
 * array.  However, this could also be used to manufacture a temporary that
 * contains runtime state (from a macro).
 */
void mu_state::mu_inline_temporary(inline_state */*ist*/,
				   mint_ref itype,
				   pres_c_inline inl)
{
	pres_c_temporary *temp = &(inl->pres_c_inline_u_u.temp);
	
	cast_expr temp_expr = 0;
	
	assert(temp->map);
	assert(itype >= 0);
	assert(itype < (mint_ref) pres->mint.defs.defs_len);
	
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
		if (temp->init)
			add_stmt(
				cast_new_stmt_expr(
					cast_new_expr_assign(temp_expr,
							     temp->init)));
		
		/* If there is a pre-encode handler, issue its macro call. */
		if ((op & MUST_ENCODE)
		    && temp->prehandler && (strlen(temp->posthandler) > 0)) {
			char *macro = flick_asprintf("flick_%s_make_%s",
						     temptype,
						     temp->prehandler);
			
			add_stmt(cast_new_stmt_expr(
				cast_new_expr_call_2(
					cast_new_expr_name(macro),
					cast_new_expr_type(temp->ctype),
					temp_expr)));
		}
	}
	assert(temp_expr);
	
	mu_mapping(temp_expr, temp->ctype, itype, temp->map);
	
	if ((op & MUST_DECODE)
	    && temp->posthandler && (strlen(temp->posthandler) > 0)) {
		char *macro = flick_asprintf("flick_%s_make_%s",
					     temptype,
					     temp->posthandler);
		
		add_stmt(cast_new_stmt_expr(
			cast_new_expr_call_2(
				cast_new_expr_name(macro),
				cast_new_expr_type(temp->ctype),
				temp_expr)));
	}
}

/* End of file. */

