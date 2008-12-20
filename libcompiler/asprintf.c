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

#include <assert.h>
#include <stdio.h>
#include <stdarg.h>

#include <mom/compiler.h>

#define BUFSIZE (256)

char *flick_asprintf(const char *fmt, ...)
{
	va_list vl;
	
	char buf[BUFSIZE];
	int buf_len;
	char *str;
#ifdef HAVE_VSNPRINTF
	int str_len;
#endif
	
	/*****/
	
	va_start(vl, fmt);
#ifdef HAVE_VSNPRINTF
	buf_len = vsnprintf(buf, BUFSIZE, fmt, vl);
#else
	/* This may overrun `buf'... */
	vsprintf(buf, fmt, vl);
	buf_len = strlen(buf);
	if (buf_len >= BUFSIZE)
		panic("In `flick_asprintf', overran formatting buffer.");
#endif
	va_end(vl);
	
	/*
	 * `vsnprintf' returns the number of characters in the formatted string
	 * (minus the terminating NUL), *even if* `buf' wasn't large enough to
	 * contain that string.
	 */
	str = (char *) mustmalloc((buf_len + 1) * sizeof(char));
#ifdef HAVE_VSNPRINTF
	if (buf_len >= BUFSIZE) {
		/*
		 * The initial buffer was not large enough to contain the
		 * formatted string.  Call `vsnprintf' with a buffer of the
		 * correct size.
		 */
		va_start(vl, fmt);
		str_len = vsnprintf(str, (buf_len + 1), fmt, vl);
		va_end(vl);
		assert(str_len == buf_len);
	} else {
#endif
		/*
		 * The initial buffer was large enough to contain the formatted
		 * string.  Copy the string out of the buffer.
		 */
		strcpy(str, buf);
		assert(strlen(str) == (unsigned int) buf_len);
#ifdef HAVE_VSNPRINTF
	}
#endif
	
	return str;
}

/* End of file. */

