/*
 * Copyright (c) 1995, 1996, 1998, 1999 The University of Utah and
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
/* w_stub - write out a stub function */

#include <mom/compiler.h>
#include <mom/c/pbe.hh>
#include <mom/c/libpres_c.h>

extern FILE *server_file;
extern FILE *client_file;

void w_stub(pres_c_1 *pres, int idx)
{
	pres_c_stub *stub = &pres->stubs.stubs_val[idx];
	switch (stub->kind) {
	case PRES_C_MARSHAL_STUB:
	case PRES_C_UNMARSHAL_STUB:
		panic("w_stub: cannot process marshal/unmarshal stubs here");
	case PRES_C_CLIENT_STUB:
		w_client_stub(pres, idx);
		break;
	case PRES_C_CLIENT_SKEL:
	case PRES_C_SERVER_SKEL:
		w_skel(pres, idx);
		break;
	case PRES_C_SEND_STUB:
		w_send_stub(pres, idx);
		break;
	case PRES_C_RECV_STUB:
		w_recv_stub(pres, idx);
		break;
	case PRES_C_MESSAGE_MARSHAL_STUB:
		w_msg_marshal_stub(pres, idx);
		break;
	case PRES_C_MESSAGE_UNMARSHAL_STUB:
		w_msg_unmarshal_stub(pres, idx);
		break;
	case PRES_C_CONTINUE_STUB:
		w_continue_stub(pres, idx);
		break;
	default:
		panic("w_stub: unknown stub kind %d", stub->kind);
	}
}

