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

#include <mom/compiler.h>
#include <mom/c/be/client_mu_state.hh>

client_mu_state::client_mu_state(be_state *_state,
				 mu_state_op _op,
				 int _assumptions,
				 const char *which)
	: mu_state(_state, _op, _assumptions, which)
{
}

mu_state *client_mu_state::another(mu_state_op /*op*/)
{
	panic("client_mu_state::another should never be called.");
	return 0;
}

void client_mu_state::mu_mapping_simple(cast_expr /*expr*/,
					cast_type /*ctype*/,
					mint_ref /*itype*/)
{
	panic("client_mu_state::mu_mapping_simple should never be called.");
}

target_mu_state *client_mu_state::mu_make_target_mu_state(
	be_state * /*state*/,
	mu_state_op /*op*/,
	int /*assumptions*/,
	const char * /*which*/)
{
	panic(("client_mu_state::mu_make_target_mu_state should never be "
	       "called."));
	return 0;
}

client_mu_state *client_mu_state::mu_make_client_mu_state(
	be_state * /*state*/,
	mu_state_op /*op*/,
	int /*assumptions*/,
	const char * /*which*/)
{
	panic(("client_mu_state::mu_make_client_mu_state should never be "
	       "called."));
	return 0;
}

/* End of file. */

