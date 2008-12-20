/*
 * Copyright (c) 1995, 1996, 1997, 1999 The University of Utah and
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

#include <stdio.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>
#include <mom/compiler.h>
#include <mom/c/pfe.hh>

#include "../macros.h"

enum p_params_msg_type
{
	P_PARAMS_STUB_REQUEST = 1,
	P_PARAMS_STUB_REPLY = 2
};

/* Macro to define cast scope */
/* #define c_scope out_pres->cast */

/* Macro to define stub's location in the PRES_C */
/* #define c_stubs out_pres->stubs */

/* Lookup to get at func def inside CAST */
/* #define cf(n) (out_pres->cast.cast_scope_val[n].u.cast_def_u_u.func_type) */

/* Lookup to get at include def inside CAST */
/* #define i(n) (out_pres->cast.cast_scope_val[n].u.cast_def_u_u.include) */

FILE **cmdline(pg_state *p, int argc, char *argv[]);
pg_state *getGenerator();

/* End of file. */



