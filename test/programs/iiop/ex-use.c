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
#include "ex-client.h"
#include <sys/time.h>

int main(int argc, char **argv)
{
	CORBA_ORB orb = 0;
	CORBA_Object obj;
	CORBA_Environment _ev;
	int i;
	
	orb = CORBA_ORB_init(&argc, argv, 0 /* use default ORBid */, &_ev);
	obj = CORBA_ORB_string_to_object(orb, argv[1], &_ev);
	if (_ev._major != CORBA_NO_EXCEPTION) {
		printf("Problem converting `%s' to an object.\n", argv[1]);
		CORBA_exception_free(&_ev);
		exit(1);
	}
	printf("Beginning communication...\n");
	
        for (i = 1; i <= 50000; i ++) {
		short val = i % 50;
		short result = A_op(obj, val, &_ev);
		switch (_ev._major) {
		case CORBA_NO_EXCEPTION:
			printf("Result: %d\n", (int) result);
			break;
			
		case CORBA_USER_EXCEPTION:
			printf("%s Exception Raised! ",
			       CORBA_exception_id(&_ev));
			printf("\ta Value: %d",
			       (int)(((A_B *)CORBA_exception_value(
				       &_ev))->a));
			printf("\td Value: %s\n",
			       ((A_B *)CORBA_exception_value(&_ev))->d);
			CORBA_exception_free(&_ev);
			break;
			
		case CORBA_SYSTEM_EXCEPTION:
			printf("%s System Exception!!!  "
			       "Minor: %d  Completion: %d\n",
			       CORBA_exception_id(&_ev),
			       ((CORBA_UNKNOWN *)
				CORBA_exception_value(&_ev))->minor, 
			       ((CORBA_UNKNOWN *)
				CORBA_exception_value(&_ev))->completed); 
			CORBA_exception_free(&_ev);
			break;
		}
        }
	
	return 0;
}

/* End of file. */

