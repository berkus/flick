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
*  file.h:  Our lower level functions declarations
*
*******************************************************************************/

#ifndef _YFS_FILE_H_
#define _YFS_FILE_H_

#include <sys/types.h>
#include <string.h>
#include "ops.h"

#define bcopy(src, dst, len) memcpy(dst, src, len)

#ifdef __cplusplus
extern "C" {
#endif

int ReadDisk(int blknum, caddr_t addr, int blks);
int WriteDisk(int blknum, caddr_t addr, int blks);

int FormatDisk( );

/*
 * Set up externs to in-memory Handle data structure
 */

extern char BaseBlockBuffer[512];

extern int * FileHandle;
extern int * Time;
extern int * Dir;
extern int * DirEntryCount;
extern int * FileSize;
extern int * LinkCount;
extern int * Spare1;
extern int * Spare2;

extern int * Map;
extern int * Direct;
extern int * Indirect;

extern char * FirstData;

/*
 * Our function declarations
 */

int StartFileSystem ();
int FormatDisk ();

Handle FindFilename (char *);
int   Access   (Handle);
int Update ();
int FileRead (int count, int offset, char *buffer);
int FileWrite (int count, int offset, char *buffer);
Handle MakeFile ();
int DeleteFile ();
int AddFilename (Handle, char *);
int RemoveFilename (char *);
int TimeStamp ();
int PrintBlock (char *);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _YFS_FILE_H_ */
