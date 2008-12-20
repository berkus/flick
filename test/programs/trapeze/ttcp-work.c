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
#include "ttcp-server.h"

ttcp obj;

void register_objects(CORBA_ORB orb, CORBA_BOA boa, int argc, char **argv,
		      CORBA_Environment *ev) 
{
	CORBA_ReferenceData key;
	
	CORBA_BOA_set_exception(boa, ev, CORBA_NO_EXCEPTION, 0, 0);
	/* CORBA_BOA_create must be called for each object instance.
	   Each time it is called a different key value will be returned.
	   Here there is just one instance. */
	obj = CORBA_BOA_create(boa, &key,
			       "ttcp",
			       &ttcp_server,
			       ev);
}

int ttcp_inttest(ttcp o, intBuffer *data, CORBA_Environment *_ev) 
{
	static int qty = 0;
	if (!(++qty & 127))
		printf("Integers still working...%d - %d\n",
		       qty, data->_length);
	return 0;
}

int ttcp_structtest(ttcp o, structBuffer *data, CORBA_Environment *_ev)
{
	static int qty = 0;
	if (!(++qty & 127))
		printf("Structures still working...%d - %d\n",
		       qty, data->_length);
	return 0;
}

/* End of file. */

