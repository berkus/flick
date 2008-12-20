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
 * This file provides a simple server skeleton
 * All you need to do is to pass it via -Dcall_server=<function> the
 * server function and via -DPROGRAM_NAME=\"name\" the program name
 * under which to register
 *
 * it uses the Mach name service, needs to be linked with -lnetname
 */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <mach_init.h>
#include <mach/message.h>
#include <mach/port.h>
#include <mach/mig_errors.h>

/* this is the function called by the server skeleton */
#ifndef call_server
#error Please define the function 'call_server'
#endif
/* name under which we register */
#ifndef PROGRAM_NAME
#error Please define the program name as "prg/version"
#endif

/* These need to be success/failure functions
   that call the respective client/server stubs
   */
int call_server(mach_msg_header_t *request_ptr, mach_msg_header_t *reply_ptr);

#define MSGBUFSIZE (65*1024)

void serverloop(mach_port_t right, mach_port_t task_self);

void
die(char *mes, int code)
{
	mach_error(mes, code);
	printf("About to seg fault [weird - exit(1) causes a seg fault].\n");
	exit(1);
	printf("We haven't died!\n");
}

#define diemach(func, params) \
({ int err = func params; if (err) die (#func, err); })

__inline mach_msg_header_t *
dispatch(mach_msg_header_t *request_ptr, mach_msg_header_t *reply_ptr)
{
	if (!call_server(request_ptr, reply_ptr)) {
		printf("%08x %08x %d\n",
		       request_ptr->msgh_bits,
		       request_ptr->msgh_size,
		       request_ptr->msgh_id);
		printf("Invalid message (id = %d) not deallocated.\n",
		       request_ptr->msgh_id);
	}
	return reply_ptr;       
}

int
main(void)
{
	mach_port_t task_self = mach_task_self();
	mach_port_t right;

	/* Create a receive right to receive requests from the client */
	diemach(mach_port_allocate,
		(task_self, MACH_PORT_RIGHT_RECEIVE, &right));
	diemach(netname_check_in,
		(name_server_port, PROGRAM_NAME, task_self, right));
	serverloop(right, task_self);
	return 0;
}

typedef union {
	mach_msg_header_t	hdr;
	mig_reply_header_t	death_pill;
	char			space[MSGBUFSIZE];
} reply_message;

void
serverloop(mach_port_t right, mach_port_t task_self)
{
	int i;
	reply_message msg_buffer_1, msg_buffer_2;

	mach_msg_header_t * request_ptr;
	mig_reply_header_t * reply_ptr;
	mach_msg_header_t * tmp;

	int ret;
	int act_sp;
	mach_port_t act;
	int dp;
      
	request_ptr = &msg_buffer_1.hdr;
	reply_ptr = &msg_buffer_2.death_pill;
	/*
	printf("server buffers at %08x-%08x and %08x-%08x\n",
	       request_ptr, (long)request_ptr+MSGBUFSIZE,
	       reply_ptr, (long)reply_ptr+MSGBUFSIZE);
	*/

	while (1) {
		ret = mach_msg(request_ptr, MACH_RCV_MSG,
			       0, sizeof msg_buffer_1,
			       right,
			       MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
		
		if (ret != MACH_MSG_SUCCESS)
			die("ux_server_loop: receive", ret);
		while (ret == MACH_MSG_SUCCESS) {
			mig_reply_header_t *reply2 = reply_ptr;

			reply2->Head = *(mach_msg_header_t *)dispatch(
						request_ptr, &(reply2->Head));

			if (reply2->Head.msgh_remote_port == MACH_PORT_NULL) {
				/* no reply port, just get another request */
				break;
			}
	    
			if (reply2->RetCode == MIG_NO_REPLY) {
				/* deallocate reply port right */
				(void) mach_port_deallocate(mach_task_self(),
					    reply2->Head.msgh_remote_port);
				break;
			}

			/* Send reply to prev request and receive another: */
			ret = mach_msg(&reply2->Head,
				       MACH_SEND_MSG|MACH_RCV_MSG,
				       (reply2->Head.msgh_size + 3)&~3,
				       sizeof msg_buffer_2,
				       right,
				       MACH_MSG_TIMEOUT_NONE,
				       MACH_PORT_NULL);
			
			if (ret != MACH_MSG_SUCCESS) {
				if (ret == MACH_SEND_INVALID_DEST) {
				/* deallocate reply port right */
				/* XXX should destroy entire reply msg */
					(void) mach_port_deallocate(
						mach_task_self(),
						reply2->Head.msgh_remote_port);
				} else
					die ("ux_server_loop: rpc", ret);
			}
			
			tmp = request_ptr;
			request_ptr = (mach_msg_header_t *) reply2;
			if (reply_ptr != (mig_reply_header_t *) tmp)
				reply_ptr = (mig_reply_header_t *) tmp;
		}
	}
}

