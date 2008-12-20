/*
 * Copyright (c) 1995, 1996, 1997, 1999 The University of Utah and
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

#include <stdlib.h>
#include <string.h>

#include <mom/c/libcast.h>
#include "sun.h"

void sun_mu_state::mu_prefix_params() 
{
	if ((op & MUST_ENCODE) &&
	    !strcmp("client", which_stub)) {
		cast_stmt comment;
		comment = cast_new_text(("/* Encode the 'authorization' stuff "
					 "(We're currently assuming it's ALL "
					 "NULL) */"));
		add_stmt(comment);
		
		cast_expr cexpr_zero
			= cast_new_expr_lit_int(0, CAST_MOD_UNSIGNED);
		cast_type ctype_ulong
			= cast_new_prim_type(CAST_PRIM_INT, CAST_MOD_UNSIGNED);
		
		mu_mapping_simple(cexpr_zero,
				  ctype_ulong,
				  pres->mint.standard_refs.unsigned32_ref);
		mu_mapping_simple(cexpr_zero,
				  ctype_ulong,
				  pres->mint.standard_refs.unsigned32_ref);
		mu_mapping_simple(cexpr_zero,
				  ctype_ulong,
				  pres->mint.standard_refs.unsigned32_ref);
		mu_mapping_simple(cexpr_zero,
				  ctype_ulong,
				  pres->mint.standard_refs.unsigned32_ref);
	}
	
	if ((op & MUST_DECODE)  &&
	    !strcmp("server", which_stub)) {
		cast_type ctype_ulong
			= cast_new_prim_type(CAST_PRIM_INT, CAST_MOD_UNSIGNED);
		cast_expr cexpr_toss
			= add_temp_var("ignore", ctype_ulong);
		
		cast_stmt comment;
		comment = cast_new_text(("/* Decode the authorization stuff "
					 "(we ignore it & assume it's null) "
					 "*/"));
		add_stmt(comment);
		
		mu_mapping_simple(cexpr_toss,
				  ctype_ulong,
				  pres->mint.standard_refs.unsigned32_ref);
		mu_mapping_simple(cexpr_toss,
				  ctype_ulong,
				  pres->mint.standard_refs.unsigned32_ref);
		mu_mapping_simple(cexpr_toss,
				  ctype_ulong,
				  pres->mint.standard_refs.unsigned32_ref);
		mu_mapping_simple(cexpr_toss,
				  ctype_ulong,
				  pres->mint.standard_refs.unsigned32_ref);
	} 
}

/* End of file. */

