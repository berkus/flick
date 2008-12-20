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
#include <stdlib.h>
#include "file.h"
#include "cache.h"

/*******************************************************************************
*
* Block is our basic cache structure.
*
*******************************************************************************/

typedef struct Block
{
  char * data;
  
  int dirty;  
  int block;
  int file;
  
  struct Block * older;
  struct Block * newer;
  
  struct Block * nextfile;
  struct Block * lastfile;
} Block;

/*******************************************************************************
*
* Few global definitions.
*
*******************************************************************************/

Block cache[512];
typedef Block * BP;

BP map[4096];
BP file[4096];

Block * oldest, * newest;

Block * hit;

/*******************************************************************************
*
* int InitCache ()
*
*	This function intializes our 512 block cache.  Returns 0 on success.
*******************************************************************************/

int InitCache ()
{
  int count;
  
/* Initialize the map and file */

  for (count = 4096; count--;)
  {
    map[count] = NULL;
    file[count] = NULL;
  } 
  
/* Initialize all 512 blocks and allocate all necessary memory */

  for (count = 512; count--;)
  {
    cache[count].data = malloc(512);
    
    if (cache[count].data == NULL)
    {
     return -1;
    }
  
    cache[count].block = -1;
    cache[count].file = -1;
    
/* Initialize a sorted linked lists for LRU replacement and lists associated */
/*    with each file */

    if (count != 0)
      cache[count].older = &(cache[count-1]);
    else
      cache[count].older = NULL;
      
    if (count != 511)
      cache[count].newer = &(cache[count+1]);
    else
      cache[count].newer = NULL;
      
    cache[count].nextfile = NULL;
    cache[count].lastfile = NULL;
    
    cache[count].dirty = 0;    
  }
  
  oldest = &(cache[0]);
  newest = &(cache[511]);
  
  return 0;
}

/*******************************************************************************
*
* char * TestHit (int b, int f)
*
*	This function tests for a cache hit.  If indeed a cache hit, place
* the block at the "newest" end of our queue and return the associated data.
* Otherwise, return NULL.
*******************************************************************************/

char * TestHit (int b, int f)
{

/* Check if block is already in the cache */

  if (map[b] != NULL)
  {

/* Modify the links; remove oldest from the cache list */

    if (map[b]->older != NULL)
      map[b]->older->newer = map[b]->newer;
    else
      oldest = map[b]->newer;

/* Remove the Block structure from the cache linked list */
      
    if (map[b]->newer != NULL)
      map[b]->newer->older = map[b]->older;
    else
      newest = map[b]->older;

/* Move the Block structure to the newest position */

    newest->newer = map[b];
    map[b]->older = newest;
    map[b]->newer = NULL;
    newest = map[b];

/* Remove map[b] from the previous file cache entry list */

    if ( map[b]->nextfile != NULL )
      map[b]->nextfile->lastfile = map[b]->lastfile;
    if ( map[b]->lastfile != NULL )
      map[b]->lastfile->nextfile = map[b]->nextfile;
    else
      if ( map[b]->file >= 0 )
        file[ map[b]->file ] = map[b]->nextfile;

/* Link map[b] into the file cache entry list */

    map[b]->lastfile = NULL;
    map[b]->nextfile = file[f];
    file[f] = map[b];
    if (file[f]->nextfile != NULL)
      file[f]->nextfile->lastfile = file[f];

/* Set file, data associated with Block map[b].  Assign hit to map[b]. */
    
    map[b]->file = f;
    
    hit = map[b];
    return map[b]->data;
  }
  else
    return NULL;
}

/*******************************************************************************
*
* char * CacheBlock (int b, int f)
*
*	This function replaces the Least Recently Used block with the new one
* (with Block b).  f specifies the filehandle.  
*******************************************************************************/

char * CacheBlock (int b, int f)
{
  Block * temp;

/* Do we need to write the old block back to disk? */

  if ((oldest->block >= 0) && oldest->dirty)
    if (WriteDisk (oldest->block, oldest->data, 1) == -1)
      return (char *) -1;

/* Oldest Block no longer in the cache */
    
  if (oldest->block >= 0)
    map[oldest->block] = NULL;

/* Remove the oldest Block from the file cache list.    */

  if (oldest->nextfile != NULL)
    oldest->nextfile->lastfile = oldest->lastfile;
  if (oldest->lastfile != NULL)
    oldest->lastfile->nextfile = oldest->nextfile;
  else if (oldest->file >= 0)
    file[oldest->file] = oldest->nextfile;
        
/* Fun with links -- move the Block structure from the "oldest" position */
/*    to the "newest" position */

  temp = oldest->newer;
  oldest->newer->older = NULL;
  oldest->older = newest;
  oldest->newer = NULL;    
  newest->newer = oldest;
  newest = oldest;
  oldest = temp;
 
/* Place the block into the file cache list. */
   
  newest->lastfile = NULL;
  newest->nextfile = file[f];
  file[f] = newest;
  if (file[f]->nextfile != NULL)
    file[f]->nextfile->lastfile = file[f];

/* Assign some variables */
    
  newest->block = b;
  newest->dirty = 0;
  newest->file = f;

/* Set the hit Block and map */
    
  hit = newest;
  map[b] = newest;
  return newest->data;
}

/*******************************************************************************
*
* int CacheReadDisk ( int block_num, char *data, int blocks, int f)
*
*	This function reads a disk block(s) starting at disk block block_num
* and places them into buffer data.  Returns 0 on success.
*******************************************************************************/

int CacheReadDisk ( int block_num, char *data, int blocks, int f)
{
  char * buf;

/* Check if it's a hit */
    
  f %= 4096;
  buf = TestHit (block_num, f);
  
/* Yeah! */


/* Not a cache hit therefore cache the block. */
 
  if (buf == NULL)
  {
    buf = CacheBlock (block_num, f);
    if (buf != (char *) -1 && ReadDisk (newest->block, newest->data, 1) == -1)
    {
      YfsError = EIO;
      return -1;
    }
  }
    
/* Incorrect -> return error */

  if (buf == (char *) -1)
  {
    hit->block = -1;
    YfsError = EIO;
    return -1;
  }
  
/* Cp the local buffer into data */

  bcopy (buf, data, 512);
  
  return 0;
}

/*******************************************************************************
*
* int CacheWriteDisk ( int block_num, char *data, int blocks, int f)
*
*	This function writes disk blocks starting at disk block block_num.
* The data to be written to disk is available in data.  Returns 0 on success.
*******************************************************************************/

int CacheWriteDisk ( int block_num, char *data, int blocks, int f)
{
  char * buf;
  
  f %= 4096;

/* Check if this is a hit. */
  
  buf = TestHit (block_num, f);

/* If not a cache hit, attempt to cache that block. */
  
  if (buf == NULL)
  {
    buf = CacheBlock (block_num, f);
  }

/* If incorrect buffer, return error. */

  if (buf == (char *) -1)
  {
    hit->block = -1;
    YfsError = EIO;
    return -1;
  }    
    
/* Set dirty bit */

  hit->dirty = 1;
  
/* Copy data */

  bcopy (data, buf, 512);
  
  return 0;
}

/*******************************************************************************
*
* int CacheSync (int f)
*
*	This function makes sure that info in cache is consistent with memory.
*******************************************************************************/

int CacheSync (int f)
{
  Block * temp;
  f %= 4096;
  
  if (file[f] == NULL)
    return 0;
    
  temp = file[f];
  
  while (temp != NULL)
  {
    if (temp->dirty)
    {
      if (WriteDisk (temp->block, temp->data, 1) == -1)
      {
        YfsError = EIO;
        return -1;
      }
    }
    else
      temp->dirty = 0;
    temp = temp->nextfile;
  }
  return 0;
}


/*******************************************************************************
*
* void CacheClean (int f)
*
*	This function is a helper to delete.  For all the blocks belonging to
* deleted file, there is no need to write them back to disk so mark them clean.
*******************************************************************************/

void CacheClean (int f)
{
  Block * temp;
  f %= 4096;
  
  if (file[f] == NULL)
    return;
    
  temp = file[f];

/* Mark all  */

  while (temp != NULL)
  {
    temp->dirty = 0;
    temp = temp->nextfile;
  }
}

