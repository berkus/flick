/*
 * Copyright (c) 1997, 1998 The University of Utah and
 * the Computer Systems Laboratory at the University of Utah (CSL).
 *
 * This file is part of Flick, the Flexible IDL Compiler Kit.
 *
 * Flick is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Flick is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Flick; see the file COPYING.  If not, write to
 * the Free Software Foundation, 59 Temple Place #330, Boston, MA 02111, USA.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <mach/message.h>
#include <mach/port.h>
#include <mach/mig_errors.h>
#include <memory.h>
#include <mach_error.h>

#include "mig-arrays-client.h"
#include "mig-arrays-server.h"

boolean_t call_server(mach_msg_header_t *request_ptr,
                      mach_msg_header_t *reply_ptr)
{
	boolean_t ret;
	ret = arrays_server(request_ptr, reply_ptr);
	printf("Server returned (%c)\n",(ret)?'T':'F');
	fflush(stdout);
	return ret;
}

#define _try(_size, _right, _array) {					      \
	int _result;							      \
	if ((_result = rpc_##_size##_in(_right, _array)) != KERN_SUCCESS) {   \
		fprintf(stderr,						      \
			"Array size %s failed, result = %d (0x%08x) [%s]\n",  \
			#_size, _result, _result,			      \
			mach_error_string(_result));			      \
	}								      \
}

int call_client(mach_port_t right) {
	t1024K a;
	int i;

	printf("About to run client...\n");
	for (i=0; i<16384; i++)
		a[i] = 0xabadcafe;
	
	_try(64, right, a);
	_try(128, right, a);
	_try(256, right, a);
	_try(512, right, a);
	_try(1K, right, a);
	_try(2K, right, a);
	_try(4K, right, a);
	_try(8K, right, a);
	_try(16K, right, a);
	_try(32K, right, a);
	/* Cannot exceed the 64K message buffer limit of the server!
	   (These tests will fail). */
	/* _try(64K, right, a);
	   _try(128K, right, a);
	   _try(256K, right, a);
	   _try(512K, right, a);
	   _try(1024K, right, a);
	   _try(1024K, right, a);
	   _try(512K, right, a);
	   _try(256K, right, a);
	   _try(128K, right, a);
	   _try(64K, right, a);*/
	/* End of failed tests. */
	_try(32K, right, a);
	_try(16K, right, a);
	_try(8K, right, a);
	_try(4K, right, a);
	_try(2K, right, a);
	_try(1K, right, a);
	_try(512, right, a);
	_try(256, right, a);
	_try(128, right, a);
	_try(64, right, a);

	printf("Returning from client...\n");
	
	return 0;
}

#define sr(_size, _name1, _name2)					\
int s_rpc_##_name1##_in(mach_port_t o, t##_name2 a)			\
{									\
	int i;								\
	for (i=0; i < _size; i++)					\
		if (a[i] != 0xabadcafe) {				\
			fprintf(stderr,					\
				"\t** %s check failed! (byte %d) **\n",	\
				#_name2, i);				\
			break;						\
		}							\
	return KERN_SUCCESS;						\
}


sr(    16,    64,   64B)
sr(    32,   128,  128B)
sr(    64,   256,  256B)
sr(   128,   512,  512B)
sr(   256,    1K,    1K)
sr(   512,    2K,    2K)
sr(  1024,    4K,    4K)
sr(  2048,    8K,    8K)
sr(  4096,   16K,   16K)
sr(  8192,   32K,   32K)
sr( 16384,   64K,   64K)
sr( 32768,  128K,  128K)
sr( 65536,  256K,  256K)
sr(131072,  512K,  512K)
sr(262144, 1024K, 1024K)

int s_rpc_time(mach_port_t target, timervar tt)
{
	tt[0] = 0;
	tt[1] = 0;
	return KERN_SUCCESS;
}

/* End of file. */

