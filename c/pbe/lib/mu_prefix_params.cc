/*
 * Copyright (c) 1995, 1996, 1997, 1998 The University of Utah and
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

#include <string.h>
#include <stdlib.h>
#include <mom/c/libcast.h>
#include <mom/c/pbe.hh>

void mu_state::mu_prefix_params() 
{
	if (op & (MUST_ENCODE | MUST_DECODE)) {
		cast_stmt comment = cast_new_stmt(CAST_STMT_TEXT);
		comment->cast_stmt_u_u.text =
			flick_asprintf("/* Begin %s phase on parameters */",
				       get_buf_name());
		add_stmt(comment);
	}
}

