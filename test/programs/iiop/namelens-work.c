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
#include "namelens-server.h"

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

void onewaytest_n(onewaytest _obj, CORBA_Environment *_ev)
{
	printf("onewaytest_n called\n");
}

void onewaytest_n2(onewaytest _obj, CORBA_Environment *_ev)
{
	printf("onewaytest_n2 called\n");
}

void onewaytest_n23(onewaytest _obj, CORBA_Environment *_ev)
{
	printf("onewaytest_n23 called\n");
}

void onewaytest_n234(onewaytest _obj, CORBA_Environment *_ev)
{
	printf("onewaytest_n234 called\n");
}

void onewaytest_n2345(onewaytest _obj, CORBA_Environment *_ev)
{
	printf("onewaytest_n2345 called\n");
}

void onewaytest_n23456(onewaytest _obj, CORBA_Environment *_ev)
{
	printf("onewaytest_n23456 called\n");
}

void onewaytest_n234567(onewaytest _obj, CORBA_Environment *_ev)
{
	printf("onewaytest_n234567 called\n");
}

void onewaytest_n2345678(onewaytest _obj, CORBA_Environment *_ev)
{
	printf("onewaytest_n2345678 called\n");
}

void onewaytest_n23456789(onewaytest _obj, CORBA_Environment *_ev)
{
	printf("onewaytest_n23456789 called\n");
}

void onewaytest_n234567890(onewaytest _obj, CORBA_Environment *_ev)
{
	printf("onewaytest_n234567890 called\n");
}

/* End of file. */

