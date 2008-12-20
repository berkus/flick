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

#ifndef _trapeze_link_h
#define _trapeze_link_h

/* will be set to 1 for little_endian, or 0 for big_endian */
extern char flick_is_little_endian;

#include <flick/link/trapeze.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <flick/pres/all.h>
#if HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#include <string.h>

#define flick_payload_size (TRAPEZE_MAX_PAYLOAD_SIZE)
typedef char payload_t[flick_payload_size];
#define flick_payload_dblock(i) ((vm_offset_t)((int)(DBLOCK[0])		     \
					       + (flick_payload_size * (i))))
#define flick_payload_ublock(i) ((vm_offset_t)((int)(UBLOCK[0])		     \
					       + (flick_payload_size * (i))))
#define flick_client_payloads PAGE_COUNT
#define flick_server_payloads (PAGE_COUNT / 2)

#define swap_long(val)				\
	((((val) & 0x000000FFU) << 24) |	\
	 (((val) & 0x0000FF00U) <<  8) |	\
	 (((val) & 0x00FF0000U) >>  8) |	\
	 (((val) & 0xFF000000U) >> 24))

#define swap_short(val) ((val) >> 8 | (val) << 8)

FLICK_TARGET find_implementation(CORBA_BOA ths,
				 void *_buf_start,
				 CORBA_Environment *ev);

#endif /* _trapeze_link_h */

/* End of file. */

