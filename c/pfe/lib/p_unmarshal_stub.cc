/*
 * Copyright (c) 1995, 1996, 1997 The University of Utah and
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

#include "private.hh"

void pg_state::p_unmarshal_stub(aoi_type /* at */, cast_type ctype_name,
				pres_c_mapping map, pres_c_mapping seethru_map)
{
	p_mu_stub(ctype_name,
		  map,
		  aoi_to_mint_association[cur_aoi_idx],
		  PRES_C_UNMARSHAL_STUB,
		  calc_unmarshal_stub_name(a(cur_aoi_idx).name),
		  seethru_map);
}

/* End of file. */

