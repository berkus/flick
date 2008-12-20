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

#include <stdio.h>
#include "ex-server.h"

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
			CORBA_BOA_create(b, &key, "A", &A_server, ev);
			if (ev->_major != CORBA_NO_EXCEPTION)
				return;
		}
	}
	CORBA_BOA_set_exception(boa, ev, CORBA_NO_EXCEPTION, 0, 0);
}

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

short int A_op(A o, short int b, CORBA_Environment *ev)
{
	static CORBA_short count = 0;
	if (b == 0) {
		A_B *B = CORBA_flick_alloc(sizeof(*B));
		B->a = count++;
		alloc_strcpy(B->d, "b was zero!", ev);
		CORBA_BOA_set_exception(boa, ev, CORBA_USER_EXCEPTION,
					ex_A_B, B);
		return 0;
	}
	
	return b;
}

/* End of file. */

