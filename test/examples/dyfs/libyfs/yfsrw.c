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

/*
 *	'hardware' part of libyfs.o
 */

#include <fcntl.h>
#include <errno.h>
#include <sys/file.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "disk.h"
#include "file.h"

unsigned YfsError;

static fd;
static init = 1;

#define DEFAULT_PROT 0666
#ifdef hpux
#define CREAT_PROT S_ENFMT | DEFAULT_PROT
#else
#define CREAT_PROT DEFAULT_PROT
#endif

#ifdef CacheMonitor
static int singleReads=0;
static int singleWrites=0;
static int multiReads=0;
static int multiWrites=0;
static short blocks_read[NUM_OF_BLOCKS];
static short blocks_written[NUM_OF_BLOCKS];
#endif

Initialize()
  {
    init = 0;
    fd = open(DISK_FILE_NAME, O_RDWR | O_CREAT, CREAT_PROT);
    if(fd == -1)
      exit(-1);
#if defined(hpux) || defined(__svr4__)
    lockf(fd, F_LOCK, 0);
#else
    flock(fd, LOCK_EX);
#endif
  }

DoDisk(int blknum, caddr_t addr, int blks, int op)
  {
    if(init)
      Initialize();
    if(blknum < 0 || blknum >= NUM_OF_BLOCKS)
      return EINVAL;
    else if(blknum + blks < 0 || blknum + blks > NUM_OF_BLOCKS)
      return EINVAL;
    else if(blks == 0 || blks > MAX_BLOCKS_PER_OP)
      return EINVAL;
    else if(lseek(fd, blknum * BLOCK_SIZE, 0) == -1)
      return ESPIPE;
    else if(op && write(fd, addr, blks * BLOCK_SIZE) == -1)
      return EIO;
    else if(!op && read(fd, addr, blks * BLOCK_SIZE) == -1)
      return EIO;
#ifdef CacheMonitor
    if (op && blks>1) multiWrites+=blks;
    if (op && blks==1) singleWrites++;
    if (!op && blks>1) multiReads+=blks;
    if (!op && blks==1) singleReads++;
    if (op)
	for(;blks;)
	    blocks_written[blknum + --blks]++;
    if (!op)
	for(;blks;)
	    blocks_read[blknum + --blks]++;
#endif
    return 0;
  }

ReadDisk(int blknum, caddr_t addr, int blks)
  {
    int err = DoDisk(blknum, addr, blks, 0);
    if(err)
      errno = err;
    return err? -1: 0;
  }

WriteDisk(int blknum, caddr_t addr, int blks)
  {
    int err = DoDisk(blknum, addr, blks, 1);
    if(err)
      errno = err;
    return err? -1: 0;
  }

