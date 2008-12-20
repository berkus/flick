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

#include <mom/c/scml.hh>
#include <mom/c/be/be_state.hh>

be_flags be_state::get_default_be_flags()
{
	be_flags res;
	
	res.header			= 0;
	res.prefix			= 0;
	res.system_header		= 0;
	res.inline_file			= 0;
	res.no_timestamp		= 0;
	res.no_included_implementations	= 0;
	res.no_included_declarations	= 0;
	res.all_mu_stubs		= 0;
	res.pres_impl_dirs		= scml_std_include_dirs();
	res.pres_impl			= "flick_defs.scml";
	
	return res;
}

/* End of file. */
