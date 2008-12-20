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

/*
 * This file contains the server function definitions for mig-types.defs.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <mach_init.h>
#include <mach/message.h>
#include <mach/port.h>
#include <mach/mig_errors.h>
#include <memory.h>
#include <servers/netname.h>

#include "mig-types-server.h"


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

/*
 * Server functions
 */

kern_return_t s_snd(mach_port_t target,
		    char _char,
		    byte _byte,
		    bool _bool,
		    shortshort _short8,
		    short _short16,
		    int _int,
		    long int _long,
		    signed int _signed,
		    unsigned int _unsigned,
		    float _float,
		    double _double)
{
	my_charA[0] = _char;
	my_byteA[0] = _byte;
	my_boolA[0] = _bool;
	my_short8A[0] = _short8;
	my_short16A[0] = _short16;
	my_intA[0] = _int;
	my_longA[0] = _long;
	my_signedA[0] = _signed;
	my_unsignedA[0] = _unsigned;
	my_floatA[0] = _float;
	my_doubleA[0] = _double;
	return KERN_SUCCESS;
}

kern_return_t s_rcv(mach_port_t target,
		    char *_char,
		    byte *_byte,
		    bool *_bool,
		    shortshort *_short8,
		    short *_short16,
		    int *_int,
		    long int *_long,
		    signed int *_signed,
		    unsigned int *_unsigned,
		    float *_float,
		    double *_double)
{
	*_char = my_charA[0];
	*_byte = my_byteA[0];
	*_bool = my_boolA[0];
	*_short8 = my_short8A[0];
	*_short16 = my_short16A[0];
	*_int = my_intA[0];
	*_long = my_longA[0];
	*_signed = my_signedA[0];
	*_unsigned = my_unsignedA[0];
	*_float = my_floatA[0];
	*_double = my_doubleA[0];
	return KERN_SUCCESS;
}

kern_return_t s_sndArr(mach_port_t target,
		       charA _charA,
		       byteA _byteA,
		       boolA _boolA,
		       shortshortA _short8A,
		       shortA _short16A,
		       intA _intA,
		       longA _longA,
		       signedA _signedA,
		       unsignedA _unsignedA,
		       floatA _floatA,
		       doubleA _doubleA)
{
	int i;
	for (i=0; i<9; i++) {
		my_charA[i] = _charA[i];
		my_byteA[i] = _byteA[i];
		my_boolA[i] = _boolA[i];
		my_short8A[i] = _short8A[i];
		my_short16A[i] = _short16A[i];
		my_intA[i] = _intA[i];
		my_longA[i] = _longA[i];
		my_signedA[i] = _signedA[i];
		my_unsignedA[i] = _unsignedA[i];
		my_floatA[i] = _floatA[i];
		my_doubleA[i] = _doubleA[i];
	}
	return KERN_SUCCESS;
}

kern_return_t s_rcvArr(mach_port_t target,
		       charA _charA,
		       byteA _byteA,
		       boolA _boolA,
		       shortshortA _short8A,
		       shortA _short16A,
		       intA _intA,
		       longA _longA,
		       signedA _signedA,
		       unsignedA _unsignedA,
		       floatA _floatA,
		       doubleA _doubleA)
{
	int i;
	for (i=0; i<9; i++) {
		_charA[i] = my_charA[i];
		_byteA[i] = my_byteA[i];
		_boolA[i] = my_boolA[i];
		_short8A[i] = my_short8A[i];
		_short16A[i] = my_short16A[i];
		_intA[i] = my_intA[i];
		_longA[i] = my_longA[i];
		_signedA[i] = my_signedA[i];
		_unsignedA[i] = my_unsignedA[i];
		_floatA[i] = my_floatA[i];
		_doubleA[i] = my_doubleA[i];
	}
	return KERN_SUCCESS;
}


/* This is the server function called by the test server. */
boolean_t call_server(mach_msg_header_t *request_ptr,
                      mach_msg_header_t *reply_ptr)
{
	boolean_t ret;
#if 0
	printf("RPC-RCV:\n");
	print_message(request_ptr, request_ptr->msgh_size);
#endif
	ret = types_server(request_ptr, reply_ptr);
#if 0
	printf("RPC-SND:\n");
	print_message(reply_ptr, reply_ptr->msgh_size);
#endif
	printf("Server returned (%c)\n",(ret)?'T':'F');
	return ret;
}

/* End of file. */
