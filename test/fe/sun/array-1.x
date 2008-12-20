/*
 * Copyright (c) 1996, 1997, 1999 The University of Utah and
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
/* Purpose: To test the generation of code for simple arrays. */
/* This file is also useful for timing tests. */

typedef int data64[16];
typedef int data128[32];
typedef int data256[64];
typedef int data512[128];
typedef int data1K[256];
typedef int data2K[512];
typedef int data4K[1024];
typedef int data8K[2048];
typedef int data16K[4096];
typedef int data32K[8192];
typedef int data64K[16384];

program PROG {
	version VERS {
		int op64(data64) = 1;
		int op128(data128) = 2;
		int op256(data256) = 3;
		int op512(data512) = 4;
		int op1K(data1K) = 5;
		int op2K(data2K) = 6;
		int op4K(data4K) = 7;
		int op8K(data8K) = 8;
		int op16K(data16K) = 9;
		int op32K(data32K) = 10;
		int op64K(data64K) = 11;
	} = 1234;
} = 3465;

/* End of file. */

