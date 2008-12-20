/*
 * Copyright (c) 1998 The University of Utah and
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
#include <mom/c/libpres_c.h>

#include "mach3.h"

void mach3_mu_state::mu_server_func_client(pres_c_server_func *sfunc)
{
	if (sfunc->client_i == 0) {
		/* Only spit out one decl of _reply_port */
		if (op & MUST_DECODE)
			add_var("_reply_port", cast_new_type_name("mach_port_t"));

		/* Find the marshaling parameters for this port type.  */
		char *macro_name = flick_asprintf("flick_%s_%s_%s_client",
						  get_be_name(), 
						  get_which_stub(), 
						  get_buf_name());
		cast_expr macro_expr = cast_new_expr_name(macro_name);
		
		/* Spit out the marshal/unmarshal macro call.  */
		cast_expr cex = cast_new_expr_call_4(
			macro_expr,
			cast_new_expr_name("_reply_port"),
			cast_new_expr_lit_int(0, 0),
			cast_new_expr_lit_int(0, 0),
			cast_new_expr_name(get_encode_name()));
		
		add_stmt(cast_new_stmt_expr(cex));
	} else
		mu_state::mu_server_func_client(sfunc);
}

/* End of file. */
