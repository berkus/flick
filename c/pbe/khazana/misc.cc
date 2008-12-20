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

#include <assert.h>
#include <mom/libmint.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>
#include "khazana.h"

struct be_state *get_be_state()
{
	return( new khazana_be_state );
}

khazana_be_state::khazana_be_state()
{
	this->name = "khazana";
	cast_language = CAST_C;
}

khazana_mu_state::khazana_mu_state(be_state *_state,
				   mu_state_op mu_op,
				   int mu_assumptions,
				   const char *mu_which,
				   int swap)
	: mem_mu_state(_state, mu_op, mu_assumptions, 2, 0, 8192/*XXX*/,
		       mu_which)
{
	should_swap = swap;
	principal_mark = -1;
}

khazana_mu_state::khazana_mu_state(const khazana_mu_state &must)
	: mem_mu_state(must)
{
	should_swap = must.should_swap;
	principal_mark = -1;
}

mu_state *khazana_mu_state::another(mu_state_op mu_op)
{
	return new khazana_mu_state(state, mu_op, assumptions,
				    get_which_stub(),
				    should_swap);
}

mu_state *khazana_mu_state::clone()
{
	return new khazana_mu_state(*this);
}

khazana_target_mu_state::khazana_target_mu_state(be_state *_state,
						 mu_state_op mu_op,
						 int mu_assumptions,
						 const char *mu_which,
						 int swap)
	: target_mu_state(_state, mu_op, mu_assumptions, mu_which)
{
	should_swap = swap;
}

khazana_target_mu_state::khazana_target_mu_state(const khazana_target_mu_state &must)
	: target_mu_state(must)
{
	should_swap = must.should_swap;
}

mu_state *khazana_target_mu_state::clone()
{
	return new khazana_target_mu_state(*this);
}

const char *khazana_target_mu_state::get_encode_name() 
{
	if (should_swap == KHAZANA_NO_SWAP || !(op & MUST_DECODE))
		return "cdr";
	else
		return "cdr_swap";
}

const char *khazana_target_mu_state::get_be_name()
{
	return "khazana";
}

khazana_client_mu_state::khazana_client_mu_state(be_state *_state,
						 mu_state_op mu_op,
						 int mu_assumptions,
						 const char *mu_which,
						 int swap)
	: client_mu_state(_state, mu_op, mu_assumptions, mu_which)
{
	should_swap = swap;
}

khazana_client_mu_state::khazana_client_mu_state(const khazana_client_mu_state &must)
	: client_mu_state(must)
{
	should_swap = must.should_swap;
}

mu_state *khazana_client_mu_state::clone()
{
	return new khazana_client_mu_state(*this);
}

const char *khazana_client_mu_state::get_encode_name() 
{
	if (should_swap == KHAZANA_NO_SWAP || !(op & MUST_DECODE))
		return "cdr";
	else
		return "cdr_swap";
}

const char *khazana_client_mu_state::get_be_name()
{
	return "khazana";
}

const char *khazana_mu_state::get_encode_name() 
{
	if (should_swap == KHAZANA_NO_SWAP || now_packing || !(op & MUST_DECODE))
		return "cdr";
	else
		return "cdr_swap";
}

const char *khazana_mu_state::get_be_name()
{
	return "khazana";
}

void
w_header_includes(pres_c_1 * /*p*/)
{
	w_printf("#include <flick/link/khazana.h>\n");
	w_printf("#include <flick/encode/cdr.h>\n");
}

/* End of file. */
