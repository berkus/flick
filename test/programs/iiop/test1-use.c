/*
 * Copyright (c) 1997, 1998, 1999 The University of Utah and
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
#include "test1-client.h"
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
#define C_DECLV(type) CORBA_##type	ret##type, in##type, out##type, inout##type
#define T_DECLV(type) test1_##type	ret##type, in##type, out##type, inout##type
	C_DECLV(short);
	C_DECLV(long);
	T_DECLV(ushort);
	T_DECLV(ulong);
	C_DECLV(float);
	C_DECLV(double);
	C_DECLV(char);
	C_DECLV(boolean);
	C_DECLV(octet);
	C_DECLV(Object);
	
	CORBA_char *instring, *outstring, *inoutstring, *retstring;
	CORBA_long case_num;
	CORBA_Environment ev;
	
	orb = CORBA_ORB_init(&argc, argv, 0 /* use default ORBid */, &ev);
	obj = CORBA_ORB_string_to_object(orb, argv[1], &ev);
	if (ev._major != CORBA_NO_EXCEPTION) {
		printf("Problem converting `%s' to an object.\n", argv[1]);
		exit(1);
	}
	printf("Beginning communication...\n");
#define SHOWPARMS(prn, type)						    \
	if (ev._major != CORBA_NO_EXCEPTION)				    \
		printf("Exception: `%s' EXCEPTION RAISED FOR " #type "!\n", \
		       CORBA_exception_id(&ev));			    \
	else								    \
		printf("Exception: no\n");				    \
	printf("In:       " #prn"\n", in##type);			    \
	printf("Inout:    " #prn"\n", inout##type);			    \
	printf("Out:      " #prn"\n", out##type);			    \
	printf("Result:   " #prn"\n", ret##type);
#define RUNTEST(type, inval, inoutval, otherval, prn)			\
	in##type = inval;						\
	inout##type = inoutval;						\
	ret##type = out##type = otherval;				\
	ev._major = CORBA_NO_EXCEPTION;					\
	printf("Testing " #type" Before Call:\n");			\
	SHOWPARMS(##prn, ##type);					\
	printf("Calling Server...\n");					\
	ret##type = test1_test_##type(obj, in##type, &out##type,	\
				      &inout##type, &ev);		\
	printf("After Call:\n");					\
	SHOWPARMS(##prn, ##type);
	
	RUNTEST(short, 1234, 4321, 7, %d);
	RUNTEST(ushort, 65500, 33000, 6, %d);
	RUNTEST(long, 12341234, 43214321, 2, %d);
	RUNTEST(ulong, 3000000000U, 4000000000U, 5, %u);
	RUNTEST(float, 1.234, 2.345, 3.1415, %f);
	RUNTEST(double, 1.2345678901, 2.3456789012, 2.71828, %f);
	RUNTEST(boolean, 0, 1, 2, %d);
	RUNTEST(octet, 12, 23, 34, %d);
	RUNTEST(char, 'a', 'b', 'c', %c);
	
	/* Must allocate the `inout' string with the CORBA allocator. */
	inoutstring = (char *) CORBA_alloc(sizeof("Howdy"));
	(void)strcpy(inoutstring, "Howdy");
	RUNTEST(string, "Hello", inoutstring, "Ugh", %s);
	
	/* Now run the object passing test */
        inObject = CORBA_Object_duplicate(obj, &ev);
        inoutObject = CORBA_Object_duplicate(obj, &ev);
	/* check for exception */
        retObject = outObject = 0;
        printf("Testing Object Before Call:\n");
	if (ev._major != CORBA_NO_EXCEPTION)
		printf("Exception: `%s' EXCEPTION RAISED FOR Object!\n",
		       CORBA_exception_id(&ev));
	else
		printf("Exception: no\n");
        printf("In:       %p\nURL:\t%s\nIOR:\t%s\n", inObject,
	       CORBA_ORB_object_to_readable_string(orb, inObject, &ev),
	       CORBA_ORB_object_to_string(orb, inObject, &ev));
        printf("Inout:    %p\nURL:\t%s\nIOR:\t%s\n", (void *)inoutObject,
	       CORBA_ORB_object_to_readable_string(orb, inoutObject, &ev),
	       CORBA_ORB_object_to_string(orb, inoutObject, &ev));
        printf("Out:      %p\nURL:\t%s\nIOR:\t%s\n", (void *)outObject,
	       CORBA_ORB_object_to_readable_string(orb, outObject, &ev),
	       CORBA_ORB_object_to_string(orb, outObject, &ev));
        printf("Result:   %p\nURL:\t%s\nIOR:\t%s\n", (void *)retObject,
	       CORBA_ORB_object_to_readable_string(orb, retObject, &ev),
	       CORBA_ORB_object_to_string(orb, retObject, &ev));
        printf("Calling Server...\n");
        retObject = test1_test_Object(obj, inObject, &outObject, &inoutObject,
				      &ev);
        printf("After Call:\n");
	if (ev._major != CORBA_NO_EXCEPTION) 
		printf("Exception: `%s' EXCEPTION RAISED FOR Object!\n",
		       CORBA_exception_id(&ev));
	else
		printf("Exception: no\n");
        printf("In:       %p\nURL:\t%s\nIOR:\t%s\n", inObject,
	       CORBA_ORB_object_to_readable_string(orb, inObject, &ev),
	       CORBA_ORB_object_to_string(orb, inObject, &ev));
        printf("Inout:    %p\nURL:\t%s\nIOR:\t%s\n", inoutObject,
	       CORBA_ORB_object_to_readable_string(orb, inoutObject, &ev),
	       CORBA_ORB_object_to_string(orb, inoutObject, &ev));
        printf("Out:      %p\nURL:\t%s\nIOR:\t%s\n", outObject,
	       CORBA_ORB_object_to_readable_string(orb, outObject, &ev),
	       CORBA_ORB_object_to_string(orb, outObject, &ev));
        printf("Result:   %p\nURL:\t%s\nIOR:\t%s\n", retObject,
	       CORBA_ORB_object_to_readable_string(orb, retObject, &ev),
	       CORBA_ORB_object_to_string(orb, retObject, &ev));
	
	for (case_num = -1; case_num <= 5; case_num++) {
		ev._major = CORBA_NO_EXCEPTION;
		printf("Testing test_throw Before Call:\n"); 	
		if (ev._major != CORBA_NO_EXCEPTION) 
			printf("Exception: `%s' EXCEPTION RAISED FOR throw!\n",
			       CORBA_exception_id(&ev));	       
		else
			printf("Exception: no\n");
		printf("In:       %d\n", case_num);
		printf("Calling Server...\n");
		test1_test_throw(obj, case_num, &ev);
		printf("After Call:\n");
		switch (ev._major) {
		case CORBA_USER_EXCEPTION:
			if (!strcmp(CORBA_exception_id(&ev), ex_test1_x1))
				printf("Caught exception test1_x1!  "
				       "Case_Num: %d\n",
				       ((test1_x1 *)
					CORBA_exception_value(&ev))
				       ->case_num);
			else if (!strcmp(CORBA_exception_id(&ev),
					 ex_test1_x2))
				printf("Caught exception test1_x2!  "
				       "Obj URL: %s,  value: %d\n",
				       CORBA_ORB_object_to_readable_string(
					       orb,
					       ((test1_x2 *)
						CORBA_exception_value(&ev))->
					       obj,
					       &ev),
				       ((test1_x2 *)
					CORBA_exception_value(&ev))->value);
			else
				printf("Unknown User Exception Raised: `%s' "
				       "for throw!\n",
				       CORBA_exception_id(&ev));
			break;
		case CORBA_SYSTEM_EXCEPTION:
			printf("Exception: `%s' EXCEPTION RAISED FOR throw!\n",
			       CORBA_exception_id(&ev));
			break;
		default:
			printf("Exception: no\n");
			printf("In:       %d\n", case_num);
		}
	}
	
	printf("About to tell the server to exit...\n");
	test1_please_exit(obj, &ev);
	printf("Server should be exiting any moment...\n");
	return 0;
}

/* End of file. */

