/*
 * Copyright (c) 1997, 1998, 1999 The University of Utah and
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
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <mach_init.h>
#include <mach/message.h>
#include <mach/port.h>
#include <mach/mach_port.h>
#include <mach/mig_errors.h>
#include <memory.h>
#include <servers/netname.h>
#include <flick/link/mach3mig.h>

/* defined in "-work" server program */
boolean_t call_server(mach_msg_header_t *request_ptr,
                      mach_msg_header_t *reply_ptr);

/*
 * Support functions
 */

void
die(char *mes, int code)
{
	fprintf(stderr, "fatal error: %s (%d, 0x%08x)\n", mes, code, code);
	exit(1);
	fprintf(stderr, "We haven't died!\n");
}

#define diemach(func, params) \
({ int err = func params; if (err) die (#func, err); })

__inline mach_msg_header_t *
dispatch(mach_msg_header_t *request_ptr, mach_msg_header_t *reply_ptr)
{
	if (!call_server(request_ptr, reply_ptr)) {
		fprintf(stderr, "%08x %08x %d\n",
			request_ptr->msgh_bits,
			request_ptr->msgh_size,
			request_ptr->msgh_id);
		fprintf(stderr, "Invalid message (id = %d) not deallocated.\n",
			request_ptr->msgh_id);
	}
	return reply_ptr;       
}

/*
 * Main program
 */

int
main(int argc, char *argv[])
{
	mach_port_t task_self = mach_task_self();
	mach_port_t right;
	mach_msg_size_t max_size;
	char *servername = 0;
	int ret, i;
	
	/* Check for command line options. */
	max_size = MAX_REPLY_BUFFER_SIZE;
	for (i = 1; i < argc; i++) {
		switch (argv[i][0]) {
		case '-':
			/* buffer size */
			if (strcmp(argv[i], "-max") == 0
			    && ++i < argc) {
				max_size = atoi(argv[i]);
				break;
			}
		  display_help:
			fprintf(stderr,
				("usage: %s [-max n] [-h] <name>\n"
				 "  -max n - Max reply buffer size = n.\n"
				 "  -h     - This help.\n"
				 "  <name> - Register server as <name>.\n"),
				argv[0]);
			exit(0);
		default:
			if (servername) goto display_help;
			servername = argv[i];
			break;
		}
	}
	if (!servername) goto display_help;
	
	/* Create a receive right to receive requests from the client */
	diemach(mach_port_allocate,(task_self, MACH_PORT_RIGHT_RECEIVE,
				    &right));

	/* Advertise our server name so clients can connect. */
	printf("Setting up service as: %s\n", servername);
	diemach(netname_check_in,(name_server_port, servername,
				  task_self, right));

	/* use mach_msg_server as our server handling loop */
	if ((ret = mach_msg_server(dispatch, max_size, right,
				   MACH_MSG_OPTION_NONE)) != KERN_SUCCESS) {
		die ("mach_msg_server loop", ret);
	}

	printf("\nServer terminating.\n");

	/* unregister with the name server */
	diemach(netname_check_out,(name_server_port, servername, task_self));
	return 0;
}

/* End of file. */
