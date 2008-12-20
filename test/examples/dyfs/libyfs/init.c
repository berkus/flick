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
********************************************************************************
*
*  server.c			Server for Yalnix File System
*
*  Peter A. Jensen		CS506 Second project
*  Aleksandra B. Kuswik

modified by Godmar Back - named init.c

added ReadDir functions

*
********************************************************************************
*******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "file.h"
#include "cache.h"

/*******************************************************************************
*  int Init ()	- call the initialization routines
*
*******************************************************************************/

int Init ()
{
/* Start up the file systems (traverse everything) */

  return (InitCache () != -1 &&
	  StartFileSystem () != -1);
}

char *ReadDir(Handle hh, int *offset)
{
   int	  rr = 0;
   static char	  buf[512];
   dirent *de = (dirent *)buf;

   assert(offset);
   do {
      rr = Read (hh, 150, *offset, buf);
      *offset += de->d_length;
   } while( rr > 0 && de->d_length > 0 && de->d_handle == 0 );

   if(rr > 0 && de->d_length > 0)
       return de->d_name;
   else 
       return 0;
}
