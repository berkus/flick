/*
 * Copyright (c) 1999 The University of Utah and
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
#include "inherit-server.h"

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

CORBA_Object objects[5];

void register_objects(CORBA_ORB orb, CORBA_BOA boa, int argc, char **argv,
		      CORBA_Environment *ev) 
{
	int i;
	CORBA_ReferenceData key;
	static flick_dispatch_t *servers[5] = {
		one_server, two_server, three_server,
		four_server, five_server };
	static const char *typeids[5] = {
		"one", "two", "three", "four", "five" };
	
	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-I") && (i + 1 < argc)) {
			int x;
			++i;
			key._maximum = strlen(argv[i]) + 1;
			key._length  = key._maximum;
			key._buffer  = (CORBA_octet *)
				       CORBA_alloc(key._maximum);
			strncpy((char *)key._buffer, argv[i], key._length);
			CORBA_sequence_set_release(&key, 1);
			
			/* XXX should save returned object! */
			
			for (x = 0; x < 5; x++) {
				key._buffer[key._length - 1] = '1' + x;
				objects[x] = CORBA_BOA_create(boa,
							      &key,
							      typeids[x],
							      servers[x],
							      ev);
				if (ev->_major != CORBA_NO_EXCEPTION)
					return;
			}
			key._length--; /* chop off last char */
			CORBA_BOA_create(boa,
					 &key,
					 "factory",
					 factory_server,
					 ev);
			if (ev->_major != CORBA_NO_EXCEPTION)
				return;
		}
	}
	CORBA_BOA_set_exception(boa, ev, CORBA_NO_EXCEPTION, 0, 0);
}


#define deffun(obj, opname)					\
void obj##_op_##opname(obj _obj, CORBA_Environment *ev) {	\
	printf(#obj "::op_" #opname "\n");			\
}

deffun(one, one)
deffun(two, one)
deffun(two, two)
deffun(three, one)
deffun(three, three)
deffun(four, one)
deffun(four, two)
deffun(four, three)
deffun(four, four)
deffun(five, one)
deffun(five, two)
deffun(five, three)
deffun(five, four)
deffun(five, five)

#define factory(name, num)						\
CORBA_Object factory_get_##name(factory _obj, CORBA_Environment *ev) {	\
	return CORBA_Object_duplicate(objects[num], ev);		\
}

factory(one, 0)
factory(two, 1)
factory(three, 2)
factory(four, 3)
factory(five, 4)

/* End of file. */

