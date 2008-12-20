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

#include <signal.h>

/* A signal handler is a pointer to function taking int returning void. */
typedef void (*signal_handler_t)(int);

/*****************************************************************************/

int flick_read_buf(FLICK_BUFFER *out_buf, int sock)
{
	/*
	 * Semantics of this call:
	 *   This call should block until it gets a message.
	 * Reading a message:
	 *   The first 4-byte integer received in the message will
	 *   indicate the remaining message size.  In some cases, we
	 *   may receive the next incoming message (or part of it) as
	 *   well, and must keep track of it (for future calls to this
	 *   function).  We ensure 8-byte alignment for the current
	 *   incoming message when we leave this function.
	 */
	
	int real_buf_size = ((char *) out_buf->real_buf_end)
			    - ((char *) out_buf->real_buf_start);
	int bytes_read;
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
		if ((bytes_read = read(sock, (char *) out_buf->buf_read,
				       (real_buf_size > READ_PACKET) ? READ_PACKET
				       : real_buf_size)) > 0)
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
		    || bytes_read < 4) {
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
	while (bytes_read < 4) {
		int new_read;
#ifdef TRACK_FRAGMENTATION
		mult++;
#endif /* TRACK_FRAGMENTATION */
		if ((new_read = read(sock,
				     (char *) out_buf->buf_read,
				     ((real_buf_size
				       - (((char *) out_buf->buf_read)
					  - ((char *) out_buf->real_buf_start))
					     )
				      > READ_PACKET) ? READ_PACKET
				     : (real_buf_size
					- (((char *) out_buf->buf_read)
					   - ((char *) out_buf->real_buf_start)
						))
			)) < 0)
			goto socket_error;
		
		out_buf->buf_read = ((char *) out_buf->buf_read) + new_read;
		bytes_read += new_read;
#ifdef STATS
		num_reread++;
		printf("Need to read more!\n");
#endif /* STATS */
	}
	
	buf_size = (ntohl(*(int *)out_buf->buf_start)
		    & 0x7fffffffL) + 4; /* Offset for the header... */
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
				     ((real_buf_size
				       - (((char *) out_buf->buf_read)
					  - ((char *) out_buf->real_buf_start))
					     )
				      > READ_PACKET) ? READ_PACKET
				     : (real_buf_size
					- (((char *) out_buf->buf_read)
					   - ((char *) out_buf->real_buf_start)
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

#ifdef DEBUG
void print_buf(int size, void *data)
{
	int i, j;
	for (j = 0; j < 2; j++) {
		for (i = 0; i < size; i++) {
			if (i % 16 == 0)
				printf("\n%06x: ", i);
			else if (i % 4 == 0)
				printf(" ");
			if (j)
				printf(" %c", (((char *)data)[i]) < ' ' ? ' ' : ((char *)data)[i]);
			else
				printf("%02x", ((char *)data)[i] & 0xff);
		}
	}
	printf("\n");
}
#endif /* DEBUG */

int flick_write_buf(FLICK_BUFFER *buf, int socket)
{
	signal_handler_t saved_sigpipe_handler;
	
	unsigned int size = (char *)buf->buf_current - (char *)buf->buf_start;
	int written = 0;
	int offset = 0;
	/* Set the size of the buffer */
	((int *)buf->buf_start)[0] = htonl((0x80000000 | size) - 4);
#ifdef DEBUG
	printf("Writing:");
	print_buf(size, buf->buf_start);
#endif /* DEBUG */
	
	/* Ignore SIGPIPE: writes on a socket with no reader. */
	saved_sigpipe_handler = signal(SIGPIPE, SIG_IGN);
	
	/* I'm not sure if this while loop is necessary... */
	while (size > 0) {
		int write_size = (size < WRITE_PACKET) ? size : WRITE_PACKET;
		written = write(socket,
				&((char *)buf->buf_start)[offset],
				write_size);
		
		if (written < 0)
			break;
		
		offset += written;
		size -= written;
	}

	/* Restore the old SIGPIPE handler. */
	(void) signal(SIGPIPE, saved_sigpipe_handler);
	
	if (size > 0) {
		perror("socket write failure");
		/* XXX --- We used to `close(socket)' here.  Gack! */
	}
	
	/* Return 1 (true) for success or 0 (false) for failure. */
	return (size == 0);
}

/* End of file. */
