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

void mach3_mu_state::mu_inline_message_attribute(
	inline_state *ist,
	mint_ref itype,
	pres_c_inline inl)
{
	assert(inl);
	pres_c_inline_message_attribute *msg_attr = &(inl->pres_c_inline_u_u.msg_attr);
	switch (msg_attr->kind) {
#if 0
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
#endif
	case PRES_C_MESSAGE_ATTRIBUTE_CLIENT_REFERENCE:
		if (!strcmp(which_stub, "server") && (op & MUST_DECODE)) {
			/* "expr = _buf_start->Head.msgh_remote_port" */
			add_var("_reply_port",
				       cast_new_type_name("mach_port_t"));
			cast_expr attr_expr = cast_new_expr_assign(
				cast_new_expr_name("_reply_port"),
				cast_new_expr_name(
					"_buf_start->Head."
					"msgh_remote_port"));
			add_stmt(cast_new_stmt_expr(attr_expr));
		} else if (!strcmp(which_stub, "server") &&
			   (op & MUST_ENCODE)) {
			/* "_buf_start->Head.msgh_remote_port = expr" */
			cast_expr attr_expr = cast_new_expr_assign(
				cast_new_expr_name(
					"_buf_start->Head."
					"msgh_remote_port"),
				cast_new_expr_name("_reply_port"));
			add_stmt(cast_new_stmt_expr(attr_expr));
		} else if (!strcmp(which_stub, "client") &&
			   (op & MUST_ENCODE)) {
			/* "_buf_start->Head.msgh_local_port = expr" */
			cast_expr attr_expr = cast_new_expr_assign(
				cast_new_expr_name(
					"_buf_start->Head."
					"msgh_local_port"),
				 cast_new_expr_name("mig_get_reply_port()"));
			add_stmt(cast_new_stmt_expr(attr_expr));
		}
		break;
		
	default:
		mem_mu_state::mu_inline_message_attribute(ist, itype, inl);
		break;
	}
}

/* End of file. */
