/*
 * Copyright (c) 1995, 1996 The University of Utah and
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

#include <mom/c/pbe.hh>

/* These are the `mu_state' versions of the glob management methods.
   Individual back ends are expected to override these methods in order to
   implement whatever style of memory management they require. */

void mu_state::new_glob()
{
	/* Do nothing. */
}

void mu_state::make_glob()
{
	/* Do nothing. */
}

void mu_state::break_glob()
{
	/* Do nothing. */
}

void mu_state::end_glob()
{
	/* Do nothing. */
}

void mu_state::glob_grow(int /*amount*/)
{
	/* Do nothing. */
}

void mu_state::glob_prim(int /*needed_align_bits*/, int /*prim_size*/)
{
	/* Do nothing. */
}


