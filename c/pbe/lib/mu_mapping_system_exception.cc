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

#include <assert.h>

#include <mom/compiler.h>
#include <mom/libmint.h>
#include <mom/c/libcast.h>
#include <mom/c/pbe.hh>

/* This handles PRES_C_MAPPING_SYSTEM_EXCEPTION nodes,
   which are used to opaquely represent system exceptions.
   They're opaque, because their presentation varies greatly.
*/
void mu_state::mu_mapping_system_exception(cast_expr expr,
					   cast_type /* ctype */,
					   mint_ref itype)
{
	mint_def *idef = &pres->mint.defs.defs_val[itype];
	
	assert(idef->kind == MINT_SYSTEM_EXCEPTION);
	
	// This is a system exception - deal with it separately
	// Not sure if we should break the chunk &/| glob...
        // ECP - We must break the glob until we can specialize the different
	//       back-ends to how each handles system exceptions best.
	break_glob(); 
	cast_expr macro = cast_new_expr_name(
		flick_asprintf("flick_%s_%s_system_exception",
			       pres->pres_context,
			       get_buf_name()));
	
	cast_expr cex = cast_new_expr_call_4(
		macro,
		expr,
		cast_new_expr_name(get_encode_name()),
		cast_new_expr_name(get_be_name()),
		/* It's possible for some exceptions to cause an
		   error if they were marshaling or not, so we need
		   to pass in an abort label */
		cast_new_expr_name(abort_block->use_current_label()));
	
	add_stmt(cast_new_stmt_expr(cex));
}

/* End of file. */

