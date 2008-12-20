/*
 * Copyright (c) 1995, 1996, 1997, 1999 The University of Utah and
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

#include "pg_sun.hh"
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>
#include <mom/libaoi.h>
#include <assert.h>
#include <mom/compiler.h>

void pg_sun::p_do_exceptional_case(pres_c_inline_virtual_union_case *vucase,
				   mint_union_case *ucase,
				   int /* icase */,
				   pres_c_inline_index errno_idx)
{
	/* Sun doesn't support user exceptions. */
	assert(ucase->var
	       == out_pres->mint.standard_refs.system_exception_ref);
	assert(vucase);
	
	*vucase = pres_c_new_inline_atom(
		errno_idx,
		pres_c_new_mapping(PRES_C_MAPPING_SYSTEM_EXCEPTION));
}

/* End of file. */

