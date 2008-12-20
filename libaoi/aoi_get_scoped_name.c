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

#include <string.h>
#include <mom/compiler.h>
#include <mom/libaoi.h>
#include <assert.h>

extern aoi in_aoi;
#define a(n) (in_aoi.defs.defs_val[n])

/* if names get that long, good luck typing! */
#define BUFLEN 256

char *aoi_get_scoped_name(aoi_ref aref, const char *separator)
{
	/* We need to walk back up the scope hierarchy,
	 * until we reach a 0 scope, then we've finished.
	 * separator ("_", "::", etc.) is put between each
	 * scope name.
	 */
	
	char *newname = mustmalloc(BUFLEN);
	int i;
	
	int seplen = strlen(separator);
	int curlen = strlen(a(aref).name);
	int last_scope = a(aref).scope;
	if(!newname)
		panic("Out of memory in aoi_get_scoped_name\n");
	for(i = 0; i < BUFLEN; i++)
		newname[i] = 0;
	strcpy(newname,a(aref).name);

	while(last_scope) {
		int len, pos;
		
		while(a(--aref).scope >= last_scope);
		last_scope = a(aref).scope;

		len = strlen(a(aref).name);
		assert(len + seplen + curlen < BUFLEN);
		for(pos = curlen; pos >= 0;pos--)
			newname[len + seplen + pos] = newname[pos];
		/* strcpy isn't used since it would insert nulls in the name */
		for(pos = seplen - 1; pos >= 0;pos--)
			newname[len + pos] = separator[pos];
		for(pos = len - 1; pos >= 0;pos--)
			newname[pos] = a(aref).name[pos];
		curlen += len + seplen;
	}
	return newname;
}

/* End of file. */
