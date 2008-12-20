/*
 * Copyright (c) 1995, 1996, 1997, 1998, 1999 The University of Utah and
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sysent.h>
#include <mach_init.h>
#include <mach/message.h>
#include <mach/port.h>
#include <mach/mach_port.h>
#include <mach/mig_errors.h>
#include <mach/mach_traps.h>
#include <machine/endian.h>
#include <memory.h>
#include <servers/netname.h>
#include <flick/link/mach3mig.h>

/* These need to be success/failure functions
   that call the respective client/server stubs
   */

int call_server(mach_msg_header_t *request_ptr, mach_msg_header_t *reply_ptr);
int call_client(mach_port_t right);

#define SERVER_NAME "abadcafe"
#define SERVER_RIGHT MACH_MSG_TYPE_COPY_SEND

/* The time array gets passes in timing messages and gets filled in
   at various points along the way.  */
typedef int time_array_t[32];
struct timeval tv = {1, 0};

#define RPC_IN 0
#define CALL_USER_STUB_ENTER 1
#define CALL_USER_STUB_SEND 2
#define CALL_KERN_TRAP_ENTER 3
#define CALL_KERN_TRAP_COPYIN_START 4
#define CALL_KERN_TRAP_COPYIN_END 5
#define CALL_KERN_TRAP_A 6
#define CALL_KERN_TRAP_B 7
#define CALL_KERN_TRAP_C 8
#define CALL_KERN_TRAP_D 9
#define CALL_KERN_TRAP_COPYOUT 10
#define CALL_KERN_TRAP_PUT 11
#define CALL_KERN_TRAP_LEAVE 12
#define CALL_SERVER_RECEIVE 13
#define CALL_SERVER_STUB_ENTER 14
#define CALL_SERVER_STUB_LEAVE 15
#define REPLY_SERVER_STUB_ENTER 16
#define REPLY_SERVER_STUB_LEAVE 17
#define REPLY_SERVER_SEND 18
/* Same as above */
#define REPLY_USER_STUB_RECEIVE 29
#define REPLY_USER_STUB_LEAVE 30
#define RPC_OUT 31

void doserver(mach_port_t right,
	      mach_port_t task_self,
	      mach_msg_size_t max_size);

void doclient(mach_port_t prent_right,
	      mach_port_t parent_task,
	      int server_pid);

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

int
main(int argc, char *argv[])
{
	int child_pid, i;
	mach_port_t task_self = mach_task_self();
	mach_port_t right;
	mach_msg_size_t max_size;
	int childserver;
	
	/* Check for command line options. */
	max_size = MAX_REPLY_BUFFER_SIZE;
	childserver = 0;
	for (i = 1; i < argc; i++) {
		switch (argv[i][0]) {
		case '-':
			/* run server as child (instead of parent) */
			if (strcmp(argv[i], "-cs") == 0) {
				childserver = 1;
				break;
			}
			/* buffer size */
			if (strcmp(argv[i], "-max") == 0
			    && ++i < argc) {
				max_size = atoi(argv[i]);
				break;
			}
		default:
			fprintf(stderr,
				("usage: %s [-cs] [-max n] [-h]\n"
				 "  -cs    - Child runs server.\n"
				 "  -max n - Max reply buffer size = n.\n"
				 "  -h     - This help.\n"),
				argv[0]);
			exit(0);
		}
	}
	
	/* Create a receive right to receive requests from the client */
	diemach(mach_port_allocate,
		(task_self, MACH_PORT_RIGHT_RECEIVE, &right));

	/* Fork into two processes */
	child_pid = fork ();
	if (child_pid == -1)
		die ("can't fork", errno);
	if (child_pid != 0) {
		/* parent */
		if (childserver) {
			printf("Parent executing client.\n");
			doclient(right, task_self, child_pid);
		} else {
			printf("Parent executing server.\n");
			doserver(right, task_self, max_size);
		}
	} else {
		/* child */
		if (childserver) {
			printf("Child executing server.\n");
			doserver(right, task_self, max_size);
		} else {
			printf("Child executing client.\n");
			doclient(right, task_self, getppid());
		}
	}
	return 0;
}

void
doserver(mach_port_t right, mach_port_t task_self, mach_msg_size_t max_size)
{
	int ret;
	
	/* Create a receive right to receive requests from the client */
	diemach(mach_port_allocate,(task_self, MACH_PORT_RIGHT_RECEIVE,
				    &right));
	
	/* Advertise our server name so clients can connect. */
	printf("Setting up service as: %s\n", SERVER_NAME);
	diemach(netname_check_in,(name_server_port, SERVER_NAME,
				  task_self, right));
	
	printf("Maximum reply buffer size is set as: %d\n", max_size);
	
	/* use mach_msg_server as our server handling loop */
	if ((ret = mach_msg_server(dispatch, max_size, right,
				   MACH_MSG_OPTION_NONE)) != KERN_SUCCESS) {
		die ("mach_msg_server loop", ret);
	}
	
	printf ("\nServer terminating.\n");
	
	/* unregister with the name server */
	diemach(netname_check_out,(name_server_port, SERVER_NAME, task_self));
}


void
doclient(mach_port_t parent_right, mach_port_t parent_task, int server_pid)
{
	mach_port_t right;
	
	/* wait for server to get up and running */
	sleep(1);
	
	printf("Looking up server...\n");
	if (netname_look_up(name_server_port, NULL, SERVER_NAME, &right)) {
		fprintf(stderr, "Couldn't find server!\n");
		return;
	}
	
	printf("Beginning...\n");
	
	call_client(right);
	
	printf("\nClient Terminated.\n");
	
	/* wait to see if parent dies */
	sleep(1);
	printf("Killing Server ID %d.\n", server_pid);
	kill(server_pid, 9);
}
