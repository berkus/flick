/*
 * Copyright (c) 1996 The University of Utah and
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

/* client program */

#include <stdio.h>
#include <stdlib.h>
#ifndef RPCGEN
#include "pw.h"
mom_ref_t cl;
#else
#include <rpc/rpc.h>
#include "pw.h"
CLIENT *cl;
#endif

char * host;

error(text)
char *text;
{
    fprintf(stderr, "%s", text);
    exit(1);
}

char *get_pw_gecos_new(uname)
char *uname;
{
    pw_string_result *res = get_pw_gecos_1(&uname, cl);
    if(!res)
	error("Server didn't respond\n");
    return
	res->success == PW_SUCCESS ? res->pw_res : 0;
}

main(argc, argv)
int argc;
char **argv;
{
    char *gecos;

    if(argc<2)
	error("Usage user_name\n");

    /* we will use the environment variable YFSHOST if a client
       wants to talk to a server on another host.
       Environment variable can set using 
       "setenv YFSHOST cadesm43.eng.utah.edu", for instance
       The code below shows you how to obtain their value from a program
     */
    if(!(host = getenv("YFSHOST")))
	host = "localhost";
    
    if(!(cl = clnt_create(host, PWSRV, PWSRVVERS, "udp")))
	error("Server not found\n");

    if(gecos = get_pw_gecos_new(argv[1]))
	printf("GECOS is: %s \n", gecos);
    else
	printf("No such user!\n");
}
