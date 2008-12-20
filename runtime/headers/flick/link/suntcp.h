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

#ifndef __flick_link_suntcp_h
#define __flick_link_suntcp_h

#include <flick/link/all.h>

/* The ONC link layer uses `realloc'; see below. */
#include <stdlib.h>

#define CLIENT __OLD_CLIENT
#include <rpc/rpc.h>
#undef CLIENT

/*
 * XXX --- Skip almost everything in this file if `RPCGEN' is defined.  This is
 * supposed to make it easier to write ONC/RPC-based programs that can use
 * either Flick-generated stubs or `rpcgen'-generated stubs.
 */
#ifndef RPCGEN

#define _buf_start	(_stream->buf_start)
#define _buf_current	(_stream->buf_current)
#define _buf_end	(_stream->buf_end)
#define _stub_state	(_stream->stub_state)

/*
 * `flick_msg_t' is a handle to a message under Flick's ``decomposed'' stub
 * presentation.  Although not currently used or supported in the ONC/TCP
 * runtime, this structure must be defined when including <flick/pres/corba.h>.
 */
typedef struct {
	int dummy; /* Unused, but ANSI C doesn't allow 0-slot structs. */
} *flick_msg_t;


/******************************************************************************
 ******************************************************************************
 ****
 **** Initialization and exit code.
 ****
 ******************************************************************************
 *****************************************************************************/

#define flick_suntcp_goto_message_body()			\
	_stream->buf_current = &((int *) _stream->buf_start)[6]

/*
 * Client-side macros.
 */

#define flick_suntcp_client_start_encode() {			\
	_stream = _obj->in;					\
	((int *) _stream->buf_start)[1] = _obj->header.xid;	\
	((int *) _stream->buf_start)[2] = _obj->header.dir;	\
	((int *) _stream->buf_start)[3] = _obj->header.rpcvers;	\
	((int *) _stream->buf_start)[4] = _obj->header.prog;	\
	((int *) _stream->buf_start)[5] = _obj->header.vers;	\
	flick_suntcp_goto_message_body();			\
}

#define flick_suntcp_client_end_encode() /* Nothing to do. */

#define flick_suntcp_client_start_decode() {	\
	_stream = _obj->out;			\
	flick_suntcp_goto_message_body();	\
}

#define flick_suntcp_client_end_decode() /* Nothing to do. */

/*
 * Server-side macros.
 */

#define flick_suntcp_server_start_decode() {	\
	_stream = InHeadP;			\
	flick_suntcp_goto_message_body();	\
}

#define flick_suntcp_server_end_decode() /* Nothing to do. */

#define flick_suntcp_server_start_encode() {				   \
	_stream = OutHeadP;						   \
	((int *) _stream->buf_start)[1] = ((int *) InHeadP->buf_start)[1]; \
	((int *) _stream->buf_start)[2] = htonl(REPLY);			   \
	((int *) _stream->buf_start)[3] = 0;				   \
	((int *) _stream->buf_start)[4] = 0;				   \
	((int *) _stream->buf_start)[5] = 0;				   \
	flick_suntcp_goto_message_body();				   \
}

#define flick_suntcp_server_restart_encode()	\
	flick_suntcp_goto_message_body()

#define flick_suntcp_server_end_encode() {}


/******************************************************************************
 ******************************************************************************
 ****
 **** Globbing, chunking, and spanning code.
 ****
 ******************************************************************************
 *****************************************************************************/

#define flick_suntcp_encode_new_glob(max_size, _onerror) {		   \
	if (((char *) _stream->buf_end) - ((char *) _stream->buf_current)  \
	    < (int) (max_size)) {					   \
		int __ofs = ((char *) _stream->buf_current)		   \
			     - ((char *) _stream->buf_start);		   \
		int __siz = __ofs * 2 + (max_size);			   \
									   \
		if(!(_stream->buf_start					   \
		     = (void *) realloc(_stream->buf_start, __siz))) {	   \
			_stream->buf_start				   \
				= ((char *) _stream->buf_current) - __ofs; \
			flick_stub_error(FLICK_ERROR_NO_MEMORY, _onerror); \
		}							   \
		_stream->buf_current = (((char *) _stream->buf_start)	   \
					+ __ofs);			   \
		_stream->buf_end = (((char *) _stream->buf_start)	   \
				    + __siz);				   \
	}								   \
}

#define flick_suntcp_encode_new_glob_plain(max_size, _onerror) \
	flick_suntcp_encode_new_glob(max_size, _onerror)
#define flick_suntcp_encode_end_glob(max_size)	/* We don't do squat */
#define flick_suntcp_encode_end_glob_plain(max_size)
#define flick_suntcp_encode_new_chunk(size)	/* Don't do anything */
#define flick_suntcp_encode_new_chunk_plain(size)
#define flick_suntcp_encode_end_chunk(size)	\
	_stream->buf_current = ((char *) _stream->buf_current) + (size)
#define flick_suntcp_encode_end_chunk_plain(size) \
	flick_suntcp_encode_end_chunk(size)
#define flick_suntcp_encode_new_chunk_align(size, final_bits, init_bits, \
					    init_ofs) {			 \
	unsigned int _align = (1 << (final_bits)) - 1;			 \
									 \
	_stream->buf_current						 \
		= ((void *)						 \
		   (((unsigned long) (((char *) _stream->buf_current)	 \
				     + _align))				 \
		    & ~_align));					 \
}

#define flick_suntcp_check_span(_size, _onerror) {			\
	if ((((char *) _stream->buf_current) + _size)			\
	    > ((char *) _stream->buf_end))				\
		flick_stub_error(FLICK_ERROR_COMMUNICATION, _onerror);	\
}

#define flick_suntcp_decode_new_glob(max_size)	/* We don't do squat */
#define flick_suntcp_decode_end_glob(max_size)	/* Again - do nothing */
#define flick_suntcp_decode_new_chunk(size)	/* Rien */
#define flick_suntcp_decode_end_chunk(size)	\
	_stream->buf_current = ((char *) _stream->buf_current) + (size)
#define flick_suntcp_decode_new_chunk_align(size, final_bits, init_bits, \
					    init_ofs) {			 \
	unsigned int _align = (1 << (final_bits)) - 1;			 \
									 \
	_stream->buf_current						 \
		= ((void *)						 \
		   (((unsigned long) (((char *) _stream->buf_current)	 \
				     + _align))				 \
		    & ~_align));					 \
}


/******************************************************************************
 ******************************************************************************
 ****
 **** Macros for special data encodings, determined by the link layer.
 ****
 ******************************************************************************
 *****************************************************************************/

/*
 * Target object references.
 */

#define flick_suntcp_mark_port_for_cleanup(a, b)

#define flick_suntcp_client_encode_target(a, b, c, d) {	\
	if (0)						\
		goto d;					\
}
#define flick_suntcp_client_decode_target(a, b, c, d) {	\
	if (0)						\
		goto d;					\
}
#define flick_suntcp_server_encode_target(a, b, c, d) {	\
	if (0)						\
		goto d;					\
}
#define flick_suntcp_server_decode_target(a, b, c, d) {			    \
	/* Avoid a compiler warning about uninit'ed object ref: init it! */ \
	(a) = 0;							    \
	if (0)								    \
		goto d;							    \
}


/******************************************************************************
 ******************************************************************************
 ****
 **** The implementation of the ONC/TCP link layer.
 ****
 ******************************************************************************
 *****************************************************************************/

typedef struct call_header {
	unsigned int xid;
	unsigned int dir;
	unsigned int rpcvers;
	unsigned int prog;
	unsigned int vers;
} call_header;

typedef struct flick_target_struct {
	short port;
	struct sockaddr_in addr;
	int socket_fd;
	call_header header;
	struct FLICK_BUFFER *in, *out;
} FLICK_TARGET_STRUCT;

typedef FLICK_TARGET_STRUCT *FLICK_TARGET;

/*
 * These are not currently used for the Sun back end, but must be defined in
 * order to satisfy some `typedef's in the <flick/pres/corba.h> header file.
 */
typedef int FLICK_PSEUDO_CLIENT;
typedef int flick_invocation_id;

typedef FLICK_TARGET mom_ref_t;

#define FLICK_SUN_UDP (0)
#define FLICK_SUN_TCP (1)

/* May already be defined from <flick/pres/sun.h> */
#ifndef _typedef___FLICK_SERVER_LOCATION
#define _typedef___FLICK_SERVER_LOCATION
typedef struct FLICK_SERVER_LOCATION {
	char *server_name;
	unsigned int prog_num;
	unsigned int vers_num;
} FLICK_SERVER_LOCATION;
#endif

typedef struct FLICK_SERVER_DESCRIPTOR {
	unsigned int prog_num;
	unsigned int vers_num;
	struct sockaddr_in addr;
} FLICK_SERVER_DESCRIPTOR;

typedef struct FLICK_BUFFER *flick_marshal_stream_t;

#define _typedef___FLICK_SERVER

typedef flick_operation_success_t (*FLICK_SERVER)(flick_marshal_stream_t,
						  flick_marshal_stream_t);
typedef flick_operation_success_t flick_dispatch_t(flick_marshal_stream_t,
						   flick_marshal_stream_t);


/*
 * This one must be used by the client to open a connection to the server.
 */
int flick_client_create(FLICK_TARGET, FLICK_SERVER_LOCATION);

/*
 * This one must be used by the client to close a connection to the server.
 */
void flick_client_destroy(FLICK_TARGET);

/*
 * This one _can_ be used to register servers (if they want to build their own
 * main function).
 */
int flick_server_register(FLICK_SERVER_DESCRIPTOR, FLICK_SERVER);

/*
 * This one _can_ be used to begin grabbing messages (if they want to build
 * their own main function).
 */
void flick_server_run();

/*
 * This stuff is used by Flick for transport-independent buffer management and
 * message transmission.  This should NOT be used by user code --- Flick's code
 * is the only stuff that should be using it.
 */

typedef struct FLICK_BUFFER {
	void *buf_start;
	void *buf_current;
	void *buf_end;
	
	/* These not used for encoding and sending !!! */
	void *real_buf_start; /* start of allocated block w/multiple messages*/
	void *buf_read;       /* unfilled part of allocated block */
	void *real_buf_end;   /* end of allocated block */
	
	struct flick_stub_state stub_state;
} FLICK_BUFFER;

int	flick_server_get_request(FLICK_TARGET, FLICK_BUFFER *);
int	flick_client_get_reply(FLICK_TARGET, FLICK_BUFFER *);
int	flick_server_send_reply(FLICK_TARGET, FLICK_BUFFER *);
int	flick_client_send_request(FLICK_TARGET, FLICK_BUFFER *);
int	flick_read_buf(FLICK_BUFFER *, int socket);
int	flick_write_buf(FLICK_BUFFER *, int socket);
void	print_buf(int size, void *data);

#define flick_buffer_contains_more(buf)		\
	((buf)->buf_end < (buf)->buf_read)

#ifndef READ_PACKET
#define READ_PACKET 8192
#endif

#ifndef WRITE_PACKET
#define WRITE_PACKET 5000000
#endif

#ifndef SOCKET_BUF_SIZE
#define SOCKET_BUF_SIZE 65536
#endif


/******************************************************************************
 ******************************************************************************
 ****
 **** Stubs for Flick-specific idioms that may be used when compiling a program
 **** to use `rpcgen'-generated stubs.
 ****
 ******************************************************************************
 *****************************************************************************/

#else /* RPCGEN */

typedef __OLD_CLIENT FLICK_TARGET_STRUCT;

typedef struct FLICK_SERVER_LOCATION {
	char *server_name;
	unsigned int prog_num;
	unsigned int vers_num;
} FLICK_SERVER_LOCATION;

#define create_client(clnt, srvr)			\
	(clnt) = clnt_create((srvr).server_name,	\
			     (srvr).prog_num,		\
			     (srvr).vers_num,		\
			     "tcp")

#endif /* RPCGEN */


/*****************************************************************************/

#endif /* __flick_link_suntcp_h */

/* End of file. */

