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
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include "mach3.h"

void mach3_mu_state::mu_mapping_argument(cast_expr expr, cast_type ctype,
					 mint_ref itype, pres_c_mapping map)
{
	pres_c_mapping_argument *mamap = &map->pres_c_mapping_u_u.argument;
	
	assert(map->kind == PRES_C_MAPPING_ARGUMENT);
	
	/*
	 * For the mach3mig back end, we don't want array lengths
	 * directly m/u-ed; the length is combined with the type header,
	 * and thus handled in mach3_mu_state::mu_array().
	 */
	assert(mamap->arg_name);
	int old_inhibit_marshal = inhibit_marshal;
	if (strcmp(mamap->arg_name, "length") == 0) {
		/*
		 * XXX - set our magic flag that tells us not to emit
		 * m/u code for this.
		 */
		inhibit_marshal = 1;
	}
	
	mem_mu_state::mu_mapping_argument(expr, ctype, itype, map);
	
	/* Restore the original inhibit flag. */
	inhibit_marshal = old_inhibit_marshal;
}

/* End of file. */
