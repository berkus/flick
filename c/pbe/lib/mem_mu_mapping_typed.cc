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

#include <mom/compiler.h>
#include <mom/c/be/mem_mu_state.hh>

/*
 * This method handles `PRES_C_MAPPING_TYPED' presentations for memory-buffer-
 * based stubs.
 *
 * For details and opinions about `PRES_C_MAPPING_TYPED', see the comments for
 * `mu_state::mu_mapping_typed'.
 */
void
mem_mu_state::mu_mapping_typed(
	cast_expr cexpr,
	cast_type ctype,
	mint_ref itype
	)
{
	/*
	 * `mu_state::mu_mapping_typed' produces a special macro call to
	 * process the type-tagged value.  In `mem_mu_state's, this macro must
	 * be invoked outside of any active glob or chunk; it manages the
	 * stream itself.
	 */
	break_glob();
	mu_state::mu_mapping_typed(cexpr, ctype, itype);
	
	/*
	 * Fix our memory alignment state.  After processing a type-tagged
	 * value, we are left with just 1-byte alignment.  Sigh.
	 */
	max_msg_size = MAXUINT_MAX;
	align_bits = 0;
	align_ofs = 0;
}

/* End of file. */

