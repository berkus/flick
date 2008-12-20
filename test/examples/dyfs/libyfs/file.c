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
*  file.c			Low level file operations
*
********************************************************************************
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <string.h>
#include "disk.h"
#include "file.h"
#include "cache.h"

#define bzero(a, b)	memset(a, 0, b)

/*******************************************************************************
* Data space for in-memory copy of on-disk structures
*******************************************************************************/

/* Base block buffer area */

char BaseBlockBuffer[512];

/* File handle number is placed first */

int * FileHandle 	= (int *) (BaseBlockBuffer);
int * Time 		= (int *) (BaseBlockBuffer) + 1;
/* Dir == 1 when it's a dir */
int * Dir 		= (int *) (BaseBlockBuffer) + 2;  
int * DirEntryCount 	= (int *) (BaseBlockBuffer) + 3;
int * FileSize 		= (int *) (BaseBlockBuffer) + 4;
int * LinkCount		= (int *) (BaseBlockBuffer) + 5;
int * Spare1 		= (int *) (BaseBlockBuffer) + 6;
int * Spare2 		= (int *) (BaseBlockBuffer) + 7;

/* We allow the first 31 blocks to hold the miscellanous information */
/*   above.  The Direct block addresses begin at the 33rd byte, and */
/*   the Indirect block addresses begin at the 57th byte. */

int * Map 		= (int *) (BaseBlockBuffer + 32);
int * Direct		= (int *) (BaseBlockBuffer + 32);
int * Indirect		= (int *) (BaseBlockBuffer + 32) + 24;

/* The data of the file starts at the 257th byte. */

char * FirstData 	= BaseBlockBuffer + 256;
int * FirstDataInt	= (int *) (BaseBlockBuffer + 256);


/* Indirect block mapping buffer area */

char IndirectBuffer [512];

int * IndirectDirect = (int *) IndirectBuffer;


/* Temporary block buffer */

char Block [512];


/* Temporary data buffer */

char Data [2048];
int * DataInt = (int *) Data;


/*******************************************************************************
* In memory free list, handle list, and other local data
*******************************************************************************/

int UniqueNumber;

Handle HandleList [4096];

int FreeBlockCount;
int FreeBlocks[4096];
int UsedBlocks[4096];

/*******************************************************************************
* int BadHandle (Handle h)
*
*	This function validates a file handle.
*******************************************************************************/

int BadHandle (Handle h)
{
  if (h == HandleList[h % 4096])
    return 0;
    
  YfsError = EBADF;
  return -1;
}


/*******************************************************************************
* int Access (Handle h)
*
*	Sets up a file for low level operations by reading in its file
* information and disk mappings into the BaseBlockBuffer.
*******************************************************************************/

int Access (Handle h)
{
 
/* Make sure the file handle is valid */
  
  if (BadHandle (h))
    return -1;

/* If we have already loaded the handle, return */
    
  if (h == *FileHandle)
    return 0;

/* Attempt to load the new file handle */
    
  if (CacheReadDisk (h % 4096, BaseBlockBuffer, 1, h))
  {
    *FileHandle = 0;
    return -1;
  }

/* Return success */
  
  return 0;
}

/*******************************************************************************
* int Update ()
*
*        This function writes the in-memory block back onto the disk (cache).
*******************************************************************************/

int Update ()
{
  
/* Make sure that we have a valid file handle in memory */

    if (BadHandle (*FileHandle))
      return -1;

/* Attempt to write the file handle */
    
  if (CacheWriteDisk (*FileHandle % 4096, BaseBlockBuffer, 1, *FileHandle))
    return -1;

/* Return success */
  
  return 0;
}

/*******************************************************************************
* Handle MakeFile ()
*
*        This function creates a Handle for a file; however, it does not
* fill in any file specific information except the FileNumber.
*******************************************************************************/

Handle MakeFile ()
{
  
/* Make sure there is space on the disk */

  if (FreeBlockCount == 0)
  {
    YfsError = ENOSPC;
    return -1;
  }

/* Clean up the base block */
  
  bzero (BaseBlockBuffer, 512);
  
/* Allocate a file handle (disk block) for this file */
  
  *FileHandle = ((++UniqueNumber) * 4096) + FreeBlocks[--FreeBlockCount];
  
  HandleList[*FileHandle % 4096] = *FileHandle;
  
/* Return success */
  
  return *FileHandle;
}

/*******************************************************************************
* int DeleteFile ()
*
*       This function deletes the file whose handle information is currently
* in memory.  It returns success if delete succeeded, or if the number of
* links was greater than 1.
*******************************************************************************/

int DeleteFile ()
{
  int pool, loop;
  
 
/* Make sure that we have a valid file handle in memory */

  if (BadHandle (*FileHandle))
    return -1;
      
/* decrement the link count, and return success */

  if ( (*LinkCount)-- > 1 )    
    return 0;

/* Remove the file from the HandleList */

  HandleList[(*FileHandle) % 4096] = 0;

/* Traverse through the Direct and Indirect Mappings.  Add all the blocks */
/*   that the file uses to the FreeBlocks array and increment the */
/*   FreeBlockCount */

  loop = -1;
  while ( Direct[++loop] && loop < 24 )
    FreeBlocks[FreeBlockCount++] = Direct[loop];

  loop = -1;
  while ( Indirect[++loop] && loop < 32 )
  {
    if ( CacheReadDisk( Indirect[loop], IndirectBuffer, 1, *FileHandle) == -1 )
    {
      FreeBlocks[FreeBlockCount++] = (*FileHandle) % 4096;
      *FileHandle = 0;
      return -1;
    }
    pool = -1;
    while ( IndirectDirect[++pool] && pool < 128 )
      FreeBlocks[FreeBlockCount++] = IndirectDirect[pool];
    FreeBlocks[FreeBlockCount++] = Indirect[loop];
  }

/* Add the BaseBlockBuffer to the list of free blocks */

  FreeBlocks[FreeBlockCount++] = (*FileHandle) % 4096;

/* Invalidate the filehandle */

  CacheClean (*FileHandle);
  *FileHandle = 0;

  return 0;
}

/*******************************************************************************
* Handle FindFilename (char * name)
*
*       This function searches the in-memory directory for a file named name.
*******************************************************************************/

Handle FindFilename (char * name)
{

/* A directory entry size is determined in the following way: */
/*   1 byte for the Null character, 4 bytes for the length, 4 bytes for the */
/*   Handle, and 3 bytes for correct round off. */

  int SearchSize = ((strlen (name) + 1 + 4 + 4 + 15) / 16) * 16;
  int pos = 0;

/* Make sure that we have a valid file handle in memory */

  if (BadHandle (*FileHandle))
    return -1;

/* Traverse the directory looking for the entry.  Read in 128+20 bytes, which */
/* is more than necessary. */
  
  while (pos < *FileSize)
  {
    if (FileRead (160, pos, Data) == -1)
      return -1;
    if ( DataInt[0] == 0 )
      break;
    if (SearchSize == DataInt[0] && DataInt[1] && strcmp (Data + 8, name) == 0)
      return DataInt[1];
    pos += DataInt[0];
  }  
  
  YfsError = ENOENT;
  return -1;
}


/*******************************************************************************
* int AddFilename (Handle h, char * name)
*
*
*******************************************************************************/

int AddFilename (Handle h, char * name)
{
 
/* A directory entry size is determined in the following way: */
/*   1 byte for the Null character, 4 bytes for the length, 4 bytes for the */
/*   Handle, and 3 bytes for correct round off. */

  int SearchSize = ((strlen (name) + 1 + 4 + 4 + 15) / 16) * 16;
  int pos = 0;

/* Make sure that we have a valid file handle in memory */

  if (BadHandle (h))
    return -1;

/* Traverse the directory looking for the entry.  Read in 128+20 bytes, which */
/* is more than necessary. */
  
  while (pos < *FileSize)
  {
    if (FileRead (128 + 20, pos, Data) == -1)
      return -1;
    if ( DataInt[0] == 0 || (DataInt[1] == 0 && DataInt[0] == SearchSize))
      break;
    pos += DataInt[0];
  }  

/* Add the entry */
  
  DataInt[0] = SearchSize;
  DataInt[1] = h;
  bzero ( Data + 8, SearchSize - 8);
  strcpy (Data + 8, name);
  
  if (FileWrite (DataInt[0], pos, Data) == -1)
    return -1;   
    
  (*DirEntryCount)++;  

  return 0; 
}


/*******************************************************************************
* int RemoveFilename (char *)
*
*
*******************************************************************************/

int RemoveFilename (char *name)
{
  int pos = 0;
  

/* Make sure that we have a valid file handle in memory */

  if (BadHandle (*FileHandle))
    return -1;
  
/* Traverse the directory looking for the entry.  Read in 128+20 bytes, which */
/* is more than necessary. */
  
   while (pos < *FileSize)
  {
    if (FileRead (128 + 20, pos, Data) == -1)
      return -1;
    if ( DataInt[0] == 0 )
      break;

/* Delete the entry by zeroing out the Handle field */

    if (!strcmp( name, Data+8))
    {
      DataInt[1] = 0;
      if (FileWrite (DataInt[0], pos, Data) == -1)
        return -1;
      return 0;
    }
    pos += DataInt[0];
  }   
 
  YfsError = ENOENT;  
  return -1;
}


/*******************************************************************************
* int TimeStamp ()
*
*	This function returns the time stamp for the open file.
*******************************************************************************/

int TimeStamp ()
{
   
/* Make sure that we have a valid file handle in memory */

    if (BadHandle (*FileHandle))
      return -1;

/* Stamp in the time */

  time((time_t *) Time);
  
  return 0;
}

/*******************************************************************************
* int Xlate ( int offset, int allocate ) 
*
*	This function will return 0 on success.  It associates a block
* with an offset, and if necessary, it allocates a block(s).
* If the disk read failed, return -1.  If no block is mapped in, return 1
* or allocate a block.
*******************************************************************************/

int OffsetBlock, OffsetByte;

int Xlate ( int offset, int allocate ) 
{
  int TempBlock, Temp;

/* offset located in the Handle block (in the latter 256 byte part) */
  
  if ( offset < 0x100 )
  {
    OffsetBlock = 0;
    OffsetByte = offset + 0x100;
    return 0;
  }
  
  offset = offset - 0x100;  
  Temp = offset / BLOCK_SIZE;

/* See if the offset is located in one of the direct blocks */
  
  if ( offset < 24 * BLOCK_SIZE )
  {
    if ( Direct[Temp] == 0 && allocate && FreeBlockCount )
    {
      Direct[Temp] = FreeBlocks[--FreeBlockCount];
   }
    OffsetBlock = Direct[Temp];
    OffsetByte = offset % BLOCK_SIZE;
    return ( OffsetBlock == 0 ? -1 : 0 );
  }
  
  offset = offset - 24 * BLOCK_SIZE;
  Temp = offset / (128 * BLOCK_SIZE);

/* The offset is located in one of the blocks with one level of indirection. */
/*    Just a little more calculations are necessary. */
  
  if ( Indirect[Temp] == 0 && allocate  && FreeBlockCount )
  {
    Indirect[Temp] = FreeBlocks[--FreeBlockCount];
    bzero( IndirectBuffer, BLOCK_SIZE);
    if ( CacheWriteDisk( Indirect[Temp], IndirectBuffer, 1, *FileHandle) == -1 )
      return -1;
  }  

  TempBlock = Indirect[Temp];
  if ( TempBlock == 0 )
    return -1;
  
  if ( CacheReadDisk( TempBlock, IndirectBuffer, 1, *FileHandle) == -1 )
    return -1;
  
  offset = offset % ( 128 * BLOCK_SIZE );
  Temp = offset / BLOCK_SIZE; 
  
  if ( IndirectDirect[Temp] == 0 && allocate && FreeBlockCount )
  {
    IndirectDirect[Temp] = FreeBlocks[--FreeBlockCount];
    if ( CacheWriteDisk( TempBlock, IndirectBuffer, 1, *FileHandle) == -1 )
      return -1;
  }
      
  OffsetBlock = IndirectDirect[Temp];
  OffsetByte = offset % BLOCK_SIZE;
  return ( OffsetBlock == 0 ? 1 : 0 );
}

/*******************************************************************************
* int FileRead (int count, int offset, char *buffer)
*
*      This function reads count bytes from the file whose inode information
* is currently in memory ( BaseBlockBuffer).  It starts at the offset byte,
* in the file, and reads in the data into the buffer.  If offset+count > size,
* read in only size-offset bytes.
*******************************************************************************/

int FileRead (int count, int offset, char *buffer)
{
  int length, oldcount;
  char * readbuffer;
  
  
/* Make sure that we have a valid file handle in memory */

  if (BadHandle (*FileHandle))
    return -1;
    
/* Clip the length */

  if (offset + count > *FileSize)
    count = *FileSize - offset;
    
  oldcount = count;
      
  while ( count > 0 )
  {
    if ( Xlate ( offset, 0 ) != 0 )
    {
      YfsError = EIO;
      return -1;
    }
   
/* Deal with the three possible cases: */
/*    - read an entire block (512bytes) */
/*    - read part of block starting at offset 0 */
/*    - read block starting at some offset != 0 */

    length = BLOCK_SIZE - OffsetByte > count ? count : BLOCK_SIZE - OffsetByte;
    
    
    readbuffer = length == BLOCK_SIZE ? buffer : Block;

    if (OffsetBlock == 0)
      readbuffer = BaseBlockBuffer;
    else
      if ( CacheReadDisk ( OffsetBlock, readbuffer, 1, *FileHandle ) == -1)
        return -1;
      
    if ( length != BLOCK_SIZE )
      bcopy ( readbuffer + OffsetByte, buffer, length );
    
    count -= length;
    buffer += length;
    offset += length;
  }
  
  return oldcount;
}

/*******************************************************************************
* int FileWrite (int count, int offset, char *buffer)
*
*       This function writes count bytes from the buffer to a to the file
* starting at offset bytes.  The file to be written is currently in memory
* (BaseBlockBuffer).
*******************************************************************************/

int FileWrite (int count, int offset, char *buffer)
{
  int length, oldcount = count;
  int OldUsed, NewUsed;

  
/* Make sure that we have a valid file handle in memory */

  if (BadHandle (*FileHandle))
    return -1;
  
  if ( *FileSize < offset + count )
  {

  
/* the number of data blocks used aside from Handle data */

    OldUsed = (*FileSize - 0x100 ) / BLOCK_SIZE + 1;
    NewUsed = (offset + count - 0x100 ) / BLOCK_SIZE + 1;

/* modified by the number of indirect blocks */

    if ( OldUsed > 24 )
      OldUsed += ( OldUsed - 24 ) / 128 + 1;
    if ( NewUsed > 24 )
      NewUsed += ( NewUsed - 24 ) / 128 + 1; 
      
 
    if (NewUsed - OldUsed > FreeBlockCount)
    {
      YfsError = ENOSPC;
      return -1;
    }
  }
  
/* Assume that we will not get an error due to lack of space */
    
  while ( count > 0 )
  {
    if ( Xlate ( offset, 1 ) )
    {
      YfsError = EIO;
      return -1;
    }
    
    length = BLOCK_SIZE - OffsetByte < count ? BLOCK_SIZE - OffsetByte : count;
          
 
    if ( length != BLOCK_SIZE )
    {
      if (OffsetBlock == 0)
        bcopy ( buffer, BaseBlockBuffer + OffsetByte, length );
      else
      {
        if ( CacheReadDisk ( OffsetBlock, Block, 1, *FileHandle ) == -1)
          return -1;
        bcopy ( buffer, Block + OffsetByte, length );
        if ( CacheWriteDisk ( OffsetBlock, Block, 1, *FileHandle ) == -1)
          return -1;
      }
    }
    else
      if ( CacheWriteDisk ( OffsetBlock, buffer, 1, *FileHandle ) == -1)
        return -1;
       
    count -= length;
    buffer += length;
    offset += length;
  }

  if (offset > *FileSize)
    *FileSize = offset;
  
  return oldcount;
}

/*******************************************************************************
* int Traverse ( Handle h)
*
*        This function traverses through all our blocks and marks them
* as used in the UsedBlocks list.  It is used upon starting the file system.
* Returns -1 on failure.
*******************************************************************************/

int Traverse ( Handle h )
{
  int loop, pool, pos;

/* Determine the current UniqueNumber */

  if (h / 4096 >= UniqueNumber)
    UniqueNumber = h / 4096 + 1;

/* Mark this handle as used */

  HandleList[ h % 4096 ] = h;
  
/* Read in this file's header */
  
  if ( Access (h) == -1 )
    return -1;

/* Mark the header, direct and indirect block as used */
  
  UsedBlocks[ h % 4096 ]++;
  
  for ( loop = 24; loop--;)
    UsedBlocks[Direct [loop] ]++;
  
  for ( loop = 32; loop--;)
  {
    UsedBlocks[Indirect [loop] ]++;
    if ( Indirect[loop] == 0 )
      continue;
      
    if ( CacheReadDisk( Indirect[loop], IndirectBuffer, 1, h) == -1 )
      return -1;

    for ( pool = 128; pool--; )
      UsedBlocks[ IndirectDirect [pool] ]++;
  }

/* If this file is not a directory, return */
   
  if ( ! *Dir )
    return 0;
  
/* Traverse all the entries in this directory which have not been traversed */

  pos = 0;
  while (pos < *FileSize)
  {
    if (FileRead (128 + 20, pos, Data) == -1)
      return -1;
      
    if ( DataInt[0] == 0 )
      break;
    pos += DataInt[0];
      
    if ( HandleList[ DataInt[1] % 4096 ] == 0 )
    {
      if ( Traverse ( DataInt[1] ) == -1) 	/* This closes file h */
        return -1;
    }

    if ( Access (h) == -1 )			/* Reopen file h */
      return -1; 
  } 
  
  return 0; 
}

/*******************************************************************************
* int StartFileSystem ()
*
*       This function is responsible for correct initialization of all our
* lists and global variables.  It traverses through all blocks.  Returns -1
* on failure.
*******************************************************************************/

int StartFileSystem ()
{
  int loop;
  
 
/* Initialize the unique number and trash the currently loaded file handle */
  
  UniqueNumber = 0;
  *FileHandle = 0;

/* Clean up the free block list and the used handle list */
  
  for ( loop = 4096; loop--;)
    HandleList[loop] = UsedBlocks[loop] = 0;

/* Block 0 can never be used, invalidate the entries */
    
  HandleList[0]--;
  UsedBlocks[0]++;

/* Traverse starting at the root directory */

  if ( Traverse (1) == -1 )
    return -1;

/* Mark any unused blocks as free */

  FreeBlockCount = 0;
  
  for ( loop = 4096; loop--;)
    if ( ! UsedBlocks[loop] )
      FreeBlocks[FreeBlockCount++] = loop;

  return 0;
}


/*******************************************************************************
* int FormatDisk ()
*
*        This function formats our disk into 4096 blocks.  It creates the
* root directory in block 1.
*******************************************************************************/

int FormatDisk ()
{
  int i, j;
  SuperBlock * super_blk = (SuperBlock *) Data;
  char buf[BLOCK_SIZE];

/* Attempt to write every block */

  for (i = 0; i < NUM_OF_BLOCKS; i++)
  {
    if (WriteDisk(i, buf, 1) == -1) 
      return -1;
    printf("%d\b", i); 
    j = i;
    while ( j /= 10 )
      putchar('\b');
    fflush(stdout);
  }
  
  putchar('\n');
  
/* Write the symbolic super block */

  bzero ( Data, BLOCK_SIZE );
  strcpy(&(super_blk->label[0]), DEF_LABEL);
  super_blk->sector_size       = BLOCK_SIZE;
  super_blk->sectors_per_track = DEF_SECTORS_PER_TRACK;
  super_blk->num_cylinders     = DEF_NUM_CYLINDERS;
  super_blk->num_platters      = DEF_NUM_PLATTERS;
  super_blk->checksum          = DEF_CHECKSUM;

  if(WriteDisk(0, Data, 1) == -1)
  {
    printf("Cannot write super block\n");
    return -1;
  }
  
  printf("Wrote super block\n");

/* Create the root directory Handle equal to 1 */

  bzero ( BaseBlockBuffer, BLOCK_SIZE );
  
  *FileHandle = 1;
  time((time_t *) Time);
  *Dir = 1;
  *DirEntryCount = 3;
  *FileSize = 32;
  *LinkCount = 0;
  
  FirstDataInt[0] = FirstDataInt[4] = 16;
  FirstDataInt[1] = FirstDataInt[5] = 1;
  FirstData[8] = FirstData[24] = FirstData[25] = '.';

  if(WriteDisk(1, BaseBlockBuffer, 1) == -1)
  {
    printf("Cannot write root directory\n");
    return -1;
  } 
    
  printf("Wrote root directory\n");  
  printf("Formatted %d blocks of size %d\n", NUM_OF_BLOCKS, BLOCK_SIZE);

  return 0;
}

