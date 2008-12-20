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
#include <mom/c/libcast.h>
#include <mom/libmint.h>
#include "sun.h"

struct be_state *get_be_state()
{
	return( new sun_be_state );
}

sun_be_state::sun_be_state()
{
	this->name = "suntcp";
	cast_language = CAST_C;
}

sun_mu_state::sun_mu_state(be_state *_state,
			   mu_state_op mu_op,
			   int mu_assumptions,
			   const char *mu_which)
	: mem_mu_state(_state, mu_op, mu_assumptions, 2, 0, 8192/*XXX*/,
		       mu_which)
{
	in_packed_array = 0;
}

sun_mu_state::sun_mu_state(const sun_mu_state &must)
	: mem_mu_state(must)
{
	in_packed_array = must.in_packed_array;
}

mu_state *sun_mu_state::another(mu_state_op mu_op)
{
	return new sun_mu_state(state, mu_op, assumptions, which_stub);
}

mu_state *sun_mu_state::clone()
{
	return new sun_mu_state(*this);
}

void sun_mu_state::get_prim_params(mint_ref itype,
				   int *out_size, int *out_align_bits,
				   char **out_macro_name)
{
	mem_mu_state::get_prim_params(itype,
				      out_size, out_align_bits,
				      out_macro_name);

	if (in_packed_array) {
		/* Packed arrays are either strings or arrays of opaque data;
		   the size is always one, and no required alignment. */
		assert(*out_size == 1);
		assert(*out_align_bits == 0);
		*out_macro_name = flick_asprintf("%s_packed", *out_macro_name);
        } else {
		if (*out_size < 4)
			*out_size = 4;
		*out_align_bits = 2;
	}
}

sun_target_mu_state::sun_target_mu_state(be_state *_state,
					 mu_state_op mu_op,
					 int mu_assumptions,
					 const char *mu_which)
	: target_mu_state(_state, mu_op, mu_assumptions, mu_which)
{
}

sun_target_mu_state::sun_target_mu_state(const sun_target_mu_state &must)
	: target_mu_state(must)
{
}

mu_state *sun_target_mu_state::clone()
{
	return new sun_target_mu_state(*this);
}

const char *sun_target_mu_state::get_encode_name()
{
	return "xdr";
}

const char *sun_target_mu_state::get_be_name()
{
	return "suntcp";
}

sun_client_mu_state::sun_client_mu_state(be_state *_state,
					 mu_state_op mu_op,
					 int mu_assumptions,
					 const char *mu_which)
	: client_mu_state(_state, mu_op, mu_assumptions, mu_which)
{
}

sun_client_mu_state::sun_client_mu_state(const sun_client_mu_state &must)
	: client_mu_state(must)
{
}

mu_state *sun_client_mu_state::clone()
{
	return new sun_client_mu_state(*this);
}

const char *sun_client_mu_state::get_encode_name()
{
	return "xdr";
}

const char *sun_client_mu_state::get_be_name()
{
	return "suntcp";
}

const char *sun_mu_state::get_be_name()
{
	return "suntcp";
}

const char *sun_mu_state::get_encode_name()
{
	return "xdr";
}

void
w_header_includes(pres_c_1 *p)
{
	int prognum = 0x56789abc;

	w_printf("#include <flick/link/suntcp.h>\n");
	w_printf("#include <flick/encode/xdr.h>\n");
	// XXX - This will output any PROGRAM & VERSION macros for use with
	// clnt_create functions.
	// Eventually this should be changed to allow a cmd-line setting for
	// The prognum value.
	for (int i = 0; i < (signed int) p->a.defs.defs_len; i++) {
		if (p->a.defs.defs_val[i].binding &&
		    (p->a.defs.defs_val[i].binding->kind == AOI_INTERFACE)){
			if (p->a.defs.defs_val[i].binding->aoi_type_u_u.
			    interface_def.idl != AOI_IDL_SUN) {
				w_printf("#define _PROG_%s "
					 "((u_long)%d)\n",
					 p->a.defs.defs_val[i].name,
					 prognum++);
				w_printf("#define _VERS_%s "
					 "((u_long)1)\n",
					 p->a.defs.defs_val[i].name);
			}
		}
	}
}

/* End of file. */

