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

#include <mom/compiler.h>
#include <mom/libaoi.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include <mom/c/pg_corba.hh>

void pg_corba::p_any_type(p_type_collection **/*out_ptc*/)
{
	/*
	 * We should only get here if we have an `AOI_ANY' that is not in the
	 * `type' field of a containing `AOI_TYPED'.  (See `p_typed_type'.)
	 *
	 * In other words, if we get here, we have some sort of standalone,
	 * *untyped* `any' value, and there's no standard CORBA presentation
	 * for that.
	 */
	panic("In `pg_corba::p_any_type', "
	      "there is no standard CORBA presentation for untyped `any' "
	      "values.");
}

/* End of file. */

