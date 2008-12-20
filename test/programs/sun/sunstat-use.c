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
#include <flick/link/suntcp.h>
#ifndef RPCGEN
#include "sunstat-client.h"
#else
#include "sunstat.h"
#endif

#include <sys/time.h>

#define INTSIZE 4
#define INTQTY 262144
#define INTDONE 4194304
#define STRUCTSIZE 16
#define STRUCTQTY 262144
#define STRUCTDONE 4194304
#define STATSIZE 256
#define STATQTY 65536
#define STATDONE 524288
/*4194304*/

/* Function to print statistics */
void print_stats(int size, double msecs, int samples)
{
	printf("%d\t%d\t%d.%03d\n", size, samples, (int)(msecs/1000), (((int)msecs)%1000));
}

int main(int argc, char **argv)
{
	CLIENT tmp, *c;
	FLICK_SERVER_LOCATION s;
	int qty, i, j;
	int *val;
	
        structlist	structs;
        longlist	longs;
	directory	stats;
	
	unsigned int size;
	double curtime;
	struct timeval start, stop;
	struct timezone tz;
	c = &tmp;
	
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <host>\n", argv[0]);
		exit(1);
	}
	s.server_name = argv[1];
	s.prog_num = PROGNAME;
	s.vers_num = VERSNAME;
	create_client(c, s);
	
	printf("Stat Structures\n");
	printf("Size\tQty\tTime\n");
	size = STATSIZE;
	qty = STATQTY;
	for (i = 256 / size; i <= (int)(STATDONE / size); i *= 2) {
		char filename[128];
		curtime = 0.0;
		stats.directory_len = i;
		stats.directory_val = (entry *)calloc(i, sizeof(entry));
		strcpy(filename,
		       "01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345");
		for (j = 0; j < i; j++)
			stats.directory_val[j].filename = filename;
		/* Prime the pump... */
		val = dirlst_2(&stats, c);
		if (!val) {
			fprintf(stderr, "Failed to receive _ANY_ result\n");
			exit(1);
		}
		gettimeofday(&start, &tz);
		for (j = 0; j < qty; j++)
			dirlst_2(&stats, c); /* no error test (null result) */
		gettimeofday(&stop, &tz);
		curtime = ((double)(stop.tv_sec  - start.tv_sec )) * 1000.0 +
			  ((double)(stop.tv_usec - start.tv_usec)) / 1000.0;
		print_stats(i * size, curtime, qty);
		qty /= 2;
		free(stats.directory_val);
	}
	printf("Small Structures\n");
	printf("Size\tQty\tTime\n");
	size = STRUCTSIZE;
	qty = STRUCTQTY;
        for (i = 64 / size; i <= (int)(STRUCTDONE / size); i *= 2) {
		curtime = 0.0;
                structs.structlist_len = i;
		structs.structlist_val = (outer *)calloc(i, sizeof(outer));
		/* Prime the pump... */
		val = strct_2(&structs, c);
		if (!val) {
			fprintf(stderr, "Failed to receive _ANY_ result\n");
			exit(1);
		}
		gettimeofday(&start, &tz);
                for (j=0; j < qty; j++) 
                        strct_2(&structs, c); /* no error test (null result) */
		gettimeofday(&stop, &tz);
		curtime = ((double)(stop.tv_sec  - start.tv_sec )) * 1000.0 +
			  ((double)(stop.tv_usec - start.tv_usec)) / 1000.0;
                print_stats(i*16, curtime, qty);
		qty /= 2;
		free(structs.structlist_val);
        }
	
	printf("Integers\n");
	printf("Size\tQty\tTime\n");
	size = INTSIZE;
	qty = INTQTY;
        for (i = 64 / size; i <= (int)(INTDONE / size); i *= 2) {
		curtime = 0.0;
		longs.longlist_len = i;
		longs.longlist_val = (int *)calloc(i, sizeof(int));
		/* Prime the pump... */
		val = lng_2(&longs, c);
		if (!val) {
			fprintf(stderr, "Failed to receive _ANY_ result\n");
			exit(1);
		}
		gettimeofday(&start, &tz);
                for (j = 0; j < qty; j++) 
			lng_2(&longs, c); /* no error test (null result) */
		gettimeofday(&stop, &tz);
		curtime = ((double)(stop.tv_sec  - start.tv_sec )) * 1000.0 +
			  ((double)(stop.tv_usec - start.tv_usec)) / 1000.0;
                print_stats(i * 4, curtime, qty);
		qty /= 2;
		free(longs.longlist_val);
        }
	
	return 0;
}

/* End of file. */

