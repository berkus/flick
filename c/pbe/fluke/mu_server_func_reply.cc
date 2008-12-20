/*
 * Copyright (c) 1996, 1997, 1998, 1999 The University of Utah and
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

#include <mom/c/libcast.h>

#include "fluke.h"

/*
 * Generate the code to marshal and possibly dispatch a reply message from a
 * server.
 */

/*
 * This version of `mu_server_func_reply' is intended to be identical to the
 * library version, *except* that:
 *
 *   (1) we don't process the IDL and interface IDs, and
 *   (2) we emit `start_encode' and `end_encode' macros.
 */

void fluke_mu_state::mu_server_func_reply(pres_c_server_func *sfunc,
					  pres_c_skel *sstub)
{
	mu_state      *must_out = another(MUST_ENCODE | MUST_DEALLOCATE);
	
	mint_ref       simple_reply_itype;
	pres_c_inline  simple_reply_inline;
	
	/*
	 * Strip away the ``collapsed union'' goo that encodes IDL and
	 * interface information.  We don't need to encode that data for Fluke
	 * IPC because it is manifest in the object references.  Similarly,
	 * strip away the ``collapsed union'' goo that represents the
	 * operation's reply code (which is a fixed value --- NOT an indicator
	 * of success or failure).  The client knows what operation it invoked.
	 */
	remove_idl_and_interface_ids(pres,
				     sstub->reply_itype, sfunc->reply_i,
				     &simple_reply_itype, &simple_reply_inline
		);
	remove_operation_id(pres,
			    simple_reply_itype, simple_reply_inline,
			    &simple_reply_itype, &simple_reply_inline
		);
	
	add_stmt(cast_new_stmt_expr(
		cast_new_expr_call(
			cast_new_expr_name("flick_fluke_server_start_encode"),
			0)));
	
	/* Build the reply marshal code. */
	must_out->abort_block = abort_block;
	must_out->arglist = arglist;
	must_out->mu_server_func_target(sfunc);
	must_out->mu_func_params(sfunc->c_func,
				 simple_reply_itype, simple_reply_inline);
	must_out->mu_end();
	
	/* Move the generated code back into our initial `mu_state' object and
	   delete the now unneeded `must_out'. */
	add_stmt(must_out->c_block);
	delete must_out;
	
	add_stmt(cast_new_stmt_expr(
		cast_new_expr_call(
			cast_new_expr_name("flick_fluke_server_end_encode"),
			0)));
	add_stmt(change_stub_state(FLICK_STATE_EPILOGUE));
	add_stmt(cast_new_return(
		cast_new_expr_name(FLUKE_SERVER_NORMAL_REPLY_VALUE)));
}

/* End of file. */

