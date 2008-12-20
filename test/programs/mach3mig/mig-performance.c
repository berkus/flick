/*
 * Copyright (c) 1998 The University of Utah and
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
#include "mig-arrays-client.h"
#include "mig-arrays-server.h"
#include "timer.h"

/* Number of times to bounce a message back and forth */
#define COUNT 67108864

/*#define DEBUG*/

#ifndef DEBUG
boolean_t call_server(mach_msg_header_t *request_ptr,
                      mach_msg_header_t *reply_ptr)
{
	boolean_t res = arrays_server(request_ptr, reply_ptr);
	if (!res) {
		printf("Server function failed, result = %d/0x%08x\n",
		       res, res);
	}
	return res;
}
#else
boolean_t call_server(mach_msg_header_t *request_ptr,
                      mach_msg_header_t *reply_ptr) = arrays_server;
#endif

#ifdef DEBUG
#define _try(_size, _right, _array) {\
	int _result;\
	if ((_result = rpc_##_size##_in(_right, _array)) != KERN_SUCCESS) {\
		fprintf(stderr, "Array size %s failed, result = %d\n", #_size, _result);\
	}\
}
#else
#define _try(_size, _right, _array) rpc_##_size##_in(_right, _array)
#endif

#ifdef PERF_Marshal_Client
#define _time(_size, _right, _array, _bytes)\
{\
	rpc_##_size##_in(_right, _array);\
	CLEAR_TIME(timer);\
	for (i = 0; i < COUNT/_bytes; i++) {\
		START_TIME(timer);\
		rpc_##_size##_in(_right, _array);\
	}\
	printf("%s\t%8d\t%5ld.%06lds\t%9.3fmb/s\t\n", #_size, COUNT/_bytes, (long)(timer/CPU_HZ), (long)(timer/(CPU_HZ/1000000))%1000000, ((double)COUNT*CPU_HZ)/((double)1048576.0*timer));\
}
#elif defined(PERF_Unmarshal_Client)
#define _time(_size, _right, _array, _bytes)\
{\
	rpc_##_size##_in(_right, _array);\
	CLEAR_TIME(timer);\
	for (i = 0; i < COUNT/_bytes; i++) {\
		rpc_##_size##_in(_right, _array);\
		STOP_TIME(timer);\
	}\
	printf("%s\t%8d\t%5ld.%06lds\t%9.3fmb/s\t\n", #_size, COUNT/_bytes, (long)(timer/CPU_HZ), (long)(timer/(CPU_HZ/1000000))%1000000, ((double)COUNT*CPU_HZ)/((double)1048576.0*timer));\
}
#elif defined(PERF_Marshal_Server) || defined(PERF_Unmarshal_Server)
#define _time(_size, _right, _array, _bytes)\
{\
	rpc_time(_right, (unsigned *)&timer);\
	rpc_##_size##_in(_right, _array);\
	CLEAR_TIME(timer);\
	for (i = 0; i < COUNT/_bytes; i++) {\
		rpc_##_size##_in(_right, _array);\
	}\
	rpc_time(_right, (unsigned *)&timer);\
	printf("%s\t%8d\t%5ld.%06lds\t%9.3fmb/s\t\n", #_size, COUNT/_bytes, (long)(timer/CPU_HZ), (long)(timer/(CPU_HZ/1000000))%1000000, ((double)COUNT*CPU_HZ)/((double)1048576.0*timer));\
}
#elif defined(PERF_MSG)
#define _time(_size, _right, _array, _bytes)\
{\
	rpc_##_size##_in(_right, _array);\
	CLEAR_TIME(timer);\
	for (i = 0; i < COUNT/_bytes; i++) {\
		rpc_##_size##_in(_right, _array);\
	}\
	printf("%s\t%8d\t%5ld.%06lds\t%9.3fmb/s\t\n", #_size, COUNT/_bytes, (long)(timer/CPU_HZ), (long)(timer/(CPU_HZ/1000000))%1000000, ((double)COUNT*CPU_HZ)/((double)1048576.0*timer));\
}
#else
#define _time(_size, _right, _array, _bytes)\
{\
	rpc_##_size##_in(_right, _array);\
	CLEAR_TIME(timer);\
	START_TIME(timer);\
	for (i = 0; i < COUNT/_bytes; i++) {\
		rpc_##_size##_in(_right, _array);\
	}\
	STOP_TIME(timer);\
	printf("%s\t%8d\t%5ld.%06lds\t%9.3fmb/s\t\n", #_size, COUNT/_bytes, (long)(timer/CPU_HZ), (long)(timer/(CPU_HZ/1000000))%1000000, ((double)COUNT*CPU_HZ)/((double)1048576.0*timer));\
}
#endif

long long timer;

int call_client(mach_port_t right) {
	t1024K a;
	int i;
	
	for (i=0; i<262144; i++)
		a[i] = 0xabadcafe;

	/* prime buffer */
/*	rpc_512K_in(right, a);*/
	
	/* 64 bytes */
	_time(64, right, a, 64);
	_time(128, right, a, 128);
	_time(256, right, a, 256);
	_time(512, right, a, 512);
	_time(1K, right, a, 1024);
	_time(2K, right, a, 2048);
	_time(4K, right, a, 4096);
	_time(8K, right, a, 8192);
	_time(16K, right, a, 16384);
	_time(32K, right, a, 32768);
	_time(64K, right, a, 65536);
	_time(128K, right, a, 131072);
	_time(256K, right, a, 262144);
	_time(512K, right, a, 524288);
/*	_time(1024K, right, a, 1048576);*/
	
#if 0
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
	_try(64K, right, a);
#endif

	return 0;
}

int s_rpc_time(mach_port_t target, timervar tt)
{
	tt[0] = ((unsigned *)&timer)[0];
	tt[1] = ((unsigned *)&timer)[1];
	CLEAR_TIME(timer);
	return KERN_SUCCESS;
}

#define sr(_size, _name1, _name2)\
int s_rpc_##_name1##_in(mach_port_t o, t##_name2 a)\
{\
	 return KERN_SUCCESS;\
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

/* End of file. */

