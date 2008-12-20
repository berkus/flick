/*
 * Copyright (c) 1997, 1998 The University of Utah and
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

#include <assert.h>
#include <string.h>

#include "phone-server.h"

void signal_no_memory(CORBA_Environment *);
void signal_initialize(CORBA_Environment *);
void signal_object_not_exist(CORBA_Environment *);
void signal_unknown(CORBA_Environment *);

/*****************************************************************************/

/*
 * This server maintains a list of phonebooks, represented as a linked list of
 * `struct impl_node_t's.  Each node contains the data for a single phonebook.
 */

typedef struct impl_node_t *impl_node;

struct impl_node_t {
	CORBA_ReferenceData id;		/* The search key for this node. */
	data_phonebook obj;		/* CORBA Object ref for this.    */
	
	data_entry **pb;		/* The array of ptrs to entries. */
	int pb_elems;			/* # of entries in `pb'.         */
	int pb_size;			/* The size of the `pb' array.   */
	
	impl_node next;			/* Ptr to the next list element. */
};

/* `pb_impl_list' is the list of phonebook implementations. */

impl_node pb_impl_list = 0;

/* `NULL_PB_INDEX' is an invalid index into a `pb' array. */
#define NULL_PB_INDEX (-1)

/* `orb' and `boa' are handles to our ORB and BOA, respectively. */

CORBA_ORB orb = 0;
CORBA_BOA boa = 0;

/*****************************************************************************/

/*
 * `register_objects' is called by the Flick-generated `main' function for the
 * phonebook server.  Here, we scan the command line and create the objects
 * that the server will make available to clients.
 */

void register_objects(CORBA_ORB o, CORBA_BOA b, int argc, char **argv,
		      CORBA_Environment *ev)
{
	int i;
	
	/* Save our ORB and BOA handles for later use. */
	orb = o;
	boa = b;
	
	pb_impl_list = 0;
	
	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-I") && (i + 1 < argc)) {
			CORBA_ReferenceData id;
			data_phonebook obj;
			impl_node list = pb_impl_list;
			
			obj = CORBA_BOA_create(boa, &id, 
					       "data::phonebook",
					       &data_phonebook_server,
					       ev);
			if (ev->_major != CORBA_NO_EXCEPTION)
				return;
			
			pb_impl_list = (impl_node)
				       malloc(sizeof(*pb_impl_list));
			if (!pb_impl_list) {
				signal_no_memory(ev);
				return;
			}
			
			pb_impl_list->id = id;
			pb_impl_list->obj = obj;
			pb_impl_list->pb = 0;
			pb_impl_list->pb_elems = 0;
			pb_impl_list->pb_size = 0;
			pb_impl_list->next = list;
		}
	}
	
	/* Signal failure if no objects were registered. */
	if (!pb_impl_list) {
		signal_initialize(ev);
		return;
	}
	
	/*
	 * It is not necessary to signal ``no exception'' explicitly, but we
	 * do so just to be pedantic.
	 */
	CORBA_BOA_set_exception(boa, ev, CORBA_NO_EXCEPTION, 0, 0);
}

/*****************************************************************************/

/*
 * The following functions signal various standard CORBA exceptions.
 */

void signal_no_memory(CORBA_Environment *ev)
{
	CORBA_NO_MEMORY *no_memory = CORBA_NO_MEMORY__alloc();
	
	/*
	 * Note: `CORBA_NO_MEMORY__alloc' is implemented to return storage that
	 * was allocated by the Flick runtime *before* we ran out of memory!
	 * Still, be paranoid and check if the allocation failed :-(.
	 */
	if (!no_memory)
		assert(!"Can't allocate memory to signal CORBA_NO_MEMORY.");
	
	no_memory->minor = 0;
	no_memory->completed = CORBA_COMPLETED_NO;
	
	CORBA_BOA_set_exception(boa, ev,
				CORBA_SYSTEM_EXCEPTION,
				ex_CORBA_NO_MEMORY, no_memory);
	/*
	 * `CORBA_BOA_set_exception' adopts the `no_memory' storage --- i.e.,
	 * we should not an must not free `no_memory' here.  So, we're done.
	 */
}

void signal_initialize(CORBA_Environment *ev)
{
	CORBA_INITIALIZE *initialize = CORBA_INITIALIZE__alloc();
	
	if (!initialize) {
		signal_no_memory(ev);
		return;
	}
	
	initialize->minor = 0;
	initialize->completed = CORBA_COMPLETED_NO;
	
	CORBA_BOA_set_exception(boa, ev,
				CORBA_SYSTEM_EXCEPTION,
				ex_CORBA_INITIALIZE, initialize);
	/*
	 * `CORBA_BOA_set_exception' adopts the `initialize' storage --- i.e.,
	 * we should not an must not free `initialize' here.  So, we're done.
	 */
}

void signal_object_not_exist(CORBA_Environment *ev)
{
	CORBA_OBJECT_NOT_EXIST *no_object = CORBA_OBJECT_NOT_EXIST__alloc();
	
	if (!no_object) {
		signal_no_memory(ev);
		return;
	}
	
	no_object->minor = 0;
	no_object->completed = CORBA_COMPLETED_NO;
	
	CORBA_BOA_set_exception(boa, ev,
				CORBA_SYSTEM_EXCEPTION,
				ex_CORBA_OBJECT_NOT_EXIST, no_object);
	/*
	 * `CORBA_BOA_set_exception' adopts the `no_object' storage --- i.e.,
	 * we should not an must not free `no_object' here.  So, we're done.
	 */
}

void signal_unknown(CORBA_Environment *ev)
{
	CORBA_UNKNOWN *unknown = CORBA_UNKNOWN__alloc();
	
	if (!unknown) {
		signal_no_memory(ev);
		return;
	}
	
	unknown->minor = 0;
	unknown->completed = CORBA_COMPLETED_NO;
	
	CORBA_BOA_set_exception(boa, ev,
				CORBA_SYSTEM_EXCEPTION,
				ex_CORBA_UNKNOWN, unknown);
	/*
	 * `CORBA_BOA_set_exception' adopts the `unknown' storage --- i.e.,
	 * we should not an must not free `unknown' here.  So, we're done.
	 */
}

/*
 * `find_impl' finds the implementation of a phonebook object, given a
 * reference to the object.
 */

impl_node find_impl(data_phonebook obj, CORBA_Environment *ev)
{
	impl_node this_impl = pb_impl_list;
	CORBA_ReferenceData *obj_id = CORBA_BOA_get_id(boa, obj, ev);
	
	if (ev->_major != CORBA_NO_EXCEPTION)
		return 0;
	
	while (this_impl) {
		if (this_impl->id == *obj_id)
			break;
		this_impl = this_impl->next;
	}
	if (!this_impl) {
		/* This should never be reached. */
		signal_object_not_exist(ev);
	}
	
	CORBA_free(obj_id);
	
	return this_impl;
}

/*
 * `find_empty_entry' returns the index of an empty entry in a given phonebook
 * implementation.  The storage in the phonebook may be allocated/reallocated
 * in order to create space for new entries.  If no empty entry can be found,
 * this function raises an exception.
 */

int find_empty_entry(impl_node impl, CORBA_Environment *ev)
{
	int empty_index = 0;
	
	data_entry **new_pb;
	int new_pb_size;
	int i;
	
	if (!impl->pb) {
		impl->pb_size = 10;
		impl->pb_elems = 0;
		impl->pb = (data_entry **) malloc(sizeof(*impl->pb)
						  * impl->pb_size);
		if (!impl->pb) {
			impl->pb_size = 0;
			signal_no_memory(ev);
			return NULL_PB_INDEX;
		}
		for (i = 0; i < impl->pb_size; ++i)
			impl->pb[i] = 0;
		
		empty_index = 0;
		
	} else if (impl->pb_elems == impl->pb_size) {
		new_pb_size = impl->pb_size + 10;
		new_pb = (data_entry **) realloc(impl->pb,
						 (sizeof(*impl->pb)
						  * new_pb_size));
		if (!new_pb) {
			signal_no_memory(ev);
			return NULL_PB_INDEX;
		}
		for (i = impl->pb_size; i < new_pb_size; ++i)
			new_pb[i] = 0;
		
		impl->pb_size = new_pb_size;
		impl->pb = new_pb;
		
		empty_index = impl->pb_elems;
		
	} else {
		for (i = 0; i < impl->pb_size; ++i)
			if (impl->pb[i] == 0)
				break;
		
		if (i >= impl->pb_size) {
			/*
			 * What?  We have fewer elements than slots, but no
			 * free slots?!  Our data structures are goofed up.
			 */
			signal_unknown(ev);
			return NULL_PB_INDEX;
		}
		
		empty_index = i;
	}
	
	return empty_index;
}

/*****************************************************************************/

/*
 * Here are the functions that implement the operations on phonebook objects.
 */

void data_phonebook_add(data_phonebook obj,
			data_entry *arg,
			CORBA_Environment *ev)
{
	impl_node impl = find_impl(obj, ev);
	int i;
	
	/* If we failed to find the implementation, return. */
	if (ev->_major != CORBA_NO_EXCEPTION)
		return;
	
	/* See if this entry is already in the phonebook. */
	for (i = 0; i < impl->pb_size; ++i) {
		if (impl->pb[i]
		    && !strcmp(impl->pb[i]->n, arg->n)) {
			/*
			 * We found a duplicate!  Raise a `data_duplicate'
			 * exception.
			 */
			data_duplicate *d =
				(data_duplicate *)
				CORBA_alloc(sizeof(data_duplicate));
			
			if (!d) {
				signal_no_memory(ev);
				return;
			}
			d->p = (CORBA_char *)
			       CORBA_alloc(strlen(impl->pb[i]->p) + 1);
			if (!d->p) {
				CORBA_free(d);
				signal_no_memory(ev);
				return;
			}
			strcpy(d->p, impl->pb[i]->p);
			
			CORBA_BOA_set_exception(boa, ev,
						CORBA_USER_EXCEPTION,
						ex_data_duplicate, d);
			return;
		}
	}
	
	/* Find an empty entry in `impl'; grow the phonebook if necessary. */
	i = find_empty_entry(impl, ev);
	if (ev->_major != CORBA_NO_EXCEPTION)
		return;
	
	/*
	 * Allocate memory for the new entry.  Note that we have to copy the
	 * `arg' data because CORBA says we can't keep pointers into `in' data
	 * after this function has returned.
	 */
	impl->pb[i] = (data_entry *) malloc(sizeof(data_entry));
	if (!impl->pb[i]) {
		signal_no_memory(ev);
		return;
	}
	
	impl->pb[i]->n = (char *) malloc(sizeof(char) * (strlen(arg->n) + 1));
	impl->pb[i]->p = (char *) malloc(sizeof(char) * (strlen(arg->p) + 1));
	if (!(impl->pb[i]->n) || !(impl->pb[i]->p)) {
		/* Free what we have allocated and signal an exception. */
		if (impl->pb[i]->n)
			free(impl->pb[i]->n);
		if (impl->pb[i]->p)
			free(impl->pb[i]->p);
		free(impl->pb[i]);
		impl->pb[i] = 0;
		
		signal_no_memory(ev);
		return;
	}
	
	/* Copy the `arg' information into our phonebook. */
	strcpy(impl->pb[i]->n, arg->n);
	strcpy(impl->pb[i]->p, arg->p);
	
	/* Increment the number of entries in our phonebook. */
	impl->pb_elems++;
	
	/* Success! */
	return;
}

void data_phonebook_remove(data_phonebook obj,
			   data_name arg,
			   CORBA_Environment *ev)
{
	impl_node impl = find_impl(obj, ev);
	int i;
	
	/* If we failed to find the implementation, return. */
	if (ev->_major != CORBA_NO_EXCEPTION)
		return;
	
	/* Find the name `arg' in the phonebook. */
	for (i = 0; i < impl->pb_size; ++i)
		if (impl->pb[i]
		    && !strcmp(impl->pb[i]->n, arg))
			break;
	
	if (i >= impl->pb_size) {
		/*
		 * We didn't find the name.  Raise a `data_notfound'
		 * exception.
		 */
		CORBA_BOA_set_exception(boa, ev,
					CORBA_USER_EXCEPTION,
					ex_data_notfound, 0);
		return;
	}
	
	/* Clear the entry. */
	free(impl->pb[i]->n);
	free(impl->pb[i]->p);
	free(impl->pb[i]);
	impl->pb[i] = 0;
	
	/* Decrement the number of entries in our phonebook. */
	impl->pb_elems--;
	
	/* Success! */
	return;
}

data_phone data_phonebook_find(data_phonebook obj,
			       data_name arg,
			       CORBA_Environment *ev)
{
	impl_node impl = find_impl(obj, ev);
	data_phone result;
	int i;
	
	/* If we failed to find the implementation, return. */
	if (ev->_major != CORBA_NO_EXCEPTION)
		return;
	
	/* Find the name `arg' in the phonebook. */
	for (i = 0; i < impl->pb_size; ++i)
		if (impl->pb[i]
		    && !strcmp(impl->pb[i]->n, arg))
			break;
	
	if (i >= impl->pb_size) {
		/*
		 * We didn't find the name.  Raise a `data_notfound'
		 * exception.
		 */
		CORBA_BOA_set_exception(boa, ev,
					CORBA_USER_EXCEPTION,
					ex_data_notfound, 0);
		return;
	}
	
	/* Allocate and return the located phone number. */
	result = (CORBA_char *) CORBA_alloc(sizeof(CORBA_char)
					    * (strlen(impl->pb[i]->p) + 1));
	if (!result) {
		signal_no_memory(ev);
		return;
	}
	strcpy(result, impl->pb[i]->p);
	
	/* Success! */
	return result;
}

/* End of file. */

