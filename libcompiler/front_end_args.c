/*
 * Copyright (c) 1997, 1999 The University of Utah and
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

/*
 * `FE_FLAGS' is the number of flags ``hardcoded'' in this file --- yuck!
 * These flags include:
 *
 * `-c' / `--cppflags'
 *		to pass the given command line options to the C preprocessor
 * `-n' / `--nocpp'
 *		to tell the front end not to run the input file through the
 *		C preprocessor
 */
#define FE_FLAGS (2)

/*
 * Parse the command line arguments to a Flick front end.
 */
fe_flags front_end_args(int argc, char **argv, const char *info)
{
	flags_in in[STD_FLAGS + FE_FLAGS];
	flags_out out;
	fe_flags res;
	
	int flags_index;
	
	/* Initialize the array of command line options. */
	
	set_def_flags(in);
	flags_index = STD_FLAGS;
	
	in[flags_index].sng = 'c';
	in[flags_index].dbl = "cppflags";
	in[flags_index].kind = fk_STRING;
	in[flags_index].max_occur = 1;
	in[flags_index].dfault.string = 0;
	in[flags_index].explain = "Pass the given flags to the C preprocessor";
	++flags_index;
	
	in[flags_index].sng = 'n';
	in[flags_index].dbl = "nocpp";
	in[flags_index].kind = fk_FLAG;
	in[flags_index].max_occur = FLAG_UNLIMITED_USE_LAST;
	in[flags_index].dfault.flag = 0;
	in[flags_index].explain = "Don't call the C preprocessor";
	++flags_index;
	
	assert(flags_index == (STD_FLAGS + FE_FLAGS));
	
	/* Parse the actual command line arguments. */
	
	out = parse_args(argc, argv, (STD_FLAGS + FE_FLAGS), in);
	std_handler(out,
		    (STD_FLAGS + FE_FLAGS), in,
		    "<optional input filename>", info);
	
	res.output = out.flag_seqs[OUTPUT_FILE_FLAG].values[0].string;
	if (out.other_count == 1)
		res.input = *out.other;
	else if (out.other_count) {
		print_args_usage(out.progname,
				 (STD_FLAGS + FE_FLAGS), in,
				 "<optional input filename>", info);
		exit(1);
	} else
		res.input = 0;
	if (res.input && !res.output)
		res.output = resuffix(res.input, ".aoi");
	progname = out.progname;
	
	/* These had better line up with the list above! */
	flags_index = STD_FLAGS;
	
	res.cpp_flags = out.flag_seqs[flags_index++].values[0].string;
	res.nocpp     = out.flag_seqs[flags_index++].values[0].flag;
	
	assert(flags_index == (STD_FLAGS + FE_FLAGS));
	
	return res;
}

/* End of file. */

