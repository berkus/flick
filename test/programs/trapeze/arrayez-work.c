/*
 * Copyright (c) 1998 The University of Utah and
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
#include "arrayez-server.h"

arrayez obj;

void register_objects(CORBA_ORB orb, CORBA_BOA boa, int argc, char **argv,
		     CORBA_Environment *ev) 
{
	CORBA_ReferenceData key;
	
	CORBA_BOA_set_exception(boa, ev, CORBA_NO_EXCEPTION, 0, 0);
	/* CORBA_BOA_create must be called for each object instance.
	   Each time it is called a different key value will be returned.
	   Here there is just one instance. */
	obj = CORBA_BOA_create(boa, &key, 
			       "arrayez_server",
			       &arrayez_server,
			       ev);
}

CORBA_long arrayez_test_in8k(arrayez _obj, arrayez_arr8k a, CORBA_Environment *_ev) 
{
	int i;
	printf("checking in 8k...");
	for (i = 0; i < 2048; i++)
		if (a[i] != 1) {
			printf("failed at byte %d!\n", i*4);
			return 0;
		}
	printf("ok.\n");
	return 1;
}

CORBA_long arrayez_test_out8k(arrayez _obj, arrayez_arr8k a, CORBA_Environment *_ev)
{
	int i;
	printf("filling 8k...");
	for (i = 0; i < 2048; i++)
		a[i] = 1;
	printf("done.\n");
	return 1;
}

CORBA_long arrayez_test_io8k(arrayez _obj, arrayez_arr8k a, arrayez_arr8k b, CORBA_Environment *_ev)
{
	int i;
	printf("checking in 8k...");
	for (i = 0; i < 2048; i++)
		if (a[i] != 1) {
			printf("failed at byte %d!\n", i*4);
			return 0;
		}
	printf("ok.\n");
	printf("filling 8k with 2's...");
	for (i = 0; i < 2048; i++)
		b[i] = 2;
	printf("done.\n");
	return 1;
}
	
/* End of file. */

