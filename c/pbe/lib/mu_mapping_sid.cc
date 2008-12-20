/*
 * Copyright (c) 1997 The University of Utah and
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

#include <mom/c/libcast.h>

#include <mom/c/pbe.hh>

void mu_state::mu_mapping_sid(cast_expr cexpr, cast_type /*ctype*/, mint_ref /*itype*/,
			      pres_c_mapping_sid *sid_map)
{
	switch (sid_map->kind) {
	case PRES_C_SID_CLIENT:
		/*
		 * ``Marshal'' or ``unmarshal'' a client SID by recording the
		 * CAST expression for the SID (i.e., an expression that refers
		 * to the client SID that appears in the stub parameter list).
		 * We can't actually *do* anything with it, however; the
		 * derived BE must handle that.
		 */
		if (client_sid_cexpr != 0)
			panic("In `mu_state::mu_mapping_sid', "
			      "too many client SIDs.");
		client_sid_cexpr = cexpr;
		break;
		
	case PRES_C_SID_SERVER:
		/*
		 * ``Marshal'' or ``unmarshal'' a server SID by recording the
		 * CAST expression for the SID.  Again, we can't do anything
		 * with the SID other than this.
		 */
		if (server_sid_cexpr != 0)
			panic("In `mu_state::mu_mapping_sid', "
			      "too many server SIDs.");
		server_sid_cexpr = cexpr;
		break;
		
	default:
		panic("In `mu_state::mu_mapping_sid', "
		      "unrecognized SID type %d.",
		      sid_map->kind);
		break;
	}
}

/* End of file. */

