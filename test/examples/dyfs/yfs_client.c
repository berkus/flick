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

#include "yfs.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#ifndef RPCGEN
mom_ref_t client;
#define clnt_pcreateerror(x) {}
#else
#include <rpc/rpc.h>
CLIENT  *client;
#endif

static  init = 0;
args	arg;
yfsres  *res;
char	*host;
unsigned YfsError;

Init()
{	
	struct timeval tv = { 5, 0 };
	char *prot = "udp";

	init++;
	if(!(host = getenv("YFSHOST"))) host = "localhost";
	if(!(client=clnt_create(host, YFS_PROGRAM, YFS_VERSION, prot))) {
	    clnt_pcreateerror( host );
	    exit(-1);
	}
#ifdef RPCGEN
	clnt_control( client, CLSET_TIMEOUT, (char*)&tv);
#endif
}

HandleError()
{
#ifdef RPCGEN
	struct rpc_err rpcerr;
	clnt_geterr( client, &rpcerr );
	switch(rpcerr.re_status) {
	    case RPC_TIMEDOUT:
		fprintf(stderr, 
		        "YFS server %s not responding, still trying...\n", 
			host);
		break;
	    default:
		clnt_perror( client, host );
		exit(0);
	}
#else
	fprintf(stderr, "rpc failed, exiting\n");
	exit(-1);
#endif
	return 1;
}

#define template1( stubfunc, rpcfunc ) \
Handle stubfunc (char * name, Handle dir) \
{ \
	if(!init) Init(); \
	arg.name = name; \
	arg.obj  = dir; \
	while(!(res = rpcfunc( &arg, client ))) \
	    HandleError();  \
	YfsError = res->yfserror; \
	return res->retcode; \
}

template1( Lookup, lookup_1 )
template1( CreateFile, createfile_1 )
template1( CreateDir, createdir_1 )
template1( Delete, delete_1 )

int CreateLink (char * linkname, Handle dir, Handle fh)
{
	linkargs  arg;

	if(!init) Init(); 
	arg.name = linkname; 
	arg.dir  = dir; 
	arg.obj  = fh; 
	while(!(res = createlink_1( &arg, client )))
	    HandleError(); 
	YfsError = res->yfserror; 
	return res->retcode; 
}

int Read (Handle file, int count, int offset, char * buffer)
{
	readargs   arg;
	yfsreadres *res;

	if(!init) Init(); 
	arg.obj  = file; 
	arg.offset = offset; 
	arg.count = count; 
	while(!(res = read_1( &arg, client )))
	    HandleError(); 
	bcopy( res->data.data_val, buffer, res->data.data_len );
	YfsError = res->yfserror; 
	return res->retcode; 
}

int Write (Handle file, int count, int offset, char * buffer)
{
	writeargs  arg;

	if(!init) Init(); 
	arg.obj  = file; 
	arg.offset  = offset; 
	arg.data.data_len = count; 
	arg.data.data_val = buffer; 
	while(!(res = write_1( &arg, client ))) 
	    HandleError();  
	YfsError = res->yfserror; 
	return res->retcode; 
}

#define template2( stubfunc, rpcfunc ) \
int stubfunc ( Handle file ) \
{ \
	if(!init) Init(); \
	while(!(res = rpcfunc( &file, client ))) \
	    HandleError();  \
	YfsError = res->yfserror; \
	return res->retcode; \
}

template2( Sync, sync_1 )
template2( Stamp, stamp_1 )
template2( Stat, stat_1 )

#define template3( stubfunc, rpcfunc ) \
int stubfunc ( ) \
{ \
	if(!init) Init(); \
	while(!(res = rpcfunc( 0, client ))) \
	    HandleError();  \
	YfsError = res->yfserror; \
	return res->retcode; \
}

template3( SyncDisk, syncdisk_1 )
template3( Shutdown, shutdown_1 )

#if 0
yfsdirlist *ReadDirectory( char *name )
{
	static yfsreaddirres *res;
	if(!init) Init(); 
	while(!(res = readdir_1( &name, client ))) 
	    HandleError();  
	if((YfsError = res->yfserror) == 0)
	    return &(res->yfsreaddirres_u.list);
	else 
	    return 0;
}
#endif
