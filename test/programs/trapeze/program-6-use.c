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
#include <sys/time.h>

#ifndef RPCGEN
#include "program-6-client.h"
#else
#include "program-6.h"
#endif

double curtime;

int main(int argc, char **argv) 
{
	CLIENT tmp, *c;
	FLICK_SERVER_LOCATION s;
	int i;
	int count, len;
	struct timeval start, stop;
	struct timezone tz;
	
	c = &tmp;
	
	if (argc != 5) {
		printf("Usage: %s <host> <# of calls> <# of elements in array> <s/i>\n",argv[0]);
		exit(1);
	}
	
	s.server_name = argv[1];
	s.prog_num = MATH;
	s.vers_num = ONE;
	flick_client_create(c, s);
	
	count = atol(argv[2]);
	len = atol(argv[3]);
	
	tz.tz_minuteswest = tz.tz_dsttime = 0;
	
	if ((argv[4][0] == 's') || (argv[4][0] == 'S')) {
		int *retval;
		sb_list buf;
		
		buf.sb_list_len = len;
		buf.sb_list_val = (sb *)flick_trapeze_client_array__alloc();
		assert(buf.sb_list_val);
		
		/* Prime the pump. */
		retval = sb_test_50(&buf, c);
		if (!retval) {
			fprintf(stderr, "RPC failed!  Exiting...\n");
			flick_trapeze_client_array__free(buf.sb_list_val);
			exit(1);
		}
		
		/* Make the RPCs. */
		gettimeofday(&start, &tz);
		for (i = 0; i < count; i++)
			sb_test_50(&buf, c); /* XXX --- Check for errors! */
		gettimeofday(&stop, &tz);
		curtime = ((double)(stop.tv_sec  - start.tv_sec )) * 1000.0 +
			  ((double)(stop.tv_usec - start.tv_usec)) / 1000.0;
		
		flick_trapeze_client_array__free(buf.sb_list_val);
		
	} else {
		int *retval;
		long_list buf;
		
		buf.long_list_len = len;
		buf.long_list_val = (int *)flick_trapeze_client_array__alloc();
		assert(buf.long_list_val);
		
		/* Prime the pump. */
		retval = long_test_50(&buf, c);
		if (!retval) {
			fprintf(stderr, "RPC failed!  Exiting...\n");
			flick_trapeze_client_array__free(buf.long_list_val);
			exit(1);
		}
		
		/* Make the RPCs. */
		gettimeofday(&start, &tz);
		for (i = 0; i < count; i++)
			long_test_50(&buf, c); /* XXX --- Check for errors! */
		gettimeofday(&stop, &tz);
		curtime = ((double)(stop.tv_sec  - start.tv_sec )) * 1000.0 +
			  ((double)(stop.tv_usec - start.tv_usec)) / 1000.0;
		
		flick_trapeze_client_array__free(buf.long_list_val);
	}
	
	printf("Completed in %f milliseconds.\n", curtime);  
	return 0;
}

/* End of file. */

