/*
 * Copyright (c) 1997, 1998, 1999 The University of Utah and
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
#include <mom/c/libpres_c.h>

#include "mach3.h"

#define m(n) (&pres->mint.defs.defs_val[n])

void mach3_mu_state::mu_mapping_reference(cast_expr expr,
					  cast_type /*ctype*/,
					  mint_ref itype,
					  pres_c_mapping_reference *rmap)
{
	/* adjust the transport's count to this reference.  This may be 1
	   or more.  If sending a copy, then a 0 will be used instead. */
	assert(rmap->ref_count != 0);
	int ref_adjust = rmap->ref_count;
	cast_expr expr1, expr2;
	
	/* Find the marshaling parameters for this port type.  */
	char *macro_name;
	int size, bits;
	
	get_prim_params(itype, &size, &bits, &macro_name);
	if (!macro_name) {
		panic(("In `mach3_mu_state::mu_mapping_reference', "
		       "invalid MINT type (%d) encountered."),
		      itype);
	}
	cast_expr macro_expr = cast_new_expr_name(macro_name);
	cast_expr ofs_expr = cast_new_expr_lit_int(
		chunk_prim(bits, size), 0);
	
	is_complex = (cast_expr) 1;
	
	if (tag_cexpr) {
		expr1 = cast_new_expr_lit_int(
				-ref_adjust -1/*XXXXXX*/ /* do switch */,
				0);
		expr2 = tag_cexpr;
	}
	else {
		expr1 = cast_new_expr_lit_int(
				(tag_cexpr) ? (-1)/* do switch */
				: rmap->ref_count, 0);
		expr2 = cast_new_expr_name(get_mach_port_type(rmap, itype));
	}
	/* Spit out the marshal/unmarshal macro call.  */
	cast_expr cex = cast_new_expr_call_5(
		macro_expr,
		ofs_expr,
		expr,
		expr1,
		expr2,
		cast_new_expr_name(abort_block->use_current_label()));
	
	add_stmt(cast_new_stmt_expr(cex));
}


char *mach3_mu_state::get_mach_port_type(pres_c_mapping_reference *rmap, mint_ref itype)
{
	/* The type of port right being sent or received */
	const char *port_right;
	
	/* adjust the transport's count to this reference.  This may be 1
	   or more.  If sending a copy, then a 0 will be used instead. */
	assert(rmap->ref_count != 0);
	int ref_adjust = rmap->ref_count; 
	
	switch (rmap->kind) {
	case PRES_C_REFERENCE_COPY:
		if (op & MUST_ENCODE)
			ref_adjust = 0; /* no local references are lost */
		
		switch (m(itype)->mint_def_u.interface_def.right) {
		case MINT_INTERFACE_NAME:
			port_right = "MACH_MSG_TYPE_PORT_NAME";
			break;
		case MINT_INTERFACE_INVOKE: 
			port_right = "MACH_MSG_TYPE_COPY_SEND";
			break;
		case MINT_INTERFACE_INVOKE_ONCE:
			port_right = "MACH_MSG_TYPE_COPY_SEND_ONCE";
			break;
		case MINT_INTERFACE_SERVICE:
			panic("This back end doesn't support passing "
			      "copies of a service object references.");
			break;
		default:
			panic("mach3_mu_state: Invalid mint interface!");
		}
		break;
		
	case PRES_C_REFERENCE_MOVE:
		switch (m(itype)->mint_def_u.interface_def.right) {
		case MINT_INTERFACE_NAME:
			panic("This back end doesn't support moving "
			      "object names to other locations.");
			break;
		case MINT_INTERFACE_INVOKE: 
			port_right = (op & MUST_DECODE)
				     ? "MACH_MSG_TYPE_PORT_SEND"
				     : "MACH_MSG_TYPE_MOVE_SEND";
			break;
		case MINT_INTERFACE_INVOKE_ONCE:
			port_right = (op & MUST_DECODE)
				     ? "MACH_MSG_TYPE_PORT_SEND_ONCE"
				     : "MACH_MSG_TYPE_MOVE_SEND_ONCE";
			break;
		case MINT_INTERFACE_SERVICE:
			port_right = (op & MUST_DECODE)
				     ? "MACH_MSG_TYPE_PORT_RECEIVE"
				     : "MACH_MSG_TYPE_MOVE_RECEIVE";
			break;
		default:
			panic("mach3_mu_state: Invalid mint interface!");
		}
		break;
		
	case PRES_C_REFERENCE_COPY_AND_CONVERT:
		if (op & MUST_ENCODE)
			ref_adjust = 0; /* no local references are lost */
		
		switch (m(itype)->mint_def_u.interface_def.right) {
		case MINT_INTERFACE_NAME: 
			panic("This back end doesn't support passing "
			      "converted copies of an object name.");
			break;
		case MINT_INTERFACE_INVOKE: 
			port_right = "MACH_MSG_TYPE_MAKE_SEND";
			break;
		case MINT_INTERFACE_INVOKE_ONCE:
			port_right = "MACH_MSG_TYPE_MAKE_SEND_ONCE";
			break;
		case MINT_INTERFACE_SERVICE:
			panic("This back end doesn't support passing "
			      "converted copies of a service object "
			      "references.");
			break;
		default:
			panic("mach3_mu_state: Invalid mint interface!");
		}
		break;
		
	default:
		panic("mach3_mu_state: Invalid mapping reference!");
	}
	return flick_asprintf("%s", port_right);
}

void mach3_target_mu_state::mu_mapping_reference(
	cast_expr expr,
	cast_type /*ctype*/,
	mint_ref /*itype*/,
	pres_c_mapping_reference */*rmap*/)
{
	/* Find the marshaling parameters for this port type.  */
	char *macro_name = flick_asprintf("flick_%s_%s_%s_target",
					  get_be_name(), 
					  get_which_stub(), 
					  get_buf_name());
	cast_expr macro_expr = cast_new_expr_name(macro_name);
	
	/* Spit out the marshal/unmarshal macro call.  */
	cast_expr cex = cast_new_expr_call_4(
		macro_expr,
		expr,
		target_remote,
		target_local,
		cast_new_expr_name(get_encode_name()));
	
	add_stmt(cast_new_stmt_expr(cex));
}

void mach3_client_mu_state::mu_mapping_reference(
	cast_expr expr,
	cast_type /*ctype*/,
	mint_ref /*itype*/,
	pres_c_mapping_reference */*rmap*/)
{
	/* Find the marshaling parameters for this port type.  */
	char *macro_name = flick_asprintf("flick_%s_%s_%s_client",
					  get_be_name(), 
					  get_which_stub(), 
					  get_buf_name());
	cast_expr macro_expr = cast_new_expr_name(macro_name);
	
	/* Spit out the marshal/unmarshal macro call.  */
	cast_expr cex = cast_new_expr_call_4(
		macro_expr,
		expr,
		client_remote,
		client_local,
		cast_new_expr_name(get_encode_name()));
	
	add_stmt(cast_new_stmt_expr(cex));
}

/* End of file. */

