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
#include <stdlib.h>
#include <string.h>

#include <mom/compiler.h>
#include <mom/pres_c.h>
#include <mom/c/pbe.hh>
#include <mom/c/be/be_state.hh>

extern char *progname;

/*
 * `BE_FLAGS' is the number of flags ``hardcoded'' in this file --- yuck!
 * These flags include:
 *
 * `-h' / `--header'
 *		to specify the name of the output `.h' file
 * `-p' / `--prefix'
 *		to specify the path prefix for #includes that refer to the
 *		generated header file
 * `-s' / `--system_header'
 *		to specify that the generated header file should be #included
 *		as a system header (i.e., using <>'s)
 *
 * `-n' / `--inline'
 *		to specify the name of the output inline file
 *
 * `--no_timestamp'
 *		to suppress the timestamp information that is otherwise output
 *		into the generated `.h' and `.c' files; useful when doing
 *		regression testing on the generated output
 *
 * `-i' / `--no_included_implementations'
 *		to suppress client stubs and server funcs that were generated
 *		from IDL files that were #included by the root IDL file
 * `-d' / `--no_included_declarations'
 *              to suppress client stubs and server func declarations that were
 *              generated from IDL files that were #include by the root IDL
 *              file
 *
 * `-m' / `--all_mu_stubs'
 *		to generate ALL marshal and unmarshal stubs (rather than only
 *		those that are necessary).
 *
 * `-P' / `--presentation_implementation'
 *		to use the named SCML file as the presentation implementation
 * `--nostdinc'
 *		to remove the standard directories from the SCML search path
 * `-I' / `--include'
 *		to add a directory to the list of directories searched for SCML
 *		files
 *
 * `-f' / `--source_include_file'
 *		to `#include' a specific file in the generated source file
 * `-F' / `--header_include_file'
 *		to `#include' a specific file in the generated header file
 *
 * `-D' / `--define'
 *		Define an SCML variable
 */
#define BE_FLAGS (14)

/* Forward declaration of `be_args' helper function. */
static const char **
pres_impl_dir_list(
	flag_value_seq *dir_seq,
	const char **std_list,
	const char *pres_impl);

/*
 * Parse the command line arguments to a Flick back end.
 */
be_flags
be_state::be_args(int argc, char **argv, const be_flags &def_flags,
		  const char *info)
{
	flags_in in[STD_FLAGS + BE_FLAGS];
	flags_out out;
	be_flags res;
	
	int nostdinc;
	
	int flags_index;
	
	/* Initialize the array of command line options. */
	
	set_def_flags(in);
	flags_index = STD_FLAGS;
	
	in[flags_index].sng = 'h';
	in[flags_index].dbl = "header";
	in[flags_index].kind = fk_STRING;
	in[flags_index].max_occur = 1;
	in[flags_index].dfault.string = def_flags.header;
	in[flags_index].explain
		= "Set the name of the output header file to <string>";
	++flags_index;
	
	in[flags_index].sng = 'p';
	in[flags_index].dbl = "prefix";
	in[flags_index].kind = fk_STRING;
	in[flags_index].max_occur = 1;
	in[flags_index].dfault.string = def_flags.prefix;
	in[flags_index].explain
		= "Set the prefix for the path to the header to <string>";
	++flags_index;
	
	in[flags_index].sng = 's';
	in[flags_index].dbl = "system_header";
	in[flags_index].kind = fk_FLAG;
	in[flags_index].max_occur = FLAG_UNLIMITED_USE_LAST;
	in[flags_index].dfault.flag = def_flags.system_header;
	in[flags_index].explain
		= "When including the header, use <>, not \"\"";
	++flags_index;
	
	in[flags_index].sng = 'n';
	in[flags_index].dbl = "inline";
	in[flags_index].kind = fk_STRING;
	in[flags_index].max_occur = 1;
	in[flags_index].dfault.string = def_flags.inline_file;
	in[flags_index].explain
		= "Set the name of the output inline file to <string>";
	++flags_index;
	
	in[flags_index].sng = 0;
	in[flags_index].dbl = "no_timestamp";
	in[flags_index].kind = fk_FLAG;
	in[flags_index].max_occur = FLAG_UNLIMITED_USE_LAST;
	in[flags_index].dfault.flag = def_flags.no_timestamp;
	in[flags_index].explain
		= ("Don't output the timestamp comment block into generated "
		   "files");
	++flags_index;
	
	in[flags_index].sng = 'i';
	in[flags_index].dbl = "no_included_implementations";
	in[flags_index].kind = fk_FLAG;
	in[flags_index].max_occur = FLAG_UNLIMITED_USE_LAST;
	in[flags_index].dfault.flag = def_flags.no_included_implementations;
	in[flags_index].explain
		= ("Don't define stubs for interfaces that were `#include'd");
	++flags_index;
	
	in[flags_index].sng = 'd';
	in[flags_index].dbl = "no_included_declarations";
	in[flags_index].kind = fk_FLAG;
	in[flags_index].max_occur = FLAG_UNLIMITED_USE_LAST;
	in[flags_index].dfault.flag = def_flags.no_included_declarations;
	in[flags_index].explain
		= ("Don't declare stubs for interfaces that were `#include'd");
	++flags_index;
	
	in[flags_index].sng = 'm';
	in[flags_index].dbl = "all_mu_stubs";
	in[flags_index].kind = fk_FLAG;
	in[flags_index].max_occur = FLAG_UNLIMITED_USE_LAST;
	in[flags_index].dfault.flag = def_flags.all_mu_stubs;
	in[flags_index].explain
		= ("Generate ALL marshal and unmarshal stubs");
	++flags_index;
	
	in[flags_index].sng = 'P';
	in[flags_index].dbl = "presentation_implementation";
	in[flags_index].kind = fk_STRING;
	in[flags_index].max_occur = 1;
	in[flags_index].dfault.string = def_flags.pres_impl;
	in[flags_index].explain
		= ("Use the named SCML file as the presentation "
		   "implementation");
	++flags_index;
	
	in[flags_index].sng = 0;
	in[flags_index].dbl = "nostdinc";
	in[flags_index].kind = fk_FLAG;
	in[flags_index].max_occur = FLAG_UNLIMITED_USE_LAST;
	in[flags_index].dfault.flag = 0;
	in[flags_index].explain
		= ("Do not search the standard directories for SCML files");
	++flags_index;
	
	/*
	 * `-I' must come after `-P' and after `--nostdinc'; see how
	 * `res.pres_impl_dirs' is set at the end of this function.
	 */
	in[flags_index].sng = 'I';
	in[flags_index].dbl = "include";
	in[flags_index].kind = fk_STRING;
	in[flags_index].max_occur = FLAG_UNLIMITED_LIST;
	in[flags_index].dfault.string = 0;
	in[flags_index].explain
		= ("Search the named directory for SCML files");
	++flags_index;
	
	in[flags_index].sng = 'f';
	in[flags_index].dbl = "source_include_file";
	in[flags_index].kind = fk_STRING;
	in[flags_index].max_occur = FLAG_UNLIMITED_LIST;
	in[flags_index].dfault.string = 0;
	in[flags_index].explain
		= ("`#include \"...\"' a file within the generated source "
		   "code file");
	++flags_index;
	
	in[flags_index].sng = 'F';
	in[flags_index].dbl = "header_include_file";
	in[flags_index].kind = fk_STRING;
	in[flags_index].max_occur = FLAG_UNLIMITED_LIST;
	in[flags_index].dfault.string = 0;
	in[flags_index].explain
		= ("`#include \"...\"' a file within the generated header "
		   "file");
	++flags_index;
	
	in[flags_index].sng = 'D';
	in[flags_index].dbl = "define";
	in[flags_index].kind = fk_STRING;
	in[flags_index].max_occur = FLAG_UNLIMITED_LIST;
	in[flags_index].dfault.string = 0;
	in[flags_index].explain
		= ("Define an SCML variable");
	++flags_index;
	
	assert(flags_index == (STD_FLAGS + BE_FLAGS));
	
	/* Parse the actual command line arguments. */
	
	out = parse_args(argc, argv, (STD_FLAGS + BE_FLAGS), in);
	std_handler(out,
		    (STD_FLAGS + BE_FLAGS), in,
		    "<optional input filename>", info);
	
	res.output = out.flag_seqs[OUTPUT_FILE_FLAG].values[0].string;
	if (out.other_count == 1)
		res.input = *out.other;
	else if (out.other_count) {
		print_args_usage(out.progname,
				 (STD_FLAGS + BE_FLAGS), in,
				 "<optional input filename>", info);
		exit(1);
	} else
		res.input = 0;
	progname = out.progname;
	
	/* These had better line up with the list above! */
	flags_index = STD_FLAGS;
	
	res.header         = out.flag_seqs[flags_index++].values[0].string;
	res.prefix         = out.flag_seqs[flags_index++].values[0].string;
	res.system_header  = out.flag_seqs[flags_index++].values[0].flag;
	res.inline_file    = out.flag_seqs[flags_index++].values[0].string;
	res.no_timestamp   = out.flag_seqs[flags_index++].values[0].flag;
	res.no_included_implementations
		= out.flag_seqs[flags_index++].values[0].flag;
	res.no_included_declarations
		= out.flag_seqs[flags_index++].values[0].flag;
	res.all_mu_stubs   = out.flag_seqs[flags_index++].values[0].flag;
	res.pres_impl      = out.flag_seqs[flags_index++].values[0].string;
	nostdinc           = out.flag_seqs[flags_index++].values[0].flag;
	res.pres_impl_dirs
		= pres_impl_dir_list(&(out.flag_seqs[flags_index++]),
				     (nostdinc ?
				      0 :
				      def_flags.pres_impl_dirs),
				     res.pres_impl);
	res.src_includes   = out.flag_seqs[flags_index++]; /* struct copy */
	res.hdr_includes   = out.flag_seqs[flags_index++]; /* struct copy */
	res.scml_defs      = out.flag_seqs[flags_index++]; /* struct copy */
	
	assert(flags_index == (STD_FLAGS + BE_FLAGS));
	
	/*
	 * If no special source includes were specified on the command line,
	 * the default (null) string was stored into the value list of
	 * `src_includes'.  We don't want it, so take it out now.  Do the same
	 * for `hdr_includes' as well.
	 */
	if ((res.src_includes.len == 1) && !res.src_includes.values[0].string)
		res.src_includes.len = 0;
	if ((res.hdr_includes.len == 1) && !res.hdr_includes.values[0].string)
		res.hdr_includes.len = 0;
	if ((res.scml_defs.len == 1) && !res.scml_defs.values[0].string)
		res.scml_defs.len = 0;
	
	tag_item *ti;
	
	if( !(ti = find_tag(this->root_scope->get_values(), "system_info")) ) {
		tag_list *tl;
		
		tl = create_tag_list(0);
		ti = add_tag(this->root_scope->get_values(), "system_info",
			     TAG_TAG_LIST, tl);
	}
	add_tag(ti->data.tag_data_u.tl, "flags", TAG_TAG_LIST,
		flags_to_tag_list(in, &out, STD_FLAGS + BE_FLAGS));
	return res;
}

/*****************************************************************************/

/*
 * A helper function for `be_args': convert the `flag_value_seq' representation
 * of the list of presentation implementation search directories into a null-
 * terminated array of strings.  Additionally, append the standard search
 * directories to the search path, and if `pres_impl' contains a directory
 * component, add that directory to the search path as well.
 */
static const char**
pres_impl_dir_list(
	flag_value_seq *dir_seq,
	const char **std_list,
	const char *pres_impl)
{
	unsigned int	dir_seq_len;
	unsigned int	std_list_len;
	
	char *		pres_impl_slash;
	char *		pres_impl_dir;
	int		pres_impl_dir_strlen;
	
	const char **	dir_list;
	unsigned int	dir_list_len;
	unsigned int	dir_list_index;
	unsigned int	i;
	
	/*****/
	
	/* Determine the number of strings in `dir_seq'. */
	if ((dir_seq->len == 1) && !dir_seq->values[0].string)
		/* Special case: discard the default null string. */
		dir_seq_len = 0;
	else
		dir_seq_len = dir_seq->len;
	
	/* Determine the number of strings in `std_list'. */
	if (!std_list)
		/* Special case: ignore a null `std_list'. */
		std_list_len = 0;
	else {
		for (std_list_len = 0; std_list[std_list_len]; ++std_list_len)
			/* No body required. */
			;
	}
	
	/* Does `pres_impl' contain a directory component?  If so, get it. */
	pres_impl_slash = (pres_impl ? strrchr(pres_impl, '/') : 0);
	if (pres_impl_slash) {
		if (pres_impl_slash == pres_impl)
			/* `pres_impl' is in `/'. */
			++pres_impl_slash;
		pres_impl_dir_strlen = pres_impl_slash - pres_impl;
		pres_impl_dir = (char *)
				mustmalloc((pres_impl_dir_strlen + 1)
					   * sizeof(char));
		strncpy(pres_impl_dir, pres_impl, pres_impl_dir_strlen);
		pres_impl_dir[pres_impl_dir_strlen] = 0;
		
	} else
		pres_impl_dir = 0;
	
	/* Create and fill the `dir_list'. */
	dir_list_len   = dir_seq_len
			 + (pres_impl_dir ? 1 : 0)
			 + std_list_len;
	dir_list       = (const char **)
			 mustmalloc((dir_list_len + 1)
				    * sizeof(const char *));
	dir_list_index = 0;
	
	for (i = 0; i < dir_seq_len; ++i)
		dir_list[dir_list_index++] = dir_seq->values[i].string;
	if (pres_impl_dir)
		dir_list[dir_list_index++] = pres_impl_dir;
	for (i = 0; i < std_list_len; ++i)
		dir_list[dir_list_index++] = std_list[i];
	dir_list[dir_list_len] = 0;
	
	return dir_list;
}

/* End of file. */

