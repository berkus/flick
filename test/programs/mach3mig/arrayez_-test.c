/*
 * Copyright (c) 1996, 1997 The University of Utah and
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
#include "arrayez_-client.h"
#include "arrayez_-server.h"

/*****************************************************************************/

CORBA_long
arrays_server_test8(arrays o, arrays_arr8 a, CORBA_Environment *env)
{
	int i;
	CORBA_long res = 0;
	
	for (i = 0; i < 8; i++) {
		res += a[i];
	}
	return res;
}

CORBA_long
arrays_server_test256(arrays o, arrays_arr256 a, CORBA_Environment *env)
{
	int i;
	CORBA_long res = 0;
	
	for (i = 0; i < 256; i++) {
		res += a[i];
	}
	return res;
}

CORBA_long
arrays_server_test8k(arrays o, arrays_arr8k a, CORBA_Environment *env)
{
	int i;
	CORBA_long res = 0;
	
	for (i = 0; i < 10; i++) {
		res += a[i];
	}
	return res;
}

/*****************************************************************************/

int
call_server(mach_msg_header_t *request_ptr, mach_msg_header_t *reply_ptr)
{
	return arrays_server(request_ptr, reply_ptr);
}

int
call_client(mach_port_t right)
{
	int result;
	arrays_arr8k a;
	int i;
	CORBA_Environment env;
	
	for (i = 0; i < 8192; i++)
		a[i] = rand() % 2000 - 1000;
	
	result = arrays_test8k(right, a, &env);
	if (env._major != CORBA_NO_EXCEPTION)
		printf("Exception raised: `%s' raised!\n",
		       CORBA_exception_id(&env));
	
	result = (result != arrays_server_test8k(right, a, &env));
	if (env._major != CORBA_NO_EXCEPTION)
		printf("Exception raised: `%s' raised!\n",
		       CORBA_exception_id(&env));
	CORBA_exception_free(&env);
	return result;
}

/* End of file. */

