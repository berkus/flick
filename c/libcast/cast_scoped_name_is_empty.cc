/*
 * Copyright (c) 1998 The University of Utah and
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
#include <mom/c/libcast.h>

int cast_scoped_name_is_empty(cast_scoped_name *scname)
{
	/*
	 * A scoped name is empty if it has no components, or if it has exactly
	 * one component and that component is the empty string.
	 */
	return ((scname->cast_scoped_name_len == 0)
		|| ((scname->cast_scoped_name_len == 1)
		    && (scname->cast_scoped_name_val[0].name[0] == 0))
		);
}

/* End of file. */

