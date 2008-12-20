/*
 * Copyright (c) 1998 The University of Utah and
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

#include <mom/compiler.h>

/* dl_list Functions */

/* Remove node from the list. */
void remove_node(struct list_node *node)
{
	node->pred->succ = node->succ;
	node->succ->pred = node->pred;
}

/* Initialize a list structure. */
void new_list(struct dl_list *list)
{
	list->head = (struct list_node *) &(list->tail);
	list->tail = 0;
	list->tail_pred = (struct list_node *)list;
}

/* Add node to the head of list. */
void add_head(struct dl_list *list, struct list_node *node)
{
	node->succ = list->head;
	node->pred = (struct list_node *) list;
	list->head->pred = node;
	list->head = node;
}

/* Add node to the tail of list. */
void add_tail(struct dl_list *list, struct list_node *node)
{
	list->tail_pred->succ = node;
	node->pred = list->tail_pred;
	list->tail_pred = node;
	node->succ = (struct list_node *) &(list->tail);
}

/* Remove the first node from list and return it. */
struct list_node *rem_head(struct dl_list *list)
{
	struct list_node *remnode = 0;
	
	if (list->head->succ) {
		remnode = list->head;
		list->head = remnode->succ;
		list->head->pred = (struct list_node *) list;
	}
	return remnode;
}

/* Remove the last node from list and return it. */
struct list_node *rem_tail(struct dl_list *list)
{
	struct list_node *remnode = 0;
	
	if (list->tail_pred->pred) {
		remnode = list->tail_pred;
		list->tail_pred = remnode->pred;
		list->tail_pred->succ = (struct list_node *) &(list->tail);
	}
	return remnode;
}

/* Return TRUE if list is empty. */
int empty_list(struct dl_list *list)
{
	return (list->tail_pred == ((struct list_node *) list));
}

/* Insert node after pred in a list. */
void insert_node(struct list_node *pred, struct list_node *node)
{
	node->succ = pred->succ;
	pred->succ = node;
	node->pred = pred;
	node->succ->pred = node;
}

/* End of file. */

