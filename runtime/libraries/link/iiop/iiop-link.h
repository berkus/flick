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

#ifndef _iiop_link_h
#define _iiop_link_h

/* will be set to 1 for little_endian, or 0 for big_endian */
extern char flick_is_little_endian;

/* initialized in `iiop_orb_ior.c' */
extern const unsigned int null_obj_ior[];
extern const unsigned int null_obj_ior_len;

#include <stdio.h>
#include <stdlib.h>
#include <flick/link/iiop.h>
#include <flick/pres/corba.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <sys/time.h>
#if HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#include <errno.h>
#include <netdb.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>

#ifndef PACKET_SIZE
#define PACKET_SIZE 65000
#endif

#ifndef SOCKET_BUF_SIZE
#define SOCKET_BUF_SIZE 65536
#endif

#ifndef CONNECT_RETRIES
#define CONNECT_RETRIES 5
#endif

#ifndef CONNECT_RETRY_DELAY
#define CONNECT_RETRY_DELAY 5
#endif

#define swap_long(val)				\
	((((val) & 0x000000FFU) << 24) |	\
	 (((val) & 0x0000FF00U) <<  8) |	\
	 (((val) & 0x00FF0000U) >>  8) |	\
	 (((val) & 0xFF000000U) >> 24))

#define swap_short(val) ((val) >> 8 | (val) << 8)

/*
 * This function converts an object into an IOR.  It is used by
 * `CORBA_ORB_object_to_string()' and `flick_cdr_encode_IOR_internal()'.
 * `buf' is a destination buffer which may be reallocated if necessary.
 * `buffer_len' is an inout parameter which is how much memory is
 * allocated for buf.
 * `start_pos' is an index into buf, which is where the ior will be placed.
 * The function returns the actual size of the ior in bytes or 0 for Error.
 */
unsigned int flick_cdr_make_ior(/*  in   */ FLICK_TARGET obj,
				/* inout */ char **buf,
				/* inout */ unsigned int *buffer_len,
				/*  in   */ unsigned int start_pos);


/* extracts the data from an IOR.  Returns the length of the data */
int flick_cdr_parse_IOR(char *buf, int cdr_swap /* 1 if swapping is active */,
			char **out_OAaddr,
			unsigned short *out_boa_port,
			char **out_obj_type_id,
			unsigned int *out_obj_type_id_len,
			CORBA_octet **out_obj_key,
			unsigned int *out_obj_key_len,
			CORBA_Environment *ev);


FLICK_TARGET flick_create_object(char *ORBid, char *OAid, char *OAaddr,
				 unsigned short boa_port,
				 char *obj_type_id,
				 unsigned int obj_type_id_len,
				 CORBA_octet *obj_key,
				 unsigned int obj_key_len,
				 CORBA_Environment *ev);

FLICK_TARGET find_implementation(CORBA_BOA ths,
				 FLICK_BUFFER *input_buffer,
				 /* OUT */ unsigned int *request_id,
				 CORBA_Environment *ev);

#endif /* _iiop_link_h */

/* End of file. */

