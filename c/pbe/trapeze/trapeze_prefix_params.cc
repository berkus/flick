/*
 * Copyright (c) 1997, 1998 The University of Utah and
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

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <mom/c/libcast.h>
#include "trapeze.h"

void trapeze_mu_state::mu_prefix_params() 
{
	if ((op & MUST_ENCODE) &&
	    !strcmp("client", get_which_stub())) {
		/*
		 * Get and encode a reply token
		 */
		cast_stmt comment = cast_new_stmt(CAST_STMT_TEXT);
		
		comment->cast_stmt_u_u.text
			= flick_asprintf(
				"/* Default reply token is TPZ_CTRL. */");
		add_stmt(comment);

		cast_expr token
			= cast_new_expr_name("TPZ_CTRL");
		cast_type ttype
			= cast_new_prim_type(CAST_PRIM_INT, CAST_MOD_UNSIGNED);
		
		mu_mapping_simple(token, ttype,
				  pres->mint.standard_refs.unsigned32_ref);
	}
	
	if ((op & MUST_DECODE) &&
	    !strcmp("server", get_which_stub())) {
		/*
		 * Decode and save the reply token
		 */
		cast_stmt comment = cast_new_stmt(CAST_STMT_TEXT);
		
		comment->cast_stmt_u_u.text
			= flick_asprintf(
				"/* Decode and save the reply token. */");
		add_stmt(comment);

		cast_expr token
			= cast_new_expr_name("_replytoken");
		cast_type ttype
			= cast_new_prim_type(CAST_PRIM_INT, CAST_MOD_UNSIGNED);
		
		mu_mapping_simple(token, ttype,
				  pres->mint.standard_refs.unsigned32_ref);
	}
	
	mem_mu_state::mu_prefix_params();
}

/* End of file. */

