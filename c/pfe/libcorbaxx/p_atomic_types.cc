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
#include <mom/c/libpres_c.h>
#include <mom/libaoi.h>
#include <mom/c/libcast.h>
#include <mom/compiler.h>

#include <mom/c/pg_corbaxx.hh>

cast_expr pg_corbaxx::p_const_translate(aoi_const_def *acd)
{
	aoi_const ac = acd->value;
	
	switch(ac->kind) {
	case AOI_CONST_INT:
		return cast_new_expr_lit_int(ac->aoi_const_u_u.const_int ,0);
	case AOI_CONST_CHAR:
		return cast_new_expr_lit_char(ac->aoi_const_u_u.const_char ,0);
	case AOI_CONST_FLOAT:
		return cast_new_expr_lit_float(ac->aoi_const_u_u.const_float);
	default:
		return pg_state::p_const_translate(acd);
	}
}
