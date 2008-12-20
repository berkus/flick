/*
 * Copyright (c) 1996 The University of Utah and
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

/*
 * this file is the beginning of a "support library" for runtime
 * support when porting Sun ONC apps to Mach (huch!)
 *
 * if you pass it -DPROGRAM_NAME="\name\", it will try to get the
 * server port under that name from the Mach name server
 *
 * when PROGRAM_NAME is not defined, the combination "PRGNUMBER/VERSION" 
 * will be used (my convention)
 */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <mach_init.h>
#include <mach/flick_mach3mig.h>
#include <mach/mig_errors.h>

void
die(char *mes, int code)
{
	printf("fatal error: %s (%d)\n", mes, code);
	printf("About to seg fault [weird - exit(1) causes a seg fault].\n");
	exit(1);
	printf("We haven't died!\n");
}

#define diemach(func, params) \
({ int err = func params; if (err) die (#func, err); })

mom_ref_t 
clnt_create(char *host, int prognum, int progversion, char *protocol)
{
    	mom_ref_t	pserver;
#ifndef PROGRAM_NAME
	char		buf[128];
	sprintf(buf, "%d/%d", prognum, progversion);
#define PROGRAM_NAME	buf
#else
#endif
  	diemach(netname_look_up, 
		(name_server_port, "", PROGRAM_NAME, &pserver));
	return	pserver;
}

