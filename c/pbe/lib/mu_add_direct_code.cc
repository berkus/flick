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

#include <assert.h>
#include <mom/c/libcast.h>
#include <mom/c/pbe.hh>

/*
 * Insert a line of code within the scope segement of the current m/u code
 * block (i.e., within the definitions).  This can be used to place "#if 0" 's
 * and other preprocessor directives there.
 */
void mu_state::add_direct_code(char *code_string)
{
	assert(code_string);
	
	/* Make sure the block exists. */
	if (!c_block)
		c_block = cast_new_block(0, 0);
	
	/* Add the line of code. */
	int cs = cast_add_def(&(c_block->cast_stmt_u_u.block.scope),
			      null_scope_name,
			      CAST_SC_NONE,
			      CAST_DIRECT_CODE,
			      PASSTHRU_DATA_CHANNEL,
			      CAST_PROT_NONE);
	cast_def *id = &(c_block->
			 cast_stmt_u_u.block.scope.cast_scope_val[cs]);
	
	id->u.cast_def_u_u.direct.code_string = code_string;
}

/* End of file. */

