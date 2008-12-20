/*
 * Copyright (c) 1997, 1998, 1999 The University of Utah and
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

#include "trapeze.h"

void trapeze_mu_state::mu_mapping_reference(cast_expr /*expr*/,
					    cast_type /* ctype */,
					    mint_ref /*itype*/,
					    pres_c_mapping_reference */*rmap*/)
{
	/*
	 * XXX --- We don't yet handle object reference parameters in the
	 * Trapeze back end.
	 */
	static int warned = 0;
	
	if (!warned++)
		warn(("This back end does not yet support object reference "
		      "parameters."));
	
	/*
	 * Create a `c_block' anyway, because some other functions (`mu_array')
	 * expect that we will create some code.
	 */
	if (!c_block)
		c_block = cast_new_block(0, 0);
		
}

/* End of file. */

