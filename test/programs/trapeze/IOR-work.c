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

#include <stdio.h>
#include "IOR-server.h"

IORtest_A A_ref = 0;
IORtest_B B_ref = 0;

void register_objects(CORBA_ORB orb, CORBA_BOA boa, int argc, char **argv,
		      CORBA_Environment *ev) 
{
	CORBA_ReferenceData key;
	CORBA_BOA_set_exception(boa, ev, CORBA_NO_EXCEPTION, 0, 0);
	/* CORBA_BOA_create must be called for each object instance.
	   Each time it is called a different key value will be returned. */
	A_ref = CORBA_BOA_create(boa, &key,
				 "IORtest_A",
				 &IORtest_A_server,
				 ev);
	B_ref = CORBA_BOA_create(boa, &key, 
				 "IORtest_B",
				 &IORtest_B_server,
				 ev);
	return;
}

IORtest_B IORtest_A_getB(IORtest_A _obj, CORBA_Environment *_ev) 
{
	return B_ref;
}

CORBA_long IORtest_B_getLong(IORtest_B _obj, CORBA_Environment *_ev) 
{
	return 12345;
}


/* End of file. */

