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
#include "oneway-client.h"
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
	CORBA_short retshort = -1;
	CORBA_long retlong = -1, inlong, outlong, inoutlong;
	CORBA_char *instring, *outstring, *inoutstring, *retstring;
	CORBA_Environment ev;
	
	orb = CORBA_ORB_init(&argc, argv, 0 /* use default ORBid */, &ev);
	obj = CORBA_ORB_string_to_object(orb, argv[1], &ev);
	if (ev._major != CORBA_NO_EXCEPTION) {
		printf("%s exception: Problem converting `%s' to an object.\n",
		       CORBA_exception_id(&ev), argv[1]);
		exit(1);
	}
	printf("Beginning communication...\n");
	
	instring = (char *)CORBA_alloc(sizeof("Enter Name"));
	strcpy(instring, "Enter Name");
	outstring = (char *)CORBA_alloc(sizeof("<nothing1>"));
	strcpy(outstring, "<nothing1>");
	inoutstring = (char *)CORBA_alloc(sizeof("An InOutString"));
	strcpy(inoutstring, "An InOutString");
	retstring = (char *)CORBA_alloc(sizeof("<nothing2>"));
	strcpy(retstring, "<nothing2>");
	
	ev._major = 0;
	printf("Before Call...");
	printf("Exception:%s\n", ev._major ? CORBA_exception_id(&ev): "no");
	printf("In:       %s\n", instring);	       
	printf("Calling Server onewaytest_onewaystring...\n");
	onewaytest_onewaystring (obj, instring, &ev);
	printf("After Call:\n");
	printf("Exception:%s\n", ev._major ? CORBA_exception_id(&ev): "no");
	printf("In:       %s\n", instring);	       
	
	ev._major = 0;
	printf("Before Call...");
	printf("Exception:%s\n", ev._major ? CORBA_exception_id(&ev): "no");
	printf("Result:   %s\n", retstring);
	printf("Calling Server onewaytest_stringreturn...\n");
	retstring = onewaytest_stringreturn (obj, &ev);
	printf("After Call:\n");
	printf("Result of st: %s\n", retstring);
	printf("Exception:%s\n", ev._major ? CORBA_exception_id(&ev): "no");
	printf("Result:   %s\n", retstring);
	
	inoutlong = 5; 
	ev._major = 0;
	printf("Before Call...");
	printf("Exception:%s\n", ev._major ? CORBA_exception_id(&ev): "no");
	printf("Inout:    %i\n", inoutlong);
	printf("Result:   %i\n", retshort);
	printf("Calling Server onewaytest_shortlong...\n");
	retshort = onewaytest_shortlong (obj, &inoutlong, &ev);
	printf("After Call:\n");
	printf("Exception:%s\n", ev._major ? CORBA_exception_id(&ev): "no");
	printf("Inout:    %i\n", inoutlong);
	printf("Result:   %i\n", retshort);
	
	ev._major = 0;
	printf("Before Call...");
	printf("Exception:%s\n", ev._major ? CORBA_exception_id(&ev): "no");
	printf("Result:   %s\n", retstring);
	printf("Calling Server onewaytest_stringreturn...\n");
	retstring = onewaytest_stringreturn (obj, &ev);
	printf("After Call:\n");
	printf("Result of st: %s\n", retstring);
	
	printf("Exception:%s\n", ev._major ? CORBA_exception_id(&ev): "no");
	printf("Result:   %s\n", retstring);
	
	inlong = 4;
	ev._major = 0;
	printf("Before Call...");
	printf("Exception:%s\n", ev._major ? CORBA_exception_id(&ev): "no");
	printf("In:       %i\n", inlong);
	printf("Calling Server onewaytest_voidlong...\n");
	onewaytest_voidlong (obj, inlong, &ev);
	printf("After Call:\n");
	printf("Exception:%s\n", ev._major ? CORBA_exception_id(&ev): "no");
	printf("In:       %i\n", inlong);
	
	inlong = 3;
	ev._major = 0;
	printf("Before Call...");
	printf("Exception:%s\n", ev._major ? CORBA_exception_id(&ev): "no");
	printf("In:       %i\n", inlong);
	printf("Calling Server onewaytest_onewaylong...\n");
	onewaytest_onewaylong (obj, inlong, &ev);
	printf("After Call:\n");
	printf("Exception:%s\n", ev._major ? CORBA_exception_id(&ev): "no");
	printf("In:       %i\n", inlong);
	
	ev._major = 0;
	printf("Before Call...");
	printf("Exception:%s\n", ev._major ? CORBA_exception_id(&ev): "no");
	printf("Inout:    %s\n", inoutstring);
	printf("Result:   %i\n", retlong);
	printf("Calling Server onewaytest_longstring...\n");
	retlong = onewaytest_longstring (obj, &inoutstring, &ev);
	printf("After Call:\n");
	printf("Exception:%s\n", ev._major ? CORBA_exception_id(&ev): "no");
	printf("Inout:    %s\n", inoutstring);
	printf("Result:   %i\n", retlong);
	
	ev._major = 0;
	outlong = 0;
	printf("Before Call...");
	printf("Exception:%s\n", ev._major ? CORBA_exception_id(&ev): "no");
	printf("Out:      %i\n", outlong);
	printf("Result:   %s\n", retstring);
	printf("Calling Server onewaytest_stringlong...\n");
       	retstring = onewaytest_stringlong (obj, &outlong, &ev);
	printf("After Call:\n");
	printf("Exception:%s\n", ev._major ? CORBA_exception_id(&ev): "no");
	printf("Out:      %i\n", outlong);
	printf("Result:   %s\n", retstring);
	
	/* Test a onewaytest again */
	ev._major = 0;
	printf("Before Call...");
	printf("Exception:%s\n", ev._major ? CORBA_exception_id(&ev): "no");
	printf("In:       %s\n", instring);
	printf("Calling Server onewaytest_onewaystring...\n");
	onewaytest_onewaystring (obj, instring, &ev);
	printf("After Call:\n");
	printf("Exception:%s\n", ev._major ? CORBA_exception_id(&ev): "no");
	printf("In:       %s\n", instring);
	
	return 0;
}

/* End of file. */

