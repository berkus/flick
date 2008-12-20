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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "yfs.h"
#ifndef RPCGEN
typedef mom_ref_t	request_t;
#else
#include <rpc/rpc.h>
typedef struct svc_req *request_t;
#endif

#include "ops.h"

#define  YFSMAXDATA	8192

static yfsres		res;
static yfsreadres	readres;
static int init=0;

#define  template1( stubname, funcname ) \
yfsres * stubname ## _1( args *args, request_t o) \
{ \
	if(!init) init++, Init(); \
	res.retcode = funcname( args->name, args->obj ); \
	res.yfserror= YfsError; \
	return &res; \
}

template1( lookup, Lookup )
template1( createfile, CreateFile )
template1( createdir, CreateDir )
template1( delete, Delete )

yfsres *createlink_1( linkargs *args, request_t o )
{
	if(!init) init++, Init(); 
	res.retcode = CreateLink( args->name, args->dir, args->obj );
	res.yfserror= YfsError;
	return &res;	
}

yfsreadres *read_1( readargs *rargs, request_t o )
{
	if(!init) init++, Init(); 
	if(!readres.data.data_val)
	    readres.data.data_val = (char *)malloc( YFSMAXDATA );
	readres.retcode = 
		Read( rargs->obj, rargs->count, 
		      rargs->offset, readres.data.data_val );
	if (readres.retcode == -1) {
	    readres.data.data_len = 0;
	} else {
	    readres.data.data_len = readres.retcode; 
	}
	readres.yfserror = YfsError;
	return &readres;	
}

yfsres *write_1( writeargs *wargs, request_t o )
{
	if(!init) init++, Init(); 
	res.retcode = Write( wargs->obj, wargs->data.data_len,
			     wargs->offset, wargs->data.data_val );
	res.yfserror= YfsError;
	return &res;	
}

#define template2( stubname, funcname ) \
yfsres *stubname##_1( Handle *handle, request_t o ) \
{ \
	if(!init) init++, Init(); \
	res.retcode = funcname( *handle ); \
	res.yfserror= YfsError; \
	return &res;	 \
}

template2( sync, Sync )
template2( stat, Stat )
template2( stamp, Stamp )

yfsres *syncdisk_1( void *x, request_t o ) 
{ 
	if(!init) init++, Init(); 
	res.retcode = SyncDisk( ); 
	res.yfserror= YfsError; 
	return &res;	 
}

yfsres *shutdown_1( void *x, request_t req ) 
{ 
	if(!init) init++, Init(); 
	res.retcode = Shutdown(); 
	res.yfserror= YfsError; 
#ifdef RPCGEN
        if (!svc_sendreply(req->rq_xprt,(xdrproc_t) xdr_yfsres, (char*) &res)) 
	{
            svcerr_systemerr(req->rq_xprt);
        }
#endif
	printf("Goodbye.\n");
	exit(0);

	/* never reached, hopefully */
	return &res;	 
}

#if 0
yfsreaddirres *readdir_1( char **dirname, struct svc_req * )
{
        if(!init) init++, Init(); 

        static  yfsreaddirres   res;

        Handle dh = Lookup( *dirname, 0);
        if(dh == (Handle)-1) {
             res.yfserror = YfsError;
             return &res;
        }

        xdr_free( xdr_yfsreaddirres, (char*)&res );

        char            *de;
        int             offset = 0;
        yfsdirent       *yd, **ydp = &res.yfsreaddirres_u.list.entries;
        while(de = ReadDir( dh, &offset )) {
            yd = *ydp = (yfsdirent *)malloc(sizeof(yfsdirent));  
            yd->name = de;	// already copied
            ydp = &yd->next;    // store address to next
        }
        *ydp = 0;

        res.yfserror = 0;
        res.yfsreaddirres_u.list.dir = dh;
        return &res;
}
#endif

