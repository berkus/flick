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
#include <mom/libmint.h>
#include <mom/c/libpres_c.h>
#include <mom/c/libcast.h>
#include <mom/c/scml.hh>
#include "iiopxx.h"

struct be_state *get_be_state()
{
	return( new iiopxx_be_state );
}

struct be_event *be_state_orb_squelch_handler(struct be_handler *bh,
					      struct be_event *be)
{
	struct be_state *state = (struct be_state *)bh->get_parent();
	data_channel_mask all_channels_mask;
	pres_c_1 *pres = state->get_pres();
	io_file_mask system_mask;
	io_file_index orb_idl;
	unsigned int lpc;
	
	if( be->id != BESE_CLI_ARGS )
		return( be );
	orb_idl = meta_find_file(&pres->meta_data, "orb.idl", 0, 0);
	if( orb_idl != -1 )
		pres->meta_data.files.files_val[orb_idl].flags |=
			IO_FILE_SYSTEM;
	all_channels_mask = meta_make_channel_mask(CMA_TAG_DONE);
	system_mask = meta_make_file_mask(FMA_SetFlags, IO_FILE_SYSTEM,
					  FMA_TAG_DONE);
	meta_squelch_files(&pres->meta_data,
			   &system_mask,
			   &all_channels_mask);
	for( lpc = 0;
	     lpc < pres->unpresented_channels.unpresented_channels_len;
	     lpc++ ) {
		meta_squelch_channels(&pres->meta_data,
				      pres->unpresented_channels.
				      unpresented_channels_val[lpc]);
	}
	return( be );
}

struct be_handler be_state_orb_squelch("orb squelch",
				       1,
				       be_state_orb_squelch_handler);

iiopxx_be_state::iiopxx_be_state()
{
	this->name = "iiopxx";
	cast_language = CAST_CXX;
	this->add_pres_impl(&be_tao_impl);
	this->add_handler(&be_state_orb_squelch);
}

iiopxx_mu_state::iiopxx_mu_state(be_state *_state,
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

iiopxx_mu_state::iiopxx_mu_state(const iiopxx_mu_state &must)
	: mem_mu_state(must)
{
	should_swap = must.should_swap;
	principal_mark = -1;
}

mu_state *iiopxx_mu_state::another(mu_state_op mu_op)
{
	return new iiopxx_mu_state(state, mu_op, assumptions, get_which_stub(),
				   should_swap);
}

mu_state *iiopxx_mu_state::clone()
{
	return new iiopxx_mu_state(*this);
}

iiopxx_target_mu_state::iiopxx_target_mu_state(be_state *_state,
					       mu_state_op mu_op,
					       int mu_assumptions,
					       const char *mu_which,
					       int swap)
	: target_mu_state(_state, mu_op, mu_assumptions, mu_which)
{
	should_swap = swap;
}

iiopxx_target_mu_state::iiopxx_target_mu_state(const iiopxx_target_mu_state &must)
	: target_mu_state(must)
{
	should_swap = must.should_swap;
}

mu_state *iiopxx_target_mu_state::clone()
{
	return new iiopxx_target_mu_state(*this);
}

const char *iiopxx_target_mu_state::get_encode_name() 
{
	if (should_swap == IIOPXX_NO_SWAP || !(op & MUST_DECODE))
		return "cdr";
	else
		return "cdr_swap";
}

const char *iiopxx_target_mu_state::get_be_name()
{
	return "iiopxx";
}

iiopxx_client_mu_state::iiopxx_client_mu_state(be_state *_state,
					       mu_state_op mu_op,
					       int mu_assumptions,
					       const char *mu_which,
					       int swap)
	: client_mu_state(_state, mu_op, mu_assumptions, mu_which)
{
	should_swap = swap;
}

iiopxx_client_mu_state::iiopxx_client_mu_state(const iiopxx_client_mu_state &must)
	: client_mu_state(must)
{
	should_swap = must.should_swap;
}

mu_state *iiopxx_client_mu_state::clone()
{
	return new iiopxx_client_mu_state(*this);
}

const char *iiopxx_client_mu_state::get_encode_name() 
{
	if (should_swap == IIOPXX_NO_SWAP || !(op & MUST_DECODE))
		return "cdr";
	else
		return "cdr_swap";
}

const char *iiopxx_client_mu_state::get_be_name()
{
	return "iiopxx";
}

const char *iiopxx_mu_state::get_encode_name() 
{
	if (should_swap == IIOPXX_NO_SWAP || now_packing || !(op & MUST_DECODE))
		return "cdr";
	else
		return "cdr_swap";
}

const char *iiopxx_mu_state::get_be_name()
{
	return "iiopxx";
}

void
w_header_includes(pres_c_1 * /*p*/)
{
	w_printf("#include <flick/link/iiopxx.h>\n");
	w_printf("#include <flick/encode/cdr.h>\n");
}

/* End of file. */
