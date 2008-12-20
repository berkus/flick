/*
 * Copyright (c) 1996, 1997, 1998 The University of Utah and
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

#ifndef _suntcp_link_h
#define _suntcp_link_h

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <flick/link/suntcp.h>
#include <flick/pres/all.h>
#include <sys/socket.h>
#include <sys/errno.h>
#if HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#include <errno.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <rpc/pmap_clnt.h>
#include <unistd.h>
#include <time.h>

#ifndef CONNECT_RETRIES
#define CONNECT_RETRIES 5
#endif

#ifndef CONNECT_RETRY_DELAY
#define CONNECT_RETRY_DELAY 5
#endif

#endif /* _suntcp_link_h */

/* End of file. */

