/*
 * Copyright (c) 1999 The University of Utah and
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

#include "tao/corba.h"
#include "phone_i.h"

phone_i::phone_i()
{
	/* Start with an empty list. */
	head = 0;
}

phone_i::~phone_i()
{
	impl_node_ptr curr, succ;
	
	/* Delete our list of numbers. */
	curr = head;
	while (curr) {
		succ = curr->next;
		delete curr;
		curr = succ;
	}
	head = 0;
}

/*****************************************************************************/

void phone_i::add(const data::entry &e, CORBA::Environment &ACE_TRY_ENV)
{
	impl_node_ptr node_ptr;
	
	/* Check for a duplicate name. */
	node_ptr = find_node(e.n);
	if (node_ptr) {
		/*
		 * We already have this number.  Use the `ACE_THROW' macro to
		 * throw an exception.
		 */
		ACE_THROW(data::duplicate(node_ptr->entry.p));
	} else {
		/*
		 * Make a new node and add it in.  Note that the structure
		 * assignment below does a *deep* copy: the structure type is
		 * defined in IDL, and the generated assignment operator does a
		 * deep copy as required by the CORBA C++ language mapping.
		 */
		node_ptr = new impl_node_t;
		node_ptr->entry = e; /* Deep copy. */
		node_ptr->next = head;
		head = node_ptr;
	}
}

void phone_i::remove(const char *n, CORBA::Environment &ACE_TRY_ENV)
{
	impl_node_ptr curr;
	impl_node_ptr *prev;
	int found = 0;
	
	/*
	 * Walk through our linked list, keeping track of the previous node
	 * pointer so we can set it to point to the list node after the one
	 * we're removing.
	 */
	prev = &head;
	curr = head;
	while (curr && !found) {
		if (!strcmp(n, (const char *) curr->entry.n))
			found = 1;
		else {
			prev = &(curr->next);
			curr = curr->next;
		}
	}
	if (found) {
		/* We found the one to remove. */
		*prev = curr->next;
		delete curr;
	} else {
		/* Throw a `data::notfound' exception. */
		ACE_THROW(data::notfound());
	}
}

data::phone phone_i::find(const char *n, CORBA::Environment &ACE_TRY_ENV)
{
	impl_node_ptr node_ptr;
	data::phone_var retval;
	
	node_ptr = find_node(n);
	if (node_ptr) {
		/* Make a copy and return it. */
		retval = node_ptr->entry.p;
	} else {
		/*
		 * Use the `ACE_THROW_RETURN' macro since we have to return
		 * something.
		 */
		ACE_THROW_RETURN(data::notfound(), retval);
	}
	return retval._retn();
}

/*****************************************************************************/

phone_i::impl_node_ptr phone_i::find_node(const char *name)
{
	impl_node_ptr curr;
	impl_node_ptr retval;
	
	retval = 0;
	
	/* Walk the linked list looking for a node containing `name'. */
	curr = head;
	while (curr && !retval) {
		if (!strcmp(name, (const char *) curr->entry.n))
			retval = curr;
		curr = curr->next;
	}
	return retval;
}

/* End of file. */

