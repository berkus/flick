/*
 * Copyright (c) 1999 The University of Utah and
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
#include "tpztest-client.h"

/* Function to print statistics */
void print_stats(int size, struct timeval starttime, struct timeval endtime, int samples)
{
        double msecs = (1000.0 * (endtime.tv_sec - starttime.tv_sec)) +
                ( (endtime.tv_usec - starttime.tv_usec) / 1000.0);
	printf("%d\t%3.2f Mb/s\t(%3.2f MB/s)\n", size,
	       (double)(size * samples) * 8.0/(1048576.0 * msecs / 1000.0),
	       (double)(size * samples) * 1.0/(1048576.0 * msecs / 1000.0));
}

void handle_exception(CORBA_Environment *ev);

enum WHATTODO {
	NOTHING,
	BANDWIDTH,
	RQST_RSPN
};
	
int main(int argc, char **argv)
{
	int ntimes = 0;
	tpztest_payload_slice *pld;
	enum WHATTODO whattodo = NOTHING;
	
	CORBA_ORB orb = 0;
	CORBA_Environment ev;
	CORBA_Object obj;
	
	int i;
	for (i = 1; i < argc; i++) {
		switch (argv[i][0]) {
		case '-':
			switch (argv[i][1]) {
			case 'b':
				if (whattodo != NOTHING) {
					fprintf(stderr, ("Specify only one: "
							 "-r or -b.\n"));
					return 1;
				}
				whattodo = BANDWIDTH;
				break;
			case 'r':
				if (whattodo != NOTHING) {
					fprintf(stderr, ("Specify only one: "
							 "-r or -b.\n"));
					return 1;
				}
				whattodo = RQST_RSPN;
				break;
			case 'h':
			  printhelp:
			  printf("%s [-h] {-b|-r} -n num obj\n"
				 "  -h:\tPrint this help.\n"
				 "  -b:\tBandwidth test (cannot specify -r).\n"
				 "  -r:\tRequest/response test "
				         "(cannot specify -b).\n"
				 "  -n:\tPerform test num times.\n"
				 "  obj:\tServer object reference.\n",
				 argv[0]);
			  return 0;
			case 'n':
				++i;
				if (i >= argc) goto printhelp;
				ntimes = atoi(argv[i]);
				break;
			default:
				fprintf(stderr, "Invalid switch '-%c'.\n",
					argv[i][1]);
				goto printhelp;
			}
			break;
			
		default:
			if (i == argc - 1)
				break; /* Last parameter -- object ref */
			fprintf(stderr, "Invalid parameter '%s'.\n", argv[i]);
			goto printhelp;
		}
	}

	/* Make sure `ntimes' is a valid value! */
	if (ntimes <=0) {
		fprintf(stderr, "Must specify number of tests > 0.\n");
		return 1;
	}
	
	/* Create the object reference. */
	obj = CORBA_ORB_string_to_object(orb, argv[argc-1], &ev);
	if (ev._major != CORBA_NO_EXCEPTION) {
		printf("Problem in string to object conversion...\n");
		handle_exception(&ev);
		return 1;
	}
	
	if (whattodo == BANDWIDTH) {
		struct timeval start, end;
		
		printf("** BANDWIDTH **\n");
		gettimeofday(&start, 0);
		ntimes--;
		pld = flick_trapeze_client_array__alloc();
		for (i = 0; i < ntimes; i++) {
			tpztest_bandwidth(obj, pld, &ev);
			assert(ev._major == CORBA_NO_EXCEPTION);
		}
		tpztest_bandwidth_pingback(obj, pld, &ev);
		assert(ev._major == CORBA_NO_EXCEPTION);
		flick_trapeze_client_array__free(pld);
		gettimeofday(&end, 0);
		
		/* Print the results: */
		print_stats(sizeof(tpztest_payload), start, end, ntimes + 1);
	} else if (whattodo == RQST_RSPN) {
		struct timeval start, end;
		
		printf("** REQUEST-RESPONSE **\n");
		gettimeofday(&start, 0);
		for (i = 0; i < ntimes; i++) {
			pld = tpztest_rqst_rspn(obj, &ev);
			assert(ev._major == CORBA_NO_EXCEPTION);
			flick_trapeze_client_array__free(pld);
		}
		gettimeofday(&end, 0);
		
		/* Print the results: */
		print_stats(sizeof(tpztest_payload), start, end, ntimes + 1);
	} else {
		fprintf(stderr, "Nothing to do!\n");
		return 2;
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
		fprintf(stderr, "UNKNOWN USER EXCEPTION %s!\n",
			CORBA_exception_id(ev));
		break;
	default:
		fprintf(stderr, "Unknown exception type\n");
	}
	
	CORBA_exception_free(ev);
}

/* End of file. */

