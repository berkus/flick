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

#ifndef _flick_link_ORB_h
#define _flick_link_ORB_h

#include <flick/pres/corba.h>

/*
 * The following `typedef's are contained in <flick/pres/corba.h> because a
 * CORBA presentation includes these (opaque) types.  The actual structures are
 * defined below.
 *
 * typedef struct CORBA_ORB_type *CORBA_ORB;
 * typedef struct CORBA_BOA_type *CORBA_BOA;
 */

struct CORBA_ORB_type {
	char *ORBid;
	int OA_count;
	CORBA_BOA *boas;
};

struct CORBA_BOA_type {
        FLICK_BUFFER *in, *out;
        CORBA_ORB orb;
        struct sockaddr_in *ipaddr;
	char *hostname;
	unsigned short hostport;
	int socket_fd;
	int connected; /* 0 if the socket isn't connected already */
	char *OAid;

	/* This is a count of refs on a server,
	   on a client, it's the FLICK_TARGET ref count */
        int count_servers;
	/* This is for servers */
	FLICK_TARGET refs;

	/* This is only used from within the client object list above */
	unsigned int max_name_len;
};

/*
 * `IOP' stuff that we need in order to deal with CORBA Interoperable Object
 * References (IORs).  See Section 10.6.2 of the CORBA 2.0 specification.
 */

typedef CORBA_unsigned_long IOP_ProfileId;
#define IOP_TAG_INTERNET_IOP (0)
#define IOP_TAG_MULTIPLE_COMPONENTS (1)

/*
 * All of the ORB, BOA, and Object interface operations are now prototyped in
 * `flick/pres/corba.h', because (1) those interfaces are arguably part of the
 * CORBA ``presentation'' and (2) because those prototypes rely on other parts
 * of the CORBA presentation (e.g., the presented basic CORBA types).
 */

#endif /* _flick_link_ORB_h */

/* End of file. */

