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

#include <stdio.h>
#include <stdarg.h>
#include <rpc/types.h>
#include <rpc/xdr.h>

#include <mom/mint.h>
#include <mom/cast.h>
#include <mom/pres_c.h>
#include <mom/libmeta.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>
#include <mom/compiler.h>

extern pres_c_1 pres;
extern FILE *fout;

extern void print_cast_ref(int indent, cast_ref cref);
extern void print_stub_cast_ref(int indent, cast_ref cref);
extern void print_mint_ref(int indent, mint_ref mref);
extern void print_pres_c_inline(int indent, pres_c_inline inl);
extern void print_pres_c_mapping(int indent, pres_c_mapping map);
extern void clear_mint_ref_marks();

static void print_stub_op_flags(int indent, pres_c_stub_op_flags op_flags)
{
	w_i_printf(indent, "operation flags:");
	
	/* One `if' for each possible flag. */
	if (op_flags & PRES_C_STUB_OP_FLAG_ONEWAY)
		w_printf(" oneway");
	if (op_flags & PRES_C_STUB_OP_FLAG_IDEMPOTENT)
		w_printf(" idempotent");
	
	/* If there are no flags, say so. */
	if (op_flags == PRES_C_STUB_OP_FLAG_NONE)
		w_printf(" none");
	
	w_printf("\n");
}

static void print_marshal_stub(int indent, pres_c_marshal_stub mstub)
{
	if (mstub.c_func != -1) {
		w_i_printf(indent, "C function: ");
		print_stub_cast_ref(indent+1, mstub.c_func);
	} else
		w_i_printf(indent, "Warning: No C function for stub.\n");
	
	if (mstub.itype != -1) {
		w_i_printf(indent, "MINT interface type:\n");
		print_mint_ref(indent+1, mstub.itype);
	} else
		w_i_printf(indent,
			   "Warning: No MINT interface type for stub.\n");
	
	if (mstub.i) {
		w_i_printf(indent, "PRES_C inline structure:\n");
		print_pres_c_inline(indent+1, mstub.i);
	} else
		w_i_printf(indent,
			   "Warning: No PRES_C inline structure for stub.\n");
	
	w_i_printf(indent, "connection type: (not yet printed)\n");
	
	w_i_printf(indent, "see-thru mapping:\n");
	print_pres_c_mapping(indent+1, mstub.seethru_map);
}

static void print_client_stub(int indent, pres_c_client_stub cstub)
{
	if (cstub.c_func != -1) {
		w_i_printf(indent, "C function: ");
		print_stub_cast_ref(indent+1, cstub.c_func);
	} else
		w_i_printf(indent, "Warning: No C function for stub.\n");
	
	print_stub_op_flags(indent, cstub.op_flags);
	
	if (cstub.request_itype != -1) {
		w_i_printf(indent, "MINT request interface type:\n");
		print_mint_ref(indent+1, cstub.request_itype);
	} else
		w_i_printf(indent,
			   "Warning: No MINT request interface type for stub.\n");
	
	if (cstub.reply_itype != -1) {
		w_i_printf(indent, "MINT reply interface type:\n");
		print_mint_ref(indent+1, cstub.reply_itype);
	} else
		w_i_printf(indent,
			   "Warning: No MINT reply interface type for stub.\n");
	
	if (cstub.target_itype != -1) {
		w_i_printf(indent, "MINT target interface type:\n");
		print_mint_ref(indent+1, cstub.target_itype);
	} else
		w_i_printf(indent,
			   "Warning: No MINT target interface type for stub.\n");
	
	if (cstub.client_itype != -1) {
		w_i_printf(indent, "MINT client interface type:\n");
		print_mint_ref(indent+1, cstub.client_itype);
	} else
		w_i_printf(indent,
			   "Warning: No MINT client interface type for stub.\n");
	
	if (cstub.request_i) {
		w_i_printf(indent, "request inline structure:\n");
		print_pres_c_inline(indent+1, cstub.request_i);
	} else
		w_i_printf(indent,
			   "Warning: No request inline structure for stub.\n");
	
	if (cstub.reply_i) {
		w_i_printf(indent, "reply inline structure:\n");
		print_pres_c_inline(indent+1,cstub.reply_i);
	} else
		w_i_printf(indent,
			   "Warning: No reply inline structure for stub.\n");
	
	w_i_printf(indent,"connection type: (not yet printed)\n");
}

static void print_msg_mu_stub(int indent, pres_c_msg_marshal_stub mstub)
{
	if (mstub.client)
		w_i_printf(indent, "Client ");
	else
		w_i_printf(indent, "Server ");

	if (mstub.request)
		w_i_printf(0, "Request\n");
	else
		w_i_printf(0, "Reply\n");
	
	if (mstub.c_func != -1) {
		w_i_printf(indent, "C function: ");
		print_stub_cast_ref(indent+1, mstub.c_func);
	} else
		w_i_printf(indent, "Warning: No C function for stub.\n");
	
	print_stub_op_flags(indent, mstub.op_flags);
	
	if (mstub.itype != -1) {
		w_i_printf(indent, "MINT interface type:\n");
		print_mint_ref(indent+1, mstub.itype);
	} else
		w_i_printf(indent,
			   "Warning: No MINT interface type for stub.\n");
	
	if (mstub.i) {
		w_i_printf(indent, "inline structure:\n");
		print_pres_c_inline(indent+1, mstub.i);
	} else
		w_i_printf(indent,
			   "Warning: No inline structure for stub.\n");
}

static void print_server_func(int indent, pres_c_server_func func)
{
	if (func.c_func != -1) {
		w_i_printf(indent, "C function: ");
		print_stub_cast_ref(indent+1, func.c_func);
	} else
		w_i_printf(indent, "Warning: No C function for stub.\n");
	
	print_stub_op_flags(indent, func.op_flags);
	
	if (func.target_itype != -1) {
		w_i_printf(indent, "MINT target interface type:\n");
		print_mint_ref(indent+1, func.target_itype);
	} else
		w_i_printf(indent,
			   "Warning: No MINT target interface type for stub.\n");
	
	if (func.client_itype != -1) {
		w_i_printf(indent, "MINT client interface type:\n");
		print_mint_ref(indent+1, func.client_itype);
	} else
		w_i_printf(indent,
			   "Warning: No MINT client interface type for stub.\n");
	
	if (func.request_i) {
		w_i_printf(indent, "request inline structure:\n");
		print_pres_c_inline(indent+1, func.request_i);
	} else
		w_i_printf(indent,
			   "Warning: No request inline structure for stub.\n");
	
	if (func.reply_i) {
		w_i_printf(indent, "reply inline structure:\n");
		print_pres_c_inline(indent+1, func.reply_i);
	} else
		w_i_printf(indent,
			   "Warning: No reply inline structure for stub.\n");
	
	w_i_printf(indent, "connection type: (not yet printed)\n");
}

static void print_receive_func(int indent, pres_c_receive_func func)
{
	if (func.c_func != -1) {
		w_i_printf(indent, "C function: ");
		print_stub_cast_ref(indent+1, func.c_func);
	} else
		w_i_printf(indent, "Warning: No C function for stub.\n");
	
	print_stub_op_flags(indent, func.op_flags);
	
	if (func.target_itype != -1) {
		w_i_printf(indent, "MINT target interface type:\n");
		print_mint_ref(indent+1, func.target_itype);
	} else
		w_i_printf(indent,
			   "Warning: No MINT target interface type for stub.\n");
	
	if (func.client_itype != -1) {
		w_i_printf(indent, "MINT client interface type:\n");
		print_mint_ref(indent+1, func.client_itype);
	} else
		w_i_printf(indent,
			   "Warning: No MINT client interface type for stub.\n");
	
	if (func.msg_i) {
		w_i_printf(indent, "request inline structure:\n");
		print_pres_c_inline(indent+1, func.msg_i);
	} else
		w_i_printf(indent,
			   "Warning: No request inline structure for stub.\n");
	
	w_i_printf(indent, "connection type: (not yet printed)\n");
}

static void print_skel(int indent,pres_c_skel sskel)
{
	u_int i;

	if (sskel.c_def != -1) {
		w_i_printf(indent, "C definition: ");
		print_stub_cast_ref(indent+1, sskel.c_def);
	} else
		w_i_printf(indent, "Warning: No C definition for stub.\n");
	
	if (sskel.request_itype != -1) {
		w_i_printf(indent, "MINT request interface type:\n");
		print_mint_ref(indent+1, sskel.request_itype);
	} else
		w_i_printf(indent, "Warning: No MINT request interface type for stub.\n");
	
	if (sskel.reply_itype != -1) {
		w_i_printf(indent, "MINT reply interface type:\n");
		print_mint_ref(indent+1, sskel.reply_itype);
	} else
		w_i_printf(indent,
			   "Warning: No MINT reply interface type for stub.\n");
	
	w_i_printf(indent, "number of functions: %d\n", sskel.funcs.funcs_len);
	
	for (i = 0; i < sskel.funcs.funcs_len; i++) {
		w_i_printf(indent, "function %d:\n", i);
		switch (sskel.funcs.funcs_val[i].kind) {
		case PRES_C_SERVER_FUNC:
			print_server_func(
				indent + 1,
				sskel.funcs.funcs_val[i].pres_c_func_u.sfunc);
			break;
			
		case PRES_C_RECEIVE_FUNC:
			print_receive_func(
				indent + 1,
				sskel.funcs.funcs_val[i].pres_c_func_u.rfunc);
			break;
		default:
			panic("Unknown function kind seen.");
		}
	}
}

static void print_oneway_stub(int indent, pres_c_msg_stub mstub)
{
	if (mstub.c_func != -1) {
		w_i_printf(indent, "C function: ");
		print_stub_cast_ref(indent+1, mstub.c_func);
	} else
		w_i_printf(indent, "Warning: No C function for stub.\n");
	
	if (mstub.msg_itype != -1) {
		w_i_printf(indent, "MINT message interface type:\n");
		print_mint_ref(indent+1, mstub.msg_itype);
	} else
		w_i_printf(indent,
			   "Warning: No MINT message interface type for stub.\n");
	
	if (mstub.msg_i) {
		w_i_printf(indent, "message inline structure:\n");
		print_pres_c_inline(indent+1, mstub.msg_i);
	} else
		w_i_printf(indent,
			   "Warning: No message inline structure for stub.\n");
	
	w_i_printf(indent, "connection type: (not yet printed)\n");
}

static void print_continue_stub(int indent, pres_c_continue_stub cstub)
{
	if (cstub.c_func != -1) {
		w_i_printf(indent, "C function: ");
		print_stub_cast_ref(indent+1, cstub.c_func);
	} else
		w_i_printf(indent, "Warning: No C function for stub.\n");
	
	if (cstub.itype != -1) {
		w_i_printf(indent, "MINT message interface type:\n");
		print_mint_ref(indent+1, cstub.itype);
	} else
		w_i_printf(indent,
			   "Warning: No MINT message interface type for stub.\n");
	
	if (cstub.i) {
		w_i_printf(indent, "message inline structure:\n");
		print_pres_c_inline(indent+1, cstub.i);
	} else
		w_i_printf(indent,
			   "Warning: No message inline structure for stub.\n");
	
	w_i_printf(indent, "request: %s\n", cstub.request ? "yes" : "no");
}

static const char *pres_c_stub_kind_string(pres_c_stub_kind kind)
{
	switch (kind) {
	default:			return "unknown";
	case PRES_C_MARSHAL_STUB:	return "marshaling stub";
	case PRES_C_UNMARSHAL_STUB:	return "unmarshaling stub";
	case PRES_C_CLIENT_STUB:	return "client stub";
	case PRES_C_SERVER_SKEL:	return "server skeleton";
	case PRES_C_SEND_STUB:		return "send stub";
	case PRES_C_RECV_STUB:		return "receive stub";
	case PRES_C_MESSAGE_MARSHAL_STUB:   return "message marshal stub";
	case PRES_C_MESSAGE_UNMARSHAL_STUB: return "message unmarshal stub";
	case PRES_C_CONTINUE_STUB:	return "continuation stub";
	}
}

void print_pres_c_1()
{
	tag_item ti;
	u_int i;
	
	cast_meta = &pres.meta_data;
	print_meta(&pres.meta_data, fout, 0);
	
	w_printf( "pres.unpresented_channels:\n" );
	for( i = 0;
	     i < pres.unpresented_channels.unpresented_channels_len;
	     i++ ) {
		meta_print_channel_mask(fout, 0,
					pres.unpresented_channels.
					unpresented_channels_val[i]);
		w_printf("\n");
	}
	
	w_printf( "pres.cast:\n" );
	for (i = 0; i < pres.cast.cast_scope_len; i++) {
		cast_w_def(&pres.cast.cast_scope_val[i], 0);
		w_printf("\n");
	}
	
	w_printf( "\npres.stubs_cast:\n" );
	for (i = 0; i < pres.stubs_cast.cast_scope_len; i++) {
		cast_w_def(&pres.stubs_cast.cast_scope_val[i], 0);
		w_printf("\n");
	}
	
	w_printf("number of stubs: %d\n\n", pres.stubs.stubs_len);
	
	clear_mint_ref_marks();
		
	for (i = 0; i < pres.stubs.stubs_len; i++) {
		w_printf("stub %d: %s\n",
			 i,
			 pres_c_stub_kind_string(pres.stubs.stubs_val[i].
						 kind)
			);
		
		switch(pres.stubs.stubs_val[i].kind) {
		case PRES_C_MARSHAL_STUB:
			print_marshal_stub(0, pres.stubs.stubs_val[i].
					   pres_c_stub_u.mstub);
			break;
		case PRES_C_UNMARSHAL_STUB:
			print_marshal_stub(0, pres.stubs.stubs_val[i].
					   pres_c_stub_u.mstub);
			break;
		case PRES_C_CLIENT_STUB:
			print_client_stub(0, pres.stubs.stubs_val[i].
					  pres_c_stub_u.cstub);
			break;
		case PRES_C_CLIENT_SKEL:
			print_skel(0, pres.stubs.stubs_val[i].
				   pres_c_stub_u.cskel);
			break;
			
		case PRES_C_SERVER_SKEL:
			print_skel(0, pres.stubs.stubs_val[i].
				   pres_c_stub_u.sskel);
			break;
			
		case PRES_C_SEND_STUB:
			print_oneway_stub(0, pres.stubs.stubs_val[i].
					  pres_c_stub_u.send_stub);
			break;
			
		case PRES_C_RECV_STUB:
			print_oneway_stub(0, pres.stubs.stubs_val[i].
					  pres_c_stub_u.recv_stub);
			break;
			
		case PRES_C_MESSAGE_MARSHAL_STUB:
			print_msg_mu_stub(0, pres.stubs.stubs_val[i].
					  pres_c_stub_u.mmstub);
			break;
			
		case PRES_C_MESSAGE_UNMARSHAL_STUB:
			print_msg_mu_stub(0, pres.stubs.stubs_val[i].
					  pres_c_stub_u.mustub);
			break;
			
		case PRES_C_CONTINUE_STUB:
			print_continue_stub(0, pres.stubs.stubs_val[i].
					    pres_c_stub_u.continue_stub);
			break;
			
		default:
			panic("Unknown stub type seen.");
		}
		w_printf("\n\n");
	}
	
	if( pres.pres_attrs ) {
		ti.tag = "pres_attrs";
		ti.data.kind = TAG_TAG_LIST;
		ti.data.tag_data_u.tl = pres.pres_attrs;
		print_tag(0, &ti);
	}
}

