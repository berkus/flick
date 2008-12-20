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
#include <mom/libaoi.h>
#include <mom/libmint.h>

/*
 * `get_repository_id' is declared in <mom/c/pg_corbaxx.hh>, but we can't include
 * that C++ header from this C file.
 */
extern char *get_repository_id(aoi_ref aref);

/*
 * This function is used to make CORBA exceptions' discriminator values.
 */
mint_const build_exception_const_string(aoi_ref exception_ref,
					unsigned int exception_num)
{
	return mint_new_const_string(get_repository_id(exception_ref));
}

/* End of file. */

