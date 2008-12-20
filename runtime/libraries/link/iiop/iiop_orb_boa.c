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

#include "iiop-link.h"

#include <ctype.h>

CORBA_ReferenceData *CORBA_BOA_get_id(CORBA_BOA ths, FLICK_TARGET obj,
				      CORBA_Environment *ev)
{
	CORBA_ReferenceData *res;
	FLICK_TARGET dupe = CORBA_Object_duplicate(obj, ev);
	
	if (ev->_major != CORBA_NO_EXCEPTION)
		return 0;
	res = t_malloc(CORBA_ReferenceData, 1);
	if (!res) {
		free(dupe);
		flick_set_exception(ths, ev, ex_CORBA_NO_MEMORY,
				    0, CORBA_COMPLETED_NO);
		return 0;
	}
	*res = dupe->key;
	free(dupe);
	
	/* No need to set `ev'; `CORBA_Object_duplicate' did that for us. */
	return res;
}

/* used to be called CORBA_BOA_obj_is_ready, which was entirely wrong.
   see Section 8.2 of the CORBA 2.0 Spec.   
   This function will create an object of the given type and implementation. */
FLICK_TARGET CORBA_BOA_create(CORBA_BOA ths, CORBA_ReferenceData *obj_key, 
			      const char *type_id, FLICK_SERVER impl,
			      CORBA_Environment *ev)
{
	FLICK_TARGET_STRUCT *new_refs;
	FLICK_TARGET_STRUCT *obj_ref;
	
	CORBA_octet *ref_key_buffer;
	char *ref_type_id;
	
	int type_id_len;
	
	if (ths->max_name_len < obj_key->_length)
		ths->max_name_len = obj_key->_length;
	
	/* Allocate the memory we need for the new object reference. */
	type_id_len = strlen(type_id);
	
	ref_key_buffer = t_malloc(CORBA_octet, obj_key->_length);
	ref_type_id = t_malloc(char, (type_id_len + 1));
	
	if (!ref_key_buffer || !ref_type_id) {
		if (ref_key_buffer) free(ref_key_buffer);
		if (ref_type_id) free(ref_type_id);
		flick_set_exception(ths, ev, ex_CORBA_NO_MEMORY,
				    0, CORBA_COMPLETED_NO);
		fprintf(stderr,
			"Error: can't malloc memory for new object.\n");
		return 0;
	}
	
	/* Reallocate our BOA's vector of references. */
	new_refs = t_realloc(ths->refs,
			     FLICK_TARGET_STRUCT,
			     (ths->count_servers + 1));
	if (!new_refs) {
		free(ref_key_buffer);
		free(ref_type_id);
		flick_set_exception(ths, ev, ex_CORBA_NO_MEMORY,
				    0, CORBA_COMPLETED_NO);
		fprintf(stderr,
			"Error: can't realloc memory for CORBA_BOA object "
			"references.\n");
		return 0;
	}
	ths->refs = new_refs;
	ths->count_servers += 1;
	
	obj_ref = &(ths->refs[ths->count_servers - 1]);
	
	obj_ref->boa              = ths;	
	obj_ref->server_func = impl;
	memcpy(ref_key_buffer, obj_key->_buffer, obj_key->_length);
	obj_ref->key._buffer  = ref_key_buffer;
	obj_ref->key._length  = obj_key->_length;
	obj_ref->key._maximum = obj_key->_maximum;
	
	strncpy(ref_type_id, type_id, (type_id_len + 1));
	obj_ref->type_id     = ref_type_id;
	obj_ref->type_id_len = type_id_len;
	obj_ref->ior_len = 0;
	obj_ref->ior = 0;
	if (!flick_cdr_make_ior(obj_ref, &obj_ref->ior,
				&obj_ref->ior_len, 0)) {
		flick_set_exception(boa, ev, ex_CORBA_NO_MEMORY,
				    0, CORBA_COMPLETED_NO);
		fprintf(stderr,
			"Error: can't malloc memory for IOR.\n");
		return 0;
	}
	
	/* Finally, print the URL- and IOR-style stringified object refs. */
	{
		char *url, *ior;
		unsigned int i;
		
		url = CORBA_ORB_object_to_readable_string(ths->orb, obj_ref,
							  ev);
		if (ev->_major != CORBA_NO_EXCEPTION)
			return 0;
		ior = CORBA_ORB_object_to_string(ths->orb, obj_ref, ev);
		if (ev->_major != CORBA_NO_EXCEPTION)
			return 0;
		
		printf("Object `");
		for (i = 0; i < obj_key->_length; ++i) {
			if (isprint(obj_key->_buffer[i]))
				putchar(obj_key->_buffer[i]);
			else
				printf("\\%03o", obj_key->_buffer[i]);
		}
		printf("' is ready.\n");
		printf("  URL:\t%s\n  IOR:\t%s\n", url, ior);
		
		free(url);
		free(ior);
	}
	
	return CORBA_Object_duplicate(obj_ref, ev);
}

#define swap_long(val)				\
	((((val) & 0x000000FFU) << 24) |	\
	 (((val) & 0x0000FF00U) <<  8) |	\
	 (((val) & 0x00FF0000U) >>  8) |	\
	 (((val) & 0xFF000000U) >> 24))

/*
 * This is called from the server loop to (1) find the target object of a
 * message, (2) extract the `GIOP::RequestHeader.request_id' field of the
 * request, and (3) set the message stream pointer to point to the operation
 * key in the `GIOP::RequestHeader'.
 */
FLICK_TARGET find_implementation(CORBA_BOA ths,
				 FLICK_BUFFER *input_buffer,
				 /* OUT */ unsigned int *request_id,
				 CORBA_Environment *ev)
{
	char *cursor;
	
	char message_byte_order;
	char *object_key;
	
	unsigned int service_context_seq_len;
	unsigned int object_key_len;
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
	
	/* Parse the `GIOP::RequestHeader.service_context' list. */
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
	/* Save the `GIOP::RequestHeader.request_id'. */
	*request_id = *(unsigned int *) cursor;
	if (flick_is_little_endian != message_byte_order)
		*request_id = swap_long(*request_id);
	cursor += 4;
	
	/* Skip over the `GIOP::RequestHeader.response_expected'. */
	cursor += 4;
	
	/* Get the `GIOP::RequestHeader.object_key'. */
	object_key_len = *(unsigned int *) cursor;
	if (flick_is_little_endian != message_byte_order)
		object_key_len = swap_long(object_key_len);
	cursor += 4;
	
	object_key = cursor;
	cursor += (object_key_len + 3) & ~3;
	
	if( (void *)cursor > input_buffer->buf_end )
		goto error;
	/*
	 * Now `cursor' is pointing to the `GIOP::RequestHeader.operation', and
	 * we can initailize the message stream pointer for the server dispatch
	 * function.
	 */
	input_buffer->buf_current = cursor;
	
	/*
	 * Locate the referenced object and return an appropriate reference.
	 *
	 * XXX --- We do a linear search through the list of objects, and
	 * return a duplicate reference (XXX --- Why duplicate?) to the located
	 * object.  This is painfully inefficient, but hey, it's runtime,
	 * right? :-\
	 */
	for (i = 0; i < (unsigned) ths->count_servers; ++i) {
		if ((ths->refs[i].key._length == object_key_len)
		    && (memcmp(object_key,
			       ths->refs[i].key._buffer,
			       object_key_len)
			== 0)
			)
			/* We've found the object reference. */
			return CORBA_Object_duplicate(&ths->refs[i], ev);
	}
	
	flick_set_exception(ths, ev, ex_CORBA_NO_IMPLEMENT,
			    0, CORBA_COMPLETED_NO);
	return 0;
  error:
	flick_set_exception(ths, ev, ex_CORBA_COMM_FAILURE,
			    0, CORBA_COMPLETED_NO);
	return 0;
}

void CORBA_BOA_impl_is_ready(CORBA_BOA ths, CORBA_Environment *ev)
{
	int maxconn = -1;
	fd_set fds;
	struct sockaddr_in server;
	int sock, i;
	int len = sizeof(struct sockaddr_in);
	FLICK_BUFFER the_buf, return_buf;
	int *clients = 0;
	int client_cur = 0;
	int client_max = 0;
	
	/* allocate our own buffers.  This is in preparation for being
           thread-safe. */
	the_buf.real_buf_start = the_buf.buf_read = the_buf.buf_start =
	         the_buf.buf_current = t_calloc(char, 8192);
	the_buf.real_buf_end = the_buf.buf_end = (((char *) the_buf.buf_start)
						  + 8192);
	
	return_buf.real_buf_start = return_buf.real_buf_end =
	      return_buf.buf_read = 0;
	return_buf.buf_start  = return_buf.buf_current = t_calloc(char, 8192);
	return_buf.buf_end = ((char *) return_buf.buf_start) + 8192;
	
	if(!the_buf.buf_start || !return_buf.buf_start) {
		flick_set_exception(ths, ev, ex_CORBA_NO_MEMORY,
				    0, CORBA_COMPLETED_NO);
		perror("Insufficient memory for server buffers"); 
		return;
	}
	
	FD_ZERO(&fds);
	
	/* Get the listening socket & port for this machine */
	sock = ths->socket_fd;
	memset((char *)&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	
	/* Bind a port for this skelecton function */
	server.sin_port = htons(ths->hostport);
	if (bind(sock, (struct sockaddr *)&server, len) != 0) {
		perror("cannot `bind' to socket");
		flick_set_exception(ths, ev, ex_CORBA_COMM_FAILURE,
				    0, CORBA_COMPLETED_NO);
		return;
	}
	
	if ((getsockname(sock, (struct sockaddr *)&server, &len) != 0)
	    || (listen(sock, 8) != 0)) {
		perror("cannot `getsockname' or `listen', port may be in use");
		flick_set_exception(ths, ev, ex_CORBA_COMM_FAILURE,
				    0, CORBA_COMPLETED_NO);
		return;
	}
	
	FD_SET(sock, &fds);
	
	if (sock > maxconn)
		maxconn = sock;
	
	/* Accept new clients, read client requests & send replies forever. */
	while (1) {
		FLICK_TARGET obj;
		unsigned int request_id;
		
		int qty = select(maxconn + 1, &fds, 0, 0, 0);
		
		for (i = 0; i < client_cur && qty; i++) {
			if ((clients[i] >= 0)
			    && FD_ISSET(clients[i], &fds)) {
				do {
					obj = 0;
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
						obj = find_implementation(
							ths, &the_buf,
							&request_id, ev);
						if (ev->_major
						    != CORBA_NO_EXCEPTION) {
							/* Error finding
							   object. */
							flick_server_send_exception(
								clients[i],
								request_id,
								CORBA_exception_id(ev));
							ev->_major = CORBA_NO_EXCEPTION;
						} else {
							switch ((*obj->
								 server_func)(
									 &the_buf,
									 &return_buf,
									 request_id,
									 obj)) {
							case FLICK_OPERATION_SUCCESS:
								flick_server_send_reply(
									clients[i],
									&return_buf);
								break;
								
							case FLICK_OPERATION_SUCCESS_NOREPLY:
								break;
								
							default:
								/* FLICK_OPERATION_FAILURE */
								/*
								 * XXX --- The server
								 * dispatch function
								 * may have already
								 * encoded an exception
								 * `..._decode_error'.
								 * We should send it,
								 * or the dispatch func
								 * should return FLICK_
								 * OPERATION_SUCCESS in
								 * that case.
								 */
								flick_server_send_exception(
									clients[i],
									request_id,
									ex_CORBA_NO_IMPLEMENT);
								break;
							}
						}
					}
					if (!CORBA_Object_is_nil(obj, ev))
						CORBA_Object_release(obj, ev);
					
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
					clients = t_realloc(clients, int, client_max);
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
		/* reset the file descriptor list */
		FD_ZERO(&fds);
		FD_SET(sock, &fds);
		for (i = 0; i < client_cur; i++)
			if (clients[i] >= 0)
				FD_SET(clients[i], &fds);
	}
	
	/* Should never be reached. */
	flick_set_exception(ths, ev, ex_CORBA_INTERNAL,
			    0, CORBA_COMPLETED_NO);
}

/* End of file. */

