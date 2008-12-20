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

#ifndef __DISK_H__
#define __DISK_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Default disk information, feel free to change */

#define DEF_SECTOR_SIZE		512
#define	DEF_SECTORS_PER_TRACK	46
#define	DEF_NUM_CYLINDERS	842
#define	DEF_NUM_PLATTERS	1
#define DEF_CHECKSUM		\
(DEF_SECTOR_SIZE+DEF_SECTORS_PER_TRACK+DEF_NUM_CYLINDERS+DEF_NUM_PLATTERS)
#define	DEF_LABEL		"Yalnix disk, ver 1.0"

#define SEEK_TIME_PER_TRACK	20	/* usec */
#define AVG_ROTATIONAL_LATENCY	4000	/* usec */

#define DISK_FILE_NAME		"yaldisk"
#define BLOCK_SIZE		512
#define NUM_OF_BLOCKS		4096

/* The super block */

typedef struct 
{
  char     label[128];
  unsigned sector_size;
  unsigned sectors_per_track;
  unsigned num_cylinders;
  unsigned num_platters;
  unsigned checksum;
}
SuperBlock;


/* Return codes returned by the disk operations */

#define	OK				0
#define	ERROR_BUSY			1
#define	ERROR_SECTOR_NO			2
#define ERROR_DEVICE_SEEK		3
#define ERROR_DEVICE_READ		4
#define ERROR_DEVICE_WRITE		5
#define	ERROR_INTERRUPT_FAILURE_1	6
#define	ERROR_INTERRUPT_FAILURE_2	7

#define MAX_BLOCKS_PER_OP		8

#ifdef __cplusplus
}
#endif

#endif
