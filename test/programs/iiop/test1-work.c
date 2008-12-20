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
#include "test1-server.h"


CORBA_BOA boa = 0;

void register_objects(CORBA_ORB orb, CORBA_BOA b, int argc, char **argv,
		      CORBA_Environment *ev) 
{
	int i;
	CORBA_ReferenceData key;
	
	boa = b;
	
	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-I") && (i + 1 < argc)) {
			++i;
			key._maximum = strlen(argv[i]);
			key._length  = key._maximum;
			key._buffer  = (CORBA_octet *) argv[i];
			CORBA_sequence_set_release(&key, 0);
			
			/* XXX should save returned object! */
			CORBA_BOA_create(b, &key, "test1", &test1_server, ev);
			if (ev->_major != CORBA_NO_EXCEPTION)
				return;
		}
	}
	CORBA_BOA_set_exception(boa, ev, CORBA_NO_EXCEPTION, 0, 0);
}



#define MKTST(name, type)			\
type test1_test_##name(test1 _obj,		\
		       /* in */    type a1,	\
		       /* out */   type *a2,	\
		       /* inout */ type *a3,	\
		       CORBA_Environment *_ev)	\
{						\
	*a2 = *a3;				\
	*a3 = a1;				\
	a1 = *a2;				\
	return a1;				\
}

void test1_test_void(test1 _obj, CORBA_Environment *_ev)
{
	/* Do nothing. */
}

MKTST(short, CORBA_short)
MKTST(long, CORBA_long)
MKTST(ushort, test1_ushort)
MKTST(ulong, test1_ulong)
MKTST(float, CORBA_float)
MKTST(double, CORBA_double)
MKTST(boolean, CORBA_boolean)
MKTST(char, CORBA_char)
MKTST(octet, CORBA_octet)

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

CORBA_char *test1_test_string(test1 _obj,
			 /*in*/ CORBA_char *a1, /*out*/  CORBA_char **a2,
			      /*inout*/CORBA_char **a3, CORBA_Environment *_ev)
{
	*a2 = *a3;							
	alloc_strcpy(*a3, a1, _ev);
	alloc_strcpy(a1, "Groovy!", _ev);
	return a1;
}

CORBA_Object test1_test_Object(test1 _obj, CORBA_Object a1,
			       CORBA_Object *a2, CORBA_Object *a3,
			       CORBA_Environment *ev) 
{
	*a2 = CORBA_Object_duplicate(*a3, ev);	/* XXX --- Check for error. */
	CORBA_Object_release(*a3, ev);
	*a3 = CORBA_Object_duplicate(a1, ev);	/* XXX --- Check for error. */
	CORBA_Object_release(*a2, ev);
	*a2 = CORBA_Object_duplicate(*a3, ev);	/* XXX --- Check for error. */
	return CORBA_Object_duplicate(a1, ev);	/* XXX --- Check for error. */
}


/*
 * All cases, "case_num" in the exception is the same as the 'in' param
 *	* negative or zero, throws x1
 *	* positive even cases, throws x2 with obj = null objref
 *	* positive odd cases, throws x2 with obj = target objref
 */
void test1_test_throw(test1 _obj, CORBA_long case_num, CORBA_Environment *_ev)
{
	if (case_num <= 0) {
		test1_x1 *x1 = CORBA_flick_alloc(sizeof(*x1));
		x1->case_num = case_num;
		CORBA_BOA_set_exception(boa, _ev, CORBA_USER_EXCEPTION,
					ex_test1_x1, x1);
	} else {
		test1_x2 *x2 = CORBA_flick_alloc(sizeof(*x2));
		x2->value = case_num;
		if (case_num & 1) /* odds */
			x2->obj = _obj;
		else /* evens */
			x2->obj = 0; /* null */
		CORBA_BOA_set_exception(boa, _ev, CORBA_USER_EXCEPTION,
					ex_test1_x2, x2);
	}
}

void test1_please_exit(test1 _obj, CORBA_Environment *_ev) {
	printf("Server should shut down...\n");
        close(_obj->boa->socket_fd);
	exit(0);
}

/* End of file. */

