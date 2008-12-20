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

#ifndef __flick_link_trapeze_h
#define __flick_link_trapeze_h

#define TRAPEZE

/*
 * XXX --- We `#include <rpc/rpc.h>' now because <flick/pres/sun.h> will
 * include it, and <flick/pres/sun.h> may be included by users of this file
 * (e.g., by Sun-on-Trapeze stubs).  One of the Trapeze headers (`topology.h')
 * defines a `SUCCESS' macro that conflicts with the use of `SUCCESS' in an
 * enumeration defined by of the RPC headers.  So, although we don't need
 * <flick/pres/sun.h> ourselves, we include it now for the benefit of other
 * code.
 */
#define CLIENT __OLD_CLIENT
#include <rpc/rpc.h>
#undef CLIENT

/* Myrinet's <lanai_device.h> defines DBLOCK and UBLOCK, for example. */
#include <lanai_device.h>
#include <tpz.h>
#include <tpz_api.h>

#include <flick/link/all.h>

/*
 * XXX --- Skip everything in this file if `RPCGEN' is defined.  This cruft was
 * inherited from the `suntcp.h' header, on which parts of this file are based.
 * This is supposed to make it easier to write ONC/RPC-based programs that can
 * use either Flick-generated stubs or `rpcgen'-generated stubs.
 */
#ifndef RPCGEN

#define TRAPEZE_MAX_CONTROL_SIZE	(SHMEM_PAYLOAD_BYTES)
#define TRAPEZE_MAX_PAYLOAD_SIZE	(LANAI_BUFFER_SIZE)
#define PAGE_COUNT			(16)

#define DEFAULT_MCP_FILENAME		("mcp/mcp.dat")

/*
 * Are we going to be using ONC-like message or GIOP-like messages?
 */
#ifndef FLICK_PROTOCOL_TRAPEZE_ONC
#  ifndef FLICK_PROTOCOL_TRAPEZE
#    define FLICK_PROTOCOL_TRAPEZE
#  endif
#endif

/*
 * These are used to distinguish different message types.  These are used for
 * both ONC RPC-like and GIOP-like messages; see below.
 */
#define TRAPEZE_RPC_REQUEST	(0)
#define TRAPEZE_RPC_REPLY	(1)

/*
 * `flick_trapeze_sys_ex_id_t' is the enumeration that defines the on-the-wire
 * codes that identify system exceptions in Trapeze messages.  The enumeration
 * parallels the list of CORBA system exception kinds; the runtime function
 * that translates between trapeze and CORBA exceptions *relies* on this.
 */
typedef enum {
	ex_TRAPEZE_UNKNOWN			= 0,
	ex_TRAPEZE_BAD_PARAM			= 1,
	ex_TRAPEZE_NO_MEMORY			= 2,
	ex_TRAPEZE_IMP_LIMIT			= 3,
	ex_TRAPEZE_COMM_FAILURE			= 4,
	ex_TRAPEZE_INV_OBJREF			= 5,
	ex_TRAPEZE_NO_PERMISSION		= 6,
	ex_TRAPEZE_INTERNAL			= 7,
	ex_TRAPEZE_MARSHAL			= 8,
	ex_TRAPEZE_INITIALIZE			= 9,
	ex_TRAPEZE_NO_IMPLEMENT			= 10,
	ex_TRAPEZE_BAD_TYPECODE			= 11,
	ex_TRAPEZE_BAD_OPERATION		= 12,
	ex_TRAPEZE_NO_RESOURCES			= 13,
	ex_TRAPEZE_NO_RESPONSE			= 14,
	ex_TRAPEZE_PERSIST_STORE		= 15,
	ex_TRAPEZE_BAD_INV_ORDER		= 16,
	ex_TRAPEZE_TRANSIENT			= 17,
	ex_TRAPEZE_FREE_MEM			= 18,
	ex_TRAPEZE_INV_IDENT			= 19,
	ex_TRAPEZE_INV_FLAG			= 20,
	ex_TRAPEZE_INTF_REPOS			= 21,
	ex_TRAPEZE_BAD_CONTEXT			= 22,
	ex_TRAPEZE_OBJ_ADAPTER			= 23,
	ex_TRAPEZE_DATA_CONVERSION		= 24,
	ex_TRAPEZE_OBJECT_NOT_EXIST		= 25,
	ex_TRAPEZE_TRANSACTION_REQUIRED		= 26,
	ex_TRAPEZE_TRANSACTION_ROLLEDBACK	= 27,
	ex_TRAPEZE_INVALID_TRANSACTION		= 28
} flick_trapeze_sys_ex_id_t;

/*
 * `flick_trapeze_sys_ex_completed_t' is the enumeration that defines the
 * on-the-wire operation completion flags contained within encoded system
 * exceptions.  Again, these values parallel the corresponding CORBA values.
 */
typedef enum {
	TRAPEZE_COMPLETED_YES			= 0,
	TRAPEZE_COMPLETED_NO			= 1,
	TRAPEZE_COMPLETED_MAYBE			= 2
} flick_trapeze_sys_ex_completed_t;
 
/*
 * 'flick_msg_t' is a handle to a message under Flick's ``decomposed'' stub
 * presentation.  Although not currently used or supported in the Trapeze
 * runtime, this structure must be defined when including <flick/pres/corba.h>.
 */
typedef struct {
	int dummy; /* Unused, but ANSI C doesn't allow 0-slot structs. */
} *flick_msg_t;


/*****************************************************************************/

extern int flick_seqnum;

/*
 * `mcp_filename' MUST BE SET to the path name for the "mcp.dat" file.  The
 * actual variable is defined in the Trapeze API file `tpz_init.c', so this
 * should be declared by one of the Trapeze headers --- but it isn't.
 */
extern char *mcp_filename;

/*
 * This is the free receive DBLOCK for the server.  This is for swapping
 * payloads.  This is defined in `trapeze_orb_boa.c'.
 */
extern int flick_trapeze_server_free_recv_buf;

/*
 * In Trapeze stubs, the Flick stream is implemented by a collection of
 * stub-local variables and by the global `_stub_state' (XXX --- which should
 * also be a stub local):
 *
 *   tpz_msgbuf_t _msg_buf;	--- the Trapeze message buffer.
 *   caddr_t _buf_start;	--- start of current message.
 *   void *_buf_current;	--- position in current message.
 *   int _msg_ctl_len;		--- len of the ``control'' part of the message.
 *
 * Compare this with the implementation of streams in other back ends.  For
 * example, the ONC/TCP and IIOP back ends implement the stream as a single
 * stub-local variable called `_stream'.
 */
struct flick_stub_state _stub_state;

/* XXX --- What should *flick_marshal_stream_t be (if anything)? */
typedef tpz_msgbuf_t FLICK_BUFFER;
typedef void *flick_marshal_stream_t;

void trapeze_mcp_init(int is_server);

void *flick_trapeze_client_array__alloc();
void  flick_trapeze_client_array__free(void *payload_ptr);
void *flick_trapeze_server_array__alloc();


/******************************************************************************
 ******************************************************************************
 ****
 **** ONC message format macros.
 ****
 ******************************************************************************
 *****************************************************************************/

#ifdef FLICK_PROTOCOL_TRAPEZE_ONC

/* We implement version 2 of the ONC RPC protocol, more or less. */
#define TRAPEZE_RPCVERS (2)

/*
 * When sending ONC RPC-like messages, the message format is as follows:
 *
 *   WORD <--- 4 bytes --->
 *        +-------+-------+    --- == unused (set to zero)
 *     0  |  ---  | host  |    host == sender's Myrinet host ID, big-endian
 *        +-------+-------+
 *     1  | xid           |    xid == ONC RPC transaction ID, XDR format
 *        +---------------+
 *     2  | mtype         |    mtype == ONC RPC msg. type, XDR format
 *        +---------------+
 *     3  | rpcvers       |    rpcvers == ONC RPC version, XDR format
 *        +---------------+
 *     4  | prog          |    prog == ONC RPC program ID, XDR format
 *        +---------------+
 *     5  | vers          |    vers == ONC RPC prog. vers. ID, XDR format
 *        +---------------+
 *                             ^^^ Above: fields marshaled by runtime.
 *                             vvv Below: fields marshaled by stub code.
 *   REQUESTS
 *
 *        +---------------+
 *     6  | proc          |    proc == ONC RPC procedure ID, XDR format
 *        +---------------+
 *     7  | reply token   |    reply token == Trapeze reply token, XDR format
 *        +---------------+
 *     8  | body          |    Operation request parameters, XDR format
 *   ...  | ...           |
 *        +---------------+
 *
 *   REPLIES
 *
 *        +---------------+
 *     6  | stat          |    stat == ONC RPC accept status, XDR format
 *        +---------------+
 *     7  | body          |    Operation reply parameters (if stat == 0),
 *   ...  | ...           |    XDR format
 *        +---------------+
 *
 * Remember that XDR uses big-endian encoding of integer values, which is also
 * what one gets from `htonl'.
 *
 * The principal differences from the ONC RPC message format are these: (1) The
 * first word of an ONC RPC message is a fragment header, whereas we store
 * other data there; (2) An ONC RPC request message header includes `cred' and
 * `verf' fields after the `proc'; (3) An ONC RPC reply message header has a
 * different set of fields following `mtype'; (4) We add a `reply token' field
 * to requests; (5) Our `stat' values are zero for success and -1 for failure;
 * and (6) In our system, large arrays may be encoded in a separate ``payload''
 * buffer.
 *
 * Also note that the Trapeze back end imposes its own encoding of system
 * exceptions.  See the macros and runtime functions that deal with system
 * exceptions.
 *
 * An RPC message is encoded in the ``control'' portion of a Trapeze message
 * (except for large arrays; see next paragraph).  An RPC message must fit
 * within a single Trapeze message --- we do not yet allow a single RPC request
 * or reply to span multiple Trapeze messages.  As a result, requests and
 * replies must be relatively small, since the control portion of a Trapeze
 * message can hold only `TRAPEZE_MAX_CONTROL_SIZE' (about 120) bytes.
 *
 * A single fixed- or variable-length array may be carried in the payload that
 * is attached to the Trapeze message.  In the case of variable-length arrays,
 * the array length is carried in the control portion of the Trapeze message
 * and the array data is carried in the payload.  Note that the payload is
 * encoded in XDR, as usual.
 */

typedef struct flick_client_struct svc_req;

/*
 * This is used for both client request and server reply.
 */
#define flick_trapeze_build_message_header(_obj, msg_type) {		\
	((short *) _buf_start)[0] = 0; /* Unused. */			\
	((short *) _buf_start)[1] = htons((_obj)->host);		\
	((int *)   _buf_start)[1] = htonl((_obj)->u.header.xid);	\
	((int *)   _buf_start)[2] = htonl((msg_type));			\
	((int *)   _buf_start)[3] = htonl(TRAPEZE_RPCVERS);		\
	((int *)   _buf_start)[4] = htonl((_obj)->u.header.prog);	\
	((int *)   _buf_start)[5] = htonl((_obj)->u.header.vers);	\
        flick_trapeze_goto_message_body();				\
}

#define flick_trapeze_get_prog_and_vers(prog, vers) {	\
	(prog) = ntohl(((int *) _buf_start)[4]);	\
	(vers) = ntohl(((int *) _buf_start)[5]);	\
}

#define flick_trapeze_goto_message_body()			\
	_buf_current = (void *) (((int *) _buf_start) + 6)

#ifdef VERIFY
/*
 * This is used for both client request and server reply.
 */
#  define flick_trapeze_verify_message_header(msg_type) {		   \
	if (   (ntohs(((short *) _buf_start)[0]) != 0) /* Unused, 0'ed. */ \
	    || (ntohl(((int *)   _buf_start)[2]) != (msg_type))		   \
	    || (ntohl(((int *)   _buf_start)[3]) != TRAPEZE_RPCVERS)	   \
	    )								   \
		fprintf(stderr, "Malformed Message Header!\n");		   \
}
#else
#  define flick_trapeze_verify_message_header(msg_type)
#endif


/******************************************************************************
 ******************************************************************************
 ****
 **** Pseudo-GIOP message format macros.
 ****
 ******************************************************************************
 *****************************************************************************/

#else /* FLICK_PROTOCOL_TRAPEZE_ONC is not defined. */

/*
 * When sending GIOP-like messages, the message format is as follows:
 *
 *   WORD <--- 4 bytes --->
 *        +---+---+-------+    E == endian flag (0 == big, 1 == little)
 *     0  | E | R | host  |    R == request/reply flag (0 == req, 1 == rep)
 *        +---+---+-------+    host == sender's Myrinet host ID, CDR format
 *     1  | obj           |    obj == target obj. key (requests only), CDR fmt.
 *        +---------------+
 *                             ^^^ Above: fields marshaled by runtime.
 *                             vvv Below: fields marshaled by stub code.
 *   REQUESTS
 *
 *        +---------------+
 *     2  | op            |    op == operation ID, CDR format
 *        +---------------+
 *     3  | reply token   |    reply token == Trapeze reply token, CDR format
 *        +---------------+
 *     4  | body          |    Operation request parameters, CDR format
 *   ...  | ...           |
 *        +---------------+
 *
 *   REPLIES
 *
 *        +---------------+
 *     2  | reply status  |    reply status == GIOP reply status, CDR format
 *        +---------------+
 *     3  | body          |    Operation reply parameters or exception data
 *   ...  | ...           |    (depending on `reply status'), CDR format
 *        +---------------+
 *
 * Although CDR is a bi-endian format, we currently force the endianness flag
 * to 1, indicating little endianness.  In effect, this means that messages
 * only work between same-endian hosts.
 *
 * Obviously, our GIOP-like messages are not very GIOP-like at all.  We are
 * missing most of the GIOP-specified header fields; we encode the object key
 * and operation ID as fixed-length (4-byte) values; we add a Trapeze reply
 * token; and large arrays are encoded in a separate ``payload'' buffer.  In
 * other words, except for the use of CDR, we are entirely unlike GIOP :-).
 *
 * Also note that the Trapeze back end imposes its own encoding of system
 * exceptions.  See the macros and runtime functions that deal with system
 * exceptions.
 *
 * An RPC message is encoded in the ``control'' portion of a Trapeze message
 * (except for large arrays; see next paragraph).  An RPC message must fit
 * within a single Trapeze message --- we do not yet allow a single RPC request
 * or reply to span multiple Trapeze messages.  As a result, requests and
 * replies must be relatively small, since the control portion of a Trapeze
 * message can hold only `TRAPEZE_MAX_CONTROL_SIZE' (about 120) bytes.
 *
 * A single fixed- or variable-length array may be carried in the payload that
 * is attached to the Trapeze message.  In the case of variable-length arrays,
 * the array length is carried in the control portion of the Trapeze message
 * and the array data is carried in the payload.  Note that the payload is
 * encoded in CDR, as usual.
 */

/*
 * This is used for both client request and server reply.
 */
#define flick_trapeze_build_message_header(_obj, msg_type) {		  \
	/*								  \
	 * XXX --- The endianness flag is currently hardwired to indicate \
	 * little endianness.  The Trapeze BE does not currently generate \
	 * swapping-CDR code.						  \
	 */								  \
	((char *)  _buf_start)[0] = 1; /* XXX --- hardwired! */		  \
	((char *)  _buf_start)[1] = (msg_type);				  \
	((short *) _buf_start)[1] = (_obj)->host;			  \
	/*								  \
	 * `msg_type' is a compile-time constant, so the following `if'	  \
	 * will be compiled away.					  \
	 */								  \
	if ((msg_type) == TRAPEZE_RPC_REQUEST)				  \
		/* Request message. */					  \
		((int *) _buf_start)[1] = (_obj)->u.info.key;		  \
	else								  \
		/* Reply message. */					  \
		((int *) _buf_start)[1] = 0;				  \
									  \
        flick_trapeze_goto_message_body();				  \
}

#define flick_trapeze_goto_message_body()			\
	_buf_current = (void *) (((int *) _buf_start) + 2)

#ifdef VERIFY
/*
 * This is used for both client request and server reply.
 */
#  define flick_trapeze_verify_message_header(msg_type) {		\
	if (   (((char *) _buf_start)[0] != 1) /* XXX --- hardwired! */	\
	    || (((char *) _buf_start)[1] != (msg_type))			\
	    )								\
		fprintf(stderr, "Malformed Message Header!\n");		\
}
#else
#  define flick_trapeze_verify_message_header(msg_type)
#endif

#endif /* FLICK_PROTOCOL_TRAPEZE_ONC */


/******************************************************************************
 ******************************************************************************
 ****
 **** Initialization and exit code.
 ****
 ******************************************************************************
 *****************************************************************************/

/*
 * Client-side macros.
 */

#define flick_trapeze_client_start_encode() {				  \
	_msg_buf = tpz_get_sendmsg_spinwait();				  \
	if (!_msg_buf) {						  \
		fprintf(stderr,						  \
			"Can't get send message buffer from Trapeze!\n"); \
		exit(1);						  \
	}								  \
	_buf_start = tpz_mtod(_msg_buf);				  \
	flick_trapeze_build_message_header(_obj, TRAPEZE_RPC_REQUEST);	  \
	flick_trapeze_goto_message_body();				  \
}

#define flick_trapeze_client_end_encode() {				\
	_msg_ctl_len = ((char *) _buf_current) - ((char *) _buf_start);	\
}

#define flick_trapeze_client_start_decode() {			\
	_buf_start = tpz_mtod(_msg_buf);			\
	flick_trapeze_verify_message_header(TRAPEZE_RPC_REPLY);	\
	flick_trapeze_goto_message_body();			\
}

#define flick_trapeze_client_end_decode()	\
        tpz_release_rcvmsg(_msg_buf)

/*
 * Server-side macros.
 */

#define flick_trapeze_server_start_decode() {				\
	_replytoken = TPZ_CTRL;						\
	_this_obj->dest = ntohs(((short *) _buf_start)[1]);		\
	flick_trapeze_verify_message_header(TRAPEZE_RPC_REQUEST);	\
	flick_trapeze_goto_message_body();				\
}

#define flick_trapeze_server_end_decode()	\
	tpz_release_rcvmsg(_msg_buf)

#define flick_trapeze_server_start_encode() {				  \
        _msg_buf = tpz_get_sendmsg_spinwait();				  \
	_buf_start = tpz_mtod(_msg_buf);				  \
	flick_trapeze_build_message_header(_this_obj, TRAPEZE_RPC_REPLY); \
	flick_trapeze_goto_message_body();				  \
}

#define flick_trapeze_server_restart_encode()	\
	flick_trapeze_goto_message_body()

#define flick_trapeze_server_end_encode() {		\
	tpz_release_sendmsg(_msg_buf,			\
			    _this_obj->dest,		\
			    _replytoken,		\
			    (((char *) _buf_current)	\
			     - ((char *) _buf_start))	\
			    );				\
}


/******************************************************************************
 ******************************************************************************
 ****
 **** Globbing, chunking, and spanning code.
 ****
 ******************************************************************************
 *****************************************************************************/

#define flick_trapeze_encode_new_glob(max_size, _onerror) {		\
        if (((char *) _buf_current) + (max_size)			\
	    > ((char *) _buf_start) + TRAPEZE_MAX_CONTROL_SIZE) {	\
		fprintf(stderr, "Control buffer overflow!\n");		\
		flick_stub_error(FLICK_ERROR_COMMUNICATION, _onerror);	\
	}								\
}

#define flick_trapeze_encode_end_glob(max_size)		/* We don't do squat */
#define flick_trapeze_encode_new_chunk(size)		/* Don't do anything */
#define flick_trapeze_encode_end_chunk(size)	\
	_buf_current = ((char *) _buf_current) + (size)
#define flick_trapeze_encode_new_chunk_align(size, final_bits, init_bits,    \
					     init_ofs) {		     \
	unsigned int _align = (1 << (final_bits)) - 1;			     \
									     \
	_buf_current = ((void *)					     \
			(((unsigned int) (((char *) _buf_current) + _align)) \
			 & ~_align));					     \
}

#define flick_trapeze_encode_new_glob_plain(max_size, _onerror)	\
        flick_trapeze_encode_new_glob(max_size, _onerror)
#define flick_trapeze_encode_end_glob_plain(max_size)	\
	flick_trapeze_encode_end_glob(max_size)
#define flick_trapeze_encode_new_chunk_plain(size)	\
	flick_trapeze_encode_new_chunk(size)
#define flick_trapeze_encode_end_chunk_plain(size)	\
	flick_trapeze_encode_end_chunk(size)

#define flick_trapeze_check_span(_size, _onerror) {			\
	if ((((char *) _buf_current) + (_size))				\
	    > (((char *) _buf_start) + TRAPEZE_MAX_CONTROL_SIZE))	\
		flick_stub_error(FLICK_ERROR_COMMUNICATION, _onerror);	\
}

#define flick_trapeze_decode_new_glob(max_size)	/* We don't do squat */
#define flick_trapeze_decode_end_glob(max_size)	/* Again - do nothing */
#define flick_trapeze_decode_new_chunk(size)	/* Fait rien */
#define flick_trapeze_decode_end_chunk(size)	\
	_buf_current = ((char *) _buf_current) + (size)
#define flick_trapeze_decode_new_chunk_align(size, final_bits, init_bits,    \
					     init_ofs) {		     \
	unsigned int _align = (1 << (final_bits)) - 1;			     \
									     \
	_buf_current = ((void *)					     \
			(((unsigned int) (((char *) _buf_current) + _align)) \
			 & ~_align));					     \
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

/* The following stuff probably belongs in `corba_on_trapeze.h'. */

#define flick_trapeze_client_encode_target(_obj, _ofs, ENCNAME, _onerror) { \
	flick_trapeze_##ENCNAME##_client_encode_target(_obj);		    \
	if (0) goto _onerror;						    \
}

#define flick_trapeze_cdr_client_encode_target(_obj) {			\
	/* This is done by `flick_trapeze_build_message_header'. */	\
	/* ((int *) _buf_start)[1] = (_obj)->u.info.key */		\
}

#define flick_trapeze_xdr_client_encode_target(_obj)

#define flick_trapeze_client_decode_target(_obj, _ofs, ENCNAME, _onerror) \
	if (0) goto _onerror
#define flick_trapeze_server_encode_target(_obj, _ofs, ENCNAME, _onerror) \
	if (0) goto _onerror
#define flick_trapeze_server_decode_target(_obj, _ofs, ENCNAME, _onerror) { \
	/*								    \
	 * The following `void *' cast eliminates compile-time warnings	    \
	 * from Sun-on-Trapeze code.  In Sun presentations, `_obj' is a	    \
	 * `struct svc_req *' while `_this_obj' is a `FLICK_TARGET'.  We    \
	 * don't support real `svc_req's even in our native Sun code.	    \
	 */								    \
	(_obj) = (void *) _this_obj;					    \
	if (0) goto _onerror;						    \
}

/*
 * System exceptions.
 *
 * These are handled by dispatching to an appropriate function in the Trapeze
 * library.  (Obviously this approach won't scale to lots of presentations, but
 * we only support CORBA and ONC for now.)
 */

#define flick_trapeze_encode_system_exception(_ofs, _exc, PRESNAME)	\
	flick_trapeze_encode_##PRESNAME##_system_exception(		\
		_exc,							\
		(((char *) _buf_current) + (_ofs))			\
		)

#define flick_trapeze_decode_system_exception(_ofs, _exc, PRESNAME)	\
	flick_trapeze_decode_##PRESNAME##_system_exception(		\
		_exc,							\
		(((char *) _buf_current) + (_ofs))			\
		)

/*
 * Arrays are encoded in the Trapeze message payload.
 */

#define _BYTESWAPSHORT(a) ((a >> 8) | (a << 8))

#define _BYTESWAPLONG(a) ((a >> 24) |			\
			  ((a & 0x00FF0000) >> 8 ) |	\
			  ((a & 0x0000FF00) << 8 ) |	\
			  (a << 24))

#define _BYTESWAPHYPER(a) ((a >> 56) |				\
			   ((a & 0x00FF000000000000LL) >> 40) |	\
			   ((a & 0x0000FF0000000000LL) >> 24) |	\
			   ((a & 0x000000FF00000000LL) >> 8) |	\
			   ((a & 0x00000000FF000000LL) << 8) |	\
			   ((a & 0x0000000000FF0000LL) << 24) |	\
			   ((a & 0x000000000000FF00LL) << 40) |	\
			   (a << 56))

#define flick_trapeze_array_swap16(_ptr, _size) {		\
	int _i;							\
	for (_i = 0; _i < (_size)/2; _i++) {			\
		((short *) (_ptr))[_i] =			\
			_BYTESWAPSHORT(((short *) (_ptr))[_i]);	\
	}							\
}

#define flick_trapeze_array_swap32(_ptr, _size) {		\
	int _i;							\
	for (_i = 0; _i < (_size)/4; _i++) {			\
		((int *) (_ptr))[_i] =				\
			_BYTESWAPLONG(((int *) (_ptr))[_i]);	\
	}							\
}

#define flick_trapeze_array_swap64(_ptr, _size) {			\
	int _i;								\
	for (_i = 0; _i < (_size)/8; _i++) {				\
		((long long *) (_ptr))[_i] =				\
			_BYTESWAPHYPER(((long long *) (_ptr))[_i]);	\
	}								\
}

#define flick_trapeze_client_encode_array(_array_ptr, _size) {		 \
	tpz_attach_sendmsg(_msg_buf,					 \
			   ((vm_offset_t)				 \
			    ((int) (DBLOCK[0]) +			 \
			     ((int) (_array_ptr) - (int) (UBLOCK[0])))), \
			   0 /* No callback function */,		 \
			   0);						 \
	tpz_set_payload_len(_msg_buf, (_size));				 \
}

/*
 * The trapeze API requires us to pre-allocate the array buffer prior to
 * actually sending the request.  When the reply message arrived, the array
 * just magically appeared in the allocated space, and we don't have to do
 * anything here.
 */
#define flick_trapeze_client_decode_array(_array_ptr, _size)

#define flick_trapeze_server_encode_array(_array_ptr, _size) {		 \
	tpz_attach_sendmsg(_msg_buf,					 \
			   ((vm_offset_t)				 \
			    ((int) (DBLOCK[0]) +			 \
			     ((int) (_array_ptr) - (int) (UBLOCK[0])))), \
			   0 /* No callback function */,		 \
			   0);						 \
	tpz_set_payload_len(_msg_buf, (_size));				 \
}

#define flick_trapeze_server_decode_array(_array_ptr, _size) {		 \
	int _tmp = (tpz_detach_rcvmsg(_msg_buf) - (int) (DBLOCK[0]));	 \
									 \
	(_array_ptr) = (void *) ((int) (UBLOCK[0]) + _tmp);		 \
        tpz_attach_rcvmsg(						 \
                _msg_buf,						 \
		((vm_offset_t)						 \
		 ((int) (DBLOCK[0])					 \
		  + (LANAI_BUFFER_SIZE					 \
		     * flick_trapeze_server_free_recv_buf)))		 \
		);							 \
        flick_trapeze_server_free_recv_buf = (_tmp / LANAI_BUFFER_SIZE); \
}

#define flick_trapeze_mu_encode_array(_array_ptr, _size)
#define flick_trapeze_mu_decode_array(_array_ptr, _size)

#define flick_trapeze_mark_port_for_cleanup(a, b)


/******************************************************************************
 ******************************************************************************
 ****
 **** The implementation of the Trapeze link layer.
 ****
 ******************************************************************************
 *****************************************************************************/

typedef struct flick_target_struct *FLICK_TARGET;

/*
 * These are not currently used for the Trapeze back end, but must be defined
 * in order to satisfy some `typedef's in the <flick/pres/corba.h> header file.
 */
typedef int FLICK_PSEUDO_CLIENT;
typedef int flick_invocation_id;

#define _typedef___FLICK_SERVER
typedef flick_operation_success_t (*FLICK_SERVER)(FLICK_BUFFER,
						  caddr_t,
						  FLICK_TARGET);
typedef int mom_server_t(FLICK_BUFFER, FLICK_BUFFER *);

/*****************************************************************************/

/*
 * CORBA-specific definitions.
 */

typedef int CORBA_ReferenceData;

#include <flick/pres/corba.h>

struct CORBA_BOA_type {
        char *OAid;
        int count_servers;
        FLICK_TARGET refs;
        CORBA_ORB orb;
};

struct CORBA_ORB_type {
        CORBA_BOA boas[1];
        int OA_count;
        char *ORBid;
};

typedef struct Object_info {
	CORBA_BOA boa;
        CORBA_ReferenceData key;
	const char *type_id; /* considered a constant literal string */
	unsigned int type_id_len; /* strlen(typeid) */
} Object_info;

/*****/

#if 0

/*
 * Runtime marshal/unmarshal functions.
 */
void
flick_cdr_encode_IOR_internal(
	void *_buf_current,
	FLICK_TARGET obj,
	const char *link,
	int ref_adjust);

FLICK_TARGET
flick_cdr_decode_IOR_internal(
	flick_marshal_stream_t stream,
	int cdr_swap,
	const char *link,
	int ref_adjust);

#endif /* 0 */

void
flick_trapeze_encode_corba_system_exception(
	CORBA_Environment *ev,
	void *buf_current);

void
flick_trapeze_decode_corba_system_exception(
	CORBA_Environment *ev,
	void *buf_current);

void
flick_trapeze_server_send_corba_exception(
	FLICK_BUFFER _msg_buf,
	char *exception_id);

/*****************************************************************************/

/*
 * ONC-specific definitions.
 */

/* #include <flick/pres/sun.h> */

typedef struct call_header {
	unsigned int xid;
	/* The next two values are built into the generated stub code. */
	/* unsigned int dir; */
	/* unsigned int rpcvers; */
	unsigned int prog;
	unsigned int vers;
} call_header;

typedef struct FLICK_SERVER_DESCRIPTOR {
	unsigned int prog_num;
	unsigned int vers_num;
	/* struct sockaddr_in addr; */
} FLICK_SERVER_DESCRIPTOR;

/*****/

#ifdef FLICK_PROTOCOL_TRAPEZE_ONC

/*
 * This one _can_ be used to register servers (if they want to build their own
 * main function).
 */
int
flick_server_register(
	FLICK_SERVER_DESCRIPTOR,
	FLICK_SERVER);

/*
 * This one _can_ be used to begin grabbing messages (if they want to build
 * their own main function).
 */
void
flick_server_run();

#endif /* FLICK_PROTOCOL_TRAPEZE_ONC */

void
flick_trapeze_encode_sun_system_exception(
	int ev,
	void *buf_current);

void
flick_trapeze_decode_sun_system_exception(
	int ev,
	void *buf_current);

void
flick_trapeze_server_send_sun_exception(
	FLICK_BUFFER _msg_buf,
	int exception_id);

/*****************************************************************************/

/*
 * MOM-specific definitions.
 */

typedef FLICK_TARGET mom_ref_t;

/*****************************************************************************/

/*
 * Generic `typedef's and function prototypes.
 */

typedef struct flick_target_struct {
	unsigned int dest;
	unsigned int host;
        FLICK_SERVER server_func;
	union {
		call_header header; /* for SUN/ONC */
		Object_info info;   /* for CORBA */
	} u;
} FLICK_TARGET_STRUCT;

typedef flick_operation_success_t flick_dispatch_t(FLICK_BUFFER,
						   caddr_t,
						   FLICK_TARGET);

/*****/

int
flick_trapeze_client_rpc_receiving_payload(
        FLICK_TARGET obj,
	FLICK_BUFFER *msg_buf,
	int msg_ctl_len,
	unsigned int msg_replytoken_word_index,
	tpz_msgspec_t specifier,
	void *out_payload_buf);

int
flick_trapeze_client_rpc(
	FLICK_TARGET obj,
	FLICK_BUFFER *msg_buf,
	int msg_ctl_len,
	tpz_msgspec_t specifier);

int
flick_trapeze_client_send(
	FLICK_TARGET obj,
	FLICK_BUFFER msg_buf,
	int msg_ctl_len,
	tpz_msgspec_t specifier);

int
flick_trapeze_server_get_request(
	FLICK_BUFFER *msg_buf);

/* This should never be called.  It is handled by end_decode. */
int
flick_trapeze_server_send_reply(
        FLICK_BUFFER msg_buf,
	tpz_msgspec_t specifier);

flick_system_exception *
flick_system_exception_alloc();

void
flick_system_exception_free(
	void *system_except);

char *
find_system_exception_id(
	char *name,
	int size);


/*****************************************************************************/

#endif  /* RPCGEN */

#endif /* __flick_link_trapeze_h */

/* End of file. */

