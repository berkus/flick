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

#ifndef _mom_c_be_looper_hh_
#define _mom_c_be_looper_hh_

#include <mom/compiler.h>

/*
 * Some generic event for a handler to `handle'.  The list_node is for queueing
 * events, however, any event passed to a handler must be unlinked from any
 * lists
 */
struct be_event {
	struct list_node link;
	int id;
};

class be_looper;

/* Generic class for specifying handler functions */
class be_handler : public list_node {
	
public:
	be_handler();
	be_handler(const char *name, int pri,
		   struct be_event *(*handler)(struct be_handler *bh,
					       struct be_event *be));
	virtual ~be_handler();
	
	/* Set/get the container for this handler */
	void set_parent(struct be_looper *parent);
	struct be_looper *get_parent();
	
	/* Call the handler for an event */
	virtual struct be_event *handle(struct be_event *event);
	
	friend class be_looper;
	
	struct be_looper *parent;
	const char *name;
	int pri;
protected:
	struct be_event *(*handler)(struct be_handler *, struct be_event *);
};

class be_looper : public be_handler {

public:
	be_looper();
	be_looper(const char *name, int pri);
	~be_looper();
	
	/* Add/Find a specific handler */
	void add_handler(struct be_handler *handler);
	struct be_handler *find_handler(const char *name);
	
	/* Queue an event to be handled */
	void queue_event(struct be_event *be);
	
	struct be_event *make_event(int id, ...);
	
	virtual struct be_event *handle(struct be_event *event);
	
protected:
	virtual struct be_event *vmake_event(int id, va_list args);
	struct dl_list handlers;
	struct dl_list event_queue;
};

#define REQUIRE_HANDLER(x) (-((x).pri))

#endif /* _mom_c_be_looper_hh_ */
