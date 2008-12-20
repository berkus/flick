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

#include "private.hh"


void pg_state::gen()
{
	/* Initialize the interface table. */
	p_interface_table_clear();
	
	/* Allow any prepended CAST to be built. */
	build_init_cast();
	
	/* The first question is which interface defs we want to generate
	   presentations for.
	   Current answer: all named interface defs.
	   Later, maybe there'll be a way to get better control.
	   (For example, to get a presentation of an unnamed idef.)  */
	
	gen_aoi_idx = 0;
	gen_scope(0);
	assert((unsigned)gen_aoi_idx == in_aoi->defs.defs_len);
	
	/* Allow the pg to add its own client stubs */
	p_add_builtin_client_func();
}

void pg_state::gen_scope(int scope)
{
	aoi_ref last_idx = cur_aoi_idx;
	while ((gen_aoi_idx < ((aoi_ref) in_aoi->defs.defs_len))
	       && (a(gen_aoi_idx).scope == scope)) {
		/* Convert it to a C definition. */
		cur_aoi_idx = gen_aoi_idx++;
		name = getscopedname(cur_aoi_idx);
		assert(a(cur_aoi_idx).binding);
		p_def(a(cur_aoi_idx).binding);
	}
	cur_aoi_idx = last_idx;
}

/* End of file. */

