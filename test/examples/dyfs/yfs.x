/*
 * Copyright (c) 1996 The University of Utah and
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

typedef int Handle;

struct args {
	string	name<>;
	Handle  obj;
};

struct linkargs {
	string	name<>;
	Handle  dir;
	Handle  obj;
};

struct readargs {
	Handle  obj;
	int	count;
	int	offset;
};

struct writeargs {
	Handle  obj;
	int	offset;
	opaque	data<>;
};

struct yfsres {
	int		retcode;
	unsigned	yfserror;
};

struct yfsreadres {
	int		retcode;
	unsigned	yfserror;
	opaque  	data<>;
};

/* later, when pointers are implemented */
#if 0
const   NAMELEN=128;
struct yfsdirent {
	string		name<NAMELEN>;
	yfsdirent	*next;	
};

struct yfsdirlist {
	yfsdirent	*entries;
	Handle		dir;
};

union yfsreaddirres switch( int yfserror )
{
  case 0:
	yfsdirlist	list;
  default:
	void;
};
#endif

program YFS_PROGRAM {
    version YFS_VERSION {

	yfsres LOOKUP ( args ) = 1;
	yfsres CREATEFILE ( args ) = 2;
	yfsres CREATELINK ( linkargs ) = 3;
	yfsres CREATEDIR ( args ) = 4;
	yfsres DELETE ( args ) = 5;
	yfsreadres READ ( readargs ) = 6;
	yfsres WRITE ( writeargs ) = 7;
	yfsres SYNC ( Handle ) = 8;
	yfsres SYNCDISK ( void ) = 9;
	yfsres SHUTDOWN ( void ) = 10;
	yfsres STAMP ( Handle ) = 11;
	yfsres STAT ( Handle ) = 12;
#if 0
	yfsreaddirres READDIR ( string ) = 13;
#endif

    } = 1;
} = 4711;

