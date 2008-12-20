/*
 * Copyright (c) 1997, 1998 The University of Utah and
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
#include "namelens-client.h"
#include <sys/time.h>

/* Function to print statistics */
void print_stats(int size,
		 struct timeval starttime, struct timeval endtime,
		 int samples)
{
        double msecs = (1000.0 * (endtime.tv_sec - starttime.tv_sec)) +
		       ( (endtime.tv_usec - starttime.tv_usec) / 1000.0);
	printf("%d\t%3.2f\n", size,
	       (double)(size * samples) * 8.0/(1048576.0 * msecs / 1000.0));
}

int main(int argc, char **argv)
{
	CORBA_ORB orb = 0;
	CORBA_Object obj;
	CORBA_Environment _ev;
	
	orb = CORBA_ORB_init(0, 0, 0 /* use default ORBid */, &_ev);
	obj = CORBA_ORB_string_to_object(orb, argv[1], &_ev);
	if (_ev._major != CORBA_NO_EXCEPTION) {
		printf("Problem converting `%s' to an object.\n", argv[1]);
		exit(1);
	}
	printf("Beginning communication...\n");
	
	onewaytest_n (obj, &_ev);
	onewaytest_n2 (obj, &_ev);
	onewaytest_n23 (obj, &_ev);
	onewaytest_n234 (obj, &_ev);
	onewaytest_n2345 (obj, &_ev);
	onewaytest_n23456 (obj, &_ev);
	onewaytest_n234567 (obj, &_ev);
	onewaytest_n2345678 (obj, &_ev);
	onewaytest_n23456789 (obj, &_ev);
	onewaytest_n234567890 (obj, &_ev);
	printf("Communication Done.\n");	       

	return 0;
}

/* End of file. */

