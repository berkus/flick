/*
 * Copyright (c) 1995, 1996, 1999 The University of Utah and
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
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <rpc/types.h>
#include <rpc/xdr.h>
#include <mom/compiler.h>
#include <mom/libaoi.h>
#include <mom/libmeta.h>
#include <assert.h>

#include "rpc_parse.h"
#include "rpc_util.h"

extern int crash();
extern void translate();

char *progname;

definition *defs;
extern aoi outaoi;
io_file_index root_file = -1;
io_file_index current_i_file = -1;
const char *root_filename = 0;

static void read_defs(void)
{
	definition *def, **last_def;

	last_def = &defs;
	while ((def = get_definition()))
	{
		*last_def = def;
		last_def = &def->next;
	}
	*last_def = 0;
}

int main(int argc, char **argv)
{
	/* We're just a simple filter.  */
	XDR xdrs;
	FILE *out;
	fe_flags res = front_end_args(argc, argv, "Note that the XDR '%' option for inlining code is NOT supported");

	if (!res.nocpp) {
		fin = call_c_preprocessor(res.input, res.cpp_flags);
		root_filename = res.input;
	} else if (res.input) {
		fin = fopen(res.input, "r");
		root_filename = res.input;
	} else {
		fin = stdin;
		root_filename = "<stdin>";
	}
	assert(root_filename);
	
	if (!fin)
		panic("Can't open file '%s' for reading.", res.input ? res.input : "<stdin>");
	
	init_meta(&outaoi.meta_data);
	meta_add_channel(&outaoi.meta_data,
			 meta_add_file(&outaoi.meta_data, "(generated)",
				       IO_FILE_INPUT),
			 "");
	meta_add_file(&outaoi.meta_data, "(builtin)", IO_FILE_BUILTIN);
	/*
	 * We strip off any leading path on `res.input'.  We don't want AOI
	 * files to differ based on the path to the input file.
	 */
	root_file = meta_add_file(&outaoi.meta_data,
				  file_part(root_filename),
				  IO_FILE_INPUT|IO_FILE_ROOT);
	current_i_file = root_file;
	read_defs();
	translate();
	/* print_all(outaoi); */
	
	out = (res.output ? fopen(res.output, "wb") : stdout);
	
	xdrstdio_create(&xdrs, out, XDR_ENCODE);
	if (!xdr_aoi(&xdrs, &outaoi)) {
		panic("error writing AOI file\n");
	}
	xdr_destroy(&xdrs);
	return 0;
}

