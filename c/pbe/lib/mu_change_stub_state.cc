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

#include <assert.h>

#include <mom/c/libcast.h>
#include <mom/c/pbe.hh>

const char *flick_state_names[] = {
	"FLICK_STATE_NONE",
	"FLICK_STATE_PROLOGUE",
	"FLICK_STATE_MARSHAL",
	"FLICK_STATE_SEND",
	"FLICK_STATE_SEND_RECEIVE",
	"FLICK_STATE_FUNCTION_CALL",
	"FLICK_STATE_FUNCTION_RETURN",
	"FLICK_STATE_RECEIVE",
	"FLICK_STATE_UNMARSHAL",
	"FLICK_STATE_EPILOGUE",
	"FLICK_STATE_MAX",
};

cast_stmt mu_state::change_stub_state(int stub_state)
{
	assert(stub_state >= FLICK_STATE_NONE);
	assert(stub_state < FLICK_STATE_MAX);
	
	/* Set the global stub stub_state variable. */
	return (cast_new_stmt_expr(
		cast_new_expr_assign(
			cast_new_expr_sel(cast_new_expr_name("_stub_state"),
					  cast_new_scoped_name("state", NULL)),
			cast_new_expr_name(flick_state_names[stub_state])
			)
		));
}

/* End of file. */

