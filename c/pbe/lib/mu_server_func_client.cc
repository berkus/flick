/*
 * Copyright (c) 1998, 1999 The University of Utah and
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
#include <mom/c/be/client_mu_state.hh>

void mu_state::mu_server_func_client(pres_c_server_func *sfunc)
{
	/* Don't do anything if there is no client inline. */
	if (sfunc->client_i == 0)
		return;
	
	client_mu_state *must_client
		= mu_make_client_mu_state(state, op,
					  assumptions, get_which_stub());
	
	/*
	 * Make sure that the `client_mu_state' shares the required contextual
	 * structures with the current `mu_state'.
	 */
	must_client->formal_func_invocation_cexpr
		= formal_func_invocation_cexpr;
	must_client->actual_func_invocation_cexpr
		= actual_func_invocation_cexpr;
	
	must_client->arglist = arglist;
	
	must_client->abort_block = abort_block;
	
	/*
	 * Create the CAST code to encode or decode the client object that is
	 * being operated upon.
	 */
	must_client->mu_func_params(sfunc->c_func, sfunc->client_itype,
				    sfunc->client_i);
	must_client->mu_end();
	
	/* Now take that CAST code and add it to the code in `this'. */
	absorb_stmt(must_client->c_block);
}

/* End of file. */

