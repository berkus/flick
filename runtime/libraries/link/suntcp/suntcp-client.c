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

#include "suntcp-link.h"

/* This is the first attempt at a generic flick runtime.
   It's patterned after the Sun RPC transport, right now
   cuz that's what it's going to be used for
   */

/* This one must be used by the client to open a connection to the server */
int
flick_client_create(FLICK_TARGET target, FLICK_SERVER_LOCATION server)
{
	struct sockaddr_in addr;
	int retries = CONNECT_RETRIES; /* Set the number of
					  retries to be done */
	
#ifdef _POSIX_TIMERS
	struct timespec now;
#else
	struct timeval now;
#endif
	
	struct hostent *he = gethostbyname(server.server_name);
	if(!he)
		return 0;
	
	memset(&addr, 0, sizeof(addr));
	addr.sin_addr.s_addr = *(int *)*he->h_addr_list;
	addr.sin_family = AF_INET;
  	addr.sin_port = 0;
	
#ifdef _POSIX_TIMERS
	(void)clock_gettime(CLOCK_REALTIME, &now);
	target->header.xid = htonl(getpid() ^ now.tv_sec ^ now.tv_nsec);
#else	
	(void)gettimeofday(&now, (struct timezone *)0);
	target->header.xid = htonl(getpid() ^ now.tv_sec ^ now.tv_usec);
#endif
	target->header.dir = htonl(CALL);
	target->header.rpcvers = htonl(2);
	target->header.prog = htonl(server.prog_num);
	target->header.vers = htonl(server.vers_num);
	
	target->port = htons(pmap_getport(&addr,
					  server.prog_num,
					  server.vers_num,
					  IPPROTO_TCP));
	if (target->port == 0)
		return 0;
	
	addr.sin_port = target->port;
	target->socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (target->socket_fd < 0) {
		perror("socket");
		return 0;
	}
	
	{
		int val = SOCKET_BUF_SIZE;
		errno = 0;
#ifdef SO_SNDBUF
		printf("Target sendbuf: %d %d %d\n", setsockopt(target->socket_fd, SOL_SOCKET, SO_SNDBUF, ((void *) &val), sizeof(val)), errno, val);
#else
		printf("Target sendbuf: Using default settings\n");
#endif
#ifdef SO_RCVBUF
		printf("Target recvbuf: %d %d %d\n", setsockopt(target->socket_fd, SOL_SOCKET, SO_RCVBUF, ((void *) &val), sizeof(val)), errno, val);
#else
		printf("Target recvbuf: %Using default settings\n");
#endif
	}
	while (retries && (connect(target->socket_fd,
		    (struct sockaddr *) &addr, sizeof(addr)) < 0)) {
		switch( errno ) {
		case ECONNREFUSED:
		case ETIMEDOUT:
			perror( "connect" );
			fprintf( stderr, "Retrying...\n" );
				/* We have to close the socket and make
				   a new one in order to retry */
			close( target->socket_fd );
			target->socket_fd
				= socket(AF_INET,
					 SOCK_STREAM,
					 IPPROTO_TCP);
			if (target->socket_fd < 0) {
				perror("socket");
				return 0; /* return false */
			}
				/* Delay for some time */
			sleep( CONNECT_RETRY_DELAY );
			retries--;
			break;
				/* The rest are considered to be
				   unrecoverable errors so no retry */
		default:
			perror( "connect" );
			retries = 0;
			break;
		}
	}

	if( !retries ) {
		perror("connect");
		close(target->socket_fd);
		return 0;
	}
	
	/* Grab the buffers for this target */
	target->in = (FLICK_BUFFER *)malloc(sizeof(FLICK_BUFFER));
	target->out = (FLICK_BUFFER *)malloc(sizeof(FLICK_BUFFER));
	if (!(target->in && target->out))
		return 0;
	
	target->in->real_buf_start = target->in->buf_read
				   = target->in->real_buf_end = 0;
	target->in->buf_current = target->in->buf_start = (void *)calloc(8192, 1);
	target->in->buf_end = ((char *) target->in->buf_start) + 8188;
	target->out->buf_current = target->out->real_buf_start
				 = target->out->buf_read
				 = target->out->buf_start
				 = (void *)calloc(8192, 1);
	target->out->buf_end = target->out->real_buf_end
			     = ((char *) target->out->buf_start) + 8188;
	
	return target->in->buf_current && target->out->buf_current;
}

/* This one must be used by the client to close a connection to the server */
void
flick_client_destroy(FLICK_TARGET target)
{
	close(target->socket_fd);
	free(target->in->real_buf_start);
	free(target->out->buf_start);
	free(target->in);
	free(target->out);
}


int
flick_client_send_request(FLICK_TARGET target, FLICK_BUFFER *in_buf)
{
	return flick_write_buf(in_buf, target->socket_fd);
}

int
flick_client_get_reply(FLICK_TARGET target, FLICK_BUFFER *out_buf)
{
	return flick_read_buf(out_buf, target->socket_fd);
}

/* End of file. */

