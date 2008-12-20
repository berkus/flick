/*
 * Copyright (c) 1997, 1999 The University of Utah and
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
#include <stdlib.h>
#ifndef RPCGEN
#include "parms-client.h"
#else
#include <flick/link/suntcp.h>
#include "parms.h"
#endif

void run_long_seq_test(CLIENT *c);
	
int main(int argc, char **argv) 
{
	CLIENT tmp, *c;
	FLICK_SERVER_LOCATION s;
	c = &tmp;
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <host>\n", argv[0]);
		exit (1);
	}
  
	s.server_name = argv[1];
	s.prog_num = PARM;
	s.vers_num = ONE;
	create_client(c, s);
#define SHOWTST(prn)				\
	printf("arg:\t"#prn"\n",arg);		\
	if (res)				\
		printf("res:\t"#prn"\n",*res);	\
	else					\
		fprintf(stderr, "Failed to receive _ANY_ result\n");
  
#define RUNTST(type, name, init1, init2, prn) {		\
		type arg, tmp, *res;			\
		arg = init1;				\
		tmp = init2;				\
		res = &tmp;				\
		printf("Before calling "#name"\n");	\
		SHOWTST(##prn);				\
		printf("After calling "#name"\n");	\
		res = ##name##_test_2345(&arg, c);	\
		SHOWTST(##prn);				\
	}
  
/*  RUNTST(void,void,0,0,%x);*/
	RUNTST(char,char,'a','b',%c);
	RUNTST(int,bool,1,234,%d);
	RUNTST(short,short,123,234,%d);
	RUNTST(unsigned short,ushort,123,234,%d);
	RUNTST(int,long,66000,77777,%d);
	RUNTST(unsigned int,ulong,123454,543211,%d);
	RUNTST(float, float, 1.234, 4.321, %f);
	RUNTST(double, double, 1.234567890123, 9.876543210987, %f);
	run_long_seq_test(c);	
	return 0;
}

void init_long_seq(long_seq *arg, int len, int start) 
{
	unsigned int i;
	
	arg->long_seq_len = len;
	arg->long_seq_val = malloc(sizeof(arg->long_seq_val[0])
				   * arg->long_seq_len);
	if (!arg->long_seq_val) {
		printf("Insufficient memory!\n");
		exit(1);
	}
	for (i = 0; i < arg->long_seq_len; i++)
		arg->long_seq_val[i] = start + i;
}

void print_long_seq(long_seq *arg)
{
	unsigned int i;
	printf("Seq<%d>", arg->long_seq_len);
	for (i = 0; i < arg->long_seq_len; i++)
		printf("\t%d", arg->long_seq_val[i]);
	printf("\n");
}


void run_long_seq_test(CLIENT *c) 
{
	long_seq arg, tmp, *res;
	init_long_seq(&arg, 2, 0);
	init_long_seq(&tmp, 5, 1);
	res = &tmp;			
	printf("Before calling long_seq_test\n");
	printf("arg:\t");
	print_long_seq(&arg);
	printf("res:\t");
	if (res)			
		print_long_seq(res);
	else				
		fprintf(stderr, "Failed to receive _ANY_ result\n");
	printf("After calling long_seq_test\n");
	res = long_seq_test_2345(&arg, c);
	printf("arg:\t");
	print_long_seq(&arg);
	printf("res:\t");
	if (res)			
		print_long_seq(res);
	else				
		fprintf(stderr, "Failed to receive _ANY_ result\n");
}

/* End of file. */

