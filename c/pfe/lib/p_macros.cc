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

#include "private.hh"

aoi_def &pg_state::a(int i) {
	return in_aoi->defs.defs_val[i];
}

mint_def &pg_state::m(int i) {
	return out_pres->mint.defs.defs_val[i];
}

cast_def &pg_state::c(int i) {
	return ((cast_scope *)top_ptr(scope_stack))->cast_scope_val[i];
}

pres_c_stub &pg_state::s(int i) {
	return out_pres->stubs.stubs_val[i];
}

cast_func_type &pg_state::cf(int n) {
	return ((cast_scope *)top_ptr(scope_stack))->cast_scope_val[n].u.
		cast_def_u_u.func_type;
}

data_channel_index &pg_state::ch(aoi_ref ar, int channel_type) {
	return( pg_channel_maps[channel_type][a(ar).idl_file] );
}

/* End of file. */
