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

/* This stuff is used by Flick for transport independent
 * buffer management & message transmission.
 * This should NOT be used by user code - Flick's code is the only stuff
 * that should be using it.
 */

int
flick_server_get_request(FLICK_TARGET target,
			 FLICK_BUFFER *out_buf)
{
	return flick_read_buf(out_buf, target->socket_fd);
}

int
flick_server_send_reply(FLICK_TARGET target,
			FLICK_BUFFER *in_buf)
{
	return flick_write_buf(in_buf, target->socket_fd);
}

/* This one _can_ be used to register servers
   (if they want to build their own main function)
   */
typedef struct flick_server_list
{
	FLICK_SERVER_DESCRIPTOR server;
	FLICK_SERVER func;
	int sock;
	int listen;
	struct flick_server_list *next;
} flick_server_list;

static flick_server_list *lst = 0;

int
flick_server_register(FLICK_SERVER_DESCRIPTOR server, FLICK_SERVER func)
{
	flick_server_list *itm = (flick_server_list *)malloc(sizeof(flick_server_list));
	if (!itm)
		return 0;
	itm->next = lst;
	itm->server = server;
	itm->func = func;
	itm->listen = 1;
	itm->sock = 0;
	lst = itm;
	return 1;
}


static int
get_a_socket(void)
{
	int res = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	int socket_reuseaddr_p = 1;
	int val = SOCKET_BUF_SIZE;

	if (res < 0) {
		perror("TCP Socket creation problem");
		exit(1);
	}
	if (setsockopt(res,
		       SOL_SOCKET, SO_REUSEADDR,
		       ((void *) &socket_reuseaddr_p),
		       sizeof(socket_reuseaddr_p))
	    != 0) {
		perror("setsockopt");
		close(res);
		exit(1);
	}
	
#ifdef SO_SNDBUF
	printf("Server sendbuf: %d %d %d\n", setsockopt(res, SOL_SOCKET, SO_SNDBUF, ((void *) &val), sizeof(val)), errno, val);
#else
	printf("Server sendbuf: Using default settings\n");
#endif
#ifdef SO_RCVBUF
	printf("Server recvbuf: %d %d %d\n", setsockopt(res, SOL_SOCKET, SO_RCVBUF, ((void *) &val), sizeof(val)), errno, val);
#else
	printf("Server recvbuf: Using default settings\n");
#endif

	return res;
}

/* This one _can_ be used to begin grabbing messages
   (if they want to build their own main function)
   */
void
flick_server_run()
{
	int maxconn = -1;
	fd_set fds;
	flick_server_list *tmp = lst;
	struct sockaddr_in server;
	short port = -1;
	int sock;
	int len = sizeof(struct sockaddr_in);
	FLICK_BUFFER the_buf, return_buf;
	
	the_buf.real_buf_start = the_buf.buf_read = the_buf.buf_start =
	         the_buf.buf_current = (void *)calloc(8192, 1);
	
	the_buf.real_buf_end = the_buf.buf_end = (((char *) the_buf.buf_start)
						  + 8188);
	
	return_buf.real_buf_start = return_buf.real_buf_end =
	      return_buf.buf_read = 0;
	return_buf.buf_start  = return_buf.buf_current = (void *)calloc(8192, 1);
	return_buf.buf_end = ((char *) return_buf.buf_start) + 8188;
	
	if(!the_buf.buf_start || !return_buf.buf_start) {
		perror("Insufficient memory for server buffers");
		exit(1);
	}
	
	FD_ZERO(&fds);
	
	/* Get the listening socket & port for this machine */
	while(tmp) {
		
		/* Get a socket for this skeleton function */
		sock = get_a_socket();
		memset((char *)&server, 0, sizeof(server));
		server.sin_family = AF_INET;
		
		/* Get a port for this skeleton function */
		server.sin_port = 0;
		if (bind(sock, (struct sockaddr *) &server, len) != 0) {
			perror("cannot `bind' to socket");
			exit(1);
		}
		if ((getsockname(sock, (struct sockaddr *) &server, &len) != 0)
		    || (listen(sock, 8) != 0)) {
			perror("cannot `getsockname' or `listen'");
			exit(1);
		}
		
		tmp->sock = sock;
		port = ntohs(server.sin_port);
		
		/* Register the port with the RPC port mapper & initialize the fd_set */
		pmap_unset(tmp->server.prog_num, tmp->server.vers_num);
		pmap_set(tmp->server.prog_num, tmp->server.vers_num, IPPROTO_TCP, port);
		FD_SET(sock, &fds);
		
		if (sock > maxconn)
			maxconn = sock;
		
		tmp = tmp->next;
	}
	
	/* Accept new clients, read client requests & send replies forever... */
	while (1) {
		int qty = select(maxconn + 1, &fds, 0, 0, 0);
		flick_server_list *last = 0;
		
		tmp = lst;
		while (qty > 0) {
			if (FD_ISSET(tmp->sock, &fds)) {
				if (tmp->listen) {
					flick_server_list *new_list = (flick_server_list *)malloc(sizeof(flick_server_list));
					if (!new_list)
						exit(1);
					new_list->next = lst;
					lst = new_list;
					lst->sock = accept(tmp->sock, 0, 0);
					lst->func = tmp->func;
					lst->server = tmp->server;
					lst->listen = 0;
					if (lst->sock > maxconn)
						maxconn = lst->sock;
				} else do {
					FLICK_TARGET_STRUCT ts;
					ts.socket_fd = tmp->sock;
					ts.port = port;
					
					if (flick_server_get_request(&ts, &the_buf)) {
						switch ((*tmp->func)(&the_buf, &return_buf)) {
						case FLICK_OPERATION_SUCCESS:{
							flick_server_send_reply(&ts, &return_buf);
							break;
						}
						case FLICK_OPERATION_SUCCESS_NOREPLY:{
							break;
						}
						default /* FLICK_OPERATION_FAILURE */ :{
							/* the socket has been closed */
							if (last) {
								last->next = tmp->next;
								free(tmp);
								tmp = last;
							} else {
								lst = tmp->next;
								free(tmp);
								tmp = 0;
							}
							break;
						}}
					} else {
						/* the socket has been closed */
						if (last) {
							last->next = tmp->next;
							free(tmp);
							tmp = last;
						} else {
							lst = tmp->next;
							free(tmp);
							tmp = 0;
						}
					}
				} while (flick_buffer_contains_more(&the_buf));
				qty--;
			}
			last = tmp;
			if (tmp)
				tmp = tmp->next;
			else
				tmp = lst;
		}
		if (qty < 0)
			fprintf(stderr,"Error %d in server\n", errno);
		
		/* reset the file descriptor list */
		FD_ZERO(&fds);
		tmp = lst;
		while (tmp) {
			FD_SET(tmp->sock, &fds);
			tmp = tmp->next;
		}
	}
	exit(1);
}

/* End of file. */

