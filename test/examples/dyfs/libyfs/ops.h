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

/*******************************************************************************
*
*  ops.h: declarations of our operations.
*
*******************************************************************************/

#ifndef _YFS_OPS_H_
#define _YFS_OPS_H_
#include <errno.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Set up a type to serve as our file handles (Handles, ect.)
 */
#if !defined(_typedef___Handle) && !defined(_YFS_H_RPCGEN)
#define _typedef___Handle
typedef int Handle;
#endif

/*
 * this variable contains the error code
 */
extern unsigned	YfsError;

/*
 * various functions exported by libyfs.a
 */
Handle Lookup (char * name, Handle dir);
int CreateFile (char * filename, Handle dir);
int CreateLink (char * linkname, Handle dir, Handle fh);
int CreateDir (char * dirname, Handle dir);
int Delete (char * name, Handle dir);
int Read (Handle file, int count, int offset, char * buffer);
int Write (Handle file, int count, int offset, char * buffer);
int Sync (Handle file);
int SyncDisk ();
int Shutdown ();
int Stamp (Handle file);
int Stat (Handle file);

/*
 * initializes server, must be called once before any other function is called
 */
int Init();

typedef struct {
	int	d_length;
        Handle	d_handle;
        char	d_name[128+1];
} dirent;

char *ReadDir( Handle dhandle, int *offset);

#ifdef __cplusplus
}
#endif

#endif	/* _YFS_OPS_H_ */
