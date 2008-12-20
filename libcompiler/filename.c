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

#include <ctype.h>
#include <string.h>

#include <mom/compiler.h>

#define FILE_PATH_SEPARATOR '/'

char *resuffix(const char *orig, const char *newsuffix)
{
	char *str = (char *) mustmalloc(strlen(orig) + strlen(newsuffix) + 1);
	char *dot;
	
	strcpy(str, orig);
	if ((dot = strrchr(str, '.')))
		*dot = 0;
	strcat(str, newsuffix);
	
	return str;
}

const char *file_part(const char *filename)
{
	int len = strlen(filename);
	const char *retval;
	
	for( retval = filename + len;
	     (retval > filename) && (*retval != FILE_PATH_SEPARATOR);
	     retval-- );
	if( *retval == FILE_PATH_SEPARATOR )
		retval++;
	return( retval );
}

const char *dir_part(const char *filename)
{
	int len = strlen(filename), dir_len;
	const char *str;
	char *retval;
	
	for( str = filename + len;
	     (str > filename) && (*str != FILE_PATH_SEPARATOR);
	     str-- );
	dir_len = str - filename;
	retval = (char *)mustmalloc(dir_len + 1);
	strncpy(retval, filename, dir_len);
	retval[dir_len] = 0;
	return( retval );
}

const char *add_file_part(const char *pathname, const char *filename)
{
	int pathlen, filelen, newlen;
	char *str, *retval = 0;
	
	pathlen = strlen(pathname);
	filelen = strlen(filename);
	newlen = pathlen + filelen;
	if( pathname[pathlen - 1] != FILE_PATH_SEPARATOR )
		newlen++;
	retval = (char *)mustmalloc(newlen + 1);
	strcpy(retval, pathname);
	str = &retval[pathlen];
	if( pathname[pathlen - 1] != FILE_PATH_SEPARATOR ) {
		*str = FILE_PATH_SEPARATOR;
		str++;
	}
	strcpy(str, filename);
	return( retval );
}

int absolute_path(const char *filename)
{
	return( filename[0] == FILE_PATH_SEPARATOR );
}

int current_dir(const char *filename)
{
	return( (filename[0] == '.') && (filename[1] == 0) );
}

void filename_to_c_id(char *filename)
{
	for( ; *filename; filename++ ) {
		if( !isalnum(*filename) )
			*filename = '_';
	}
}
