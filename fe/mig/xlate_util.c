/*
 * Copyright (c) 1995, 1996, 1997, 1998, 1999 The University of Utah and
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
#include <stdlib.h>
#include <stdarg.h>
#include <rpc/types.h>

#include <mom/c/libcast.h>
#include <mom/pres_c.h>

/* #include "cpu.h" */
#include "type.h"
#include "routine.h"
#include "statement.h"
#include "mom_routines.h"
#include "boolean.h"
#include "xlate_util.h"

/* Variable to store presentation stuff.  Declared in `main.c'. */
extern pres_c_1 out_pres_c;

/*
 * Output an `#include' directive for the given filename.  This function is
 * adapted from `pfe/lib/p_emit_include_stmt.cc'.
 */
void emit_include_stmt(char *filename, int system_only)
{
	u_int i;
	cast_def *cast_defs;
	
	cast_defs = out_pres_c.cast.cast_scope_val;
	
	/*
	 * Check the output CAST to see if we have already output the desired
	 * `#include' statement.  If so, just return.
	 */
	for (i = 0; i < out_pres_c.cast.cast_scope_len; ++i)
		if (cast_defs[i].u.kind == CAST_INCLUDE
		    && !strcmp(filename,
			       cast_defs[i].u.cast_def_u_u.include.filename)
		    && (cast_defs[i].u.cast_def_u_u.include.system_only
			== system_only))
			return;
	
	/* Otherwise... */
	i = cast_add_def(&(out_pres_c.cast),
			 empty_scope_name,
			 CAST_SC_NONE,
			 CAST_INCLUDE,
			 PASSTHRU_DATA_CHANNEL,
			 CAST_PROT_NONE);
	cast_defs = out_pres_c.cast.cast_scope_val; /* Maybe realloc'ed. */
	
	cast_defs[i].u.cast_def_u_u.include.filename = filename;
	cast_defs[i].u.cast_def_u_u.include.system_only = system_only;
}

/*
 * Output an `#include' directive to get Flick's MIG presentation header file.
 * This code is adapted from `pfe/lib/build_init_cast.cc'.
 */
void build_init_cast(void)
{
	emit_include_stmt("flick/pres/mig.h", 1 /* 1 == system header */);
}

/*
 * Debugging `printf'.
 */
void debug_printf(int line, const char *file, const char *format, ...)
{
    va_list vl;
    
    fprintf(stderr, "Line %d in file %s: ", line, file);
    
    va_start(vl, format);
    vfprintf(stderr, format, vl);
    fprintf(stderr, "\n");
    va_end(vl);
}

/* End of file. */

