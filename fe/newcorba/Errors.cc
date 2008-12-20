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

#include "Errors.hh"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mom/compiler.h>

int lineno = 0;
char linebuf[BUFSIZE] = ""; /* buffer for entire lines of IDL code */
char errorlinebuf[BUFSIZE] = "";
int tokenPos = 0, nextPos = 0, errorPos;
int errorcount = 0;
int warningcount = 0;
int fail;
int errorflag;
extern char *infilename;

/*****************************************************************************/

static void
ShowErrorPos()
{
	if (errorlinebuf[strlen(errorlinebuf) - 1] == '\n') 
		fprintf(stderr, "%s%*s\n", errorlinebuf, 1+errorPos, "^");
	else
		fprintf(stderr, "%s\n%*s\n", errorlinebuf, 1+errorPos, "^");
}

/*
 * Report an error from the lexical scanner.
 */
void 
LexError(const char *format, ...)
{
	va_list vl;
	
	strcpy(errorlinebuf, linebuf);
	errorPos = tokenPos;
	
	fprintf(stderr,
		"%s:%d:%d: ",
		infilename, lineno, 1+errorPos);
	va_start(vl, format);
	vfprintf(stderr, format, vl);
	va_end(vl);
	fprintf(stderr, "\n");
	ShowErrorPos();
	
	++errorcount;
	fail = 1;
}

/*
 * Report a warning from the lexical scanner.
 */
void
LexWarning(const char *format, ...)
{
	va_list vl;
	
	strcpy(errorlinebuf, linebuf);
	errorPos = tokenPos;
	
	fprintf(stderr,
		"%s:%d:%d: warning: ",
		infilename, lineno, 1+errorPos);
	va_start(vl, format);
	vfprintf(stderr, format, vl);
	va_end(vl);
	fprintf(stderr, "\n");
	ShowErrorPos();
	
	++warningcount;
}

/*
 * Report an error from the parser --- i.e., a mismatch between the input token
 * stream and the IDL grammar.
 *
 * This function should be called only from `error' rules within the parser.
 * In particular, do not use this function to report semantic errors!
 */
void
ParseError(const char *format, ...)
{
	va_list vl;
	
	if (errorflag) {
		errorflag = 0;
		
		fprintf(stderr,
			"%s:%d:%d: ",
			infilename, lineno, 1+errorPos);
		va_start(vl, format);
		vfprintf(stderr, format, vl);
		va_end(vl);
		fprintf(stderr, "\n");
		ShowErrorPos();
		
		++errorcount;
	}
	fail = 1;
}

/*
 * Report an error involving a common mistake in IDL grammar.
 *
 * This function is called by the parser from within certain parser rules that
 * catch common IDL mistakes (e.g., the use of `int' instead of `long').  This
 * is slightly different than a real parse error (handled by `ParseError').
 * The errors reported here are not errors as seen by the parser code; in
 * terms of the parser grammar, the IDL is correct.  But it is ``correct'' only
 * because we have added rules to specially handle common IDL mistakes.  Those
 * rules still need to report errors, and those errors are reported here.
 */
void
GrammarError(const char *format, ...)
{
	va_list vl;
	
	strcpy(errorlinebuf, linebuf);
	errorPos = tokenPos;
	
	fprintf(stderr,
		"%s:%d:%d: ",
		infilename, lineno, 1+errorPos);
	va_start(vl, format);
	vfprintf(stderr, format, vl);
	va_end(vl);
	fprintf(stderr, "\n");
	ShowErrorPos();
	
	++errorcount;
	fail = 1;
}

/*
 * Report a semantic error --- i.e., an error arising from the interpretation
 * of (grammatically correct) IDL.
 */
void
SemanticError(const char *format, ...)
{
	va_list vl;
	
	strcpy(errorlinebuf, linebuf);
	errorPos = tokenPos;
	
	fprintf(stderr,
		"%s:%d:%d: ",
		infilename, lineno, 1+errorPos);
	va_start(vl, format);
	vfprintf(stderr, format, vl);
	va_end(vl);
	fprintf(stderr, "\n");
	ShowErrorPos();
	
	++errorcount;
	fail = 1;
}

/*
 * Report a semantic warning --- i.e., a warning about the interpretation of
 * (grammatically correct) IDL.
 */
void
SemanticWarning(const char *format, ...)
{
	va_list vl;
	
	strcpy(errorlinebuf, linebuf);
	errorPos = tokenPos;
	
	fprintf(stderr,
		"%s:%d:%d: warning: ",
		infilename, lineno, 1+errorPos);
	va_start(vl, format);
	vfprintf(stderr, format, vl);
	va_end(vl);
	fprintf(stderr, "\n");
	ShowErrorPos();
	
	++warningcount;
}

/*
 * Report an error and exit.  This is different than `panic'; `panic' causes us
 * to dump core.
 */
DEFN_NORETURN(
void
Exit(const char *format, ...)
)
{
	va_list vl;
	
	va_start(vl, format);
	vfprintf(stderr, format, vl);
	va_end(vl);
	fprintf(stderr, "\n");
	
	exit(1);
}

/*
 * Report that we're bailing out because we're confused by previous errors.
 */
DEFN_NORETURN(
void
ConfusedExit()
)
{
	Exit("%s:%d: confused by previous errors, bailing out",
	     infilename, lineno);
}

/*
 * Report an internal compiler error.  This never returns.
 */
DEFN_NORETURN(
void
InternalError(const char *format, ...)
)
{
	va_list vl;
	
	strcpy(errorlinebuf, linebuf);
	errorPos = tokenPos;
	
	fprintf(stderr,
		"%s:%d:%d: internal compiler error: ",
		infilename, lineno, 1+errorPos);
	va_start(vl, format);
	vfprintf(stderr, format, vl);
	va_end(vl);
	fprintf(stderr, "\n");
	ShowErrorPos();
	
	panic("Exiting due to internal error.");
}

/* End of file. */

