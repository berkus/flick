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

#include <assert.h>
#include <mom/libmint.h>
#include <mom/pres_c.h>
#include <mom/c/libcast.h>
#include <mom/compiler.h>
#include <runtime/headers/flick/pres/all.h>

/* Generate the mappings from FLICK_ERROR codes
   into presentation preferred codes */
void gen_error_mappings( pres_c_1 *out_pres )
{
	cast_expr *error_map;

	out_pres->error_mappings.error_mappings_len = FLICK_ERROR_MAX;
	out_pres->error_mappings.error_mappings_val =
		(cast_expr *)mustcalloc(
			out_pres->error_mappings.error_mappings_len *
			sizeof( cast_expr ) );
	error_map = out_pres->error_mappings.error_mappings_val;
	error_map[FLICK_ERROR_NONE] = cast_new_expr_name( "err_none" );
	error_map[FLICK_ERROR_CONSTANT] =
		cast_new_expr_name( "MIG_BAD_ARGUMENTS" );
	error_map[FLICK_ERROR_VIRTUAL_UNION] =
		cast_new_expr_name( "MIG_BAD_ARGUMENTS" );
	error_map[FLICK_ERROR_STRUCT_UNION] =
		cast_new_expr_name( "MIG_BAD_ARGUMENTS" );
	error_map[FLICK_ERROR_DECODE_SWITCH] =
		cast_new_expr_name( "MIG_BAD_ARGUMENTS" );
	error_map[FLICK_ERROR_COLLAPSED_UNION] =
		cast_new_expr_name( "MIG_BAD_ARGUMENTS" );
	error_map[FLICK_ERROR_VOID_UNION] =
		cast_new_expr_name( "MIG_BAD_ARGUMENTS" );
	error_map[FLICK_ERROR_COMMUNICATION] =
		cast_new_expr_name( "MIG_SERVER_DIED" );
	error_map[FLICK_ERROR_OUT_OF_BOUNDS] =
		cast_new_expr_name( "MIG_BAD_ARGUMENTS" );
	error_map[FLICK_ERROR_INVALID_TARGET] =
		cast_new_expr_name( "MIG_BAD_ARGUMENTS" );
	error_map[FLICK_ERROR_NO_MEMORY] =
		cast_new_expr_name( "MIG_ARRAY_TOO_LARGE" );
}

/* End of file. */

