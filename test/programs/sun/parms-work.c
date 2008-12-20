/*
 * Copyright (c) 1997, 1998, 1999 The University of Utah and
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
#ifndef RPCGEN
#include "parms-server.h"
#else
#include <flick/link/suntcp.h>
#include "parms.h"
#endif

#define MKFUNC(type, name)				\
type * ##name##_test_2345(type *arg, struct svc_req *o)	\
{							\
	static type res;				\
							\
	res = *arg;					\
	return &res;					\
}

void *void_test_2345(void *arg, struct svc_req *o)
{
	static int res;
	
	return &res;
}

MKFUNC(char, char)
MKFUNC(int, bool)
MKFUNC(short, short)
MKFUNC(unsigned short, ushort)
MKFUNC(int, long)
MKFUNC(unsigned int, ulong)
MKFUNC(float, float)
MKFUNC(double, double)

long_seq *long_seq_test_2345(long_seq *arg, struct svc_req *o)
{
	unsigned int i;
	static long_seq res;
	
	res.long_seq_len = arg->long_seq_len;
	res.long_seq_val = malloc(sizeof(res.long_seq_val[0])
				  * res.long_seq_len);
	if (!res.long_seq_val) {
		printf("Insufficient memory!\n");
		exit(1);
	}
	for (i = 0; i < arg->long_seq_len; i++)
		res.long_seq_val[i] = arg->long_seq_val[i];
	return &res;
}

/* End of file. */

