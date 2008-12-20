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

#include <assert.h>
#include <mom/libmint.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include "mach3.h"

#define m(n) (&pres->mint.defs.defs_val[n])

void mach3_mu_state::mu_mapping_message_attribute(
	cast_expr expr,
	cast_type ctype,
	mint_ref itype,
	pres_c_mapping_message_attribute *attr_map) 
{
	assert((m(itype))->kind == MINT_VOID);
	switch (attr_map->kind) {
	case PRES_C_MESSAGE_ATTRIBUTE_FLAGS:
		if (op & MUST_ENCODE) {
			if (msg_option_expr)
				msg_option_expr =
					cast_new_binary_expr(
						CAST_BINARY_BOR,
						msg_option_expr,
						expr);
			else
				msg_option_expr = expr;
		} else if (op & MUST_DECODE) {
			panic("message flags cannot be explicitly decoded!\n");
		}
		break;
		
	case PRES_C_MESSAGE_ATTRIBUTE_TIMEOUT:
		if (op & MUST_ENCODE) {
			if (msg_option_expr)
				msg_option_expr =
					cast_new_binary_expr(
						CAST_BINARY_BOR,
						msg_option_expr,
						cast_new_expr_name(
							"MACH_RCV_TIMEOUT"));
			else
				msg_option_expr =
					cast_new_expr_name("MACH_RCV_TIMEOUT");
			if (timeout_expr)
				panic("Cannot handle multiple timeout "
				      "parameters!\n");
			else
				timeout_expr = expr;
		} else if (op & MUST_DECODE) {
			panic("A message timeout cannot be explicitly"
			      " decoded!\n");
		}
		break;
		
	case PRES_C_MESSAGE_ATTRIBUTE_SEQUENCE_RECEIVED:
		if (op & MUST_DECODE) {
			/* "expr = _buf_start->Head.msgh_seqno" */
			cast_expr attr_expr = cast_new_expr_assign(
				expr, cast_new_expr_name(
					"_buf_start->Head.msgh_seqno"));
			add_stmt(cast_new_stmt_expr(attr_expr));
		} else if (op & MUST_ENCODE) {
			panic("A message sequence number cannot be explicitly"
			      " encoded!\n");
		}
		break;
		
	case PRES_C_MESSAGE_ATTRIBUTE_CLIENT_REFERENCE:
		if (!strcmp(which_stub, "server") && (op & MUST_DECODE)) {
			/* "expr = _buf_start->Head.msgh_remote_port" */
			cast_expr attr_expr = cast_new_expr_assign(
				expr, cast_new_expr_name(
					"_buf_start->Head.msgh_remote_port"));
			add_stmt(cast_new_stmt_expr(attr_expr));
		} else if (!strcmp(which_stub, "client") && (op & MUST_ENCODE)) {
			/* "_buf_start->Head.msgh_local_port = expr" */
			cast_expr attr_expr = cast_new_expr_assign(
				cast_new_expr_name(
					"_buf_start->Head.msgh_local_port"),
				expr);
			add_stmt(cast_new_stmt_expr(attr_expr));
		}
		break;
		
	case PRES_C_MESSAGE_ATTRIBUTE_SERVERCOPY:
		if (!strcmp(which_stub, "server") && (op & MUST_DECODE)) {
			/* "expr = !ool_check" */
			assert(last_ool_check);
			cast_expr attr_expr = cast_new_expr_assign(
				expr, cast_new_unary_expr(CAST_UNARY_LNOT,
							  last_ool_check));
			add_stmt(cast_new_stmt_expr(attr_expr));
		} else {
			panic("Server copy information cannot be explicitly"
			      " encoded!\n");
		}
		break;
		
	default:
		mem_mu_state::mu_mapping_message_attribute(expr, ctype, itype, attr_map);
		break;
	}
}

/* End of file. */
