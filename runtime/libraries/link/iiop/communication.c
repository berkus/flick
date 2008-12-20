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

#include <signal.h>
#include <assert.h>

/* A signal handler is a pointer to function taking int returning void. */
typedef void (*signal_handler_t)(int);

/*****************************************************************************/

#ifdef DEBUG
static void printbuf(FLICK_BUFFER *buf)
{
	int i;
	int size = buf->buf_read - buf->real_buf_start;
	printf("Buffer size: %06x", size);
	for (i = 0; i < size; i++) {
		if (!(i & 15))
			printf("\n%06x:", i);
		if (!(i & 7))
			printf(" ");
		printf("%02x ", ((int)((char *)buf->real_buf_start)[i]) & 0xFF);
	}
	for (i = 0; i < size; i++) {
		char c = ((char *)buf->real_buf_start)[i];
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
}
#endif /* DEBUG */

static int recv_buf(int sock, FLICK_BUFFER *out_buf)
{
	/*
	 * Semantics of this call:
	 *   This call should block until it gets a message.
	 * Reading a message:
	 *   Each incoming message contains a GIOP header (see CORBA
	 *   2.0 Specification, section 12.4.1).  This 12-byte header
	 *   describes the network byte order used and the length of
	 *   the message following the header.  Once this header is
	 *   received, we can determine the remainder of the message
	 *   size and block until the entire message is read.  In some
	 *   cases, we may receive the next incoming message (or part
	 *   of it) as well, and must keep track of it (for future
	 *   calls to this function).  We ensure 8-byte alignment for
	 *   the current incoming message when we leave this function.
	 */
	
	int real_buf_size = ((char *) out_buf->real_buf_end)
			    - ((char *) out_buf->real_buf_start);
	int bytes_read;
	int test = 1;
	int buf_size;
#ifdef TRACK_FRAGMENTATION
	int mult = 0;
#endif /* TRACK_FRAGMENTATION */
#ifdef STATS
	static int num_realign = 0;
	static int num_realloc = 0;
	static int num_reread = 0;
	static int num_msg = 0;
	num_msg++;
#endif /* STATS */
	/*
	 * out_buf->buf_read -> how much we have already read from the socket.
	 * out_buf->buf_start -> start of the last message read.
	 * out_buf->buf_end -> end of the last message read.
	 */
	if ((out_buf->buf_read == out_buf->real_buf_start)/* new buffer */
	    || (out_buf->buf_read <= out_buf->buf_end)) /* buf is empty */ {

		out_buf->buf_start = out_buf->buf_read
				   = out_buf->real_buf_start;
#ifdef TRACK_FRAGMENTATION
		mult++;
#endif /* TRACK_FRAGMENTATION */
		if ((bytes_read = read(sock,
				       (char *) out_buf->buf_read,
				       real_buf_size)) > 0)
			out_buf->buf_read = ((char *) out_buf->buf_read)
					    + bytes_read;
	} else {
		/* go to the next message */
		out_buf->buf_start = out_buf->buf_end;
		bytes_read = ((char *) out_buf->buf_read)
			     - ((char *) out_buf->buf_start);
		/*
		 * Flick stubs assume 8-byte alignment
		 * Also, realign if we don't even have the header:
		 *   we may need as much space as we can get, so
		 *   we might as well copy the buffer while it's
		 *   small.
		 */
		if (((unsigned long) out_buf->buf_start) & 7
		    || bytes_read < 12) {
			/* Fix the alignment to 8-bytes */
			/* (copy down to beginning of buffer) */
			memcpy(out_buf->real_buf_start,
			       out_buf->buf_start,
			       bytes_read);
			out_buf->buf_start = out_buf->real_buf_start;
			out_buf->buf_read = ((char *) out_buf->buf_start)
					    + bytes_read;
#ifdef STATS
			num_realign++;
			printf("Aligning buffer!\n");
#endif /* STATS */
		}
	}
	
	if (bytes_read <= 0)
		goto socket_error;
	
	/* Make sure we have at least the message length */
	while (bytes_read < 12) {
		int new_read;
#ifdef TRACK_FRAGMENTATION
		mult++;
#endif /* TRACK_FRAGMENTATION */
		if ((new_read = read(sock,
				     (char *) out_buf->buf_read,
				     (real_buf_size
				      - (((char *) out_buf->buf_read)
					 - ((char *) out_buf->real_buf_start)))
			)) < 0)
			goto socket_error;
		
		out_buf->buf_read = ((char *) out_buf->buf_read) + new_read;
		bytes_read += new_read;
#ifdef STATS
		num_reread++;
		printf("Need to read more!\n");
#endif /* STATS */
	}
	
	buf_size = ((unsigned int *)out_buf->buf_start)[2];
	/* endian byte swap */
	if (((char *)out_buf->buf_start)[6] != ((char *)&test)[0])
		buf_size = ((buf_size >> 24) & 0x000000FF) |
			   ((buf_size >>  8) & 0x0000FF00) |
			   ((buf_size <<  8) & 0x00FF0000) |
			   ((buf_size << 24) & 0xFF000000);
	buf_size += 12; /* Offset for the header... */
	
	out_buf->buf_end = ((char *) out_buf->buf_start) + buf_size;
	
	/* if only partial message received */
	if (out_buf->buf_end > out_buf->buf_read) {
		/* if the rest of the message won't fit */
		if (out_buf->buf_end > out_buf->real_buf_end) {
			/* if the message is bigger than the buffer */
			if (buf_size > real_buf_size) {
				char *save_start = out_buf->buf_start;
				out_buf->buf_start = t_malloc(char, buf_size);
				if (!out_buf->buf_start) {	
					fprintf(stderr,
						"Error: can't realloc"
						" receive buffer!\n");
					return 0;
				}
				/* copy the partial message into the
				   new buffer */
				memcpy(out_buf->buf_start, save_start,
				       bytes_read);
				/* free the original buffer */
				free(out_buf->real_buf_start);
				/* adjust real_buf and buf values
				   because of realloc */
				real_buf_size = buf_size;
				out_buf->real_buf_end /* same as buf_end */
					= out_buf->buf_end
					= (((char *) out_buf->buf_start)
					   + buf_size);
				out_buf->buf_read
					= (((char *) out_buf->buf_start)
					   + bytes_read);
				out_buf->real_buf_start =
					out_buf->buf_start;
#ifdef STATS
				num_realloc++;
				printf("Reallocating buffer!\n");
#endif /* STATS */
			} else {
				/* copy the partial message into the
				   beginning of the buffer */
				memcpy(out_buf->real_buf_start,
				       out_buf->buf_start,
				       bytes_read);
				/* adjust buf_read */
				out_buf->buf_read
					= (((char *) out_buf->real_buf_start)
					   + bytes_read);
				out_buf->buf_start
					= out_buf->real_buf_start;
				out_buf->buf_end
					= (((char *) out_buf->buf_start)
					   + buf_size);
			}
		}
		/* read the rest of the incoming message */
		do {
#ifdef TRACK_FRAGMENTATION
			mult++;
#endif /* TRACK_FRAGMENTATION */
			if ((bytes_read = read(sock,
					       (char *) out_buf->buf_read,
					       (real_buf_size
						- (((char *) out_buf->buf_read)
						   - ((char *)
						      out_buf->real_buf_start)
							))
				)) < 0)
				goto socket_error;
#ifdef STATS
			num_reread++;
			printf("Need to read more!\n");
#endif /* STATS */
			out_buf->buf_read = ((char *) out_buf->buf_read)
					    + bytes_read;
		} /* until the whole message is read */
		while (out_buf->buf_end > out_buf->buf_read);
	}
	
#ifdef TRACK_FRAGMENTATION
	if (mult > 1)
		printf("Multiple (%d) reads on server!\n", mult);
#endif /* TRACK_FRAGMENTATION */
#ifdef DEBUG
	printbuf(out_buf);
#endif /* DEBUG */
	
	if (bytes_read <= 0)
		goto socket_error;
	
#ifdef STATS
	printf("Re-aligns: %d/%d  %%=%d\n", num_realign, num_msg, num_realign*100/num_msg);
	printf("Re-allocs: %d/%d  %%=%d\n", num_realloc, num_msg, num_realloc*100/num_msg);
	printf("Re-reads: %d/%d  %%=%d\n", num_reread, num_msg, num_reread*100/num_msg);
#endif /* STATS */
	return 1;
	
  socket_error:
	close(sock);
	return 0;
}

static int send_buf(int sock, FLICK_BUFFER *in_buf)
{
	signal_handler_t saved_sigpipe_handler;
	
	int size = (char *)in_buf->buf_current - (char *)in_buf->buf_start;
	int written = 0;
	int offset = 0;
#ifdef TRACK_FRAGMENTATION
	int mult = 0;
#endif /* TRACK_FRAGMENTATION */
#ifdef DEBUG
	in_buf->buf_read = in_buf->buf_current;
	printbuf(in_buf);
#endif /* DEBUG */
	
	/* Ignore SIGPIPE: writes on a socket with no reader. */
	saved_sigpipe_handler = signal(SIGPIPE, SIG_IGN);
	
	/* I'm not sure if this while loop is necessary... */
	((unsigned int *)in_buf->buf_start)[2] = size - 12;
	while (size > 0) {
		int amount2write = (size < PACKET_SIZE) ? size : PACKET_SIZE;
		written = write(sock,
				&((char *)in_buf->buf_start)[offset],
				amount2write);
#ifdef TRACK_FRAGMENTATION
		if (offset)
			mult++;
#endif /* TRACK_FRAGMENTATION */
		if (written < 0)
			break;
		
		size -= written;
		offset += written;
	}
	
	/* Restore the old SIGPIPE handler. */
	(void) signal(SIGPIPE, saved_sigpipe_handler);
	
#ifdef TRACK_FRAGMENTATION
	if (mult)
		printf("Multiple reads on server - %d!\n", mult);
#endif /* TRACK_FRAGMENTATION */
	if (size > 0)
		perror("socket write failure");
	
	/* Return 1 (true) for success or 0 (false) for failure. */
	return (size == 0);
}


/*****************************************************************************/
#if 0
int flick_buffer_contains_another_whole_message(FLICK_BUFFER *out_buf) {
#define next_buf_start (out_buf->buf_end)
  
        /* if buf is not empty (and holds another message length) */
        if ((char *)next_buf_start + 12 <= (char *)out_buf->buf_read) {
		int test = 1;
		int buf_size;
		if (((char *)next_buf_start)[6] != ((char *)&test)[0]) {
			((char *)&buf_size)[0] = ((char *)next_buf_start)[11];
			((char *)&buf_size)[1] = ((char *)next_buf_start)[10];
			((char *)&buf_size)[2] = ((char *)next_buf_start)[9];
			((char *)&buf_size)[3] = ((char *)next_buf_start)[8];
		} else {
			((char *)&buf_size)[0] = ((char *)next_buf_start)[8];
			((char *)&buf_size)[1] = ((char *)next_buf_start)[9];
			((char *)&buf_size)[2] = ((char *)next_buf_start)[10];
			((char *)&buf_size)[3] = ((char *)next_buf_start)[11];
		}
		buf_size += 12; /* Offset for the header... */
		
		/* if whole message found */
		return (next_buf_start + buf_size <= out_buf->buf_read);
	} else {
		return 0; /* return false */
	}
#undef next_buf_start
}
#endif


/*****************************************************************************/

int flick_client_send_request(FLICK_TARGET obj, FLICK_BUFFER *in_buf)
{
	int retries = CONNECT_RETRIES; /* Set the number of retries allowed */
	int sock = obj->boa->socket_fd;
	
	/* Check to make sure connection is still up. */
	if (sock >= 0 && obj->boa->connected) {
		static struct timeval nowait = { 0, 0 };
		fd_set fds;
		
		FD_ZERO(&fds);
		FD_SET(sock, &fds);
		if (select(sock + 1, &fds, 0, 0, &nowait) > 0) {
			/*
			 * The socket has data to be read!?  This shouldn't
			 * occur, except when a FIN comes in to indicate the
			 * client has shut down its port.  Thus, we close down
			 * our side & try again with a new connection.
			 */
			close(sock);
			retries--;
			goto MakeSocket;
		}
		return send_buf(sock, in_buf);
	}
	
	/* initialize the socket if not already done */
	if (sock < 0) {
	  MakeSocket:
		obj->boa->connected = 0;
		obj->boa->socket_fd
			= sock
			= socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (sock < 0) {
			perror("socket");
			return 0; /* return false */
		}
	}
	
	/* connect to the remote host if not already done */
	if (!obj->boa->connected) {
		if (retries &&
		    (connect(sock,
			     (struct sockaddr *) obj->boa->ipaddr,
			     sizeof(*obj->boa->ipaddr)) != 0)) {
			/* Figure out whether we should retry or not */
			switch (errno) {
			case ECONNREFUSED:
			case ETIMEDOUT:
				perror("connect");
				fprintf(stderr, "Retrying...\n");
				/* We have to close the socket and make
				   a new one in order to retry */
				close(sock);
				/* Delay for some time */
				sleep(CONNECT_RETRY_DELAY);
				retries--;
				goto MakeSocket;
				break;
			default:
				/* The rest are considered to be
				   unrecoverable errors so no retry */
				perror("connect");
				retries = 0;
				break;
			}
		}
		/* Figure out if we got the connect or not */
		if (retries)
			obj->boa->connected = 1;
		else {
			perror("connect");
			close(sock);
			obj->boa->socket_fd = -1;
			return 0; /* return false */
		}
	}
	
	return send_buf(sock, in_buf);
}

int flick_client_get_reply(FLICK_TARGET obj, FLICK_BUFFER *out_buf)
{
	int res = recv_buf(obj->boa->socket_fd, out_buf);
	if (!res)
		obj->boa->socket_fd = -1;
	return res;
}

int flick_server_get_request(int socket, FLICK_BUFFER *out_buf)
{
	return recv_buf(socket, out_buf);
}

int flick_server_send_reply(int sock, FLICK_BUFFER *in_buf)
{
	return send_buf(sock, in_buf);
}

int flick_server_send_exception(int sock,
				unsigned int request_id,
				const char *exception_type)
{
	FLICK_BUFFER flick_buf;
	
	int type_id_len = strlen(exception_type) + 1;
	int type_id_size = (type_id_len + 3) & ~3;
	
	char *buf = t_calloc(char,
			     (12 /* A `GIOP::MessageHeader'/ */
			      + 12 /* Our `GIOP::ReplyHeader'/ */
			      + 4  /* Our system exception.    */
			      + type_id_size
			      + 8  
			      ));
	if (!buf) {
		fprintf(stderr,
			"Error: can't alloc memory for "
			"`flick_server_send_exception'.\n");
		return 1;
	}
	
	/*
	 * Manufacture the `GIOP::MessageHeader'.  See Section 12.4.1 of the
	 * CORBA 2.0 specification.
	 */
	buf[0] = 'G';
	buf[1] = 'I';
	buf[2] = 'O';
	buf[3] = 'P';
	buf[4] = 1;
	buf[5] = 0;
	buf[6] = flick_is_little_endian;
	buf[7] = 1;
	
	*((unsigned int *) &buf[8])
		= (12  /* Our `GIOP::ReplyHeader' (see below). */
		   + 4 /* The exception identifier length and value. */
		   + type_id_size
		   + 8 /* The system exception body. */
		  );
	
	/*
	 * Manufacture the `GIOP::ReplyHeader'. See Section 12.4.2.
	 */
	*((unsigned int *) &buf[12]) = 0;
	*((unsigned int *) &buf[16]) = request_id;
	*((unsigned int *) &buf[20]) = CORBA_SYSTEM_EXCEPTION;
	
	/*
	 * Encode the system exception.
	 */
	*((int *) &buf[24]) = type_id_len;
	strncpy(&(buf[28]), exception_type, type_id_len);
	*((unsigned int *) &buf[28 + type_id_size]) = 0;
	*((unsigned int *) &buf[32 + type_id_size]) = CORBA_COMPLETED_NO;
	
	/* Send this exceptional reply back to the client. */
	flick_buf.buf_start = buf;
	flick_buf.buf_current = buf + 36 + type_id_size;
	flick_buf.buf_end = buf + 36 + type_id_size;
	
	return send_buf(sock, &flick_buf);
}


/*****************************************************************************/

extern flick_client_table_entry *client_table;
extern unsigned int client_table_entries;
extern unsigned int max_client_table_entries;

int flick_send_request_msg(FLICK_TARGET obj,
			   flick_msg_t  msg,
			   flick_invocation_id inv_id,
			   FLICK_PSEUDO_CLIENT cli)
{
	FLICK_BUFFER buf;
	unsigned int len = obj->key._length;
	unsigned int siz = (len + 7) & ~3;
	
	/*
	 * XXX - If we're sending this as a forwarded message,
	 * WE DON'T WANT TO REBUILD THE MSG HEADER!!
	 * However, we MUST re-encode the object key, since we will
	 * almost surely be forwarding it to a different object.
	 */
	   
	if (((signed int) (siz + 24))
	    < (((char *) msg->msg) - ((char *) msg->buf))) {
		msg->hdr = ((char *) msg->msg) - siz;
		*((unsigned int *)msg->hdr) = len;
		memcpy(((char *)msg->hdr) + 4,
		       obj->key._buffer, len);
		/* Fill in the request header info */
		msg->hdr = ((char *) msg->hdr) - 24;
		flick_iiop_build_message_header(msg->hdr,
						0 /* request */);
		/* service context, request id, & response expected */
		((unsigned int *)msg->hdr)[3] = 0;
		((unsigned int *)msg->hdr)[4] = inv_id;
		((unsigned int *)msg->hdr)[5] = msg->response;
	} else {
		/* XXX - Houston, we have a problem. */
		assert(0);
		msg->hdr = msg->msg;
	}

	((unsigned int *)msg->principal)[0]
		= cli->boa->ipaddr->sin_addr.s_addr;
	((unsigned short *)msg->principal)[2]
		= cli->boa->ipaddr->sin_port;
	((unsigned short *)msg->principal)[3]
		= 0; /* ``reserved'' */
	buf.buf_start = msg->hdr;
	buf.buf_end
		= buf.buf_current
		= ((char *) msg->msg) + msg->msg_len;
	
	/* Now set up what we need to service the reply when it comes */
	if (msg->response) {
		flick_client_table_entry *ent;
		if (client_table_entries == max_client_table_entries) {
			max_client_table_entries += 8;
			client_table = t_realloc(client_table,
						 flick_client_table_entry,
						 max_client_table_entries);
		}
		ent = &client_table[client_table_entries++];
		ent->inv_id = inv_id;
		ent->client = cli;
		ent->obj = obj;
		ent->func = msg->func;
	}
	
	return flick_client_send_request(obj, &buf);
}

int flick_send_reply_msg(FLICK_PSEUDO_CLIENT cli,
			 flick_msg_t  msg,
			 flick_invocation_id inv_id,
			 FLICK_TARGET obj)
{
	FLICK_BUFFER buf;
	FLICK_TARGET_STRUCT pseudo;
	
	/* There's always at least room for 20 */
	msg->hdr = ((char *) msg->hdr) - 20;
	flick_iiop_build_message_header(msg->hdr, 1 /* reply */);
	/* service context & request id */
	((unsigned int *)msg->hdr)[3] = 0;
	((unsigned int *)msg->hdr)[4] = inv_id;
	
	buf.buf_start = msg->hdr;
	buf.buf_end
		= buf.buf_current
		= ((char *) msg->msg) + msg->msg_len;
	
	/* This is a really bad hack, but it makes it easy to reuse code.
	 * The only thing flick_client_send_request() uses the object for
	 * is to access the boa.  We set the boa of a fake object to the
	 * boa for our client object, and then initiate a send.
	 * Also, please note that it may *look* like we're sending a request
	 * when it should be a reply, but that actually doesn't matter either
	 * -- we just need the connection made to the client.
	 */
	pseudo.boa = cli->boa;

	return flick_client_send_request(&pseudo, &buf);
}


/*****************************************************************************/

int
flick_continue_request_msg(
	FLICK_TARGET obj,
	flick_msg_t msg,
	flick_invocation_id inv_id,
	FLICK_PSEUDO_CLIENT cli,
	flick_request_continuer continue_func,
	void *continue_data)
{
	assert(!"`flick_continue_request_msg' not yet implemented!");
	return 0;
}

int
flick_continue_reply_msg(
	FLICK_PSEUDO_CLIENT cli,
	flick_msg_t msg,
	flick_invocation_id inv_id,
	FLICK_TARGET obj,
	flick_reply_continuer continue_func,
	void *continue_data)
{
	assert(!"`flick_continue_reply_msg' not yet implemented!");
	return 0;
}

/* End of file. */

