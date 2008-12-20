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

/*
 * This file contains a set of tests using the functions defined in
 * mig-types.defs.  
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <mach/message.h>
#include <mach/port.h>
#include <mach/mig_errors.h>
#include <memory.h>
#include <mach_error.h>

#include "mig-types-client.h"

/* macro for calling a client stub */
#define _try(_func, _params) {\
	int _result;\
	printf("Calling function %s...\n", #_func);\
	if ((_result = _func _params) != KERN_SUCCESS) {\
		fprintf(stderr, "%s%s failed, result = %d (0x%08x) [%s]\n",\
			#_func, #_params, _result, _result,\
			mach_error_string(_result));\
	}\
}

/* function to print an array of integers */
void parr(int *a, int size)
{
	int i;
	printf("{ ");
	for (i=0; i<size; i++) {
		if (i) printf(", ");
		printf("%d",a[i]);
	}
	printf(" }\n");
}

/*
 * This is the main body of the client-side tests.
 */
int mig_types_client(mach_port_t right)
{
	charA my_charA;
	byteA my_byteA;
	boolA my_boolA;
	shortshortA my_short8A;
	shortA my_short16A;
	intA my_intA;
	longA my_longA;
	signedA my_signedA;
	unsignedA my_unsignedA;
	floatA my_floatA;
	doubleA my_doubleA;
	
	charA _charA;
	byteA _byteA;
	boolA _boolA;
	shortshortA _short8A;
	shortA _short16A;
	intA _intA;
	longA _longA;
	signedA _signedA;
	unsignedA _unsignedA;
	floatA _floatA;
	doubleA _doubleA;
	int i;
	
	int err = 0;
	
	for (i=0; i<9; i++) {
		my_charA[i] = (char) rand();
		my_byteA[i] = (char) rand();
		my_boolA[i] = (int) rand();
		my_short8A[i] = (char) rand();
		my_short16A[i] = (short) rand();
		my_intA[i] = (int) rand();
		my_longA[i] = (long) rand();
		my_signedA[i] = (signed) rand();
		my_unsignedA[i] = (unsigned) rand();
		*(int *)(&my_floatA[i]) = rand();
		((int *)(&my_doubleA[i]))[0] = rand();
		((int *)(&my_doubleA[i]))[1] = rand();
		
		_charA[i] = ~my_charA[i];
		_byteA[i] = ~my_byteA[i];
		_boolA[i] = ~my_boolA[i];
		_short8A[i] = ~my_short8A[i];
		_short16A[i] = ~my_short16A[i];
		_intA[i] = ~my_intA[i];
		_longA[i] = ~my_longA[i];
		_signedA[i] = ~my_signedA[i];
		_unsignedA[i] = ~my_unsignedA[i];
		*(int *)(&_floatA[i]) = ~*(int *)(&my_floatA[i]);
		((int *)(&my_doubleA[i]))[0] = ~((int *)(&my_doubleA[i]))[0];
		((int *)(&my_doubleA[i]))[1] = ~((int *)(&my_doubleA[i]))[1];
		
		snd(right,
		    my_charA[i],
		    my_byteA[i],
		    my_boolA[i],
		    my_short8A[i],
		    my_short16A[i],
		    my_intA[i],
		    my_longA[i],
		    my_signedA[i],
		    my_unsignedA[i],
		    my_floatA[i],
		    my_doubleA[i]);
		
		rcv(right,
		    &(_charA[i]),
		    &(_byteA[i]),
		    &(_boolA[i]),
		    &(_short8A[i]),
		    &(_short16A[i]),
		    &(_intA[i]),
		    &(_longA[i]),
		    &(_signedA[i]),
		    &(_unsignedA[i]),
		    &(_floatA[i]),
		    &(_doubleA[i]));
		
		if (my_charA[i] != _charA[i]) {
			err++;
			fprintf(stderr, "FAILURE: char '%c' != '%c'\n",
				my_charA[i], _charA[i]);
		}
		if (my_byteA[i] != _byteA[i]) {
			err++;
			fprintf(stderr, "FAILURE: byte '%d' != '%d'\n",
				(int)my_byteA[i], (int)_byteA[i]);
		}
		if (my_boolA[i] != _boolA[i]) {
			err++;
			fprintf(stderr, "FAILURE: bool '%d' != '%d'\n",
				my_boolA[i], _boolA[i]);
		}
		if (my_short8A[i] != _short8A[i]) {
			err++;
			fprintf(stderr, "FAILURE: short8 '%d' != '%d'\n",
				(int)my_short8A[i], (int)_short8A[i]);
		}
		if (my_short16A[i] != _short16A[i]) {
			err++;
			fprintf(stderr, "FAILURE: short16 '%d' != '%d'\n",
				(int)my_short16A[i], (int)_short16A[i]);
		}
		if (my_intA[i] != _intA[i]) {
			err++;
			fprintf(stderr, "FAILURE: int '%d' != '%d'\n",
				my_intA[i], _intA[i]);
		}
		if (my_longA[i] != _longA[i]) {
			err++;
			fprintf(stderr, "FAILURE: long '%ld' != '%ld'\n",
				my_longA[i], _longA[i]);
		}
		if (my_signedA[i] != _signedA[i]) {
			err++;
			fprintf(stderr, "FAILURE: signed '%d' != '%d'\n",
				my_signedA[i], _signedA[i]);
		}
		if (my_unsignedA[i] != _unsignedA[i]) {
			err++;
			fprintf(stderr, "FAILURE: unsigned '%u' != '%u'\n",
				my_unsignedA[i], _unsignedA[i]);
		}
		if (my_floatA[i] != _floatA[i]) {
			err++;
			fprintf(stderr, "FAILURE: float '%g' != '%g'\n",
				(double)my_floatA[i], (double)_floatA[i]);
		}
		if (my_doubleA[i] != _doubleA[i]) {
			err++;
			fprintf(stderr, "FAILURE: double '%g' != '%g'\n",
				my_doubleA[i], _doubleA[i]);
		}
	}
	
	for (i=0; i<9; i++) {
		my_charA[i] = (char) rand();
		my_byteA[i] = (char) rand();
		my_boolA[i] = (int) rand();
		my_short8A[i] = (char) rand();
		my_short16A[i] = (short) rand();
		my_intA[i] = (int) rand();
		my_longA[i] = (long) rand();
		my_signedA[i] = (signed) rand();
		my_unsignedA[i] = (unsigned) rand();
		*(int *)(&my_floatA[i]) = rand();
		((int *)(&my_doubleA[i]))[0] = rand();
		((int *)(&my_doubleA[i]))[1] = rand();

		_charA[i] = ~my_charA[i];
		_byteA[i] = ~my_byteA[i];
		_boolA[i] = ~my_boolA[i];
		_short8A[i] = ~my_short8A[i];
		_short16A[i] = ~my_short16A[i];
		_intA[i] = ~my_intA[i];
		_longA[i] = ~my_longA[i];
		_signedA[i] = ~my_signedA[i];
		_unsignedA[i] = ~my_unsignedA[i];
		*(int *)(&_floatA[i]) = ~*(int *)(&my_floatA[i]);
		((int *)(&my_doubleA[i]))[0] = ~((int *)(&my_doubleA[i]))[0];
		((int *)(&my_doubleA[i]))[1] = ~((int *)(&my_doubleA[i]))[1];
	}
	
	sndArr(right,
	       my_charA,
	       my_byteA,
	       my_boolA,
	       my_short8A,
	       my_short16A,
	       my_intA,
	       my_longA,
	       my_signedA,
	       my_unsignedA,
	       my_floatA,
	       my_doubleA);
	
	rcvArr(right,
	       _charA,
	       _byteA,
	       _boolA,
	       _short8A,
	       _short16A,
	       _intA,
	       _longA,
	       _signedA,
	       _unsignedA,
	       _floatA,
	       _doubleA);
	
	for (i=0; i<9; i++) {
		if (my_charA[i] != _charA[i]) {
			err++;
			fprintf(stderr,
				"ARRAY FAILURE: char[%d] '%c' != '%c'\n",
				i, my_charA[i], _charA[i]);
		}
		if (my_byteA[i] != _byteA[i]) {
			err++;
			fprintf(stderr,
				"ARRAY FAILURE: byte[%d] '%d' != '%d'\n",
				i, (int)my_byteA[i], (int)_byteA[i]);
		}
		if (my_boolA[i] != _boolA[i]) {
			err++;
			fprintf(stderr,
				"ARRAY FAILURE: bool[%d] '%d' != '%d'\n",
				i, my_boolA[i], _boolA[i]);
		}
		if (my_short8A[i] != _short8A[i]) {
			err++;
			fprintf(stderr,
				"ARRAY FAILURE: short8[%d] '%d' != '%d'\n",
				i, (int)my_short8A[i], (int)_short8A[i]);
		}
		if (my_short16A[i] != _short16A[i]) {
			err++;
			fprintf(stderr,
				"ARRAY FAILURE: short16[%d] '%d' != '%d'\n",
				i, (int)my_short16A[i], (int)_short16A[i]);
		}
		if (my_intA[i] != _intA[i]) {
			err++;
			fprintf(stderr,
				"ARRAY FAILURE: int[%d] '%d' != '%d'\n",
				i, my_intA[i], _intA[i]);
		}
		if (my_longA[i] != _longA[i]) {
			err++;
			fprintf(stderr,
				"ARRAY FAILURE: long[%d] '%ld' != '%ld'\n",
				i, my_longA[i], _longA[i]);
		}
		if (my_signedA[i] != _signedA[i]) {
			err++;
			fprintf(stderr,
				"ARRAY FAILURE: signed[%d] '%d' != '%d'\n",
				i, my_signedA[i], _signedA[i]);
		}
		if (my_unsignedA[i] != _unsignedA[i]) {
			err++;
			fprintf(stderr,
				"ARRAY FAILURE: unsigned[%d] '%u' != '%u'\n",
				i, my_unsignedA[i], _unsignedA[i]);
		}
		if (my_floatA[i] != _floatA[i]) {
			err++;
			fprintf(stderr,
				"ARRAY FAILURE: float[%d] '%g' != '%g'\n",
				i, (double)my_floatA[i], (double)_floatA[i]);
		}
		if (my_doubleA[i] != _doubleA[i]) {
			err++;
			fprintf(stderr,
				"ARRAY FAILURE: double[%d] '%g' != '%g'\n",
				i, my_doubleA[i], _doubleA[i]);
		}
	}
	
	return !err;
}

/* This is the client function called by the test client. */
int call_client(mach_port_t right) {
	printf("About to run client... \n");
	printf("Returning from client(%c)...\n",
	       (mig_types_client(right))?'T':'F');
	return 0;
}

/* End of file. */
