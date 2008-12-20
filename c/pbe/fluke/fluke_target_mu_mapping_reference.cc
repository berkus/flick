/*
 * Copyright (c) 1996, 1997 The University of Utah and
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

#include "fluke.h"

/*
 * This version of `mu_mapping_reference' doesn't output any CAST directly.
 * Rather, it sets the `flick_target_mu_state' object's `target_cast_expr' and
 * `target_cast_type' slots to be expressions that refers to the object.  This
 * expression is later used to construct the statements that actually invoke
 * the RPC.  See the file `client.cc' for more information.
 */

void fluke_target_mu_state::mu_mapping_reference(
	cast_expr expr,
	cast_type ctype,
	mint_ref /*itype*/,
	pres_c_mapping_reference * /*rmap*/)
{
	target_cast_expr = expr;
	target_cast_type = ctype;
}

/* End of file. */

