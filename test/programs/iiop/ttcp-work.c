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
			CORBA_BOA_create(boa, &key, "ttcp", &ttcp_server, ev);
			if (ev->_major != CORBA_NO_EXCEPTION)
				return;
		}
	}
	CORBA_BOA_set_exception(boa, ev, CORBA_NO_EXCEPTION, 0, 0);
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

