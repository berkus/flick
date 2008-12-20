/*
 * Copyright (c) 1997, 1998, 1999 The University of Utah and
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mom/compiler.h>
#include <mom/c/pfe.hh>
#include "private.hh"

/*
 * `PG_FLAGS' is the number of flags ``hardcoded'' in this file --- yuck!
 * These flags include:
 *
 * `-c' / `--client'
 *		to generate the client-side presentation
 * `-s' / `--server'
 *		to generate the server-side presentation
 * `-a' / `--async_stubs'
 *		to generate async. send/receive stubs and separate m/u stubs
 * `--with_sids'
 *		to add client and server SIDs to the presentation
 * `--client_stubs_for_inherited_operations'
 *		to control the creation of client stubs for inherited ops
 * `--server_funcs_for_inherited_operations'
 *		to control the creation of server funcs for inherited ops
 */
#define PG_FLAGS (6)

/*
 * `name_fmt_options' and `name_lit_options' are defined in `p_calc_name.cc'.
 */
extern name_fmt_option_struct name_fmt_options[];
extern name_lit_option_struct name_lit_options[];

pg_flags pg_state::args(int argc, char **argv, char *info)
{
	flags_in *in;
	int count = build_flags(&in);
	flags_out out;
	pg_flags res;
	
	out = parse_args(argc, argv, count, in);
	// print_args_flags(out, count, in);
	
	std_handler(out, count, in, "<optional input filename>", info);
	res = handler(out, in, info, count);
	
	return res;
}

pg_flags pg_state::handler(flags_out out, flags_in *in, char *info, int count)
{
	pg_flags res;
	
	int out_index;
	int name_options_index;
	
	/* Get our program name. */
	progname = out.progname;
	
	/* Decide whether we are generating client or server. */
	if (out.flag_seqs[STD_FLAGS].values[0].flag
	    && out.flag_seqs[STD_FLAGS + 1].values[0].flag)
		panic("Can't build both the client stubs and the server "
		      "skeleton.");
	res.client = !out.flag_seqs[STD_FLAGS + 1].values[0].flag;
	
	/* Get the name of our output file. */
	res.output = out.flag_seqs[OUTPUT_FILE_FLAG].values[0].string;
	
	/* Get the name of our input file. */
	if (out.other_count == 1)
		res.input = *out.other;
	else if (out.other_count) {
		for (int i = 0; i < out.other_count; i++)
			fprintf(stderr,
				"Arg #%d - '%s'\n",
				i + 1, out.other[i]);
		print_args_usage(out.progname,
				 count,
				 in,
				 "<optional input filename>",
				 info);
		exit(1);
	} else
		res.input = 0;
	
	/* Decide if we should implement the decomposed stub presentation. */
	res.async_stubs = out.flag_seqs[STD_FLAGS + 2].values[0].flag;
	
	/* Decide if we should include SIDs in the presentation. */
	res.use_sids = out.flag_seqs[STD_FLAGS + 3].values[0].flag;
	
	/*
	 * Decide if we should create client stubs and/or server functions for
	 * inherited operations.
	 */
	res.client_stubs_for_inherited_operations
		= out.flag_seqs[STD_FLAGS + 4].values[0].flag;
	res.server_funcs_for_inherited_operations
		= out.flag_seqs[STD_FLAGS + 5].values[0].flag;
	
	/* Process any name-mangling options that were specified. */
	for (out_index = STD_FLAGS + PG_FLAGS, name_options_index = 0;
	     name_fmt_options[name_options_index].name != 0;
	     ++out_index, ++name_options_index) {
		if (out.flag_seqs[out_index].values[0].string)
			/*
			 * The user specified a format string.  Set the right
			 * `names.formats' to be that string.
			 */
			names.formats[name_fmt_options[name_options_index].
				     index]
				= out.flag_seqs[out_index].values[0].string;
	}
	/* `out_index' is now pointing to the first literal option. */
	for (name_options_index = 0;
	     name_lit_options[name_options_index].name != 0;
	     ++out_index, ++name_options_index) {
		if (out.flag_seqs[out_index].values[0].string) {
			/*
			 * The user specified a literal string.  Set the right
			 * `names.literals' to be that string.
			 */
			names.literals[name_lit_options[name_options_index].
				      index].str
				= out.flag_seqs[out_index].values[0].string;
			names.literals[name_lit_options[name_options_index].
				      index].len
				= strlen(out.flag_seqs[out_index].values[0].
					 string);
		}
	}
	
	return res;
}

int pg_state::build_flags(flags_in **in) 
{
	int flags_count;
	int flags_index;
	
	int name_flags_count;
	int i;
	
	/* Count the number of options that we will have. */
	name_flags_count = 0;
	for (i = 0; name_fmt_options[i].name; ++i)
		++name_flags_count;
	for (i = 0; name_lit_options[i].name; ++i)
		++name_flags_count;
	flags_count = STD_FLAGS
		      + PG_FLAGS         /* `-c', `-s', ... */
		      + name_flags_count /* the name-mangling options */
		      ;
	
	*in = (flags_in *) mustmalloc(sizeof(flags_in) * flags_count);
	set_def_flags(*in);
	
	flags_index = STD_FLAGS;
	
	/* Flags to generate client or server presentation. */
	(*in)[flags_index].sng = 'c';
	(*in)[flags_index].dbl = "client";
	(*in)[flags_index].kind = fk_FLAG;
	(*in)[flags_index].max_occur = FLAG_UNLIMITED_USE_LAST;
	(*in)[flags_index].dfault.flag = 0;
	(*in)[flags_index].explain =
		"Build the client-side presentation (default)";
	++flags_index;
	
	(*in)[flags_index].sng = 's';
	(*in)[flags_index].dbl = "server";
	(*in)[flags_index].kind = fk_FLAG;
	(*in)[flags_index].max_occur = FLAG_UNLIMITED_USE_LAST;
	(*in)[flags_index].dfault.flag = 0;
	(*in)[flags_index].explain =
		"Build the server-side presentation";
	++flags_index;
	
	/* Flags to change the presentation style. */
	(*in)[flags_index].sng = 'a';
	(*in)[flags_index].dbl = "async_stubs";
	(*in)[flags_index].kind = fk_FLAG;
	(*in)[flags_index].max_occur = FLAG_UNLIMITED_USE_LAST;
	(*in)[flags_index].dfault.flag = async_stubs;
	(*in)[flags_index].explain =
		("Produce asynchronous send/recv stubs, separate "
		 "marshal/unmarshal stubs");
	++flags_index;
	
	/* Flags to add SIDs to the presentation. */
	(*in)[flags_index].sng = 0;
	(*in)[flags_index].dbl = "with_sids";
	(*in)[flags_index].kind = fk_FLAG;
	(*in)[flags_index].max_occur = FLAG_UNLIMITED_USE_LAST;
	(*in)[flags_index].dfault.flag = 0;
	(*in)[flags_index].explain =
		("Include client and server SIDs (security IDs) in "
		 "parameter lists");
	++flags_index;
	
	/*
	 * Flags to control the creation of certain client stubs and server
	 * work functions.
	 */
	(*in)[flags_index].sng = 0;
	(*in)[flags_index].dbl = "client_stubs_for_inherited_operations";
	(*in)[flags_index].kind = fk_FLAG;
	(*in)[flags_index].max_occur = FLAG_UNLIMITED_USE_LAST;
	(*in)[flags_index].dfault.flag = client_stubs_for_inherited_operations;
	(*in)[flags_index].explain =
		"Produce client stubs for inherited operations";
	++flags_index;
	
	(*in)[flags_index].sng = 0;
	(*in)[flags_index].dbl = "server_funcs_for_inherited_operations";
	(*in)[flags_index].kind = fk_FLAG;
	(*in)[flags_index].max_occur = FLAG_UNLIMITED_USE_LAST;
	(*in)[flags_index].dfault.flag = server_funcs_for_inherited_operations;
	(*in)[flags_index].explain =
		"Produce server functions for inherited operations";
	++flags_index;
	
	assert(flags_index == (STD_FLAGS + PG_FLAGS));
	
	/*
	 * Flags to change the name-generating format strings.  It is important
	 * that we create the flags in the same order as they appear in
	 * `name_fmt_options' --- this order is assumed by `pg_state::handler'
	 * above.
	 */
	for (i = 0; name_fmt_options[i].name; ++i, ++flags_index){
		(*in)[flags_index].sng = 0;
		(*in)[flags_index].dbl = name_fmt_options[i].name;
		(*in)[flags_index].kind = fk_STRING;
		(*in)[flags_index].max_occur = 1;
		/*
		 * Perhaps `name.formats[name_fmt_options[i].index]' should be
		 * the default value?  Currently, we use null to indicate that
		 * no value was specified by the user.
		 */
		(*in)[flags_index].dfault.string = 0;
		(*in)[flags_index].explain = name_fmt_options[i].explain;
	}
	
	/*
	 * Flags to change the name-generating literal strings.  Again, it is
	 * important that we create the flags in the same order as they appear
	 * in `name_lit_options' --- this order is assumed by the function
	 * `pg_state::handler' above.
	 *
	 * `pg_state::handler' also assumes that the literal string options
	 * immediately follow the format string options.
	 */
	for (i = 0; name_lit_options[i].name; ++i, ++flags_index){
		(*in)[flags_index].sng = 0;
		(*in)[flags_index].dbl = name_lit_options[i].name;
		(*in)[flags_index].kind = fk_STRING;
		(*in)[flags_index].max_occur = 1;
		/*
		 * Perhaps `name.literals[name_lit_options[i].index]' should be
		 * the default value?  Currently, we use null to indicate that
		 * no value was specified by the user.
		 */
		(*in)[flags_index].dfault.string = 0;
		(*in)[flags_index].explain = name_lit_options[i].explain;
	}
	
	return flags_count;
}

/* End of file. */

