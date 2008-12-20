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

#include <stdio.h>
#include "oneway-server.h"

#define alloc_strcpy(dest, src, ev) {					\
	dest = (char *)CORBA_alloc(sizeof(char)*strlen(src)+1);		\
	if (!dest) {						       	\
		CORBA_NO_MEMORY *no_mem;				\
		no_mem = CORBA_NO_MEMORY__alloc();			\
		CORBA_BOA_set_exception(0 /*boa*/, ev,			\
					CORBA_SYSTEM_EXCEPTION,		\
					ex_CORBA_NO_MEMORY, no_mem);	\
	} else								\
		strcpy(dest, src);					\
}

void register_objects(CORBA_ORB orb, CORBA_BOA boa, int argc, char **argv,
		      CORBA_Environment *ev) 
{
	int i;
	CORBA_ReferenceData key;
	
	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-I") && (i + 1 < argc)) {
			++i;
			key._maximum = strlen(argv[i]);
			key._length  = key._maximum;
			key._buffer  = (CORBA_octet *) argv[i];
			CORBA_sequence_set_release(&key, 0);
			
			/* XXX should save returned object! */
			CORBA_BOA_create(boa,
					 &key,
					 "onewaytest",
					 &onewaytest_server,
					 ev);
			if (ev->_major != CORBA_NO_EXCEPTION)
				return;
		}
	}
	CORBA_BOA_set_exception(boa, ev, CORBA_NO_EXCEPTION, 0, 0);
}


CORBA_short onewaytest_shortlong(onewaytest _obj, CORBA_long *x, CORBA_Environment *_ev)
{
   printf("onewaytest_shortlong Received InOut %d.\n",*x);
   *x *= 3;
   return 3;
}

void onewaytest_voidlong(onewaytest _obj, CORBA_long x, CORBA_Environment *_ev)
{
	printf("onewaytest_voidlong Received In %d.\n",x);
}

void onewaytest_onewaylong(onewaytest _obj, CORBA_long x, CORBA_Environment *_ev)
{
	printf("onewaytest_onewaylong Oneway received In %d.\n",x);
}

void onewaytest_onewaystring(onewaytest _obj, CORBA_char *x, CORBA_Environment *_ev)
{
	printf("onewaytest_onewaystring Oneway string received: %s.\n",x);
}

CORBA_char *onewaytest_stringreturn(onewaytest _obj, CORBA_Environment *_ev)
{
	char *result;
	printf("onewaytest_stringreturn Sending Hello World.\n");

	alloc_strcpy(result, "Hello World", _ev);
	return result;
}

CORBA_long onewaytest_longstring(onewaytest _obj, CORBA_char **x, CORBA_Environment *_ev)
{
	printf("onewaytest_longstring Received InOut %s: Onewaytest",*x);
	CORBA_free(*x);
        alloc_strcpy(*x, "Fabulous!", _ev);
	return 21;
}

CORBA_char *onewaytest_stringlong(onewaytest _obj, CORBA_long *x, CORBA_Environment *_ev)
{
	char *result;
	*x = 957;
	printf("onewaytest_stringlong sending Out %d and Out 'My House Number'.\n",*x);
	alloc_strcpy(result, "My House Number", _ev);
	return result;
}

/* End of file. */

