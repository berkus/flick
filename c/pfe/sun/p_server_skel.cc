/*
 * Copyright (c) 1995, 1996, 1998 The University of Utah and
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
#include <mom/compiler.h>
#include "pg_sun.hh"

/* Generate a server skeleton presentation for an AOI interface. */
int pg_sun::p_skel(aoi_interface *a)
{
	/* This method weeds out the AOI interfaces that represent programs.
	   We want to generate server skeletons for *versions*, but not for
	   *programs*.  A program is essentially just an abstract container.
	   */
	if (a->parents.parents_len == 0) {
		/* This interface represents a Sun RPC program.
		   Do nothing. */
		return -1;
	} else {
		/* This interface represents a Sun RPC version.
		   Pass it along to the real `p_server_skel' method. */
		return pg_state::p_skel(a);
	}
}

