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

typedef long long_seq<10>;

program PARM {
    version ONE {
	void void_test(void) = 0;
	char char_test(char) = 1;
	bool bool_test(bool) = 2;
	short short_test(short) = 3;
	unsigned short ushort_test(unsigned short) = 4;
	long long_test(long) = 5;
	unsigned long ulong_test(unsigned long) = 6;
	float float_test(float) = 7;
	double double_test(double) = 8;
	long_seq long_seq_test(long_seq) = 9;
    } = 2345;
} = 4321;

/* End of file. */

