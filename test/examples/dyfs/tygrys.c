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
 * C version of tygrys.cc 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/time.h>
#include "ops.h"

/******************************************************************************
*
*        This is our little shell used to test our program.  It supports a
* number of functions.  Use the regular Unix commands to play around with
* our file system.  Enjoy!
*
* original version by Peter Jensen & Aleksandra Kuswik,
* minor additions by Godmar Back
*
******************************************************************************/

void
printinfo( char *name, Handle dir )
{
	char buf[50];
        time_t tm;
	Handle fh = Lookup( name, dir );
	tm = Stamp ( fh );
	strcpy (buf, ctime (&tm));
	buf[24] = 0;
	printf("%d\t%s\t%s\n", Stat(fh), buf, name);
}

static int readline(char *buf, int maxl)
{
        char *p;

        p = buf;
        while (read(0, p, 1) == 1 && *p != '\n' && *p != '\r' && p < buf + maxl)
        {
                write(1, p++, 1);
                if (*(p - 1) == 0x8) /* Backspace, undo the bs and prev. char */                        p-=2;
        }

        *p = '\0';
        return strlen(buf);
}

void 
main(int ac, char *av[])
  {
    char *p, buf[2000], buf2[2000], cwd[200], 
	command[40], arg1[200], arg2[200];
    
    int result;
    unsigned int running = 1, cwdhandle, file1, length;
    int temp, pos;
    FILE * real;
#ifdef DIRECT_LINK
    FormatDisk();
    Init();
#endif
 
    strcpy (cwd, "/");
 
  while (running)
  {
    result = -2;
    printf("%s> ", cwd);
    fflush (stdout);
    
    readline(buf, sizeof(buf));
    printf("\n");
    p = strtok(buf, " ");
    strcpy(command, p);
    if ((p = strtok(0, " ")) != 0) {
	strcpy(arg1, p);
	if ((p = strtok(0, " ")) != 0) 
	    strcpy(arg2, p);
    }

#if 0
/* a UNIX command */
    if(*command == '!') {
	if(command[1])
		system(command+1);
	else {
		printf("Type exit to return to %s!\n", av[0]);
		system("$SHELL");
	}
	result = 0;
    }
#endif

#if 0
/* Format the disk */
    
    if (!strcmp (command, "format"))
    {
      printf("All data will be deleted!\nARE YOU SURE (N/Y) ?");
      if(toupper(*arg1) == 'Y') {
      	result = FormatDisk();
        strcpy (cwd, "/");
      } else
 	result = 0;
    }
#endif
    
/* Create an empty file */
    
    if (!strcmp (command, "make") || !strcmp (command, "touch"))
    {
      cwdhandle = Lookup (cwd, 1);
      result = CreateFile (arg1, cwdhandle);
    }
    
/* Create a link */

    if (!strcmp (command, "makelink") || !strcmp (command, "ln"))
    {
      cwdhandle = Lookup (cwd, 1);
      file1 = Lookup (arg1, cwdhandle);
      result = CreateLink (arg2, cwdhandle, file1);
    }
    
/* Create an empty directory */

    if (!strcmp (command, "makedir") || !strcmp (command, "mkdir"))
    {
      cwdhandle = Lookup (cwd, 1);
      result = CreateDir (arg1, cwdhandle);
    }

/* Show the contents of the block */
    
    if (!strcmp (command, "show"))
    {
      temp = atoi(arg1);
      Sync (-temp);
      result = 0;
    }

/* Delete the file */
    
     if (!strcmp (command, "delete") || !strcmp (command, "del") 
		|| !strcmp (command, "rm"))
    {
      cwdhandle = Lookup (cwd, 1);
      result = Delete (arg1, cwdhandle);
    }

/* list the directory     */

    if (!strcmp (command, "ls") || !strcmp (command, "dir"))
    {
/* #ifdef DIRECT_LINK */
	Handle dir = Lookup( cwd, 0 );
	char *de;
	int offset = 0;
	if(dir != -1) {
	    while((de = ReadDir( dir, &offset )) != 0) 
		printinfo( de, dir);
	    result = 0;
	} else 
	    result = -1;
/* #endif */
    }

/* Change directory     */
   
    if (!strcmp (command, "cd"))
    {
      if (strlen(arg1) == 0)
        continue;
      result = cwdhandle = Lookup (arg1, Lookup (cwd, 1));
      if (result != -1)
        Stat(cwdhandle);
      if (result != -1 && YfsError == 0)
      {
        YfsError= ENOTDIR;
        result = -1;
      }  
      if (result != -1)
      { 
        if (arg1[0] != '/')
        {
          if (cwd[strlen (cwd) -1] != '/')
            strcat (cwd, "/");
          if (!strcmp (arg1, ".") || !strcmp (arg1, "./"))
          {}
          else if (!strcmp (arg1, "..") || !strcmp (arg1, "../"))
          {
            temp = 2;
            while (temp && strlen(cwd))
            {
              if (cwd[strlen(cwd)-1] == '/')
                temp--;
              cwd[strlen(cwd)-1] = 0;
            }
            strcat (cwd, "/");
	  }
	  else
            strcat (cwd, arg1);
             
        }
        else
          strcpy (cwd, arg1);
      }
    }

/* load or copy a Unix file into a file readable by us */
    
    if (!strcmp (command, "load"))
    {
      cwdhandle = Lookup (cwd, 1);
      result = file1 = (Lookup (arg2, cwdhandle));
      if (result == -1)
        CreateFile (arg2, cwdhandle);
        
      result = file1 = (Lookup (arg2, cwdhandle));

      real = fopen (arg1, "r");
        
      pos = 0;
      length = 1;
      while (real != NULL && length > 0 && result != -1)
      {
        length = fread (buf, 1, 1000, real);
        result = Write (file1, length, pos, buf);
        pos += length;
      }
      fclose (real);
    }

/* check if Unix and our file are the same     */

    if (!strcmp (command, "diff") || !strcmp (command, "comp"))
    {
      cwdhandle = Lookup (cwd, 1);
      result = file1 = (Lookup (arg2, cwdhandle));
      
      real = fopen (arg1, "r");
        
      pos = 0;
      length = 1;
      while (real != NULL && length > 0 && result != -1 && pos >= 0)
      {
        length = fread (buf, 1, 1000, real);
        result = Read (file1, length, pos, buf2);
        for (temp = 0; temp < length; temp++)
          if (buf[temp] != buf2[temp])
          {
            printf ("Files differ at +%i\n", pos+temp);
            pos = -100000;
            temp = 100000;
          }
        pos += length;
      }
      fclose (real);
    }

/* Cat the file */
    
    if (!strcmp (command, "cat"))
    {
      cwdhandle = Lookup (cwd, 1);
      result = file1 = (Lookup (arg1, cwdhandle));
      if (result != -1)
        length = Stat (file1);
      
      pos = 0;
      while (pos < length && result != -1)
      {
        result = Read (file1, 1000, pos, buf);
        printf ("res %i\n", result);
        if (result == -1)
          break;
        write(1, buf, result);
        pos += 1000;
      }
    }

/* shutdown our server */
    
    if (!strcmp (command, "shutdown") 
	|| !strcmp (command, "quit") 
	|| !strcmp (command, "q"))
    {
      Shutdown ();
      running = 0;
    }
    
    if (!strcmp (command, "exit") || !strcmp (command, "x"))
    {
      running = 0;
    }
    
    if (running && (result == -2 || !strcmp (command, "help")))
    {
	printf("%s", 
"Possible commands are:

  format	format the disk
  touch 	create an empty file
  ln		make a link
  rm		remove a link
  ls		show directory contents
  cd		change current directory
  mkdir		create a directory
  shutdown	shut server down
  exit		exit w/o shutting down server
  load 		load UNIX file on yalnix disk
  diff 		compare UNIX with Yalnix file
  !cmd		execute UNIX command
");
    }

    if (result == -1)
      printf ("%s\n", strerror(YfsError));
  
  }
}

