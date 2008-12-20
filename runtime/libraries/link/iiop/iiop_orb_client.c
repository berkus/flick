/*
 * Copyright (c) 1998, 1999 The University of Utah and
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

#include "iiop-link.h"
#include <flick/encode/cdr.h>
#include <ctype.h>

flick_client_table_entry *client_table = 0;
unsigned int client_table_entries = 0;
unsigned int max_client_table_entries = 0;

/* Create a client reference */
CORBA_Client CORBA_BOA_Client_create(CORBA_BOA ths,
				     /* unsigned client_id, */
				     void *client_data,
				     CORBA_Environment *ev)
{
	CORBA_Client cli;
	
	cli = t_calloc(FLICK_PSEUDO_CLIENT_STRUCT, 1);
	if (!cli)
		goto err;
	
	cli->boa = ths;
	/* cli->id = client_id; */
	cli->data = client_data;

	/* Clear the exception */
	CORBA_BOA_set_exception(ths, ev, CORBA_NO_EXCEPTION, 0, 0);
	return cli;
  err:
	flick_set_exception(ths, ev, ex_CORBA_NO_MEMORY,
			    0, CORBA_COMPLETED_NO);
	fprintf(stderr,
		"Error: can't malloc memory for new Client.\n");
	return 0;
}

/*
 * This is called from the client loop to (1) find the target object of a
 * message, (2) extract the `GIOP::RequestHeader.request_id' field of the
 * request, and (3) set the message stream pointer to point after the
 * `GIOP::ReplyHeader'.
 */
static CORBA_Client find_client(CORBA_BOA ths,
				FLICK_BUFFER *input_buffer,
				/* OUT */ CORBA_Invocation_id *request_id,
				/* OUT */ flick_client_work_func *func,
				/* OUT */ FLICK_TARGET *obj,
				CORBA_Environment *ev)
{
	char *cursor;
	
	char message_byte_order;
	
	unsigned int service_context_seq_len;
	unsigned int service_context_len;
	unsigned int i;
	
	/*****/
	
	/* Pluck the byte order out of the GIOP MessageHeader structure. */
	message_byte_order = ((char *) input_buffer->buf_start)[6];
	
	if ((((char *) input_buffer->buf_start) + 4 + 2 + 1 + 1 + 4 + 4)
	    > ((char *) input_buffer->buf_end))
		goto error;
	/* Skip past the `GIOP::MessageHeader' structure. */
	cursor = ((char *) input_buffer->buf_start)
		 + 4 /* GIOP::MessageHeader.magic */
		 + 2 /* GIOP::MessageHeader.GIOP_version */
		 + 1 /* GIOP::MessageHeader.byte_order */
		 + 1 /* GIOP::MessageHeader.message_type */
		 + 4 /* GIOP::MessageHeader.message_size */
		 ;
	
	/* Parse the `GIOP::ReplyHeader.service_context' list. */
	service_context_seq_len = *(unsigned int *) cursor;
	if (flick_is_little_endian != message_byte_order)
		service_context_seq_len = swap_long(service_context_seq_len);
	cursor += 4;
	
	for (i = 0; i < service_context_seq_len; ++i) {
		/* Skip the `IOP::ServiceContext.context_id'. */
		cursor += 4;
		
		if( (void *)(cursor + 4) > input_buffer->buf_end )
			goto error;
		/* Skip the `IOP::ServiceContext.context_data' sequence. */
		service_context_len = *(unsigned int *) cursor;
		if (flick_is_little_endian != message_byte_order)
			service_context_len = swap_long(service_context_len);
		cursor += 4;
		/*
		 * Note: It is always right to 4-byte align here, because the
		 * last service context will be followed by the unsigned int
		 * `GIOP::RequestHeader.request_id'.
		 */
		cursor += (service_context_len + 3) & ~3;
	}
	
	if( (void *)(cursor + 4 + 4 + 4) > input_buffer->buf_end )
		goto error;
	/* Save the `GIOP::ReplyHeader.request_id'. */
	*request_id = *(CORBA_Invocation_id *) cursor;
	if (flick_is_little_endian != message_byte_order)
		*request_id = swap_long(*request_id);
	cursor += 4;
	
	if( (void *)cursor > input_buffer->buf_end )
		goto error;
	/*
	 * Now `cursor' is pointing to the `GIOP::ReplyHeader.reply_status',
	 * and we can initailize the message stream pointer for the server
	 * dispatch function.
	 */
	input_buffer->buf_current = cursor;
	
	/*
	 * See if this client has an outstanding request for this reply.
	 *
	 * XXX --- We do a linear search through the list of request IDs.
	 * This is painfully inefficient, but hey, it's runtime, right? :-\
	 */
	for (i = 0; i < (unsigned) client_table_entries; i++) {
		if (client_table[i].inv_id == *request_id) {
			CORBA_Client cli;
			
			*func = client_table[i].func;
			*obj = client_table[i].obj;
			cli = client_table[i].client;
			
			/* remove this table entry */
			client_table[i] = client_table[--client_table_entries];
			
			return cli;
		}
	}
	
	flick_set_exception(ths, ev, ex_CORBA_NO_IMPLEMENT,
			    0, CORBA_COMPLETED_NO);
	return 0;
  error:
	flick_set_exception(ths, ev, ex_CORBA_COMM_FAILURE,
			    0, CORBA_COMPLETED_NO);
	return 0;
}

/*
 * Handle requests for the client.
 *
 * This function checks for incoming replies, and calls the dispatch
 * function to handle any that need servicing.
 *
 * If multiple messages are waiting, they will be received and handled.
 * This call will not block except to receive the remainder of an
 * incomplete message.
 */
void CORBA_BOA_handle_replies(CORBA_BOA ths,
			      int num_handle,
			      CORBA_Environment *ev)
{
	fd_set fds;
	static int maxconn = -1;
	static FLICK_BUFFER the_buf = {0, 0, 0, 0, 0, 0};
	static FLICK_BUFFER return_buf = {0, 0, 0, 0, 0, 0};
	static int *clients = 0;
	static int client_cur = 0;
	static int client_max = 0;
	int handled = 0;
	int sock = ths->socket_fd;
	
	/* allocate the buffers, if not done already */
	if (!the_buf.real_buf_start) {
		the_buf.real_buf_start
			= the_buf.buf_read
			= the_buf.buf_start
			= the_buf.buf_current
			= t_calloc(char, 8192);
		the_buf.real_buf_end
			= the_buf.buf_end
			= ((char *) the_buf.buf_start) + 8192;
		
		return_buf.real_buf_start
			= return_buf.real_buf_end
			= return_buf.buf_read = 0;
		return_buf.buf_start
			= return_buf.buf_current
			= t_calloc(char, 8192);
		return_buf.buf_end = ((char *) return_buf.buf_start) + 8192;
		
		if(!the_buf.buf_start || !return_buf.buf_start) {
			flick_set_exception(ths, ev, ex_CORBA_NO_MEMORY,
					    0, CORBA_COMPLETED_NO);
			perror("Insufficient memory for server buffers"); 
			return;
		}
	}
	
	/* Bind a port for this skeleton function */
	if (!ths->connected) {
		struct sockaddr_in server;
		int len = sizeof(struct sockaddr_in);
		
		if (bind(sock,
			 (struct sockaddr *)ths->ipaddr,
			 sizeof(*ths->ipaddr)) != 0) {
			perror("cannot `bind' to socket");
			flick_set_exception(ths, ev, ex_CORBA_COMM_FAILURE,
					    0, CORBA_COMPLETED_NO);
			return;
		}
		
		if ((getsockname(sock, (struct sockaddr *)&server, &len) != 0)
		    || (listen(sock, 8) != 0)) {
			perror("cannot `getsockname' or `listen', "
			       "port may be in use");
			flick_set_exception(ths, ev, ex_CORBA_COMM_FAILURE,
					    0, CORBA_COMPLETED_NO);
			return;
		}
		ths->connected = 1;
	}
	
	if (sock > maxconn)
		maxconn = sock;
	
	/* Accept new connections and read client replies. */
	do {
		static struct timeval nowait = { 0, 0 };
		CORBA_Client cli;
		flick_client_work_func func;
		flick_msg_struct_t msg;
		FLICK_TARGET obj;
		CORBA_Invocation_id request_id;
		int i;
		int qty;
		
		/* set the file descriptor list (every time) */
		FD_ZERO(&fds);
		FD_SET(sock, &fds);
		for (i = 0; i < client_cur; i++)
			if (clients[i] >= 0)
				FD_SET(clients[i], &fds);
		
		qty = select(maxconn + 1, &fds, 0, 0,
			     (num_handle ? 0 : &nowait));
		
		for (i = 0; i < client_cur && qty; i++) {
			if ((clients[i] >= 0)
			    && FD_ISSET(clients[i], &fds)) {
				do {
					request_id = 0;
					
					if (!flick_server_get_request(
						clients[i],
						&the_buf)) {
						/*
						 * Error receiving the mesg.
						 * Client FD is closed by
						 * `flick_server_get_request'.
						 */
						clients[i] = -1;
					} else {
						cli = find_client(ths,
								  &the_buf,
								  &request_id,
								  &func,
								  &obj,
								  ev);
						if (ev->_major
						    != CORBA_NO_EXCEPTION) {
							/* Error finding
							   client. */
							return;
						}
						msg.buf
							= msg.hdr
							= the_buf.buf_start;
						msg.msg = the_buf.buf_current;
						msg.msg_len
							= (unsigned int)
							((char *)
							 the_buf.buf_end
							 - (char *)
							 the_buf.buf_start);
						func(cli, &msg, request_id,
						     obj);
						handled++;
					}
#if 0
					if (!CORBA_Object_is_nil(obj, ev))
						CORBA_Object_release(obj, ev);
#endif
					
				} while (flick_buffer_contains_more(&the_buf));
				qty--;
			}
		}
		
		if (FD_ISSET(sock, &fds)) {
			int pos = -1;
			qty--;
			for (i = 0; i < client_cur; i++)
				if (clients[i] < 0)
					break;
			if (i < client_cur)
				pos = i;
			else {
				pos = client_cur++;
				if (client_cur > client_max) {
					client_max += 10;
					clients = t_realloc(clients, int,
							    client_max);
					if (!clients) {			
						flick_set_exception(
							ths, ev,
							ex_CORBA_NO_MEMORY,
							0, CORBA_COMPLETED_NO);
						return;
					}
				}
			}
			clients[pos] = accept(sock, 0, 0);
			if (clients[pos] > maxconn)
				maxconn = clients[pos];
		}
	} while (handled < num_handle);
	
	/* Successful completion (we serviced at least num_handle replies) */
	CORBA_BOA_set_exception(ths, ev, CORBA_NO_EXCEPTION, 0, 0);
}

/* End of file. */

