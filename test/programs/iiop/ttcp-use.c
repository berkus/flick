/*
 * Copyright (c) 1996, 1997, 1998, 1999 The University of Utah and
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
#include "ttcp-client.h"
#include <sys/time.h>

/* Function to print statistics */
void print_stats(int size, struct timeval starttime, struct timeval endtime, int samples)
{
        double msecs = (1000.0 * (endtime.tv_sec - starttime.tv_sec)) +
                ( (endtime.tv_usec - starttime.tv_usec) / 1000.0);
	printf("%d\t%3.2f\n", size,
	       (double)(size * samples) * 8.0/(1048576.0 * msecs / 1000.0));
}

void handle_exception(CORBA_Environment *ev);

int main(int argc, char **argv)
{
	CORBA_ORB orb = 0;
	CORBA_Environment ev;
	CORBA_Object obj;
	
	int qty = 65536, i, j;
        structBuffer sbuf;
        intBuffer ibuf;	
	
	orb = CORBA_ORB_init(0, 0, 0 /* use default ORBid */, &ev);
	obj = CORBA_ORB_string_to_object(orb, argv[1], &ev);
	if (ev._major != CORBA_NO_EXCEPTION) {
		printf("Problem in string to object conversion...\n");
		exit(1);
	}
	printf("Structures\nData\tMbps\n");
        for (i = 256; i <= 262144; i+= 64 ) {
                struct timeval starttime, endtime;
                sbuf._length = i;
		sbuf._buffer = (b *)calloc(i, sizeof(b));
		if (!sbuf._buffer) {
			printf("Error allocating s-buffer.\n");
			exit(1);
		}		
		
		ttcp_structtest(obj, &sbuf, &ev);
		qty = 0x400000 / (i * sizeof(b));
		if (qty > 1024)
			qty = 1024;
		gettimeofday(&starttime, NULL);
                for (j = 0; j < qty; j++) {
                        ttcp_structtest(obj, &sbuf, &ev);
			if (ev._major != CORBA_NO_EXCEPTION)
				handle_exception(&ev);
		}
		

                gettimeofday(&endtime, NULL);
                print_stats(i*16, starttime, endtime, qty);
		free(sbuf._buffer);
                qty /= 3;
        }
	
        qty = 65536;
	printf("Integers\nData\tMbps\n");
        for (i = 1024; i <= 1024 * 1024; i += 64) {
                struct timeval starttime, endtime;
		
		ibuf._length = i;
		ibuf._buffer = (int *)calloc(i, sizeof(int));
		if (!ibuf._buffer) {
			printf("Error allocating i-buffer.\n");
			exit(1);
		}		
		
		ttcp_inttest(obj, &ibuf, &ev);
		qty = 0x400000 / (i * sizeof(int));
		if (qty > 1024)
			qty = 1024;
		
                gettimeofday(&starttime, NULL);
                for (j = 0; j < qty; j++) {
                        ttcp_inttest(obj, &ibuf, &ev);
			if (ev._major != CORBA_NO_EXCEPTION)
				handle_exception(&ev);
		}
                gettimeofday(&endtime, NULL);
                print_stats(i * 4, starttime, endtime, qty);
		free(ibuf._buffer);
                qty /= 3;
        }
	return 0;
}

void handle_exception(CORBA_Environment *ev)
{
	switch (ev->_major) {
	case CORBA_NO_EXCEPTION:
		return; /* don't abort the program */
	case CORBA_SYSTEM_EXCEPTION:
		fprintf(stderr, "%s ERROR %d.%d\n",
			CORBA_exception_id(ev),
			((CORBA_UNKNOWN *)CORBA_exception_value(ev))->minor,
			((CORBA_UNKNOWN *)CORBA_exception_value(
				ev))->completed);
		break;
	case CORBA_USER_EXCEPTION:
        	if (!strcmp(CORBA_exception_id(ev), ex_useless))
                        /* the 'useless' exception */
			fprintf(stderr, "Useless exception raised - %d\n",
				((useless *)
				 CORBA_exception_value(ev))->baloney);
		else
			fprintf(stderr, "Unknown user exception raised!");
		break;
	default:
		fprintf(stderr, "Unknown exception type\n");
	}
	
	CORBA_exception_free(ev);
	exit(1);
}

/* End of file. */

