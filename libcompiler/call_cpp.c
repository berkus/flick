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
#include <unistd.h>
#include <string.h>
#include <mom/compiler.h>

/*
 * `CPP', `CPPFLAGS', `CXXCPP', and `CXXCPPFLAGS' should all be defined by the
 * configuration process and communicated to us to through the compilation
 * command line.
 *
 * If `gcc -E' is the C preprocessor, `CPPFLAGS' must contain `-x c' or else
 * `gcc' will assume that an input `.x' file is a linker file.  Similarly, if
 * GNU C++ is the C++ preprocessor, `CPPCXXFLAGS' must contain `-x c++'.
 */

#ifndef CPP
#  define CPP "/lib/cpp"
#endif
#ifndef CPPFLAGS
#  define CPPFLAGS "-C"
#endif

#ifndef CXXCPP
#  define CXXCPP CPP
#endif
#ifndef CXXCPPFLAGS
/* Don't suck up `CPPFLAGS', since it may have C-specific options. */
#  define CXXCPPFLAGS "-C"
#endif

static char cpp_cmd[] = CPP;
static char cpp_flags[] = CPPFLAGS;

static char cxxcpp_cmd[] = CXXCPP;
static char cxxcpp_flags[] = CXXCPPFLAGS;

#define PP_ARGC_MAX (128)
extern char *infilename;


/*****************************************************************************/

/*
 * Invoke the named preprocessor on `infile', with command arguments `pp_flags'
 * and `other_flags'.  If `infile' is null, arrange for the preprocessor to
 * read from stdin.
 * 
 * Return a `FILE *' so that we receive the preprocessor's output.  As a side
 * effect, set the global variable `infilename' to be a printable version of
 * `infile'.
 *
 * This is the internal, shared work function used by `call_c_preprocessor' and
 * `call_cxx_preprocessor', defined later in this file.
 */
static FILE *
call_preprocessor(
	const char *pp_cmd,
	const char *pp_flags,
	const char *infile,
	const char *other_flags
	)
{
	int pp_flags_p;		/* Is `pp_flags' non-empty? */
	int other_flags_p;	/* Is `other_flags' non-empty? */
	
	char *pp_cmdline;
	int pp_cmdline_len;
	
	char *pp_argv[PP_ARGC_MAX];
	int pp_argc;
	
	char *pp_infile;
	
	int pd[2];
	
	/*
	 * Parse the preprocessor command line into an argument vector.
	 */
	pp_flags_p = (pp_flags && pp_flags[0]);
	other_flags_p = (other_flags && other_flags[0]);
	
	pp_cmdline_len = strlen(pp_cmd) + 1 /* NUL */;
	if (pp_flags_p)
		pp_cmdline_len += (1 /* SPC */ + strlen(pp_flags));
	if (other_flags_p)
		pp_cmdline_len += (1 /* SPC */ + strlen(other_flags));
	
	pp_cmdline = (char *) mustmalloc(pp_cmdline_len);
	
	strcpy(pp_cmdline, pp_cmd);
	if (pp_flags_p) {
		strcat(pp_cmdline, " ");
		strcat(pp_cmdline, pp_flags);
	}
	if (other_flags_p) {
		strcat(pp_cmdline, " ");
		strcat(pp_cmdline, other_flags);
	}
	
	for (pp_argc = 0; pp_argc < PP_ARGC_MAX; ++pp_argc) {
		pp_argv[pp_argc] =
			strtok(((pp_argc == 0) ? pp_cmdline : 0), " \t");
		if (!pp_argv[pp_argc])
			/* When we get a null pointer, we're done. */
			break;
	}
	if (pp_argc == 0)
		panic("The command to run the preprocessor is empty.");
	if (pp_argc >= PP_ARGC_MAX)
		panic("Too many arguments to the preprocessor.");
	
	if (infile) {
		/*
		 * At this point, `pp_argc' is the index of the null string
		 * in `pp_argv' that was returned by `strtok'.
		 */
		pp_infile = (char *) mustmalloc(strlen(infile) + 1);
		strcpy(pp_infile, infile);
		
		pp_argv[pp_argc] = pp_infile;
		++pp_argc;
		if (pp_argc >= PP_ARGC_MAX)
			panic("Too many arguments to the preprocessor.");
		pp_argv[pp_argc] = 0;
	} else
		pp_infile = 0;
	
	/* Set up our input pipeline. */
	infilename = (((infile == NULL) || (!strcmp(infile, "-"))) ?
		      "<stdin>" : pp_infile);
	(void) pipe(pd);
	switch (fork()) {
	case 0:
		(void) close(1);
		(void) dup2(pd[1], 1);
		(void) close(pd[0]);
		execvp(pp_argv[0], pp_argv);
		perror("execvp");
		exit(1);
	case -1:
		perror("fork");
		exit(1);
	}
	(void) close(pd[1]);
	return fdopen(pd[0], "r");
}


/*****************************************************************************/

/*
 * Invoke the C preprocessor on `filename', with command argument `flags', and
 * return a `FILE *' so that we receive the preprocessor's output.
 */
FILE *
call_c_preprocessor(
	const char *filename,
	const char *flags
	)
{
#ifdef CPP_DASH_FOR_STDIN
	/*
	 * We need to specify `-' on the command line to get the preprocessor
	 * to read from stdin.
	 */
	if (!filename)
		filename = "-";
#endif /* CPP_DASH_FOR_STDIN */
	
	return call_preprocessor(cpp_cmd, cpp_flags, filename, flags);
}

/*
 * Invoke the C++ preprocessor on `filename', with command argument `flags',
 * and return a `FILE *' so that we receive the preprocessor's output.
 */
FILE *
call_cxx_preprocessor(
	const char *filename,
	const char *flags
	)
{
#ifdef CXXCPP_DASH_FOR_STDIN
	/*
	 * We need to specify `-' on the command line to get the preprocessor
	 * to read from stdin.
	 */
	if (!filename)
		filename = "-";
#endif
	
	return call_preprocessor(cxxcpp_cmd, cxxcpp_flags, filename, flags);
}

/* End of file. */

