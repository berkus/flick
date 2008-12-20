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

#ifndef __flick_link_mach3mig_h
#define __flick_link_mach3mig_h

#include <flick/link/all.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mach.h>
#include <mach/mig_errors.h>
#include <mach/mig_support.h>
#include <mach/port.h>
#include <mach/mach_port.h>

/*#include <flick/pres/mig.h>*/

#define FLICK_NO_MEMORY		499 /*XXX*/
#define MAX_REPLY_BUFFER_SIZE 65536 /*XXX*/

/******************************************************************************
 ******************************************************************************
 ****
 **** Internal Flick data types
 ****
 ******************************************************************************
 *****************************************************************************/

/*
 * 'flick_msg_t' is the handle to a message under the decomposed stub
 * presentation Flick is capable of.  Although not currently used or
 * supported in mach3mig, this structure must be defined when including
 * flick/pres/corba.h.
 */
typedef struct {} *flick_msg_t;

#define msgh_request_port	msgh_local_port
#define MACH_MSGH_BITS_REQUEST(bits)	MACH_MSGH_BITS_LOCAL(bits)
#define msgh_reply_port		msgh_remote_port
#define MACH_MSGH_BITS_REPLY(bits)	MACH_MSGH_BITS_REMOTE(bits)

struct flick_port_list {
	mach_port_t p;
	struct flick_port_list *next;
};

struct flick_marshal_stream_type {
	/* This is initially set to point to init_buf,
	   but is dynamically re-allocated if more space is needed. */
	mig_reply_header_t *msg_buf;
	void *msg_end;

	/* The pointer to the current chunk. */
	void *chunk_ptr;

	/* This is for auto-freeing of ports received as in parameters */
	struct flick_port_list *ports_to_free;
};
typedef struct flick_marshal_stream_type *flick_marshal_stream, FLICK_BUFFER;

/*extern FLICK_BUFFER *_stream;*/
extern mig_reply_header_t *_global_buf_start;
extern void               *_buf_end;
extern void               *_global_buf_current;
extern struct flick_port_list *_global_ports_to_free;

typedef mach_port_t FLICK_TARGET;

/*
 * These are not currently used for the Mach3MIG back end, but must be defined
 * in order to satisfy some `typedef's in the <flick/pres/corba.h> header file.
 */
typedef int FLICK_PSEUDO_CLIENT;
typedef int flick_invocation_id;

typedef boolean_t (flick_dispatch_t)(mach_msg_header_t *InHeadP,
				     mach_msg_header_t *OutHeadP);
#define _typedef___FLICK_SERVER
typedef flick_dispatch_t *FLICK_SERVER;

/*
 * CORBA data types
 */

/*
 * The following `typedef's are contained in <flick/pres/corba.h> because a
 * CORBA presentation includes these (opaque) types.  The actual structures are
 * defined below.
 *
 * typedef struct CORBA_ORB_type *CORBA_ORB;
 * typedef struct CORBA_BOA_type *CORBA_BOA;
 */
#include <flick/pres/corba.h>

struct CORBA_ORB_type {
	char *ORBid;
	int OA_count;
	CORBA_BOA *boas;
};

struct CORBA_BOA_type {
        CORBA_ORB orb;
	char *OAid;
	/* This designates where the nameserver can be found */
	mach_port_t name_server_port;
	
	/* The number of objects handled by this boa. */
	unsigned obj_count;

	/* The objects and their names, types, and server functions */
	FLICK_TARGET *objs;
	char **names;
	char **types;
	FLICK_SERVER *impls;
};


#define flick_mach3mig_check_span(_size, _onerror) {			\
	if ((((char *) _buf_current) + _size) > ((char *) _buf_end))	\
		flick_stub_error(FLICK_ERROR_COMMUNICATION, _onerror);	\
}

/******************************************************************************
 ******************************************************************************
 ****
 **** Encoding
 ****
 ******************************************************************************
 *****************************************************************************/

#define flick_mach3mig_encode_target(_data, _adjust)		       	\
{									\
	if ((_adjust) > 1) {						\
		if (mach_port_mod_refs(mach_task_self(), (_data),	\
			MACH_PORT_RIGHT_SEND, -((_adjust)-1)))		\
		  return _return;					\
	}								\
	_buf_start->Head.msgh_remote_port = (_data);			\
	_buf_start->Head.msgh_bits =					\
		MACH_MSGH_BITS((_adjust) ?				\
					MACH_MSG_TYPE_MOVE_SEND		\
						:			\
					MACH_MSG_TYPE_COPY_SEND,	\
			       0);					\
	_buf_current += sizeof(_buf_start->Head);			\
}

#define flick_mach3mig_encode_new_glob_plain(max_size, _onerror)	\
	flick_mach3mig_encode_new_glob(max_size, _onerror)
#define flick_mach3mig_encode_end_glob_plain(max_size)		\
	flick_mach3mig_encode_end_glob(max_size)
#define flick_mach3mig_encode_new_chunk_plain(size)	\
	flick_mach3mig_encode_new_chunk(size)
#define flick_mach3mig_encode_end_chunk_plain(size)		\
	flick_mach3mig_encode_end_chunk(size)

/* Primitive types with individual type descriptors.  */

int flick_mach3mig_rpc_grow_buf(void *, int);

#define flick_mach3mig_encode_new_glob(max_size, _onerror)			\
{										\
	if (_buf_current + (max_size) > _buf_end) {				\
		if( (_stub_state.error_number =					\
		     flick_mach3mig_rpc_grow_buf(_buf_current, max_size))	\
		    != FLICK_ERROR_NONE )					\
	                goto _onerror;						\
		_buf_start = (mig_reply_header_t *) _global_buf_start;		\
		_buf_current = _global_buf_current;				\
	}									\
}

#define flick_mach3mig_encode_end_glob(max_size) /* Do nothing. */

/*****/

#define flick_mach3mig_encode_new_chunk(size)	/* Do nothing. */
#define flick_mach3mig_encode_new_chunk_align(size, ofs, a, b)		\
{									\
	long __align = 1 << (ofs);					\
	_buf_current +=							\
		((__align - (((long)_buf_current) &			\
			     (__align - 1))) &				\
		 (__align - 1));					\
}

#define flick_mach3mig_encode_end_chunk(size) (_buf_current += (size))

/*****/

/* Array elements.  */
#define flick_mach3mig_array_encode_new_glob(max_size, _onerror) \
	flick_mach3mig_encode_new_glob(max_size, _onerror)
#define flick_mach3mig_array_encode_end_glob(max_size) \
	flick_mach3mig_encode_end_glob(max_size)
#define flick_mach3mig_array_encode_new_chunk(size) \
	flick_mach3mig_encode_new_chunk(size)
#define flick_mach3mig_array_encode_end_chunk(size) \
	flick_mach3mig_encode_end_chunk(size)
#define flick_mach3mig_array_encode_new_chunk_align(size, ofs, a, b) \
	flick_mach3mig_encode_new_chunk_align(size, ofs, a, b)

/******************************************************************************
 ******************************************************************************
 ****
 **** Decoding
 ****
 ******************************************************************************
 *****************************************************************************/

#define check_span(_size, _onerror)					\
	if( (_buf_current + _size) > _buf_end )				\
		flick_stub_error(FLICK_ERROR_COMMUNICATION, _onerror);

#if TypeCheck
#define flick_iftypecheck(code) code
#else
#define flick_iftypecheck(code)
#endif

/* Primitive types with individual type descriptors.  */
#define flick_mach3mig_decode_new_glob(max_size) /* Do nothing. */
#define flick_mach3mig_decode_end_glob(max_size) /* Do nothing. */

#define flick_mach3mig_decode_new_chunk(size)		\
{							\
	flick_iftypecheck(				\
		if (_buf_current + (size) > _buf_end)	\
			return MIG_TYPE_ERROR;		\
	);						\
}
#define flick_mach3mig_decode_new_chunk_align(size, ofs, a, b) {	\
    long __align = 1 << (ofs);						\
    _buf_current +=							\
	((__align - (((long)_buf_current) & (__align - 1))) &		\
	 (__align - 1));						\
}
#define flick_mach3mig_decode_end_chunk(size) (_buf_current += (size))

/* Array elements.  */
#define flick_mach3mig_array_decode_new_glob(max_size)	flick_mach3mig_decode_new_glob(max_size)
#define flick_mach3mig_array_decode_end_glob(max_size)	flick_mach3mig_decode_end_glob(max_size)
#define flick_mach3mig_array_decode_new_chunk(size)	flick_mach3mig_decode_new_chunk(size)
#define flick_mach3mig_array_decode_end_chunk(size)	flick_mach3mig_decode_end_chunk(size)
#define flick_mach3mig_array_decode_new_chunk_align(size, ofs, a, b) \
	flick_mach3mig_decode_new_chunk_align(size, ofs, a, b)


/******************************************************************************
 ******************************************************************************
 ****
 **** Client-side support
 ****
 ******************************************************************************
 *****************************************************************************/

mach_msg_return_t flick_mach3mig_rpc(mach_msg_option_t msg_options,
				     mach_msg_timeout_t timeout);
#if 0
mach_msg_return_t flick_mach3mig_send(void *chunk_ptr,
				      mach_msg_option_t msg_options,
				      mach_msg_timeout_t timeout);
#endif
#define flick_mach3mig_rpc_static(msg_options, timeout)		\
	mach_msg(&_buf_start->Head,				\
		 MACH_SEND_MSG|MACH_RCV_MSG|(msg_options),	\
		 _buf_start->Head.msgh_size,			\
		 sizeof(_global_buf_start),			\
		 _buf_start->Head.msgh_local_port,		\
		 (timeout),					\
		 MACH_PORT_NULL)
		
#define flick_mach3mig_rpc_dynamic(msg_options, timeout)	\
	flick_mach3mig_rpc((msg_options), (timeout));

#define flick_mach3mig_send(msg_options, timeout)	\
	mach_msg(&_buf_start->Head,			\
		 MACH_SEND_MSG|(msg_options),		\
		 _buf_start->Head.msgh_size,		\
		 0,					\
		 MACH_PORT_NULL,			\
		 (timeout),				\
		 MACH_PORT_NULL)
		

/******************************************************************************
 ******************************************************************************
 ****
 **** Server-side support
 ****
 ******************************************************************************
 *****************************************************************************/

#define flick_mach3mig_server_start_decode()				   \
	_buf_start = (mig_reply_header_t*)InHeadP;			   \
	_buf_end = ((char *)_buf_start) + _buf_start->Head.msgh_size;	   \
	_buf_current =							   \
		(void *) ((char *)_buf_start + sizeof(mach_msg_header_t)); \
	_global_ports_to_free = 0;

#define flick_mach3mig_server_end_decode() /* Do nothing. */

#define flick_mach3mig_server_restart_encode()				\
	_buf_current =							\
		(void *) ((char *)_buf_start + sizeof(mig_reply_header_t));

#define flick_mach3mig_server_start_encode()				    \
	_buf_start = (mig_reply_header_t*)OutHeadP;			    \
	_buf_end = ((char *)_buf_start) + MAX_REPLY_BUFFER_SIZE;	    \
	_buf_current =							    \
		(void *) ((char *)_buf_start + sizeof(mig_reply_header_t)); \
	/* XXX --- _global_ports_to_free = 0; --- wrong to do this here! */

#define flick_mach3mig_server_end_encode()				\
{									\
	mach_msg_type_t _ret_tmpl =					\
		{ MACH_MSG_TYPE_INTEGER_32, 32, 1, 1, 0, 0 };		\
	OutHeadP->msgh_size						\
		= (int) ((char *)_buf_current - (char *)_buf_start);	\
	OutHeadP->msgh_remote_port = InHeadP->msgh_remote_port;		\
	OutHeadP->msgh_local_port = MACH_PORT_NULL;			\
        _buf_start->RetCodeType = _ret_tmpl;				\
	while (_global_ports_to_free) {					\
	  mach_port_deallocate(mach_task_self(),			\
			       _global_ports_to_free->p);		\
	  _global_ports_to_free = _global_ports_to_free->next;		\
	}								\
}

#define flick_mach3mig_client_start_encode()				\
	_buf_start = (mig_reply_header_t *) _global_buf_start;		\
	_buf_current =							\
		(void *) ((char *)_buf_start + sizeof(mach_msg_header_t));

#define flick_mach3mig_client_end_encode()				\
	_buf_start->Head.msgh_size					\
		= (int) ((char *)_buf_current - (char *)_buf_start);

#define flick_mach3mig_client_start_decode()				\
	_buf_current =							\
		(void *) ((char *)_buf_start + sizeof(mig_reply_header_t));

#define flick_mach3mig_client_end_decode() {}

#define flick_mach3mig_client_encode_target(_target, _remote_name,	\
					    _local_name, _link)		\
	_buf_start->Head.msgh_remote_port = (_target);			\
	_buf_start->Head.msgh_seqno = 0;				\
	_buf_start->Head.msgh_bits =					\
		 MACH_MSGH_BITS(_remote_name, _local_name);

#define flick_mach3mig_client_decode_target(_target, _remote_name,	\
					    _local_name, _link)		\
	(_target) = _buf_start->Head.msgh_remote_port;

#define flick_mach3mig_server_decode_target(_target, _remote_name,	\
					    _local_name, _link)		\
	(_target) = _buf_start->Head.msgh_local_port;

#define flick_mach3mig_server_encode_target(_target, _remote_name,	\
					    _local_name, _link)		\
	_buf_start->Head.msgh_local_port = (_target);			\
	_buf_start->Head.msgh_seqno = 0;				\
	_buf_start->Head.msgh_bits =					\
		 MACH_MSGH_BITS(					\
			 MACH_MSGH_BITS_REPLY(InHeadP->msgh_bits), 0);

#define flick_mach3mig_server_encode_client(_client, _remote_name,	\
					    _local_name, _link)		\
	_buf_start->Head.msgh_remote_port = (_client);

#define flick_mach3mig_server_decode_client(_client, _remote_name,	\
					    _local_name, _link)		\
	(_client) = _buf_start->Head.msgh_remote_port;

#define flick_mach3mig_client_encode_client(_client, _remote_name,	\
					    _local_name, _link)		\
	_buf_start->Head.msgh_local_port = (_client);

#define flick_mach3mig_client_decode_client(_client, _remote_name,	\
					    _local_name, _link)		\
	(_client) = _buf_start->Head.msgh_local_port;

#endif /* __flick_link_mach3mig_h */

/* End of file. */

