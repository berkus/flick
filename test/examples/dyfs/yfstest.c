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

#include "yfs.h"
#include "ops.h"
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif
extern int random();
extern int srandom(int);
#ifdef __cplusplus
}
#endif

#if defined(__svr4__)
#define random	rand
#define srandom(x) srand(x)
#endif

/* Set to FSCK if you have a fs check routine called yfs_fsck() */
#define NOFSCK

/* Set to RANDOMOFFSETS to do reads from random offsets */
#define NORANDOMOFFSETS

/* Number of items to generate (~1/3 will be directories) */
#define MAXITEMS 50

/* Number of writes to perform during the writes phase */
#define NUMWRITES 250

/* Maximum write size */
#define BLOCKSIZ 1000


#define ISGONE 0
#define ISDIR 1
#define ISFILE 2

#define MAXSIZE 10240

struct thing {
  char *pn;
  int whatisit;
  int maxoffset;
  int children;
  int parent;
  int num;
} genedpathname[MAXITEMS+1];
 
int getdiroffset(char *pn)
{
  int i;

  for (i=strlen(pn);i>=0;i--) {
    if (pn[i]=='/')
      return i;
  }

  printf("Error: pathname is not absolute!");
  
  return -1;
}

#define CREATE 0
/* #define DELETE 1
   #define WRITE  2
*/
#define ADIRECTORY 0
#define AFILE   1

void GenData(char *data, int num, int offset, int amount)
{
  int data2[BLOCKSIZ/4+1];
  int data2offset=(offset/4)*4;
  int i;
  for (i=0; i<BLOCKSIZ/4+1; i++) data2[i]=i+num+data2offset/4;
  memcpy(data,((char *)data2)+(offset-data2offset),amount);
}

int do_op(char *pn, int op, int type, int offset, 
	  char *buf, int len,
	  int *actual)
{
  int diroff=getdiroffset(pn);
  char *thedir=(char *)malloc(strlen(pn)+1);
  char *thename=(char *)malloc(strlen(pn)+1);
  Handle handle1;
  Handle handle2;
  int rc;

  if (diroff==0) {
    strcpy(thedir,"/");
    strcpy(thename,&(pn[1]));
  } else {
    strncpy(thedir,pn,diroff);
    thedir[diroff]=0;
    strcpy(thename,&(pn[diroff+1]));
  }

#if 0
  printf("thedir='%s', thename='%s'\n",thedir, thename);
#endif

  if ((handle1=Lookup(thedir,0))==-1) {
    printf("ERROR: Lookup('%s',NULL) failed in do_op\n",thedir);
    return -1;
  }
#if 0
printf("looked up dir thename='%s'\n",thename);
#endif
  switch (op) {
  case CREATE:
    switch(type) {
    case ADIRECTORY:
      return CreateDir(thename,handle1);
      break;
    case AFILE:
      return CreateFile(thename,handle1);
      break;
    default: 
      printf("ERROR: Bad type in create\n");
    }
    break;
  case DELETE:
    if ((handle2=Lookup(thename,handle1))==-1) {
      printf("ERROR: Lookup('%s',handleof('%s')) failed in do_op\n",thename,thedir);
      return -1;
    } else {
      return Delete(thename,handle1);
    }
    break;
  case WRITE:
    if ((handle2=Lookup(thename,handle1))==-1) {
      printf("ERROR: Lookup('%s',handleof('%s')) failed in do_op\n",thename,thedir);
      return -1;
    } else {
      *actual=Write(handle2,len,offset,buf);
      if (*actual==-1)
	return -1;
      else
	return 0;
    }
    break;
  case READ:
    if ((handle2=Lookup(thename,handle1))==-1) {
      printf("ERROR: Lookup('%s',handleof('%s')) failed in do_op\n",thename,thedir);
      return -1;
    } else {
      *actual=Read(handle2,len,offset,buf);
      if (*actual==-1)
	return -1;
      else
	return 0;
    }
    break;
  default:
    printf("ERROR: Bad operation\n");
  }

  free(thedir);
  free(thename);

  return 0;
}
  


int main(int argc, char *argv[])
{
  char buf[BLOCKSIZ], check[BLOCKSIZ];
  int actualbs;
  int rc;
  int i,j;
  int item;
  int curbs;
  int curoffset;

  srandom(1234/*time(NULL) */);

  for (i=0;i<BLOCKSIZ; i++) 
    buf[i]='Z';

  genedpathname[0].pn="";
  genedpathname[0].num=getpid();
  genedpathname[0].whatisit=ISDIR;
  genedpathname[0].children=0;
  genedpathname[0].parent=-1;
  
  printf("Now Generating %d files and directories\n", MAXITEMS);

  for (i=1;i<=MAXITEMS;i++) {

    do {
      item=random()%i;
    } while ((genedpathname[item].whatisit!=ISDIR));
    
    if ((genedpathname[i].pn=(char*)malloc(
	 strlen(genedpathname[item].pn)+20))==NULL) {
      printf("Out of memory\n");
      exit(7);
    }
    genedpathname[item].children++;
    genedpathname[i].parent=item;
    genedpathname[i].children=0;
    genedpathname[i].maxoffset=0;
    genedpathname[i].num=random();
    if (item==0)
    {
      sprintf(genedpathname[i].pn, "%s/%XX%X",genedpathname[item].pn,
              genedpathname[item].num,genedpathname[i].num);
    } else {
      sprintf(genedpathname[i].pn, "%s/%X",genedpathname[item].pn,
              genedpathname[i].num);
    }
    
    genedpathname[i].whatisit=random()%3 < 2 ? ISFILE : ISDIR;
    
    printf("#%i  %s - '%s'\n", i, genedpathname[i].whatisit==ISDIR ? 
	   "DIR " : "FILE", genedpathname[i].pn);
   
    if (genedpathname[i].whatisit==ISDIR)  {
      if (do_op(genedpathname[i].pn,CREATE,ADIRECTORY,0,0,0,0)) {
	printf("ERROR: Failed to create dir %s\n",genedpathname[i].pn);
      }
    } else {
      if (do_op(genedpathname[i].pn,CREATE,AFILE,0,0,0,0)) {
	printf("ERROR: Failed to create file %s\n",genedpathname[i].pn);
      }
    }

#ifdef FSCK

    if ((rc=yfs_fsck())) {
      printf("ERROR: yfs_fsck returned %d\n", rc);
    }

#endif
    
  }

#ifdef FSCK
  puts("(Complete FS Checks Run After Each File)");
#endif

  printf("Now doing %d random writes to %d files\n",
	 NUMWRITES, MAXITEMS);

  for (i=0;i<NUMWRITES;i++) {
    do {
      item=random()%MAXITEMS+1;
    } while (genedpathname[item].whatisit!=ISFILE);
    
    printf("#%d Trying to write to '%s'\n", i, genedpathname[item].pn);
    
    curbs=random()%(BLOCKSIZ+1);

#ifdef RANDOMOFFSETS
    curoffset=random()%MAXSIZE;
#else
    curoffset=genedpathname[item].maxoffset;
#endif
    curoffset=(curoffset+curbs>MAXSIZE) ? MAXSIZE-curbs : curoffset;

    printf("  Try to write %d bytes at offset %d\n", curbs,curoffset);
    GenData(buf,genedpathname[item].num,curoffset,curbs);
    if (do_op(genedpathname[item].pn,WRITE,AFILE,curoffset,
	      buf,curbs,&actualbs)) {
      printf("failed with rc=%d\n", rc);
    }

    if ((actualbs>0) && (actualbs+curoffset>genedpathname[item].maxoffset)) {
      genedpathname[item].maxoffset=actualbs+curoffset;
    }
    
    if (actualbs!=curbs) {
      printf("ERROR: Actually wrote %d bytes\n", actualbs);
    }
    
#ifdef FSCK
    
    if ((rc=yfs_fsck())) {
      printf("ERROR: yfs_fsck returned %d\n", rc);
    }
    
#endif
    
  }

  printf("Now doing sequential reads on %d files with readsize=%d\n",
	 MAXITEMS,BLOCKSIZ);

  for (item=0;item<MAXITEMS;item++) {
    if (genedpathname[item].whatisit==ISFILE) {
      for (i=0;i+BLOCKSIZ-1<genedpathname[item].maxoffset;i+=BLOCKSIZ) {
	printf("Reading file '%s', offset=%d, len=%d\n",
	       genedpathname[item].pn,i,BLOCKSIZ);
	if (do_op(genedpathname[item].pn,READ,AFILE,i,
		  buf,BLOCKSIZ,&actualbs)) {
	  printf("ERROR: read failed!");
	}
	if (actualbs!=BLOCKSIZ) {
	  printf("ERROR: Only read %d bytes!\n",actualbs);
	}
        GenData(check,genedpathname[item].num,i,actualbs);
	for (j=0;j<actualbs;j++) {
	  if (buf[j]!=check[j]) {
	    printf("ERROR: Read Bogus data!");
	    break;
	  }
	}
      }
/* Read spare stuff */
      i=genedpathname[item].maxoffset-
	          genedpathname[item].maxoffset%BLOCKSIZ,
      printf("Reading file '%s', offset=%d, len=%d\n",
	     genedpathname[item].pn,
	     i, genedpathname[item].maxoffset%BLOCKSIZ);
      if (do_op(genedpathname[item].pn,READ,AFILE,i,
		buf,genedpathname[item].maxoffset%BLOCKSIZ,&actualbs)) {
	printf("ERROR: read failed!");
      }
      if (actualbs!=genedpathname[item].maxoffset%BLOCKSIZ) {
	printf("ERROR: Only read %d bytes!\n",actualbs);
      }
      GenData(check,genedpathname[item].num,i,actualbs);
      for (j=0;j<actualbs;j++) {
        if (buf[j]!=check[j]) {
	  printf("ERROR: Read Bogus data!");
	  break;
	}
      }
    }

#ifdef FSCK
    
    if ((rc=yfs_fsck())) {
      printf("ERROR: yfs_fsck returned %d\n", rc);
    }
    
#endif
    
  }
  
  printf("Completed reads of all %d files\n", MAXITEMS);
  
  printf("Now Deleting Files and Directories in Random Order\n");
  
  i=1;
  
  while (i<=MAXITEMS) {

/* Random is not random enough */

    if (i<(MAXITEMS*4)/5)
    {
      do {
        item=random()%(MAXITEMS) + 1;
      } while ((genedpathname[item].whatisit==ISGONE) ||
               (genedpathname[item].children>0));
    } else
    {
      for (item=1; item<=MAXITEMS; item++)
        if (genedpathname[item].whatisit!=ISGONE &&
            genedpathname[item].children==0)
          break;
    }

/* Now we have a random item */

    genedpathname[genedpathname[item].parent].children--;
    
    printf("Deleting #%d  %s - '%s'\n", i, 
	   genedpathname[item].whatisit==ISDIR ? 
	   "DIR " : "FILE", genedpathname[item].pn);
   
    if (genedpathname[item].whatisit==ISDIR)  {
      if (do_op(genedpathname[item].pn,DELETE,ADIRECTORY,0,0,0,0)) {
	printf("ERROR: Delete failed - check why!\n");
      }
    } else {
      if (do_op(genedpathname[item].pn,DELETE,AFILE,0,0,0,0)) {
	printf("ERROR: Delete failed - check why!\n");
      }
    }
    genedpathname[item].whatisit=ISGONE;

    ++i;

#ifdef FSCK

    if ((rc=yfs_fsck())) {
      printf("ERROR: yfs_fsck returned %d\n", rc);
    }

#endif
    
  }

  puts("Done");

  exit(0);
 
}
