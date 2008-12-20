/*
 * Copyright (c) 1998 The University of Utah and
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
#include "arrayez-client.h"

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
	
	int *in8k;
	int *out8k;
	int i, ok = 1;
	int res;
	
	obj = CORBA_ORB_string_to_object(orb, argv[1], &ev);
	
	in8k = (int *)flick_trapeze_client_array__alloc();
	out8k = (int *)flick_trapeze_client_array__alloc();

	if (ev._major != CORBA_NO_EXCEPTION) {
		printf("Problem in string to object conversion...\n");
		handle_exception(&ev);
		exit(1);
	}
	
	printf("filling in8k...");
	for (i = 0; i < 2048; i++) {
		in8k[i] = 1;
		out8k[i] = 0;
	}
	printf("done.\n");
	
	printf("sending in8k...");fflush(stdout);
 	res = arrayez_test_in8k(obj, in8k, &ev);
	printf("done, result=%d.\n", res);
	if (ev._major != CORBA_NO_EXCEPTION)
		handle_exception(&ev);
	
	printf("receiving out8k...");fflush(stdout);
 	res = arrayez_test_out8k(obj, out8k, &ev);
	printf("done, result=%d.\n", res);
	if (ev._major != CORBA_NO_EXCEPTION)
		handle_exception(&ev);
	else {
		ok = 1;	
		printf("checking out8k...");
		for (i = 0; i < 2048; i++)
			if (out8k[i] != 1) {
				ok = 0;
				printf("failed at byte %d!\n", i*4);
			}
		if (ok) printf("ok.\n");
	}
	
	printf("sending in8k and receiving out8k...");fflush(stdout);
 	res = arrayez_test_io8k(obj, in8k, out8k, &ev);
	printf("done, result=%d.\n", res);
	if (ev._major != CORBA_NO_EXCEPTION)
		handle_exception(&ev);
	else {
		ok = 1;	
		printf("checking out8k...");
		for (i = 0; i < 2048; i++)
			if (out8k[i] != 2) {
				ok = 0;
				printf("failed at byte %d!\n", i*4);
			}
		if (ok) printf("ok.\n");
	}

	flick_trapeze_client_array__free(in8k);
	flick_trapeze_client_array__free(out8k);
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
		fprintf(stderr, "UNKNOWN USER EXCEPTION %s!\n",
			CORBA_exception_id(ev));
		break;
	default:
		fprintf(stderr, "Unknown exception type\n");
	}
	
	CORBA_exception_free(ev);
}

/* End of file. */

