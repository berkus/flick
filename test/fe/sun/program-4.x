/*
 * Copyright (c) 1996, 1999 The University of Utah and
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

/* This file contains test input for `flick-fe-sun'. */
/* Purpose: To test the presentation of a simple interface containing multiple
   operations.  This test was derived from the CORBA `interface_.idl' test. */

struct short_short {
    short a;
    short b;
};

struct long_long {
    long a;
    long b;
};

typedef long long_list<10>;

program MATH {
    version ONE {
	short addshort(short_short) = 1;
	short subshort(short_short) = 2;
	short multshort(short_short) = 3;
	
	long addlong(long_long) = 4;
	long sublong(long_long) = 5;
	long multlong(long_long) = 6;
	
	long addlonglist(long_list) = 7;
	long multlonglist(long_list) = 8;
    } = 1;
} = 1;

/* End of file. */

