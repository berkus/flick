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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <flick/link/suntcp.h>

#ifndef RPCGEN
#include "program-5-client.h"
#else
#include "program-5.h"
#endif

int main(int argc, char **argv) 
{
	CLIENT tmp, *c;
	FLICK_SERVER_LOCATION s;
	int i;
	int *val;
	c = &tmp;
	
	if (argc != 3) {
		fprintf(stderr, "Usage: %s <host> <string size>\n", argv[0]);
		exit (1);
	}
	
	i = (int)atol(argv[2]);
	
	s.server_name = argv[1];
	s.prog_num = PROGNAME;
	s.vers_num = VERSNAME;
	create_client(c, s);
	{
		char *str = (char *)malloc(i);
		int count = 50000;
		str[--i] = 0;
		
		while(i)
			str[--i] = i%64 + 32;
		
		while (--count) {
			val = op_1(&str, c);
			if (!val) {
				fprintf(stderr,
					"Failed to receive _ANY_ result\n");
				exit(1);
			}
		}
	}
	
	printf("Completed!\n");  
	return 0;
}

/* End of file. */

