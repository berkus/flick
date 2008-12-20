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
#include <flick/encode/cdr.h> /* only used for sending exceptions */

#if 0
#define RPC_DEBUG
#endif

/*****************************************************************************/

#ifdef RPC_DEBUG
static void printbuf(FLICK_BUFFER buf)
{
	int i;
	int size = 120;
	char *start = tpz_mtod(buf);
	printf("Buffer size: %06x", size);
	for (i = 0; i < size; i++) {
		if (!(i & 15))
			printf("\n%06x:", i);
		if (!(i & 7))
			printf(" ");
		printf("%02x ", ((int)(start[i])) & 0xFF);
	}
	for (i = 0; i < size; i++) {
		char c = start[i];
		if ((c & 128) ||
		    !(c & ~31))
			c = '?';
		if (!(i & 15))
			printf("\n%06x:", i);
		if (!(i & 7))
			printf(" ");
		printf(" %c ", c);
	}
	printf("\n");
	fflush(stdout);
}
#endif


/*****************************************************************************/

int flick_trapeze_client_rpc_receiving_payload(
	FLICK_TARGET obj,
	FLICK_BUFFER *msg_buf,
	int msg_ctl_len,
	unsigned int msg_replytoken_word_index,
	tpz_msgspec_t specifier,
	void *out_payload_buf
	)
{
	tpz_replytoken_t replytoken
		= tpz_get_replytoken(
			((vm_offset_t)
			 ((int)DBLOCK[0] +
			  ((int)out_payload_buf - (int)(UBLOCK[0])))),
			ALLOW_INTERRUPT);
	
	if (replytoken == TPZ_REPLY_NOSLOT)
		return 0;
	((unsigned int *)(tpz_mtod(*msg_buf)))[msg_replytoken_word_index]
		= replytoken;
#ifdef RPC_DEBUG
	printbuf(*msg_buf);
#endif
	tpz_release_sendmsg(*msg_buf, obj->dest, specifier, msg_ctl_len);
	*msg_buf = tpz_get_rcvmsg_spinwait();
	tpz_release_replytoken(replytoken);
	return (*msg_buf != NULL);
}

int flick_trapeze_client_rpc(FLICK_TARGET obj,
			     FLICK_BUFFER *msg_buf,
			     int msg_ctl_len,
			     tpz_msgspec_t specifier)
{
#ifdef RPC_DEBUG
	printbuf(*msg_buf);
#endif
	tpz_release_sendmsg(*msg_buf, obj->dest, specifier, msg_ctl_len);
#ifdef RPC_DEBUG
	printf("client sent.\n");
#endif
	*msg_buf = tpz_get_rcvmsg_spinwait();	
#ifdef RPC_DEBUG
	printf("Client received message!\n");	
#endif
	return (*msg_buf != NULL);
}

int flick_trapeze_client_send(FLICK_TARGET obj,
			      FLICK_BUFFER msg_buf,
			      int msg_ctl_len,
			      tpz_msgspec_t specifier)
{
	tpz_release_sendmsg(msg_buf, obj->dest, specifier, msg_ctl_len);
	return 1;
}

int flick_trapeze_server_get_request(FLICK_BUFFER *msg_buf)
{  
	*msg_buf = tpz_get_rcvmsg_spinwait();
	return (*msg_buf != NULL);
}

int flick_trapeze_server_send_reply(FLICK_BUFFER msg_buf,
				    tpz_msgspec_t specifier)
{
	/*
	 * Should never be called.  All is handled by start_encode and
	 * end_encode.
	 */
	return 1;
}

void
flick_trapeze_server_send_corba_exception(
	FLICK_BUFFER _msg_buf,
	char *exception_id
	)
{
	caddr_t _buf_start;
	register void *_buf_current;
	CORBA_Environment _ev;
	FLICK_TARGET_STRUCT obj;
	FLICK_TARGET _this_obj = &obj;
	tpz_msgspec_t  _replytoken = TPZ_CTRL;
	
	/*
	 * Fabricate an object (so we can reuse the server macros), since we
	 * don't really have one.
	 */
	obj.u.info.boa = 0;
	obj.u.info.key = 0;
	obj.u.info.type_id = 0;
	obj.u.info.type_id_len = 0;
	_buf_start = tpz_mtod(_msg_buf);
	obj.dest = ((short *) _buf_start)[1];
	obj.host = 2; /* XXX - host shouldn't be hardcoded */
	obj.server_func = 0;
	
	/* Release the received message. */
	tpz_release_rcvmsg(_msg_buf);
	
	flick_set_exception(0 /*boa*/,
			    &_ev, exception_id, 0, CORBA_COMPLETED_NO);
	
	flick_trapeze_server_start_encode(); /* get message from send ring */
	flick_trapeze_server_encode_target(_this_obj, 0, cdr, on_error);
	flick_trapeze_encode_new_glob(12, on_error);
	flick_trapeze_encode_new_chunk(12);
	flick_cdr_encode_signed32(0, _ev._major, CORBA_exception_type);
	flick_trapeze_encode_system_exception(4, &_ev, corba);
	flick_trapeze_encode_end_chunk(12);
	flick_trapeze_encode_end_glob(12);
	flick_trapeze_server_end_encode(); /* actually sends reply */
	return;
	
  on_error:
	fprintf(stderr,
		("Error: `flick_trapeze_server_send_corba_exception' was "
		 "unable to manufacture a system exception!\n"));
	tpz_senderr(_msg_buf);
	/*
	 * Because we have called `tpz_senderr', Trapeze won't really send the
	 * message when we call `flick_trapeze_server_end_encode'.
	 */
	flick_trapeze_server_end_encode();
	return;
}

void
flick_trapeze_server_send_sun_exception(
	FLICK_BUFFER _msg_buf,
	int exception_id
	)
{
	caddr_t _buf_start;
	register void *_buf_current;
	FLICK_TARGET_STRUCT obj;
	FLICK_TARGET _this_obj = &obj;
	tpz_msgspec_t  _replytoken = TPZ_CTRL;
	
	/*
	 * Fabricate an object (so we can reuse the server macros), since we
	 * don't really have one.
	 */
	obj.u.info.boa = 0;
	obj.u.info.key = 0;
	obj.u.info.type_id = 0;
	obj.u.info.type_id_len = 0;
	_buf_start = tpz_mtod(_msg_buf);
	obj.dest = ((short *) _buf_start)[1];
	obj.host = 2; /* XXX - host shouldn't be hardcoded */
	obj.server_func = 0;
	
	/* Release the received message. */
	tpz_release_rcvmsg(_msg_buf);
	
	flick_trapeze_server_start_encode(); /* get message from send ring */
	/*
	 * XXX --- Yes, we should have a `struct svc_req *' to encode as our
	 * target, but it's unused by the Trapeze runtime anyway!  We pass a
	 * garbage argument here so that if the macros ever do need the right
	 * kind of target reference, we'll get a compilation error.
	 */
	flick_trapeze_server_encode_target(THE_SVC_REQ, 0, xdr, on_error);
	flick_trapeze_encode_new_glob(12, on_error);
	flick_trapeze_encode_new_chunk(12);
	flick_cdr_encode_signed32(0, exception_id, flick_env_t);
	flick_trapeze_encode_system_exception(4, exception_id, sun);
	flick_trapeze_encode_end_chunk(12);
	flick_trapeze_encode_end_glob(12);
	flick_trapeze_server_end_encode(); /* actually sends reply */
	return;
	
  on_error:
	fprintf(stderr,
		("Error: `flick_trapeze_server_send_sun_exception' was "
		 "unable to manufacture a system exception!\n"));
	tpz_senderr(_msg_buf);
	/*
	 * Because we have called `tpz_senderr', Trapeze won't really send the
	 * message when we call `flick_trapeze_server_end_encode'.
	 */
	flick_trapeze_server_end_encode();
	return;
}


/*****************************************************************************/

/* End of file. */

