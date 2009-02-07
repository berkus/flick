/*
 * Copyright (c) 1996, 1997 The University of Utah and
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
/*  We declare these individually rather than use the header because
  the HPUX and SunOS4 systems we tried compiling on have funky g++
  installations that make strrchr get munged _strrchr__FPci instead
  of _strrchr
  berkus: this breaks on gcc4
 
extern "C" char *strrchr(const char *s, int c);
extern "C" int strcmp(const char *s, const char *s2);
*/
#include "private.hh"

/*
 * This method allows a presentation generator to create some initial output
 * CAST.  The library version creates an `#include' statement for a file that
 * defines presentation-specific things (e.g., the basic object type such as
 * `CORBA_Object').
 *
 * XXX --- This code is similar to that in `pg_state::p_interface_def_include'.
 * Maybe we should write a generic `#include' statement verifier.
 */

void pg_state::build_init_cast(void)
{
	char *include_file_name;
	char *include_file_name_nondir;
	
	calc_name_component *separator_component;
	char separator_char;
	
	/*
	 * Compute the `#include' file name to get presentation-specific goo.
	 */
	include_file_name = calc_presentation_include_file_name("");
	
	/*
	 * Now we have to make sure that the file name is valid.  Dig the
	 * file name component separator character out of our `names.literals'.
	 */
	separator_component =
		&(names.literals[filename_component_separator_lit]);
	
	switch (separator_component->len) {
	case 1:
		separator_char = *(separator_component->str);
		break;
		
	case 0:
		warn("Can't really cope with a zero-character filename "
		     "component separator.");
		separator_char = '/';
		break;
		
	default:
		warn("Can't really cope with a multicharacter filename "
		     "component separator.");
		separator_char = separator_component->
				 str[separator_component->len - 1];
		break;
	}
	
	/*
	 * Now find the final file name component.
	 */
	include_file_name_nondir = strrchr(include_file_name, separator_char);
	if (include_file_name_nondir) {
		/*
		 * Don't increment in the bizarro case that `separator_char'
		 * is NUL.
		 */
		if (*include_file_name_nondir)
			++include_file_name_nondir;
	} else
		include_file_name_nondir = include_file_name;
	
	/*
	 * If the `#include' file name is good, emit the `#include' statement.
	 */
	if ((strcmp("", include_file_name_nondir) != 0)
	    && (strcmp(".h", include_file_name_nondir) != 0))
		p_emit_include_stmt(include_file_name,
				    1 /* 1 == system header */);
}

/* End of file. */

