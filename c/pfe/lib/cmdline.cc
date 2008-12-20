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

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <mom/compiler.h>
#include <mom/libaoi.h>
#include <mom/c/libpres_c.h>

#include <mom/c/pfe.hh>
#include "private.hh"

FILE **pg_state::cmdline(int argc, char *argv[])
{
	FILE *input_file, *output_file;
	static FILE *res[2];
	pg_flags flgs;
	
	flgs = args(argc, argv);
	
	if (flgs.input && !flgs.output)
		flgs.output = resuffix(flgs.input, PRES_C_SUFFIX);
	input_file = flgs.input ? fopen(flgs.input, "rb") : stdin;
	output_file = flgs.output ? fopen(flgs.output, "wb") : stdout;
	
	if (!input_file)
		panic("can't open input file '%s'",
		      (flgs.input ? flgs.input : "<stdin>"));
	if (!output_file)
		panic("can't open output file `%s'",
		      (flgs.output ? flgs.output : "<stdout>"));
	
	gen_server = 1-flgs.client;
	gen_client = flgs.client;
	
	async_stubs = flgs.async_stubs;
	gen_sids = flgs.use_sids;
	
	client_stubs_for_inherited_operations
		= flgs.client_stubs_for_inherited_operations;
	server_funcs_for_inherited_operations
		= flgs.server_funcs_for_inherited_operations;
	
	/* End of argument processing. */
	
	res[0] = input_file;
	res[1] = output_file;
	return res;
}

/* End of file. */

