/*
 * Copyright (c) 1999 The University of Utah and
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

#include <mom/compiler.h>
#include <mom/libmint.h>
#include <mom/c/libcast.h>
#include <mom/c/pbe.hh>

/*
 * This handles PRES_C_INLINE_ILLEGAL nodes, which indicate that the current
 * pres branch is not a legal or valid generated code path.  We generate code
 * to signal an error if this inline is reached.
 */

void mu_state::mu_inline_illegal(inline_state */*ist*/, mint_ref /*itype*/,
				 pres_c_inline /*inl*/)
{
	add_stmt(make_error(FLICK_ERROR_CONSTANT));
}

/* End of file. */

