/*
 * Copyright (c) 1997, 1998, 1999 The University of Utah and
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

/*
 * This file contains the server function definitions for mig-tests.defs.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <mach_init.h>
#include <mach/message.h>
#include <mach/port.h>
#include <mach/mig_errors.h>
#include <memory.h>
#include <servers/netname.h>

#include "mig-tests-server.h"


/*
 * Server functions
 */

/*
 * InLine tests
 */

/* multiply in * inout, return in out and inout */
kern_return_t s_inline_integers(mach_port_t target, Iint inInt, Iint *inoutInt, Iint *outInt)
{
	*outInt = inInt * (*inoutInt);
	*inoutInt = *outInt;
	return KERN_SUCCESS;
}

/* struct copy inout to out and in to inout */
kern_return_t s_inline_fixed_structs(mach_port_t target, IFstruct inStruct, IFstruct *inoutStruct, IFstruct *outStruct)
{
	*outStruct = *inoutStruct;
	*inoutStruct = inStruct;
	return KERN_SUCCESS;
}

/* multiply each in element by corresponding inout element, return in
   both inout and out */
kern_return_t s_inline_fixed_arrays(mach_port_t target, IFarray inArray, IFarray inoutArray, IFarray outArray)
{
	int i;
	for (i=0; i<300; i++) {
		outArray[i] = inArray[i] * inoutArray[i];
		inoutArray[i] = outArray[i];
	}
	return KERN_SUCCESS;
}

/* element-by-element copy of in to out */
kern_return_t s_inline_bounded_arrays(mach_port_t target, IBarray inArray, mach_msg_type_number_t inArrayCnt, IBarray outArray, mach_msg_type_number_t *outArrayCnt)
{
	int i;
	for (i=0; i<inArrayCnt; i++) {
		outArray[i] = inArray[i];
	}
	*outArrayCnt = inArrayCnt;
	return KERN_SUCCESS;
}

/* element-by-element copy of in to out */
kern_return_t s_inline_unbounded_arrays(mach_port_t target, Iarray inArray, mach_msg_type_number_t inArrayCnt, Iarray *outArray, mach_msg_type_number_t *outArrayCnt)
{
	int i;
#if 0
	printf("inArray[%d] = %p\n", inArrayCnt, inArray);
	printf("outArray = %p\n", outArray);
	printf("outArray[%d] = %p\n\n", *outArrayCnt, *outArray);
#endif
	*outArray = (Iarray) malloc(sizeof(int)*inArrayCnt);
	for (i=0; i<inArrayCnt; i++) {
		(*outArray)[i] = inArray[i];
	}
	*outArrayCnt = inArrayCnt;
#if 0
	printf("inArray[%d] = %p\n", inArrayCnt, inArray);
	printf("outArray = %p\n", outArray);
	printf("outArray[%d] = %p\n", *outArrayCnt, *outArray);
#endif
	return KERN_SUCCESS;
}

/* copy inout to out and in to inout */
kern_return_t s_inline_fixed_strings(mach_port_t target, IFstring inString, IFstring inoutString, IFstring outString)
{
#if 0
	printf("inString: %s\n", inString);
	printf("inoutString: %s\n", inoutString);
	printf("outString: %s\n\n", outString);
#endif
	strcpy(outString, inoutString);
	strcpy(inoutString, inString);
#if 0
	printf("inString: %s\n", inString);
	printf("inoutString: %s\n", inoutString);
	printf("outString: %s\n", outString);
#endif
	return KERN_SUCCESS;
}

/* copy in to out */
kern_return_t s_inline_bounded_strings(mach_port_t target, IVstring inString, IVstring outString)
{
#if 0
	printf("inString: %s\n", inString);
	printf("outString: %s\n\n", outString);
#endif
	strcpy(outString, inString);
#if 0
	printf("inString: %s\n", inString);
	printf("outString: %s\n", outString);
#endif
	return KERN_SUCCESS;
}

/* copy inRight to outRight, leave inoutRights unchanged */
kern_return_t s_inline_port_rights(mach_port_t target, mach_port_t inRight, mach_port_t *inoutRight, mach_port_t *outRight)
{
#if 0
	printf("inRight = 0x%08x\n", inRight);
	printf("inoutRight = 0x%08x\n", *inoutRight);
#endif
	*outRight = inRight;
#if 0
	printf("inRight = 0x%08x\n", inRight);
	printf("inoutRight = 0x%08x\n", *inoutRight);
	printf("outRight = 0x%08x\n", *outRight);
#endif
	return KERN_SUCCESS;
}

/* copy inRight to outRight */
kern_return_t s_inline_port_arrays(mach_port_t target, Imach_port_array_t inRight, mach_msg_type_number_t inRightCnt, Imach_port_array_t *outRight, mach_msg_type_number_t *outRightCnt)
{
	int i;
	vm_allocate(mach_task_self(), (vm_address_t *)outRight, inRightCnt*sizeof(mach_port_t), 1);
	*outRightCnt = inRightCnt;
	for (i=0; i< inRightCnt; i++)
		(*outRight)[i] = inRight[i];
	return KERN_SUCCESS;
}

/* copy inRight to outRight */
kern_return_t s_inline_poly_arrays(mach_port_t target, Imach_port_array_t inRight, mach_msg_type_number_t inRightCnt, Imach_port_array_t *outRight, mach_msg_type_number_t *outRightCnt)
{
	int i;
	vm_allocate(mach_task_self(), (vm_address_t *)outRight, inRightCnt*sizeof(mach_port_t), 1);
	*outRightCnt = inRightCnt;
	for (i=0; i< inRightCnt; i++)
		(*outRight)[i] = inRight[i];
	return KERN_SUCCESS;
}


/*
 * Out-of-Line tests
 */

/* multiply in * inout, return in out and inout */
kern_return_t s_outline_integers(mach_port_t target, Oint inInt, Oint *inoutInt, Oint *outInt)
{
#if 0
	printf("Got inInt=0x%08x,%d, inoutInt=0x%08x,%d, outInt=<not initialized>\n", (int)inInt, *inInt, (int)*inoutInt, **inoutInt);
#endif
	if (inInt == NULL || inoutInt == NULL || *inoutInt == NULL)
		return MIG_REMOTE_ERROR;
	vm_allocate(mach_task_self(), (vm_address_t *) outInt, sizeof(int), 1);
	**outInt = **inoutInt * *inInt;
	**inoutInt = **outInt;
#if 0
	printf("Set inInt=0x%08x,%d, inoutInt=0x%08x,%d, outInt=0x%08x,%d\n", (int)inInt, *inInt, (int)*inoutInt, **inoutInt, (int)*outInt, **outInt);
#endif
	return KERN_SUCCESS;
}

/* struct copy inout to out and in to inout */
kern_return_t s_outline_fixed_structs(mach_port_t target, OFstruct inStruct, OFstruct *inoutStruct, OFstruct *outStruct)
{
	if (inStruct == NULL || inoutStruct == NULL || *inoutStruct == NULL)
		return MIG_REMOTE_ERROR;
	vm_allocate(mach_task_self(), (vm_address_t *) outStruct, sizeof(*inStruct), 1);
	**outStruct = **inoutStruct;
	**inoutStruct = *inStruct;
	return KERN_SUCCESS;
}

/* multiply each in element by corresponding inout element, return in
   both inout and out */
kern_return_t s_outline_fixed_arrays(mach_port_t target, OFarray inArray, OFarray *inoutArray, OFarray *outArray)
{
	int i;

	if (inArray == NULL || inoutArray == NULL || *inoutArray == NULL)
		return MIG_REMOTE_ERROR;
	vm_allocate(mach_task_self(), (vm_address_t *) outArray, sizeof(int)*400, 1);
	for (i=0; i<400; i++) {
		(**outArray)[i] = (*inArray)[i] * (**inoutArray)[i];
		(**inoutArray)[i] = (**outArray)[i];
	}
	return KERN_SUCCESS;
}

/* element-by-element copy of inout to out and in to inout */
kern_return_t s_outline_unbounded_arrays(mach_port_t target, Oarray inArray, mach_msg_type_number_t inArrayCnt, Oarray *inoutArray, mach_msg_type_number_t *inoutArrayCnt, Oarray *outArray, mach_msg_type_number_t *outArrayCnt)
{
	int i, min;
	
	if (inArray == NULL || inoutArray == NULL || *inoutArray == NULL)
		return MIG_REMOTE_ERROR;
	vm_allocate(mach_task_self(), (vm_address_t *) outArray, sizeof(int) * *inoutArrayCnt, 1);
	for (i=0; i<*inoutArrayCnt; i++) {
		(*outArray)[i] = (*inoutArray)[i];
	}
	*outArrayCnt = *inoutArrayCnt;
	min = (inArrayCnt < *inoutArrayCnt)? inArrayCnt : *inoutArrayCnt;
	for (i=0; i<min; i++) {
		(*inoutArray)[i] = inArray[i];
	}
	*inoutArrayCnt = min;
	return KERN_SUCCESS;
}

/* copy inout to out and in to inout */
kern_return_t s_outline_fixed_strings(mach_port_t target, OFstring inString, OFstring *inoutString, OFstring *outString)
{
	if (inString == NULL || inoutString == NULL || *inoutString == NULL)
		return MIG_REMOTE_ERROR;
	vm_allocate(mach_task_self(), (vm_address_t *) outString, sizeof(*inString), 1);
#if 0
	printf("inString: %s\n", *inString);
	printf("inoutString: %s\n\n", **inoutString);
#endif
	strcpy(**outString, **inoutString);
	strcpy(**inoutString, *inString);
#if 0
	printf("inString: %s\n", *inString);
	printf("inoutString: %s\n\n", **inoutString);
	printf("outString: %s\n", **outString);
#endif
	return KERN_SUCCESS;
}

/* make new set of port rights, same size as inCnt, set inout to contain
   MACH_PORT_NULLs (same size) */
kern_return_t s_outline_port_arrays(mach_port_t target, mach_port_array_t inRight, mach_msg_type_number_t inRightCnt, mach_port_array_t *inoutRight, mach_msg_type_number_t *inoutRightCnt, mach_port_array_t *outRight, mach_msg_type_number_t *outRightCnt)
{
	int i;
	vm_allocate(mach_task_self(), (vm_address_t *)outRight, inRightCnt*sizeof(mach_port_t), 1);
	for (i=0; i < inRightCnt; i++)
		mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &((*outRight)[i]));
	*outRightCnt = inRightCnt;
	for (i=0; i < *inoutRightCnt; i++) {
		mach_port_deallocate(mach_task_self(), (*inoutRight)[i]);
		(*inoutRight)[i] = MACH_PORT_NULL;
	}
	return KERN_SUCCESS;
}

/* make new set of port rights, same size as inCnt, set inout to contain
   MACH_PORT_NULLs (same size) */
kern_return_t s_outline_poly_arrays(mach_port_t target, mach_port_array_t inRight, mach_msg_type_number_t inRightCnt, mach_port_array_t *inoutRight, mach_msg_type_number_t *inoutRightCnt, mach_port_array_t *outRight, mach_msg_type_number_t *outRightCnt)
{
	int i;
	vm_allocate(mach_task_self(), (vm_address_t *)outRight, inRightCnt*sizeof(mach_port_t), 1);
	for (i=0; i < inRightCnt; i++)
		mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &((*outRight)[i]));
	*outRightCnt = inRightCnt;
	for (i=0; i < *inoutRightCnt; i++) {
		mach_port_deallocate(mach_task_self(), (*inoutRight)[i]);
		(*inoutRight)[i] = MACH_PORT_NULL;
	}
	return KERN_SUCCESS;
}


/*
 * Other tests
 */

/* data = data^2, display other params */
kern_return_t s_test_reply_ports(mach_port_t reply, mach_port_t target, unsigned int *data)
{
	*data = *data * *data;
	printf("Myself: 0x%08x, Reply: 0x%08x\n", target, reply);
	return KERN_SUCCESS;
}

/* data = data^2, display other params */
kern_return_t s_test_sreply_ports(mach_port_t target, mach_port_t reply, unsigned int *data)
{
	*data = *data * *data;
	printf("Myself: 0x%08x, Reply: 0x%08x\n", target, reply);
	return KERN_SUCCESS;
}

/* data = data^2, display other params */
kern_return_t s_test_ureply_ports(mach_port_t target, unsigned int *data)
{
	*data = *data * *data;
	printf("Myself: 0x%08x\n", target);
	return KERN_SUCCESS;
}

/* data = data^2, display other params */
kern_return_t s_test_param_flags(mach_port_t target, unsigned int seq, unsigned int *data)
{
	if (*data == 0) {
		printf("Not sending a reply, Seq#: %d\n", seq);
		*data = -1;
		return MIG_NO_REPLY;
	} else if ((signed int)*data < 0) {
		printf("Waiting for %d seconds, Seq#: %d\n", -*data, seq);
		sleep(-*data);
		*data = 0;
	} else {
		*data = *data * *data;
		printf("Myself: 0x%08x, Seq#: %d\n", target, seq);
	}
	return KERN_SUCCESS;
}

/* Return the specified size array, up to 50 elements. */
kern_return_t s_test_countinout(mach_port_t target, Iarray *outArray, mach_msg_type_number_t *outArrayCnt)
{
	int i;
	
	if (*outArrayCnt > 50) *outArrayCnt = 50;
	vm_allocate(mach_task_self(), (vm_address_t *) outArray, sizeof(int) * *outArrayCnt, 1);
	for (i = 0; i < *outArrayCnt; i++)
		(*outArray)[i] = i;
	return KERN_SUCCESS;
}

/* element-by-element copy of inout to out and in to inout */
kern_return_t s_test_dealloc_pointer(mach_port_t target, Oarray inArray, mach_msg_type_number_t inArrayCnt, Oarray *inoutArray, mach_msg_type_number_t *inoutArrayCnt, Oarray *outArray, mach_msg_type_number_t *outArrayCnt)
{
	int i, min;
	
	if (inArray == NULL || inoutArray == NULL || *inoutArray == NULL)
		return MIG_REMOTE_ERROR;
	vm_allocate(mach_task_self(), (vm_address_t *) outArray, sizeof(int) * *inoutArrayCnt, 1);
	for (i=0; i<*inoutArrayCnt; i++) {
		(*outArray)[i] = (*inoutArray)[i];
	}
	*outArrayCnt = *inoutArrayCnt;
	min = (inArrayCnt < *inoutArrayCnt)? inArrayCnt : *inoutArrayCnt;
	for (i=0; i<min; i++) {
		(*inoutArray)[i] = inArray[i];
	}
	*inoutArrayCnt = min;
	return KERN_SUCCESS;
}

kern_return_t s_test_dynamic_dealloc_pointer(mach_port_t target, Oarray inArray, mach_msg_type_number_t inArrayCnt, Oarray *inoutArray, mach_msg_type_number_t *inoutArrayCnt, boolean_t *inoutArrayDealloc, Oarray *outArray, mach_msg_type_number_t *outArrayCnt, boolean_t *outArrayDealloc)
{
	int i, min;
	
	if (inArray == NULL || inoutArray == NULL || *inoutArray == NULL)
		return MIG_REMOTE_ERROR;
	vm_allocate(mach_task_self(), (vm_address_t *) outArray, sizeof(int) * *inoutArrayCnt, 1);
	for (i=0; i<*inoutArrayCnt; i++) {
		(*outArray)[i] = (*inoutArray)[i];
	}
	*outArrayCnt = *inoutArrayCnt;
	min = (inArrayCnt < *inoutArrayCnt)? inArrayCnt : *inoutArrayCnt;
	for (i=0; i<min; i++) {
		(*inoutArray)[i] = inArray[i];
	}
	*inoutArrayCnt = min;
	*inoutArrayDealloc = 0;
	*outArrayDealloc = 1;
	return KERN_SUCCESS;
}

/* data = data^2, display other params */
kern_return_t s_test_servercopy(mach_port_t target, Iarray inArray, mach_msg_type_number_t inArrayCnt, boolean_t inArraySCopy, boolean_t *scopy)
{
	if (inArraySCopy) {
		/* in-line transmission */
		printf("Sent in-line... count = %d\n", inArrayCnt);
		/* no need to deallocate */
	} else {
		/* out-of-line transmission */
		printf("Sent out-of-line... count = %d\n", inArrayCnt);
		/* need to deallocate array ourself */
		vm_deallocate(mach_task_self(),
			      (vm_address_t) inArray,
			      inArrayCnt);
	}
	*scopy = inArraySCopy;
	return KERN_SUCCESS;
}

/* terminate the server */
kern_return_t s_terminate(mach_port_t target)
{
	printf("Terminating server...\n");
	exit(0);
}


/* This is the server function called by the test server. */
boolean_t call_server(mach_msg_header_t *request_ptr,
                      mach_msg_header_t *reply_ptr)
{
	boolean_t ret;
#if 0
	printf("About to call server...\n");
	printf("RPC-RCV:\n");
	print_message(request_ptr, request_ptr->msgh_size);
#endif
	ret = tests_server(request_ptr, reply_ptr);
#if 0
	printf("RPC-SND:\n");
	print_message(reply_ptr, reply_ptr->msgh_size);
	printf("Returning from server (%c)...\n",(ret)?'T':'F');
#endif
	if (!ret) {
		printf("Server function FAILED!\n");
	}
	return ret;
}

/* End of file. */
