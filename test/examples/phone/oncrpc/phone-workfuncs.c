/*
 * Copyright (c) 1997 The University of Utah and
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

#include <stdlib.h>
#include <string.h>

#include "phone-server.h"

#define PHONE_SIZE (20 + 1)

/*****************************************************************************/

/*
 * This server maintains a single phonebook.  The phonebook is represented by
 * the following global variables.
 */

entry **pb = 0;		/* The array of pointers to entries. */
int pb_elems = 0;	/* # of entries in `pb'.             */
int pb_size = 0;	/* The size of the `pb' array.       */

/* `NULL_PB_INDEX' is an invalid index into `pb'. */
#define NULL_PB_INDEX (-1)

/*****************************************************************************/

/*
 * `find_empty_entry' returns the index of an empty entry in `pb'.  The storage
 * in the phonebook may be allocated/reallocated in order to create space for
 * new entries.  If no empty entry can be found, this function returns
 * NULL_PB_INDEX.
 */

int find_empty_entry()
{
	int empty_index = 0;
	
	entry **new_pb;
	int new_pb_size;
	int i;
	
	if (!pb) {
		pb_size = 10;
		pb_elems = 0;
		pb = (entry **) malloc(sizeof(*pb) * pb_size);
		if (!pb) {
			pb_size = 0;
			return NULL_PB_INDEX;
		}
		for (i = 0; i < pb_size; ++i)
			pb[i] = 0;
		
		empty_index = 0;
		
	} else if (pb_elems == pb_size) {
		new_pb_size = pb_size + 10;
		new_pb = (entry **) realloc(pb, (sizeof(*pb) * new_pb_size));
		if (!new_pb) {
			return NULL_PB_INDEX;
		}
		for (i = pb_size; i < new_pb_size; ++i)
			new_pb[i] = 0;
		
		pb_size = new_pb_size;
		pb = new_pb;
		
		empty_index = pb_elems;
		
	} else {
		for (i = 0; i < pb_size; ++i)
			if (pb[i] == 0)
				break;
		
		if (i >= pb_size) {
			/*
			 * What?  We have fewer elements than slots, but no
			 * free slots?!  Our data structures are goofed up.
			 */
			return NULL_PB_INDEX;
		}
		
		empty_index = i;
	}
	
	return empty_index;
}

/*****************************************************************************/

/*
 * Here are the functions that implement the operations on phonebook objects.
 * Note that eah of these functions returns a pointer to statically allocated
 * storage, as expected of ONC RPC server work functions.
 */

int *add_1(entry *arg, struct svc_req *obj)
{
	static int result;
	int i;
	
	/* Return result: zero for success, or non-zero for error. */
	result = 1;
	
	/* See if this entry is already in the phonebook. */
	for (i = 0; i < pb_size; ++i) {
		if (pb[i]
		    && !strcmp(pb[i]->n, arg->n)) {
			/* We found a duplicate!  Return an error code. */
			return &result;
		}
	}
	
	/* Find an empty entry in `pb'; grow the phonebook if necessary. */
	i = find_empty_entry();
	if (i == NULL_PB_INDEX)
		return &result;
	
	/*
	 * Allocate memory for the new entry.  Note that we have to copy the
	 * `arg' data because ONC RPC says we can't keep pointers into `in'
	 * data after this function has returned.
	 */
	pb[i] = (entry *) malloc(sizeof(entry));
	if (!pb[i])
		return &result;
	
	pb[i]->n = (char *) malloc(sizeof(char) * (strlen(arg->n) + 1));
	pb[i]->p = (char *) malloc(sizeof(char) * (strlen(arg->p) + 1));
	if (!(pb[i]->n) || !(pb[i]->p)) {
		/* Free what we have allocated and signal an exception. */
		if (pb[i]->n)
			free(pb[i]->n);
		if (pb[i]->p)
			free(pb[i]->p);
		free(pb[i]);
		pb[i] = 0;
		
		return &result;
	}
	
	/* Copy the `arg' information into our phonebook. */
	strcpy(pb[i]->n, arg->n);
	strcpy(pb[i]->p, arg->p);
	
	/* Increment the number of entries in our phonebook. */
	pb_elems++;
	
	/* Success! */
	result = 0;
	return &result;
}

int *remove_1(name *arg, struct svc_req *obj)
{
	static int result;
	int i;
	
	/* Return result: zero for success, or non-zero for error. */
	result = 1;
	
	/* Find the name `arg' in the phonebook. */
	for (i = 0; i < pb_size; ++i)
		if (pb[i]
		    && !strcmp(pb[i]->n, *arg))
			break;
	
	if (i >= pb_size)
		/* We didn't find the name.  Return an error code. */
		return &result;
	
	/* Clear the entry. */
	free(pb[i]->n);
	free(pb[i]->p);
	free(pb[i]);
	pb[i] = 0;
	
	/* Decrement the number of entries in our phonebook. */
	pb_elems--;
	
	/* Success! */
	result = 0;
	return &result;
}

phone *find_1(name *arg, struct svc_req *_obj)
{
	static phone result = 0;
	static char result_string[PHONE_SIZE];
	int i;
	
	if (!result)
		result = result_string;
	
	/* Return: non-empty string for success, empty string for error. */
	result[0] = 0;
	
	/* Find the name `arg' in the phonebook. */
	for (i = 0; i < pb_size; ++i)
		if (pb[i]
		    && !strcmp(pb[i]->n, *arg))
			break;
	
	if (i >= pb_size)
		/* We didn't find the name.  Return an empty string. */
		return &result;
	
	/* Fill in the result buffer.  Make sure the string is terminated! */
	strncpy(result, pb[i]->p, PHONE_SIZE);
	result[PHONE_SIZE - 1] = 0;
	
	return &result;
}

/* End of file. */

