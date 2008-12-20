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
#include "iiop.h"

struct be_state *get_be_state()
{
	return( new iiop_be_state );
}

iiop_be_state::iiop_be_state()
{
	this->name = "iiop";
	cast_language = CAST_C;
}

iiop_mu_state::iiop_mu_state(be_state *_state,
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

iiop_mu_state::iiop_mu_state(const iiop_mu_state &must)
	: mem_mu_state(must)
{
	should_swap = must.should_swap;
	principal_mark = -1;
}

mu_state *iiop_mu_state::another(mu_state_op mu_op)
{
	return new iiop_mu_state(state, mu_op, assumptions, get_which_stub(),
				 should_swap);
}

mu_state *iiop_mu_state::clone()
{
	return new iiop_mu_state(*this);
}

iiop_target_mu_state::iiop_target_mu_state(be_state *_state,
					   mu_state_op mu_op,
					   int mu_assumptions,
					   const char *mu_which,
					   int swap)
	: target_mu_state(_state, mu_op, mu_assumptions, mu_which)
{
	should_swap = swap;
}

iiop_target_mu_state::iiop_target_mu_state(const iiop_target_mu_state &must)
	: target_mu_state(must)
{
	should_swap = must.should_swap;
}

mu_state *iiop_target_mu_state::clone()
{
	return new iiop_target_mu_state(*this);
}

const char *iiop_target_mu_state::get_encode_name() 
{
	if (should_swap == IIOP_NO_SWAP || !(op & MUST_DECODE))
		return "cdr";
	else
		return "cdr_swap";
}

const char *iiop_target_mu_state::get_be_name()
{
	return "iiop";
}

iiop_client_mu_state::iiop_client_mu_state(be_state *_state,
					   mu_state_op mu_op,
					   int mu_assumptions,
					   const char *mu_which,
					   int swap)
	: client_mu_state(_state, mu_op, mu_assumptions, mu_which)
{
	should_swap = swap;
}

iiop_client_mu_state::iiop_client_mu_state(const iiop_client_mu_state &must)
	: client_mu_state(must)
{
	should_swap = must.should_swap;
}

mu_state *iiop_client_mu_state::clone()
{
	return new iiop_client_mu_state(*this);
}

const char *iiop_client_mu_state::get_encode_name() 
{
	if (should_swap == IIOP_NO_SWAP || !(op & MUST_DECODE))
		return "cdr";
	else
		return "cdr_swap";
}

const char *iiop_client_mu_state::get_be_name()
{
	return "iiop";
}

const char *iiop_mu_state::get_encode_name() 
{
	if (should_swap == IIOP_NO_SWAP || now_packing || !(op & MUST_DECODE))
		return "cdr";
	else
		return "cdr_swap";
}

const char *iiop_mu_state::get_be_name()
{
	return "iiop";
}

void
w_header_includes(pres_c_1 * /*p*/)
{
	w_printf("#include <flick/link/iiop.h>\n");
	w_printf("#include <flick/encode/cdr.h>\n");
}

/* End of file. */
