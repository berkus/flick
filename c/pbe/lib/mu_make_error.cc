/*
 * Copyright (c) 1995, 1996, 1998, 1999 The University of Utah and
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

/* These strings must correspond to the #defines found in
   `runtime/headers/flick/pres/all.h'.  */
const char *flick_error_names[] = {
	"FLICK_ERROR_NONE",
	"FLICK_ERROR_CONSTANT",
	"FLICK_ERROR_VIRTUAL_UNION",
	"FLICK_ERROR_STRUCT_UNION",
	"FLICK_ERROR_DECODE_SWITCH",
	"FLICK_ERROR_COLLAPSED_UNION",
	"FLICK_ERROR_VOID_UNION",
	"FLICK_ERROR_COMMUNICATION",
	"FLICK_ERROR_OUT_OF_BOUNDS",
	"FLICK_ERROR_INVALID_TARGET",
	"FLICK_ERROR_NO_MEMORY",
	"FLICK_ERROR_MAX"
};

cast_stmt mu_state::make_error(int err_val) 
{
	cast_expr call;
	
	assert(err_val >= FLICK_ERROR_NONE);
	assert(err_val < FLICK_ERROR_MAX);
	
	call = cast_new_expr_call_2(
		cast_new_expr_name("flick_stub_error"),
		cast_new_expr_name(flick_error_names[err_val]),
		cast_new_expr_name(abort_block->use_current_label())
		);
	return cast_new_stmt_expr(call);
}

/* End of file. */

