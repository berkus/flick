/*
 * Copyright (c) 1997 The University of Utah and
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
#include "IOR-client.h"
#include <sys/time.h>

int main(int argc, char **argv)
{
	CORBA_ORB orb = 0;
	CORBA_Object A, B;
	CORBA_long x;
	CORBA_Environment _ev;
	_ev._major = CORBA_NO_EXCEPTION;
	
	A = CORBA_ORB_string_to_object(orb, argv[1], &_ev);
	if (_ev._major != CORBA_NO_EXCEPTION) {
		printf("Problem converting %s to an object...\n", argv[1]);
		exit(1);
	}
	printf("Beginning communication...\n");
	
	printf("Before Call...\n");
	if (_ev._major != CORBA_NO_EXCEPTION)
		printf("Exception: `%s' EXCEPTION RAISED!\n", 
		       CORBA_exception_id(&_ev));
	else							    
		printf("Exception: no\n");			    
	printf("B:        %p\n", B);	       
	printf("Calling Server IORtest_A_getB...\n");
	
	B = IORtest_A_getB(A, &_ev);
	printf("After Call:\n");
	if (_ev._major != CORBA_NO_EXCEPTION)
		printf("Exception: `%s' EXCEPTION RAISED!\n", 
		       CORBA_exception_id(&_ev));
	else							    
		printf("Exception: no\n");			    
	printf("B:        %p\n", B);	       

        if (B) {
	        printf("Before Call...\n");
		if (_ev._major != CORBA_NO_EXCEPTION)
			printf("Exception: `%s' EXCEPTION RAISED!\n", 
			       CORBA_exception_id(&_ev));
		else							    
			printf("Exception: no\n");			    
		printf("Result:   %d\n", x);
		printf("Calling Server IORtest_B_getLong...\n");
		
		x = IORtest_B_getLong(B, &_ev);
		printf("After Call:\n");
		if (_ev._major != CORBA_NO_EXCEPTION)
			printf("Exception: `%s' EXCEPTION RAISED!\n", 
			       CORBA_exception_id(&_ev));
		else							    
			printf("Exception: no\n");			    
		printf("Result:   %d\n", x);
	} else 
	        printf("Error: Null B object ref was returned!\n");
	return 0;
}

/* End of file. */

