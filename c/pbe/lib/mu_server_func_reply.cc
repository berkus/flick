/*
 * Copyright (c) 1995, 1996, 1998 The University of Utah and
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

#include <mom/c/pbe.hh>

/* Generate the code to marshal and possibly dispatch a reply message from a
   server. */

void mu_state::mu_server_func_reply(pres_c_server_func *sfunc,
				    pres_c_skel *sstub)
{
	/* Build the reply marshal code. */
	mu_state *must_out = another(MUST_ENCODE | MUST_DEALLOCATE);

	must_out->abort_block = abort_block;
	must_out->c_block = c_block;
	must_out->arglist = arglist;
	must_out->mu_server_func_target(sfunc);
	must_out->mu_func_params(sfunc->c_func,
				 sstub->reply_itype,
				 sfunc->reply_i);
	must_out->mu_end();
	
	/* Move the generated code back into our initial `mu_state' object and
	   delete the now unneeded `must_out'. */
	c_block = must_out->c_block;
	delete must_out;
}

/* End of file. */

