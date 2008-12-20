/*
 * Copyright (c) 1995, 1996, 1997, 1998, 1999 The University of Utah and
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

#include <stdio.h>
#include <mach/message.h>
#include <mach_init.h>
#include <mach/mig_errors.h>

#define ptr_align(x)    \
        ( ( ((vm_offset_t)(x)) + (sizeof(vm_offset_t)-1) ) & ~(sizeof(vm_offset_t\
)-1) )

#define natural_t unsigned int

void print_message_head(mach_msg_header_t *head)
{
  printf("msgh_bits: %08x\n", head->msgh_bits);
  printf("msgh_size: %d\n", head->msgh_size);
  printf("msgh_remote_port: 0x%08x\n", head->msgh_remote_port);
  printf("msgh_local_port: 0x%08x\n", head->msgh_local_port);
  printf("msgh_seqno: %d\n", head->msgh_seqno);
  printf("msgh_id: %d\n", head->msgh_id);
}

void print_memory_block(vm_offset_t buf, vm_size_t length)
{
#if 0
  int i;

  printf("Addr: %#08x  Length: %d\n", (long) buf, length);

  for (i = 0; i < (length/sizeof(int)); i++)
    {
      printf("%08x ", ((int *) buf)[i]);

      if (i != 0 && i % 32 == 0)
	printf("\n");
    }
  printf("\n\n");
#else
	int i, pos = 0;
	printf("Addr: %#08lx  Length: %ld\n", (long) buf, (long) length);
	for (i=0;
	     i < ((sizeof(vm_offset_t) - (buf & (sizeof(vm_offset_t)-1)))%sizeof(vm_offset_t));
	     i++) {
		printf("  ");
		pos += 2;
	}
	while (length) {
		printf("%02x", (int)(*(unsigned char*)buf));
		pos += 2;
		if ((pos & 63) == 0) printf("\n");
		else if ((pos & 7) == 0) printf("  ");
		buf++;
		length--;
	}
	printf("\n\n");
#endif
}

void print_message_type(mach_msg_type_t *type)
{
	printf("msgt_name: %x\n", type->msgt_name);
	printf("msgt_size: %d\n", type->msgt_size);
	printf("msgt_number: %d\n", type->msgt_number);
	printf("msgt_inline: %d\n", type->msgt_inline);
	printf("msgt_longform: %d\n", type->msgt_longform);
	printf("msgt_deallocate: %d\n", type->msgt_deallocate);
	printf("msgt_unused: %d\n", type->msgt_unused);
	if (type->msgt_longform) {
		printf("msgtl_name: %x\n", ((mach_msg_type_long_t *)type)->msgtl_name);
		printf("msgtl_size: %x\n", ((mach_msg_type_long_t *)type)->msgtl_size);
		printf("msgtl_number: %x\n", ((mach_msg_type_long_t *)type)->msgtl_number);
	}
}

void print_message_buf(mach_msg_header_t *msg, vm_offset_t size)
{
  vm_offset_t saddr;
  vm_offset_t eaddr;

  saddr = (vm_offset_t) &(msg[1]);
  eaddr = (vm_offset_t) msg + size;

#if 0
  printf("Sent size: %d\n",size);
  print_memory_block(msg, size);
#endif
  
  while (saddr < eaddr) {
    mach_msg_type_long_t *type;
    mach_msg_type_name_t name;
    mach_msg_type_size_t size;
    mach_msg_type_number_t number;
    boolean_t is_inline;
    vm_size_t length;
    vm_offset_t addr;
    
    print_message_type((mach_msg_type_t *) saddr);
    
    type = (mach_msg_type_long_t *) saddr;
    is_inline = type->msgtl_header.msgt_inline;
    
    if (type->msgtl_header.msgt_longform) 
      {
	name = type->msgtl_name;
	size = type->msgtl_size;
	number = type->msgtl_number;
	saddr += sizeof(mach_msg_type_long_t);
      } 
    else 
      {
	name = type->msgtl_header.msgt_name;
	size = type->msgtl_header.msgt_size;
	number = type->msgtl_header.msgt_number;
	saddr += sizeof(mach_msg_type_t);
      }
	    
    if ((sizeof(natural_t) > sizeof(mach_msg_type_t)) &&
	((size >> 3) == sizeof(natural_t)))
      saddr = ptr_align(saddr);

    /* calculate length of data in bytes, rounding up to ints */
    length = ((((number * size) + 7) >> 3) + 3) &~ 3;

    addr = is_inline ? saddr : * (vm_offset_t *) saddr;

#if 0
    if (length == 0)
	return;
#endif

    if (saddr <= eaddr)
	  {
	    print_memory_block(addr, length);    
	  }
    
    if (is_inline) 
      {
	saddr += length;
      } 
    else 
      {              
	saddr += sizeof(vm_offset_t);
      }
  }
}

void print_message(mig_reply_header_t *rep, vm_offset_t size)
{
	print_message_head(&rep->Head);
/*	print_message_type(&rep->RetCodeType);
	printf("RetCode: %d\n",rep->RetCode);*/
	printf("Message Buffer:\n\n");
	print_message_buf(&rep->Head, size);
	fflush(stdout);
}

/* End of file. */

