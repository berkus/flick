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

#include <mom/compiler.h>
#include <mom/libmeta.h>
#include <mom/libaoi.h>

#include "helpers.hh"

extern FILE *yyin;
extern int yyparse();
extern int lineno, lnum;
extern int yydebug;
extern int fail;
extern int errorcount;
extern int warningcount;
extern aoi cur_aoi;
extern int yylex();
char *progname;
const char *infilename;
const char *root_filename;
io_file_index root_file = -1;
io_file_index current_i_file = -1;

/*****************************************************************************/

int
main(int argc, char *argv[])
{
	fe_flags res = front_end_args(argc,
				      argv,
				      ("Note that the input to this front end "
				       "must be 100% CORBA 2.0 compliant"));
	FILE *output;
	fail = 0;
	
	if (!res.nocpp)
		yyin = call_cxx_preprocessor(res.input, res.cpp_flags);
	else if (res.input)
		yyin = fopen(res.input, "r");
	else
		yyin = stdin;
	
	infilename = res.input ? res.input : "<stdin>";
	root_filename = infilename;
	
	if (!yyin)
		Exit("%s: can't open file `%s' for reading",
		     progname,
		     infilename);
	
	if (yyparse()) 
		ConfusedExit();
	
	ResolveForwardInterfaces();
	if (fail)
	        Exit("%s: %d error%s and %d warning%s",
		     infilename,
		     errorcount, ((errorcount == 1) ? "" : "s"),
		     warningcount, ((warningcount == 1) ? "" : "s"));
	if (res.output)
		output = fopen(res.output, "wb");
	else
		output = stdout;
	
	aoi_writefh(&cur_aoi, output);
	return 0;
}

/* End of file. */

