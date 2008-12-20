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

#include <string.h>
#include <stdlib.h>

#include <mom/compiler.h>


/*****************************************************************************/

/*
 * `FLAG_VALUE_SEQ_GROW_SIZE' is the number many new slots that we allocate
 * when we have to grow the set of values associated with a flag.
 */
#define FLAG_VALUE_SEQ_GROW_SIZE (4)

void grow_flag_value_seq(flag_value_seq *fvs, int needed)
{
	if( needed > fvs->max ) {
		fvs->max += needed + FLAG_VALUE_SEQ_GROW_SIZE;
		fvs->values = (flag_value *)
			mustrealloc(fvs->values,
				    fvs->max * sizeof(flag_value));
	}
}

/*
 * Add a value to the sequence of values associated with a flag.
 *
 * This function does *not* signal an error if the flag becomes associated with
 * too many values (i.e., more than `this_flag->max_occur'); that semantic
 * check is performed by the main `parse_args' function.
 */
static void
flag_add_value(flags_in *this_flag,
	       flag_value_seq *this_flag_seq, 
	       flag_value *value)
{
	unsigned int need_len;
	
	/*
	 * Figure out how many slots we now need in `this_flag_seq->values'.
	 */
	switch (this_flag->max_occur) {
	default:
	case FLAG_UNLIMITED_LIST:
		/*
		 * In these cases, we extend the list of values for this flag.
		 */
		need_len = this_flag_seq->len + 1;
		break;
	case FLAG_UNLIMITED_USE_LAST:
	case FLAG_UNLIMITED_OR:
		/*
		 * In these cases, we logically combine the new value with any
		 * existing values.
		 */
		need_len = 1;
		break;
	}
	
	/*
	 * Make sure we have space in `this_flag_seq' for the new value.
	 */
	grow_flag_value_seq(this_flag_seq, need_len);
	
	/*
	 * Add `value' to `this_flag_seq->values'.
	 */
	switch (this_flag->max_occur) {
	default:
	case FLAG_UNLIMITED_LIST:
		/* General case: just collect the value. */
		this_flag_seq->values[(this_flag_seq->len)++] = *value;
		break;
		
	case FLAG_UNLIMITED_USE_LAST:
		/* Set or replace `this_flag_seq->values[0]'. */
		this_flag_seq->values[0] = *value;
		this_flag_seq->len = 1;
		break;
		
	case FLAG_UNLIMITED_OR:
		/* Logically `or' the new value with any existing value. */
		if (this_flag->kind != fk_FLAG)
			panic(("In `flag_add_value', cannot use "
			       "`FLAG_UNLIMITED_OR' with a non-boolean "
			       "command line option."));
		
		if (this_flag_seq->len >= 1)
			this_flag_seq->values[0].flag
				= (this_flag_seq->values[0].flag
				   || value->flag);
		else
			this_flag_seq->values[0].flag = value->flag;
		this_flag_seq->len = 1;
		break;
	}
}

/*
 * Parse a single argument (the switch and its data, if any) from the command
 * line.  Return 1 if the `next' command line word was consumed; 0 if only the
 * current command line word was consumed; or -1 if there was a parsing error.
 */
static int
parse_arg(/* const */ char *	rest, /* rest of the current argument */
	  /* const */ char *	next, /* the next argument (may be 0) */
	  flags_in *		this_flag,
	  flag_value_seq *	this_flag_seq,
	  flags_out *		res)
{
	char *		arg_data;
	int		arg_data_consumed;
	flag_value	value;
	int		parse_error;
	int		i;
	
	/*****/
	
	if (rest[0]) {
		arg_data = rest;
		arg_data_consumed = 0;
	} else {
		arg_data = next;
		arg_data_consumed = (this_flag->kind != fk_FLAG);
	}
	parse_error = 0;
	
	switch (this_flag->kind) {
	case fk_FLAG:
		if (arg_data) {
			if (!strcmp(arg_data, "1")
			    || !strcmp(arg_data, "y")
			    || !strcmp(arg_data, "yes")) {
				value.flag = 1;
				flag_add_value(this_flag,
					       this_flag_seq,
					       &value);
				arg_data_consumed = (arg_data == next);
				
			} else if (!strcmp(arg_data, "0")
				   || !strcmp(arg_data, "n")
				   || !strcmp(arg_data, "no")) {
				value.flag = 0;
				flag_add_value(this_flag,
					       this_flag_seq,
					       &value);
				arg_data_consumed = (arg_data == next);
				
			} else if (rest[0] == 0) {
				/* Parse `-x' as setting the flag. */
				value.flag = 1;
				flag_add_value(this_flag,
					       this_flag_seq,
					       &value);
				arg_data_consumed = 0;
				
			} else {
				parse_error = 1;
			}
			
		} else {
			/* Parse `-x' as setting the flag. */
			value.flag = 1;
			flag_add_value(this_flag,
				       this_flag_seq,
				       &value);
			arg_data_consumed = 0;
		}
		break;
		
	case fk_STRING:
		if (arg_data) {
			value.string = arg_data;
			flag_add_value(this_flag, this_flag_seq, &value);
		} else {
			parse_error = 1;
		}
		break;
		
	case fk_NUMBER:
		if (arg_data) {
			value.number = 0;
			for (i = 0; arg_data[i]; ++i) {
				if ((arg_data[i] < '0')
				    || (arg_data[i] > '9'))
					break;
				value.number = value.number * 10
					       + (arg_data[i] - '0');
			}
			if (!arg_data[i]) {
				flag_add_value(this_flag,
					       this_flag_seq,
					       &value);
			} else {
				parse_error = 1;
			}
			
		} else {
			parse_error = 1;
		}
		break;
	}
	
	if (parse_error) {
		res->error = 1;
		return -1;
	}
	
	return arg_data_consumed;
}

/*
 * Parse the `argv' command line according to the flags described by `flags',
 * and return a `flags' out structure describing the parse.
 */
flags_out
parse_args(int argc, char *argv[], int flag_count, flags_in *flags)
{
	flags_out res;
	int i;
	
	/*****/
	
	/*
	 * Initialize `res', which will be our return value.
	 */
	
	res.progname = argv[0];
	
	res.error = 0;
	res.other_count = 0;
	res.other = 0;
	
	res.flag_seqs = (flag_value_seq *)
			mustmalloc(sizeof(flag_value_seq) * flag_count);
	for (i = 0; i < flag_count; ++i) {
		int seq_max;
		
		res.flag_seqs[i].len = 0;
		
		seq_max = flags[i].max_occur;
		/*
		 * Special case: if this flag may appear any number of times,
		 * only preallocate one `flag_value' in our value vector.
		 */
		if (FLAG_UNLIMITED_P(seq_max))
			seq_max = 1;
		
		res.flag_seqs[i].max = seq_max;
		res.flag_seqs[i].values = (flag_value *)
					  mustmalloc(sizeof(flag_value)
						     * seq_max);
	}
	
	/*
	 * Parse the command line.
	 */
	
	for (i = 1; i < argc; ++i) {
		if (argv[i][0] != '-') {
			/*
			 * It's not a valid option --- add it to the list of
			 * other arguments.
			 */
			res.other = (char **)
				    mustrealloc(res.other,
						(++res.other_count
						 * sizeof(char *)));
			res.other[res.other_count - 1] = argv[i];
			
		} else if (argv[i][1] != '-') {
			/*
			 * It's a short form (`-') option.  Short form options
			 * may be concatenated with their values (e.g., `-s1')
			 * or they may precede their values (e.g., `-s 1').
			 */
			int j, OK = 0;
			
			for (j = 0; j < flag_count && !OK; j++) {
				if (flags[j].sng
				    && (argv[i][1] == flags[j].sng)) {
					/*
					 * We found the flag, parse its args.
					 */
					int add =
						parse_arg(
							&(argv[i][2]),
							argv[i+1],
							&(flags[j]),
							&(res.flag_seqs[j]),
							&res);
					
					if (add >= 0) {
						i += add;
						OK = 1;
					}
					break;
				}
			}
			if (!OK) {
				res.other = (char **)
					    mustrealloc(res.other,
							(++res.other_count
							 * sizeof(char *)));
				res.other[res.other_count - 1] = argv[i];
				res.error = 1;
			}
			
		} else {
			/*
			 * It's a long form (`--') option.  A long form option
			 * may be concatenated with its value if the value is
			 * preceded by `=' (e.g., `--this=that').  Long form
			 * options may also may precede their values (e.g.,
			 * `--this that').
			 *
			 * Concatenation requires `=' so that we can identify
			 * the end of the option name.  Otherwise, we couldn't
			 * allow string-valued long-form options to share a
			 * common prefix.  For example, we couldn't have both
			 * `--client' and `--client_type'.
			 */
			int j, OK = 0;
			
			for (j = 0; j < flag_count && !OK; j++) {
				int len;
				
				if (!flags[j].dbl)
					continue;
				
				len = strlen(flags[j].dbl);
				
				if (!strncmp(flags[j].dbl, &(argv[i][2]),
					     len)
				    && ((argv[i][2+len] == 0)
					|| (argv[i][2+len] == '='))) {
					/*
					 * We found the flag, parse its args.
					 */
					int add =
						parse_arg(
							(argv[i][2+len] ?
							 /* Strip `='. */
							 &(argv[i][2+len+1]) :
							 /* Otherwise... */
							 &(argv[i][2+len])),
							argv[i+1],
							&(flags[j]),
							&(res.flag_seqs[j]),
							&res);
					
					if (add >= 0) {
						i += add;
						OK = 1;
					}
					break;
				}
			}
			if (!OK) {
				res.other = (char **)
					    mustrealloc(res.other,
							(++res.other_count
							 * sizeof(char *)));
				res.other[res.other_count - 1] = argv[i];
				res.error = 1;
			}
		}
	}
	
	/*
	 * Set the default values of flags that were not specified on the
	 * command line.
	 */
	for (i = 0; i < flag_count; i++) {
		if (res.flag_seqs[i].len == 0) {
			flag_add_value(&(flags[i]),
				       &(res.flag_seqs[i]),
				       &(flags[i].dfault));
		}
	}
	
	/*
	 * Signal an error if any flag was specified more than its specified
	 * maximum number of times.
	 */
	for (i = 0; i < flag_count; i++) {
		if (!FLAG_UNLIMITED_P(flags[i].max_occur)
		    && (res.flag_seqs[i].len
			> (unsigned int) flags[i].max_occur)) {
			fprintf(stderr,
				"%s: too many occurrences of ",
				res.progname);
			if (flags[i].sng)
				fprintf(stderr,
					"`-%c'%s",
					flags[i].sng,
					(flags[i].dbl ? " or " : ""));
			if (flags[i].dbl)
				fprintf(stderr, "`--%s' ", flags[i].dbl);
			fprintf(stderr, "(max. %d).\n", flags[i].max_occur);
			
			res.error = 1;
		}
	}
	
	return res;
}

/*****************************************************************************/

/*
 * Print a program usage description to `stderr'.
 */
void
print_args_usage(const char *progname,
		 int flag_count, flags_in *flags,
		 const char *cl_extra, const char *extra)
{
	int i;
	
	fprintf(stderr, "Usage: %s <options> %s\n\n", progname, cl_extra);
	
	for (i = 0; i < flag_count; i++) {
		/* Print the short and long forms of the flag. */
		if (flags[i].sng)
			fprintf(stderr, "-%c", flags[i].sng);
		if (flags[i].dbl)
			fprintf(stderr, "%s--%s",
				(flags[i].sng ? " / " : ""),
				flags[i].dbl);
		
		/* Print the type and explanation. */
		fprintf(stderr, "%s:\n\t%s.\n",
			((flags[i].kind == fk_FLAG) ? "" :
			 (flags[i].kind == fk_NUMBER) ? " <number>" :
			 (flags[i].kind == fk_STRING) ? " <string>" :
			 " <unknown type>"),
			(flags[i].explain ?
			 flags[i].explain :
			 "(No documentation available)"));
		if (flags[i].max_occur != 1) {
			if (FLAG_UNLIMITED_P(flags[i].max_occur)) {
				fprintf(stderr,
					"\tMay be repeated; ");
				switch (flags[i].max_occur) {
				case FLAG_UNLIMITED_LIST:
					fprintf(stderr,
						"values are collected");
					break;
				case FLAG_UNLIMITED_USE_LAST:
					fprintf(stderr,
						"last value is used");
					break;
				case FLAG_UNLIMITED_OR:
					fprintf(stderr,
						("flag is true if set by any "
						 "occurrence"));
					break;
				default:
					fprintf(stderr,
						"(bad collection strategy)");
					break;
				}
				fprintf(stderr, ".\n");
			} else
				fprintf(stderr,
					"\tMay be repeated up to %d times.\n",
					flags[i].max_occur);
		}
	}
	if (extra)
		fprintf(stderr, "\n%s\n", extra);
}


/*****************************************************************************/

/*
 * Fill `in' with the descriptions of the flags that are common to all Flick
 * programs.
 */
void
set_def_flags(flags_in *in)
{
	in[0].sng = 'v';
	in[0].dbl = "version";
	in[0].kind = fk_FLAG;
	in[0].max_occur = FLAG_UNLIMITED_OR;
	in[0].dfault.flag = 0;
	in[0].explain = "Print out the version information for this program";
	
	in[1].sng = 'u';
	in[1].dbl = "usage";
	in[1].kind = fk_FLAG;
	in[1].max_occur = FLAG_UNLIMITED_OR;
	in[1].dfault.flag = 0;
	in[1].explain = "Print out this message";
	
	in[OUTPUT_FILE_FLAG].sng = 'o';
	in[OUTPUT_FILE_FLAG].dbl = "output";
	in[OUTPUT_FILE_FLAG].kind = fk_STRING;
	in[OUTPUT_FILE_FLAG].max_occur = 1;
	in[OUTPUT_FILE_FLAG].dfault.string = 0;
	in[OUTPUT_FILE_FLAG].explain = "Optional output file name";
	
	in[3].sng = '?';
	in[3].dbl = "help";
	in[3].kind = fk_FLAG;
	in[3].max_occur = FLAG_UNLIMITED_OR;
	in[3].dfault.flag = 0;
	in[3].explain = "Print out this message";
}

/*
 * Handle the standard Flick common line options.
 */
void
std_handler(flags_out out, int flag_count, flags_in *in,
	    const char *cl_info, const char *info)
{
	int print_version;
	int print_usage;
	int quit = 0;
	
	/* `-v' occurrences are `or'ed by the parser. */
	print_version = out.flag_seqs[0].values[0].flag;
	/* `-u' and `-?' occurrences are also `or'ed by the parser. */
	print_usage = (out.flag_seqs[1].values[0].flag
		       || out.flag_seqs[3].values[0].flag);
	
	/* If requested, print the version. */
	if (print_version) {
		fprintf(stderr, "%s version %s\n",
			out.progname, FLICK_VERSION);
		quit = 1;
	}
	
	/* If requested, or if a parsing error occurred, print usage. */
	if (out.error || print_usage) {
		print_args_usage(out.progname, flag_count, in, cl_info, info);
		quit = 1;
	}
	
	if (quit)
		exit(out.error);
}


/*****************************************************************************/

/*
 * Print a flag description to `stderr'.  This is an auxiliary function for
 * `print_args_flags', below.
 */
static void
print_args_flag_desc(flags_in *flag)
{
	/* Print the short and long forms of the flag. */
	if (flag->sng)
		fprintf(stderr, "-%c", flag->sng);
	if (flag->dbl)
		fprintf(stderr, "%s--%s",
			(flag->sng ? " / " : ""),
			flag->dbl);
	
	/* Print the flag type. */
	fprintf(stderr, " <");
	switch (flag->kind) {
	case fk_FLAG:
		fprintf(stderr, "flag");
		break;
	case fk_STRING:
		fprintf(stderr, "string");
		break;
	case fk_NUMBER:
		fprintf(stderr, "number");
		break;
	default:
		fprintf(stderr, "unknown type");
		break;
	}

	/* Print the maximum number of occurrences. */
	fprintf(stderr, ", ");
	if (FLAG_UNLIMITED_P(flag->max_occur)) {
		fprintf(stderr, "unlimited occurrences, ");
		switch (flag->max_occur) {
		case FLAG_UNLIMITED_LIST:
			fprintf(stderr, "values are collected");
			break;
		case FLAG_UNLIMITED_USE_LAST:
			fprintf(stderr, "last value is used");
			break;
		case FLAG_UNLIMITED_OR:
			fprintf(stderr, "true if set by any occurrence");
			break;
		default:
			fprintf(stderr, "(bad collection strategy)");
			break;
		}
		
	} else {
		fprintf(stderr,
			"at most %d occurence%s",
			flag->max_occur,
			((flag->max_occur == 1) ? "" : "s"));
	}
	fprintf(stderr, ">:\n");
	
	/* Print the explanation. */
	fprintf(stderr,
		"\t%s.\n",
		(flag->explain ?
		 flag->explain :
		 "(No documentation available)"));
	
	/* Print the default value. */
	fprintf(stderr, "\tdefault: ");
	switch (flag->kind) {
	case fk_FLAG:
		fprintf(stderr, (flag->dfault.flag ? "true" : "false"));
		break;
	case fk_STRING:
		if (flag->dfault.string)
			fprintf(stderr, "\"%s\"", flag->dfault.string);
		else
			fprintf(stderr, "(null)");
		break;
	case fk_NUMBER:
		fprintf(stderr, "%d", flag->dfault.number);
		break;
	default:
		fprintf(stderr, "(unknown)");
		break;
	}
	fprintf(stderr, "\n");
}

/*
 * Print a flag's values to `stderr'.  This is an auxiliary function for
 * `print_args_flags', below.
 */
static void
print_args_flag_values(flags_in *flag, flag_value_seq *flag_values)
{
	unsigned int i;
	
	if (flag_values->len <= 0) {
		fprintf(stderr, "\tno values!\n");
		return;
	}
	
	for (i = 0; i < flag_values->len; ++i) {
		if ((flag->max_occur == 1) && (flag_values->len == 1))
			/* Common case: one value allowed, one specified. */
			fprintf(stderr, "\tvalue: ");
		else
			/* General case. */
			fprintf(stderr, "\tvalue %u: ", i);
		
		switch (flag->kind) {
		case fk_FLAG:
			fprintf(stderr,
				(flag_values->values[i].flag ?
				 "true" : "false"));
			break;
			
		case fk_STRING:
			if (flag_values->values[i].string)
				fprintf(stderr,
					"\"%s\"",
					flag_values->values[i].string);
			else
				fprintf(stderr, "(null)");
			break;
		case fk_NUMBER:
			fprintf(stderr, "%d", flag_values->values[i].number);
			break;
		default:
			fprintf(stderr, "(unknown)");
			break;
		}
		fprintf(stderr, "\n");
	}
}

/*
 * A debugging routine to print the parsed command line.
 */
void
print_args_flags(flags_out res, int flag_count, flags_in *flags)
{
	int i;
	
	char **name, **value, **dfault;
	
	name = (char **) mustcalloc(sizeof(char *) * flag_count);
	value = (char **) mustcalloc(sizeof(char *) * flag_count);
	dfault = (char **) mustcalloc(sizeof(char *) * flag_count);
	
	fprintf(stderr, "Flags for %s:\n", res.progname);
	
	for (i = 0; i < flag_count; i++) {
		print_args_flag_desc(&(flags[i]));
		print_args_flag_values(&(flags[i]), &(res.flag_seqs[i]));
	}
	
	if (res.other_count)
		fprintf(stderr, "\nAdditional command line arguments:\n");
	for (i = 0; i < res.other_count; ++i)
		fprintf(stderr, "%s\n", res.other[i]);
}


/*****************************************************************************/

/* End of file. */

