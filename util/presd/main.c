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

#include <rpc/types.h>
#include <rpc/xdr.h>

#include <mom/pres_c.h>
#include <mom/compiler.h>
#include <mom/c/libpres_c.h>

char *progname;
char *infilename;
FILE *fin = 0;
FILE *fout = 0;

pres_c_1 pres;

extern void print_pres_c_1();

void show_usage() 
{
	panic("Usage: %s [-o outfile] [infile]", progname);
}

int main(int argc, char **argv)
{
	int arg_iter;
	
	progname = argv[0];
	
	for (arg_iter = 1; arg_iter < argc; arg_iter++) {
		if (strcmp("-o", argv[arg_iter]) == 0) {
			if (!fout) {
				fout = fopen(argv[++arg_iter], "w");
				if (!fout)
					panic("Output file could not be "
					      "opened.");
			} else
				show_usage();
			
		} else if (!fin) {
			fin = fopen(argv[arg_iter], "rb");
			fout = fopen(resuffix(argv[arg_iter], PRESD_SUFFIX),
				     "w");
			if (!fin)
				panic("Input file could not be opened.");
			if (!fout)
				panic("Output file could not be opened.");
			
		} else
			show_usage();
	}
	
	if (!fin)
		fin = stdin;
	if (!fout)
		fout = stdout;
	
	w_set_fh(fout);
	pres_c_1_readfh(&pres, fin);
	print_pres_c_1();
	
	return 0;
}

/* End of file. */

