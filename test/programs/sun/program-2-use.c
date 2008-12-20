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
#include "program-2-client.h"
#else
#include "program-2.h"
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
	assert(i < 10000);
  
	s.server_name = argv[1];
	s.prog_num = PROGNAME;
	s.vers_num = VERSNAME;
	printf("Creating the client...\n");
	create_client(c, s);
	printf("Beginning the test\n");
	
	{
		parm p;
		int count = 50000;
    
		p.data.data_len = i;
		p.data.data_val = (int *)malloc(i*sizeof(int));
    
		while (--count) {
			if (!(count & 0x1ff))
				printf("%5d\n",count);
			val = op_1(&p, c);
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

