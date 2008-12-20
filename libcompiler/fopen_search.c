/*
 * Copyright (c) 1999 The University of Utah and
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
#include <stdlib.h>
#include <string.h>

#include <mom/compiler.h>

/*
 * Open the file named by `path', in the mode specified by `mode', searching
 * the directories named in `dir_list'.
 *
 * If `path' is a relative pathname, search for the file in the directories
 * specified in `dir_list'.  `dir_list' is an array of strings, terminated by a
 * null string.
 *
 * If `path' is an absolute pathname, `dir_list' is not searched; we simply try
 * to open it.  Also, as a special case, if `dir_list' is null, we just try to
 * open `path' --- i.e., we act as if `dir_list' contained an empty string.
 *
 * If the named file is opened, `*out_path' is set to contain the path of the
 * opened file, and the open `FILE *' is returned.  Otherwise, `*out_path' is
 * set to null, and NULL is returned.
 */
FILE *
fopen_search(const char *path,
	     const char * const *dir_list,
	     const char *mode,
	     /* OUT */ char **out_path)
{
	FILE *fp;
	const char * const *this_dir;
	
	if (!path)
		return 0;
	
	/* If `path' is an an absolute pathname; don't search `dir_list'. */
	/* Also, if `dir_list' is null, just try to open `path'. */
	if (absolute_path(path) || (!dir_list)) {
		fp = fopen(path, mode);
		if (fp)
			*out_path = muststrdup(path);
		else
			*out_path = 0;
		return fp;
	}
	
	/* `path' is a relative path, so we search for it. */
	for (this_dir = dir_list; *this_dir; ++this_dir) {
		/* Handle "" and "." specially. */
		if (((*this_dir)[0] == 0) || current_dir(*this_dir))
			*out_path = muststrdup(path);
		else
			*out_path = (char *)add_file_part(*this_dir, path);
		fp = fopen(*out_path, mode);
		if (fp)
			return fp;
		
		/* Don't leak useless filename strings. */
		free(*out_path);
	}
	
	/* No luck. */
	*out_path = 0;
	return 0;
}

/* End of file. */

