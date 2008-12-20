/*
 * Copyright (c) 1996, 1997, 1998, 1999 The University of Utah and
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

#define FLICK_PROTOCOL_TRAPEZE_ONC

#include "trapeze-link.h"
#include <flick/pres/sun.h>

/* This is the first attempt at a generic flick runtime.
   It's patterned after the Sun RPC transport, right now
   cuz that's what it's going to be used for
   */

/* This one must be used by the client to open a connection to the server */
int
flick_client_create(FLICK_TARGET target, FLICK_SERVER_LOCATION server)
{
#ifdef _POSIX_TIMERS
	struct timespec now;
#else
	struct timeval now;
#endif
	
	target->dest = atoi(strtok(server.server_name,"/"));
	target->host = 1;
	target->server_func = 0;
#ifdef _POSIX_TIMERS
	(void)clock_gettime(CLOCK_REALTIME, &now);
	target->u.header.xid = getpid() ^ now.tv_sec ^ now.tv_nsec;
#else	
	(void)gettimeofday(&now, (struct timezone *)0);
	target->u.header.xid = getpid() ^ now.tv_sec ^ now.tv_usec;
#endif
#if 0	/* These values are built into the generated stub code. */
	target->u.header.dir = CALL;
	target->u.header.rpcvers = 2;
#endif
	target->u.header.prog = server.prog_num;
	target->u.header.vers = server.vers_num;
	
	trapeze_mcp_init(0);
	
	return 1;
}

/* This one must be used by the client to close a connection to the server */
void
flick_client_destroy(FLICK_TARGET target)
{
	/* XXX --- Not yet implemented. */
}

/* End of file. */

