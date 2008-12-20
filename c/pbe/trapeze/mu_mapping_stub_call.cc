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

#include <mom/compiler.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include "trapeze.h"

/*
 * This function appends "_swap" to unmarshaler names if TRAPEZE_SWAP is set.
 */
cast_scoped_name trapeze_mu_state::mu_mapping_stub_call_name(int stub_idx)
{
	cast_scoped_name stub
		= mem_mu_state::mu_mapping_stub_call_name(stub_idx);
	
	if ((should_swap == TRAPEZE_SWAP) && (op & MUST_DECODE)) {
		unsigned int last = stub.cast_scoped_name_len - 1;
		stub.cast_scoped_name_val[last].name
			= flick_asprintf("%s_swap",
					 stub.cast_scoped_name_val[last].name);
	}
	
	return stub;
}

/* End of file. */

