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
*  ops.c			Server operation functions 
*
********************************************************************************
*******************************************************************************/

#include <stdio.h>
#include "file.h"
#include "cache.h"


/*******************************************************************************
* int IsDir ()
*
*	This function checks to see if the open file is a dir.
*******************************************************************************/

int IsDir ()
{
  if (!(*Dir))
    return 0;
    
  YfsError = EISDIR;
  return -1;
}


/*******************************************************************************
* int IsNotDir ()
*
*	This function checks to see if the open file is a dir.
*******************************************************************************/

int IsNotDir ()
{
  if (*Dir)
    return 0;
    
  YfsError = ENOTDIR;
  return -1;
}


/*******************************************************************************
* Handle Lookup (char * Name, Handle Dir)
*
*    This function performs a lookup on a file or directory.  The filehandle
* associated with the Name is returned; if the file doesn't exist, an error
* occurs.
*******************************************************************************/

Handle Lookup (char * Name, Handle Dir)
{
  char PartialName [129];
  int NamePos, Count;
  Handle Current;

 
  
  if (Name[0])
    if (Name[0] == '/')          /* name starts with '/', Dir is irrelevant */
    {
      Current = 1;               /* 1 is the root file handle */
      NamePos = 1;
    }
    else
    {
      Current = Dir;             /* Set to search for Name in current Dir */
      NamePos = 0;
    }
  else                           /* If name is NULL, return error */
  {
    YfsError = EINVAL;
     return (Handle) -1;
  }
  
  while (Current != ((Handle) -1) && Name[NamePos]) 
  {
    Count = 0;
    while (Name[NamePos] != 0 && Name[NamePos] != '/' && Count < 128)

/* parse the Name if it started with '/'.  Extract the string between */
/*   the first two '/'s. */

    {
      PartialName[Count++] = Name[NamePos];  /* Check these chars for validity */
      NamePos++;
    }
    
    if (Count == 0 || Count == 128)          /* if Name == NULL or Name > 128 */
    {                                        /*   error */
      YfsError = EINVAL;
      return (Handle) -1;
    }

    PartialName[Count] = 0;                 /* get ready to extract next string */
    
    if (Name[NamePos] == '/')               /* move past the '/' */
      NamePos++;
    
    if ( Access (Current) == -1  ||
         IsNotDir ())	         	    /* if unable to access the */
    {					    /*   the directory, error */
      YfsError = ENOENT;
      return (Handle) -1;
    }
      
    Current = FindFilename (PartialName);   /* search for PartialName in dir */
                                            /* that's currently accessed */
                                            /* couldn't find -> path not valid */
  }
  
  
  return Current;               /* after entire Name is parsed, return */
}


/*******************************************************************************
* int CreateFile (char * filename, Handle dir)
* 
*      This function creates a new file with the name filename and in the
* parent directory dir.
*******************************************************************************/

int CreateFile (char * filename, Handle dir)
{
  int loop = 0;
  Handle newfile;

  
  while ( filename[loop] != 0 && loop < 128 )  /* check for invalid chars */
    if ( filename[loop++] == '/' )             /* if '/', then error */
    {
      YfsError = EINVAL;
      return -1;
    }

  if (loop == 0 || loop == 128)                /* check for NULL or long filename */
  {
    YfsError = EINVAL;
    return -1;
  }
  
  newfile = MakeFile ();                       /* create a new, blank Handle */
  
  if ( newfile == -1 )                         /* if not created, error */
    return -1;

/* Fill in handle and write it to disk */

  *LinkCount = 1;
  TimeStamp ();                                /* time stamp it */
  Update ();                                   /* write all the new info to disk */

/* If either of the following checks succeeds, then CreateFile failed. */
/*   Delete the Handle, write info to disk, and error. */

  if ( Access (dir) == -1 		||     /* cannot access dir? */
       IsNotDir ()			||
       FindFilename (filename) != -1 	||     /* filename used in dir already? */
       AddFilename (newfile, filename) == -1)  /* cannot add filename? */
  {
    Access (newfile);
    DeleteFile ();
    Update ();
    return -1;
  }

  TimeStamp ();                                /* time stamp the parent dir */
  Update ();                                   /* write info to disk (stamp!) */
  return 0;
}


/*******************************************************************************
* int CreateLink (char * linkname, Handle dir, Handle file)
*
*      This function creates a hard link linkname in the directory dir to the
* file specified by file.
*******************************************************************************/

int CreateLink (char * linkname, Handle dir, Handle file)
{
  int loop = 0;


  while ( linkname[loop] != 0 && loop < 128 )  /* check for invalid chars */
    if ( linkname[loop++] == '/' )             /* if '/', error */
    {
      YfsError = EINVAL;
      return -1;
    }

/* if file name is the wrong length, error */

  if (loop == 0 || loop == 128)
  {
    YfsError = EINVAL;
    return -1;
  }

/* check if the file exists == is accessible */

  if ( Access (file) == -1 )
    return -1;

/* Make sure that the file is not a dir */

  if (IsDir ())
    return -1;

/* Increment link count in Handle and write back to disk */

  (*LinkCount)++;

  Update ();

/* Check for valid directory, existing file, and attempt */
/*   to add the file.  If any fail, delete newly created link. */

  if ( Access (dir) == -1 		||
       IsNotDir ()			||
       FindFilename (linkname) != -1 	||
       AddFilename (file, linkname) == -1)
  {
    Access (file);
    DeleteFile ();                          /* delete the newly created entry */
    Update ();                              /* write info back to disk */
    return -1;
  }

  TimeStamp ();                             /* time stamp the parent dir */
  Update ();                                /* write back to disk */
  return 0;
}


/*******************************************************************************
* int CreateDir (char * dirname, Handle dir)
*
*      This function creates a directory entry with the name dirname in the
* parent directory specified by dir.
*******************************************************************************/

int CreateDir (char * dirname, Handle dir)
{
  int loop = 0;
  Handle newdir;
  
  
/* might have to allow one slash at the end  ASK ASK ASK ABOUT THIS!!! */
/* check if dirname is valid.  Allow one slash at the end, but no */
/* slashes in between. */

  while ( dirname[loop] != 0 && loop < 128 )
    if ( dirname[loop++] == '/' && dirname[loop+1] != 0 )
    {
      YfsError = EINVAL;
      return -1;
    }

/* if file name is the wrong length, error */

  if (loop == 0 || loop == 128)
  {
    YfsError = EINVAL;
    return -1;
  }
  
  newdir = MakeFile ();                      /* create empty new dir */
  
  if ( newdir == -1 )                        /* if not created, error */
    return -1;

/* Fill in handle and '.' and '..'.  Write back to disk. */

  *Dir = 1;

  if (AddFilename ( newdir, ".") == -1 ||
      AddFilename ( dir, "..")   == -1)
  {
    DeleteFile ();
    return -1;
  }

  TimeStamp ();                              /* time stamp the new dir */
  Update ();                                 /* write back to disk */

/* check for valid directory, existing file, and attempt to add */
/*   the directory.  If either fails, delete the new directory */
/*   and write back to disk. */

  if ( Access (dir) == -1 		||
       IsNotDir ()			||
       FindFilename (dirname) != -1 	||
       AddFilename (newdir, dirname) == -1)
  {
    Access (newdir);
    DeleteFile ();
    Update ();
    return -1;
  }

  TimeStamp ();                             /* time stamp the parent dir */
  Update ();                                /* write back to disk */
  return 0;
}


/*******************************************************************************
* int Delete ( Handle file, Handle dir)
*
*      This function removes the file or the directory specified by file.
* dir is the directory where the file to be deleted resides.
*******************************************************************************/

int Delete ( char * name, Handle dir)
{
  Handle file;

 
/* Try to access the file to be deleted */

  file = Lookup (name, dir);
  if ( file == -1 || Access (file) == -1 ) 
    return -1;
  
/*  Check that if a dir, it is an empty dir */

  if (IsDir() && *DirEntryCount > 2)
  {
    YfsError = ENOTEMPTY;
    return -1;
  }

/* Check if the parent directory exists.  Attempt to remove the file from */
/*   dir.  RemoveFilename will error when file does not exist in dir. */

  if ( Access (dir) == -1 	|| 
       IsNotDir ()		||
       RemoveFilename (name) == -1 )
    return -1;
  
  (*DirEntryCount)--;
 
  TimeStamp ();          /* time stamp the parent directory */
  Update ();             /* write back */
  
  Access (file);
  DeleteFile ();	/* just update the Handle table, and free blocks */
  Update ();		/* writes back the Handle if one exists */
  
  return 0;
}


/*******************************************************************************
* int Read (Handle file, int count, int offset, char * buffer)
*
*      This function reads count bytes from the file or dir specified by file
* into the buffer.  offset specifies the starting location.  The function
* returns the number of bytes read.
*******************************************************************************/

int Read (Handle file, int count, int offset, char * buffer)
{
  int ReadSize;

  
/* check if file exists */

  if ( Access (file) == -1 )
    return -1;

/* If count is zero, exit (don't touch file) */

  if (count == 0)
    return 0;
    
/* Check for valid offset */

  if (offset < 0)
  {
    YfsError = EINVAL;
    return -1;
  }

/* check the offset to make sure it's not past the end of file */

  ReadSize = (*FileSize) - offset < count ? (*FileSize) - offset : count;
  

  if (ReadSize <= 0)
  {
    YfsError = ESPIPE;
    return -1;
  }

/* Attempt to read in the data */

  return FileRead (ReadSize, offset, buffer);
}


/*******************************************************************************
* int Write (Handle file, int count, int offset, char * buffer)
*
*      This function writes count bytes from the buffer into the file, where
* offset specifies the starting location.  The function returns the number
* of bytes written; we either write the entire count bytes, or not write the
* file at all.
*******************************************************************************/

int Write (Handle file, int count, int offset, char * buffer)
{
   
/* Attempt to access file and make sure it's not a dir */

  if ( Access (file) == -1 || IsDir ())
  {       
    YfsError = EISDIR;
    return -1;
  }
  
/* If count is zero, exit (don't touch file) */

  if (count == 0)
    return 0;
    
/* Check for valid offset */

  if (offset < 0)
  {
    YfsError = EINVAL;
    return -1;
  }

/* check the offset to make sure it's not past the end of file */

  if (*FileSize < offset)
  {
     YfsError = ESPIPE;
     return -1;
  }

/* Attempt to write out the data */

  if ( FileWrite (count, offset, buffer) == -1 )
  {
   return -1;
  }
  
  TimeStamp ();                   /* time stamp the file */
  Update ();                      /* write back */
  
  return count;
}


/*******************************************************************************
* int Sync (Handle file)
*
*      This function ensures that the info in the cache corresponding to the
* file is consistent with the contents of file on disk.
*******************************************************************************/

extern int UniqueNumber;
extern int FreeBlockCount;
extern int FreeBlocks[4096];

int Sync (Handle fh)
{
  char Data[512];

  if (fh > 0)
    return CacheSync (fh);
  else
  {
  
    if (CacheReadDisk ((int) -fh, Data, 1, 0) == -1)
      return -1;

    PrintBlock( Data);

    return 0;
  }
}


int PrintBlock (char buf[512])
{
  int i, j;

/*// Test code */
      
  for (i = 0; i < 32; i++)
  {
    for (j = 0; j < 16; j++)
      printf ("%x%x%s", ((unsigned )buf[i*16 + j] / 16) % 16, (unsigned )buf[i*16 + j] % 16, j == 3 || j == 7 || j == 11 ? "  " : " "); 
    printf ("    ");
    for (j = 0; j < 16; j++)
      printf ("%c", buf[i*16 + j] % 128 > 32 ? buf[i*16 + j] % 128: '`'); 
    printf ("\n");
  }
  
  printf ("Unique number:  %i\n", UniqueNumber);
  printf ("Next free blocks:  %i %i %i\n", FreeBlocks[FreeBlockCount - 1],FreeBlocks[FreeBlockCount - 2],FreeBlocks[FreeBlockCount - 3]);
  
  return 0;
}


/*******************************************************************************
* int SyncDisk ()
*
*      This function ensures that the info on disk is consistent with the info
* maintained in the file system in main memory.
*******************************************************************************/

int SyncDisk ()
{
  int count;

  
  for (count = 4096; count--;)
    CacheSync (count);
    
  return 0;
}


/*******************************************************************************
* int Shutdown (void)
*
*      This function requests that the file system perform an orderly shutdown.
*******************************************************************************/

int Shutdown (void)
{

  return SyncDisk ();

}


/*******************************************************************************
* int Stamp (Handle file)
*
*     This function returns the time of last modification of the file (which
* may be a file or dir).
*******************************************************************************/

int Stamp (Handle file)
{
  
  if ( Access (file) == -1 )
    return -1;
    
/* Return the time stamp */

  return *Time;
}


/*******************************************************************************
* int Stat (Handle file)
*
*     This function returns the number of bytes contained in file.
*******************************************************************************/

int Stat (Handle file)
{
  
  if ( Access (file) == -1 )      /* attempt to access file */
    return -1;
  
/* Return the size info */

  YfsError = *Dir;
  return *FileSize;
}


