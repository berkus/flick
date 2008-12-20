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

#include "fluke.h"


/*****************************************************************************/

struct be_state *get_be_state()
{
	return( new fluke_be_state );
}

fluke_be_state::fluke_be_state()
{
	this->name = "fluke";
	cast_language = CAST_C;
}

fluke_mu_state::fluke_mu_state(be_state *_state,
			       mu_state_op mu_op,
			       int mu_assumptions,
			       const char *mu_which)
	: mem_mu_state(_state, mu_op, mu_assumptions, 3, 0, 8192, mu_which)
{
	/* Nothing else to do. */
}

fluke_mu_state::fluke_mu_state(const fluke_mu_state &must)
	: mem_mu_state(must)
{
	/* Nothing else to do. */
}

mu_state *fluke_mu_state::another(mu_state_op mu_op)
{
	return new fluke_mu_state(state, mu_op, assumptions, which_stub);
}

mu_state *fluke_mu_state::clone()
{
	return new fluke_mu_state(*this);
}

const char *fluke_mu_state::get_be_name()
{
	return "fluke";
}

const char *fluke_mu_state::get_encode_name()
{
	return "fluke";
}

/*****************************************************************************/

fluke_target_mu_state::fluke_target_mu_state(be_state *_state,
					     mu_state_op mu_op,
					     int mu_assumptions,
					     const char *mu_which)
	: target_mu_state(_state, mu_op, mu_assumptions, mu_which)
{
	/* Nothing else to do. */
}

fluke_target_mu_state::fluke_target_mu_state(const fluke_target_mu_state &must)
	: target_mu_state(must)
{
	/* Nothing else to do. */
}

mu_state *fluke_target_mu_state::clone()
{
	return new fluke_target_mu_state(*this);
}

/* NOTE THAT THIS STUFF NEEDS TO BE CHANGED IN THE INCLUDES, TOO!!!! */

const char *fluke_target_mu_state::get_be_name()
{
	return "fluke";
}

const char *fluke_target_mu_state::get_encode_name()
{
	return "fluke";
}

/*****************************************************************************/

fluke_client_mu_state::fluke_client_mu_state(be_state *_state,
					     mu_state_op mu_op,
					     int mu_assumptions,
					     const char *mu_which)
	: client_mu_state(_state, mu_op, mu_assumptions, mu_which)
{
	/* Nothing else to do. */
}

fluke_client_mu_state::fluke_client_mu_state(const fluke_client_mu_state &must)
	: client_mu_state(must)
{
	/* Nothing else to do. */
}

mu_state *fluke_client_mu_state::clone()
{
	return new fluke_client_mu_state(*this);
}

/* NOTE THAT THIS STUFF NEEDS TO BE CHANGED IN THE INCLUDES, TOO!!!! */

const char *fluke_client_mu_state::get_be_name()
{
	return "fluke";
}

const char *fluke_client_mu_state::get_encode_name()
{
	return "fluke";
}

/*****************************************************************************/

void w_header_includes(pres_c_1 * /*p*/)
{
	/* These should reflect the be & encode names from above */
	w_printf("#include <flick/link/fluke.h>\n");
	w_printf("#include <flick/encode/fluke.h>\n");
}

/* `remove_idl_and_interface_ids' strips away the ``collapsed union'' stuff
   that encodes IDL and interface information.  Fluke client and server stubs
   don't need to encode this information because it is manifest in Fluke
   object references.
   
   Similarly, `remove_operation_id' strips away the ``collapsed union'' stuff
   that encodes an operation identifier.  This information isn't required in
   reply messages.
   */

void remove_idl_and_interface_ids(
	pres_c_1 *pres,
	mint_ref in_itype, pres_c_inline in_inline,
	mint_ref *out_itype, pres_c_inline *out_inline)
{
	*out_itype  = in_itype;
	*out_inline = in_inline;
	
	descend_collapsed_union(pres, out_itype, out_inline);
	descend_collapsed_union(pres, out_itype, out_inline);
}

void remove_operation_id(
	pres_c_1 *pres,
	mint_ref in_itype, pres_c_inline in_inline,
	mint_ref *out_itype, pres_c_inline *out_inline)
{
	*out_itype  = in_itype;
	*out_inline = in_inline;
	
	descend_collapsed_union(pres, out_itype, out_inline);
}

/* End of file. */

