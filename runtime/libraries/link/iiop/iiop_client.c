/*
 * Copyright (c) 1998, 1999 The University of Utah and
 * the Computer Systems Laboratory at the University of Utah (CSL).
 * All rights reserved.
 *
 * Permission to use, copy, modify and distribute this software is hereby
 * granted provided that (1) source code retains these copyright, permission,
 * and disclaimer notices, and (2) redistributions including binaries
 * reproduce the notices in supporting documentation.
 *
 * THE UNIVERSITY OF UTAH AND CSL ALLOW FREE USE OF THIS SOFTWARE IN ITS "AS
 * IS" CONDITION.  THE UNIVERSITY OF UTAH AND CSL DISCLAIM ANY LIABILITY OF
 * ANY KIND FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * CSL requests users of this software to return to csl-dist@cs.utah.edu any
 * improvements that they make and grant CSL redistribution rights.
 */

/*
 * This file contains glue for Sun presentation over IIOP.
 */

#include <flick/pres/sun.h>
#include "iiop-link.h"

/* This one must be used by the client to open a connection to the server. */
int
flick_client_create(FLICK_TARGET target, FLICK_SERVER_LOCATION server)
{
	CORBA_ORB orb = 0;
	/* FLICK_TARGET is a CORBA_Object */
	FLICK_TARGET obj;
	CORBA_Environment ev;
	
	obj = CORBA_ORB_string_to_object(orb, server.server_name, &ev);
	if (ev._major != CORBA_NO_EXCEPTION)
		return 0;
	
	*target = *obj;
	CORBA_free(obj);
	return 1;
}

/* This one must be used by the client to close a connection to the server. */
void
flick_client_destroy(FLICK_TARGET target)
{
	CORBA_free(/* discard const */ (char *) target->type_id);
	CORBA_free(target->key._buffer);
}

/* End of file. */

