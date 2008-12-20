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

#define FLICK_PROTOCOL_TRAPEZE
#include "trapeze-link.h"

#include <ctype.h>
#include <stdlib.h>

/* I don't know what hid is for, but it is used for trapeze functions. zero?*/
#define HID 0

int flick_trapeze_server_free_recv_buf;

/* the free receive flick_payload for the server.
   This is for swapping payloads. */
void *flick_trapeze_free_server_recv_buf; 

static int client_payload_slot_used[flick_client_payloads]
        = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static short int client_payload_index = 0;

void *flick_trapeze_client_array__alloc() {
	for  (;
	      client_payload_slot_used[client_payload_index];
	      client_payload_index =
		      (client_payload_index + 1) % flick_client_payloads)
		;
	
	client_payload_slot_used[client_payload_index] = 1;
	return (void *)flick_payload_ublock(client_payload_index);
}

void flick_trapeze_client_array__free(void *payload_ptr) 
{
	client_payload_slot_used[((char *)payload_ptr
				  - (char *)flick_payload_ublock(0))
				/ flick_payload_size] = 0;
}
	
	
void *flick_trapeze_server_array__alloc() {
	static short int server_payload_nextfree = 0;
	
	/* just return the next one */
	server_payload_nextfree =
		(server_payload_nextfree + 1) % flick_server_payloads;
	return (void *)flick_payload_ublock(server_payload_nextfree);
}
 
/*
 * XXX --- `CORBA_BOA_bind' isn't defined by Section 8.2 of the CORBA 2.0
 * specification.
 *
 * This function is a stand-in for `CORBA_BOA_create', which is the specified
 * function for creating new `FLICK_TARGET's but which requires arguments that
 * we can't yet provide (because we don't have the Interface and Implementation
 * Repositories).
 */
FLICK_TARGET CORBA_BOA_bind(CORBA_BOA ths,
			    const char *type_id, CORBA_ReferenceData *key,
			    CORBA_Environment *ev)
{
#if 0
	FLICK_TARGET obj = t_calloc(FLICK_TARGET_STRUCT, 1);
	
	if (!obj)
		goto err1;
	obj->u.info.boa = ths;
	
	obj->u.info.key = *key;
	obj->u.info.key._buffer = t_malloc(char, key->_length);
	if (!obj->u.info.key._buffer)
		goto err2;
	obj->u.info.key._length = key->_length;
	obj->u.info.key._maximum = key->_maximum;
	memcpy(obj->u.info.key._buffer, key->_buffer, key->_length);
	
	obj->u.info.type_id_len = strlen(type_id);
	obj->u.info.type_id = t_malloc(char, (obj->u.info.type_id_len + 1));
	if (!obj->u.info.type_id)
		goto err3;
	strncpy((char *) obj->u.info.type_id, type_id, (obj->u.info.type_id_len + 1));
	
	if (connect(ths->socket_fd,
		    (struct sockaddr *) ths->orb->ipaddr,
		    sizeof(*ths->orb->ipaddr)))
		goto err4;
	
	CORBA_BOA_set_exception(ths, ev, CORBA_NO_EXCEPTION, 0, 0);
	return obj;
	
  err4:
	free((char *) obj->u.info.type_id);
  err3:
	free(obj->u.info.key._buffer);
  err2:
	free(obj);
  err1:
	flick_set_exception(ths, ev, ex_CORBA_NO_MEMORY,
			    0, CORBA_COMPLETED_NO);
	return 0;
#endif
	return 0;
}

CORBA_ReferenceData *CORBA_BOA_get_id(CORBA_BOA ths, FLICK_TARGET obj,
				      CORBA_Environment *ev)
{
	CORBA_ReferenceData *res = t_malloc(CORBA_ReferenceData, 1);
	
	if (!res) {
		flick_set_exception(ths, ev, ex_CORBA_NO_MEMORY,
				    0, CORBA_COMPLETED_NO);
		return 0;
	}
	*res = obj->u.info.key;
	return res;
}

FLICK_TARGET CORBA_BOA_create(CORBA_BOA ths, CORBA_ReferenceData *obj_key, 
			      const char *type_id, FLICK_SERVER impl,
			      CORBA_Environment *ev)
{
	FLICK_TARGET_STRUCT *new_refs;
	FLICK_TARGET_STRUCT *this_ref;
	
	/* Reallocate our BOA's vector of references. */
	new_refs = t_realloc(ths->refs,
			     FLICK_TARGET_STRUCT,
			     (ths->count_servers + 1));
	if (!new_refs) {
		flick_set_exception(ths, ev, ex_CORBA_NO_MEMORY,
				    0, CORBA_COMPLETED_NO);
		fprintf(stderr,
			"Error: can't realloc memory for CORBA_BOA object "
			"references.\n");
		return 0;
	}
	ths->refs = new_refs;
	ths->count_servers += 1;
	
	this_ref = &(ths->refs[ths->count_servers - 1]);
	
	this_ref->u.info.boa         = ths;
	this_ref->server_func = impl;
	this_ref->dest        = 2;  /* XXX - should not be hard-coded */
	this_ref->u.info.key         = ths->count_servers - 1;
	this_ref->u.info.type_id     = type_id;
	this_ref->u.info.type_id_len = strlen(type_id);
	*obj_key = this_ref->u.info.key;
	
	printf("Object %s with id %d is ready.\n",
	       this_ref->u.info.type_id, this_ref->u.info.key);
	
	CORBA_BOA_set_exception(ths, ev, CORBA_NO_EXCEPTION, 0, 0);
	return this_ref;
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
				 void *_buf_start,
				 CORBA_Environment *ev)
{
	char message_byte_order;
	int object_key;
	
	/*****/
	
	object_key = ((int *)_buf_start)[1];
	
	/* Pluck the byte order out of the header. */
	message_byte_order = ((char *)_buf_start)[0];
	
	if (flick_is_little_endian != message_byte_order)
		object_key = swap_long(object_key);
	
	if (object_key < ths->count_servers)
		return CORBA_Object_duplicate(&ths->refs[object_key], ev);
	else
		flick_set_exception(ths, ev, ex_CORBA_NO_IMPLEMENT,
				    0, CORBA_COMPLETED_NO);
	return 0;
}

void CORBA_BOA_impl_is_ready(CORBA_BOA ths, CORBA_Environment *ev)
{
	trapeze_mcp_init(1 /* yes, I'm the server */);
	
	/* Accept new clients, read client requests & send replies forever. */
	while (1) {
		FLICK_TARGET obj;
		FLICK_BUFFER msg;
		void *buffer;
		
		flick_trapeze_server_get_request(&msg);
		buffer = tpz_mtod(msg);
		
		obj = find_implementation(
			ths, buffer, ev);
		if (ev->_major != CORBA_NO_EXCEPTION) {
			/* Error finding object. */
			flick_trapeze_server_send_corba_exception(
				msg,
				CORBA_exception_id(ev));
			ev->_major = CORBA_NO_EXCEPTION;
		} else {
			switch ((*obj->server_func)(msg, buffer, obj)) {
			case FLICK_OPERATION_SUCCESS:
				/* end_encode already sent reply */
				break;
				
			case FLICK_OPERATION_SUCCESS_NOREPLY:
				break;
				
			default:
				/* FLICK_OPERATION_FAILURE */
				/*
				 * XXX --- The server dispatch function may
				 * have already encoded an exception
				 * `..._decode_error'.  We should send it, or
				 * the dispatch func should return FLICK_
				 * OPERATION_SUCCESS in that case.
				 */
				flick_trapeze_server_send_corba_exception(
					msg,
					ex_CORBA_NO_IMPLEMENT);
				break;
			}
		}
		if (!CORBA_Object_is_nil(obj, ev))
			CORBA_Object_release(obj, ev);
	}
	
	/* Should never be reached. */
	flick_set_exception(ths, ev, ex_CORBA_INTERNAL,
			    0, CORBA_COMPLETED_YES);
}

void trapeze_mcp_init(int is_server) 
{
	tpz_msgbuf_t msg;
	int i, ring_size;
	int num_alloc;
	
	/*
	 * Initialize `mcp_filename'.  This must be set before we call
	 * `tpz_init'.  See the Trapeze source file `tpz_init.c'.
	 */
	if (!mcp_filename) {
		char *mcpname = DEFAULT_MCP_FILENAME;
		char *tpzhome = getenv("TPZ_HOME");
		if (tpzhome) {
			mcp_filename = (char *) malloc(strlen(tpzhome) +
						       strlen(mcpname) +
						       2);
			strcpy(mcp_filename, tpzhome);
			if (mcp_filename[strlen(mcp_filename)-1] != '/')
				strcat(mcp_filename, "/");
			strcat(mcp_filename, mcpname);
		} else {
			mcp_filename = (char *) malloc(strlen(mcpname) + 1);
			strcpy(mcp_filename, mcpname);
		}
	}
	
	if (tpz_ready(HID)) return;
	
	/***
	 * 1.) Call tpz_init.  This loads and initializes mcp memory.
	 ***/
	tpz_init(HID);
	
	/***
	 * 2.) Stock the recv ring with unsolicited DMA data
	 * addresses note: flick_payload is host memory
	 * the card can use. while UBLOCK are user addresses. The
	 * copy block is 128k. Hence the i mod 8 (for half)
	 * (flick_payloads are attached to several messages at
	 * once). Page (PAGE_COUNT - num_alloc) is used as a swap
	 * buffer. The first (PAGE_COUNT-num_alloc-1) pages are
	 * used for send data.
	 ***/
	ring_size = tpz_rcv_ring_size();

	if (is_server)
		num_alloc = flick_server_payloads;
	else
		num_alloc = 1;
	
	for (i = 0; i < ring_size; i++){
		msg = tpz_prime_rcvmsg();
		if (((ring_entry*)msg)->dma_data_addr != NULL)
			printf("Expecting msg dma_data_addr "
			       "to be NULL! (it's not)\n");
		tpz_attach_rcvmsg(msg,
				  (vm_offset_t)
				  (flick_payload_dblock(i%num_alloc)));
		tpz_release_rcvmsg(msg);
	}
	
	if (is_server)
		flick_trapeze_server_free_recv_buf
			= i%num_alloc;
	else
		client_payload_slot_used[0] = 1;
		
	/***
	 * 3.) Enable the MCP.  This finishes the handshake with the
	 * control program enabling it to start sending and recieving
	 * packets.
	 ***/
	tpz_enable();       
}

/* End of file. */

