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
#include <mom/libmeta.h>
#include <mom/c/libpres_c.h>
#include <mom/c/pg_corbaxx.hh>
#include <mom/c/libcast.h>
#include "../macros.h"

void pg_corbaxx::gen()
{
	/* Initialize the interface table. */
	p_interface_table_clear();
	
	/* Allow any prepended CAST to be built. */
	build_init_cast();
	
	push_ptr(poa_scope_stack, root_scope);
	
	gen_aoi_idx = 0;
	gen_scope(0);
	assert((unsigned)gen_aoi_idx == in_aoi->defs.defs_len);
	
	/* Add any builtin client stubs */
	p_add_builtin_client_func();
	
	if( gen_client ) {
		pres_c_add_unpresented_channel(
			out_pres,
			meta_make_channel_mask(CMA_MatchesID, "server",
					       CMA_TAG_DONE));
	} else if( gen_server ) {
		pres_c_add_unpresented_channel(
			out_pres,
			meta_make_channel_mask(CMA_MatchesID, "client",
					       CMA_TAG_DONE));
	}
}

void pg_corbaxx::gen_scope(int scope)
{
	aoi_ref last_cur_aoi_idx = cur_aoi_idx;
	while ( (gen_aoi_idx < ((aoi_ref) in_aoi->defs.defs_len)) &&
		(a(gen_aoi_idx).scope == scope) ) {
		/* Convert it to a C definition. */
		cur_aoi_idx = gen_aoi_idx++;
		name = getscopedname(cur_aoi_idx);
		assert(a(cur_aoi_idx).binding);
		p_def(a(cur_aoi_idx).binding);
	}
	cur_aoi_idx = last_cur_aoi_idx;
}

/* End of file. */

