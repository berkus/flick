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

#ifndef __Errors_hh
#define __Errors_hh

/* Use <mom/compiler.h> to get DECL_NORETURN. */
#include <mom/compiler.h>

void LexError(const char *format, ...);
void LexWarning(const char *format, ...);
void ParseError(const char *format, ...);
void GrammarError(const char *format, ...);
void SemanticError(const char *format, ...);
void SemanticWarning(const char *format, ...);

DECL_NORETURN(void Exit(const char *format, ...));
DECL_NORETURN(void ConfusedExit());
DECL_NORETURN(void InternalError(const char *format, ...));

extern int lineno;
extern int tokenPos, nextPos, errorPos;
#define BUFSIZE 500
extern char linebuf[BUFSIZE]; /* Buffer for entire lines of IDL code. */
extern char errorlinebuf[BUFSIZE];
extern int fail;
extern int errorflag;

#endif /* __Errors_hh */

/* End of file. */

