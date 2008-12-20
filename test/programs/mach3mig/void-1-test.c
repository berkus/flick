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

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <mach/message.h>
#include <mach/port.h>
#include <mach/mig_errors.h>

#define SERVER_FILE_NAME "void-1-server.c"
#define CLIENT_FILE_NAME "void-1-client.c"

/*****************************************************************************/

#define PROGRAM_NAME prog_2

#define op_2 prog_2_op_2
#include SERVER_FILE_NAME

void *op_2(void *arg, mom_ref_t o)
{
	static char ret;
	
	return (void *)(&ret);
}

#undef op_2

/***/

int call_server(mach_msg_header_t *request_ptr, mach_msg_header_t *reply_ptr)
{
	return PROGRAM_NAME(request_ptr, reply_ptr);
}

/*****************************************************************************/

#include CLIENT_FILE_NAME

int call_client(mach_port_t server)
{
	void *ret;
	
	ret = op_2(0, server);
	return 0;
}

/* End of file. */
