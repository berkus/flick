/*
 * Copyright (c) 1997, 1998, 1999 The University of Utah and
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

#include "fluke.h"

void fluke_mu_state::get_prim_params(mint_ref itype,
				     int *out_size,
				     int *out_align_bits,
				     char **out_macro_name)
{
	mint_def *def;
	
	/* By default, do the `mem_mu_state' thing. */
	mem_mu_state::get_prim_params(itype,
				      out_size, out_align_bits,
				      out_macro_name);
	
	/*
	 * Now handle our special cases.
	 */
	def = &(pres->mint.defs.defs_val[itype]);
	
	if (def->kind == MINT_INTERFACE) {
		/*
		 * Because port references are marshaled in a buffer separate
		 * from the normal data marshal buffer, ports are effectively
		 * zero-byte objects.
		 *
		 * However, with every port reference, we send along a 32-bit
		 * flags word to support such things as MOM notifications.
		 * So, instead of being zero-byte objects, ports appear to be
		 * 4-byte objects.
		 */
		*out_size = 4;
		*out_align_bits = 2;
		
		/*
		 * Further, the runtime needs to do different things for client
		 * and server so that MOM can do notifications.  So we must
		 * manufacture a special macro name, too, based on the value of
		 * `get_which_stub'.
		 */
		*out_macro_name = flick_asprintf("flick_%s_%s_%s_%s%d",
						 get_encode_name(),
						 get_which_stub(),
						 get_buf_name(),
						 "port",
						 ((*out_size) * 8));
	}
}

/* End of file. */

