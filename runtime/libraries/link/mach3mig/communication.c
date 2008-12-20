/*
 * Copyright (c) 1995, 1996, 1997, 1998 The University of Utah and
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

/*
 * Client-side helper routines for the Flick Mach3 backend
 */

#include <stdio.h>
#include <stdlib.h>
#include <mach/mig_support.h>
#include <mach/mig_errors.h>
#include <mach/port.h>
#include <mach/message.h>
#include <flick/link/mach3mig.h>
#include <flick/encode/mach3mig.h>

#define BUFSIZE 4096
char __buffer[BUFSIZE];
#if 0
FLICK_BUFFER __stream = { (mig_reply_header_t *)__buffer,
			  (void *)(((char *)__buffer)+BUFSIZE),
			  (void *)__buffer, 0 };
FLICK_BUFFER *_stream = &__stream;
#else
/* stream structure */
mig_reply_header_t *_global_buf_start = (mig_reply_header_t *)__buffer;
void               *_buf_end = (void *)(((char *)__buffer)+BUFSIZE);
void               *_global_buf_current = (void *)__buffer;
struct flick_port_list *_global_ports_to_free = 0;
#endif

void print_message(mig_reply_header_t *rep, vm_offset_t size);

int
flick_mach3mig_rpc_grow_buf(void *buf_current, int size)
{
  int buf_size = (int)_buf_end - (int)_global_buf_start;
  int offset = (int)buf_current - (int)_global_buf_start;
  int new_size = (buf_size + size + 3) & ~3;
  
#if 0
  printf("Growing buffer...");
#endif

  if (_global_buf_start == (void *)__buffer) {
	  if (!(_global_buf_start =
		(mig_reply_header_t *)malloc(new_size))) {
		  return FLICK_ERROR_NO_MEMORY;
	  }
	  bcopy(__buffer, _global_buf_start, buf_size);
  } else {
	  if (!(_global_buf_start 
		= (mig_reply_header_t *)nonposix_realloc(_global_buf_start, new_size))) {
		  return FLICK_ERROR_NO_MEMORY;
	  }
  }
  _buf_end = (void *)&(((char *)_global_buf_start)[new_size]);
  _global_buf_current = (void *)&(((char *)_global_buf_start)[offset]);
  
#if 0
  printf("done!\n");
#endif
  
  return FLICK_ERROR_NONE;
}

mach_msg_return_t flick_mach3mig_rpc(mach_msg_option_t msg_options,
				     mach_msg_timeout_t timeout)
{
  register mig_reply_header_t *_buf_start = _global_buf_start;
  mach_msg_return_t mr;

  int buf_size = ((int)_buf_end - (int)_buf_start);

  /*
   * Consider the following cases:
   *	1) Errors in pseudo-receive (eg, MACH_SEND_INTERRUPTED
   *	plus special bits).
   *	2) Use of MACH_SEND_INTERRUPT/MACH_RCV_INTERRUPT options.
   *	3) RPC calls with interruptions in one/both halves.
   *	4) Exception reply messages that are bigger
   *	   than the expected non-exception reply message.
   *
   * We refrain from passing the option bits that we implement
   * to the kernel.  This prevents their presence from inhibiting
   * the kernel's fast paths (when it checks the option value).
   */
  
  
#if 0
  printf("RPC-SND:\n");
  print_message(_buf_start, _buf_start->Head.msgh_size);
#endif
  
  mr = mach_msg(&_buf_start->Head,
		MACH_SEND_MSG | MACH_RCV_MSG | MACH_RCV_LARGE | msg_options,
		_buf_start->Head.msgh_size, buf_size,
		_buf_start->Head.msgh_local_port, timeout, MACH_PORT_NULL);

  if (mr != MACH_MSG_SUCCESS)
    {
      while (mr == MACH_SEND_INTERRUPTED)
	{
	  /* Retry both the send and the receive.  */
	  mr = mach_msg_trap(&_buf_start->Head,
			     (MACH_SEND_MSG | MACH_RCV_MSG |
			      MACH_RCV_LARGE | msg_options),
			     _buf_start->Head.msgh_size, buf_size,
			     _buf_start->Head.msgh_local_port,
			     timeout, MACH_PORT_NULL);
	}
	  
      while ((mr == MACH_RCV_INTERRUPTED) || (mr == MACH_RCV_TOO_LARGE))
	{
	  if (mr == MACH_RCV_TOO_LARGE)
	    {
	      /* Oops, message too large - grow the buffer.  */
	      buf_size = _buf_start->Head.msgh_size;
	      if (!(_buf_start = (mig_reply_header_t *)
		    nonposix_realloc(_buf_start, buf_size)))
		{
		  mig_dealloc_reply_port(/*_buf_start->Head.msgh_local_port*/);
		  return FLICK_NO_MEMORY;
		}
#if 0
	      __buffer = _buf_start;
	      __buflen = buf_size;
#endif
	    } 
	      
	      
	  /* Retry the receive only
	     (the request message has already been sent successfully).  */
	  mr = mach_msg_trap(&_buf_start->Head,
			     MACH_RCV_MSG|MACH_RCV_LARGE|msg_options,
			     0, buf_size, _buf_start->Head.msgh_local_port,
			     timeout, MACH_PORT_NULL);
	}
	  
      if (mr != MACH_MSG_SUCCESS)
	{
#if 0
	  if ((mr == MACH_SEND_INVALID_REPLY) ||
	      (mr == MACH_SEND_INVALID_MEMORY) ||
	      (mr == MACH_SEND_INVALID_RIGHT) ||
	      (mr == MACH_SEND_INVALID_TYPE) ||
	      (mr == MACH_SEND_MSG_TOO_SMALL) ||
	      (mr == MACH_RCV_INVALID_NAME))
	    mig_dealloc_reply_port(_buf_start->Head.msgh_local_port);
	  else
	    mig_put_reply_port(_buf_start->Head.msgh_local_port);
#endif
	  return mr;
	}
    }
	
#if 0
  printf("RPC-RCV: (%d, 0x%08x)\n", mr, mr);
  print_message(_buf_start, _buf_start->Head.msgh_size);
  scanf("%d",&mr);
#endif

  /* Stash the reply port again for future use.  */
  /* mig_put_reply_port(_buf_start->Head.msgh_local_port); */
	
#if 0
  if (_buf_start->RetCode != KERN_SUCCESS)
    fprintf(stderr,"Return error code: %x\n", _buf_start->RetCode);
#endif
  
  return mr;
}


#if 0 /* the send is handled by a direct call to mach_msg now */
mach_msg_return_t flick_mach3mig_send(void *buf_current,
				      mach_msg_option_t msg_options,
				      mach_msg_timeout_t timeout)
{
  register mig_reply_header_t *_buf_start = _global_buf_start;
  register void *_buf_current = buf_current;
  mach_msg_return_t mr;

  _buf_start->Head.msgh_local_port = MACH_PORT_NULL;

#if 0
  printf("Send:\n");
  print_message(_buf_start, _buf_start->Head.msgh_size);
#endif
  mr = mach_msg(&_buf_start->Head, MACH_SEND_MSG|msg_options,
		_buf_start->Head.msgh_size, 0,
		MACH_PORT_NULL, timeout, MACH_PORT_NULL);
  
  if (mr != MACH_MSG_SUCCESS)
    {
	  
      while (mr == MACH_SEND_INTERRUPTED)
	{
	  /* Retry both the send and the receive.  */
	  mr = mach_msg_trap(&_buf_start->Head,
			     MACH_SEND_MSG,
			     _buf_start->Head.msgh_size, 0,
			     MACH_PORT_NULL, 0, 0);
	}
	  
      if (mr != MACH_MSG_SUCCESS)
	{
#if 0
	if ((mr == MACH_SEND_INVALID_REPLY) ||
	      (mr == MACH_SEND_INVALID_MEMORY) ||
	      (mr == MACH_SEND_INVALID_RIGHT) ||
	      (mr == MACH_SEND_INVALID_TYPE) ||
	      (mr == MACH_SEND_MSG_TOO_SMALL) ||
	      (mr == MACH_RCV_INVALID_NAME))
	    mig_dealloc_reply_port(_buf_start->Head.msgh_local_port);
	  else
	    mig_put_reply_port(_buf_start->Head.msgh_local_port);
#endif
	return mr;
	}
    }

#if 0
  if (_buf_start->RetCode != KERN_SUCCESS)
    fprintf(stderr,"Return error code: %x\n", _buf_start->RetCode);
#endif
  
  return mr;
}
#endif
/* End of file. */

