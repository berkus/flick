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

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <mom/compiler.h>
#include <mom/pres_c.h>
#include <mom/c/be/be_looper.hh>

be_handler::be_handler()
{
	this->name = "";
	this->pri = 0;
	this->handler = 0;
}

be_handler::be_handler(const char *the_name, int the_pri,
		       struct be_event *(*the_handler)(struct be_handler *bh,
						       struct be_event *be))
	: name(the_name), pri(the_pri), handler(the_handler)
{
}

be_handler::~be_handler()
{
}

void be_handler::set_parent(struct be_looper *the_parent)
{
	this->parent = the_parent;
}

struct be_looper *be_handler::get_parent()
{
	return( this->parent );
}

struct be_event *be_handler::handle(struct be_event *be)
{
	return( this->handler(this, be) );
}

be_looper::be_looper()
{
	new_list(&this->handlers);
	new_list(&this->event_queue);
}

be_looper::be_looper(const char *the_name, int the_pri)
	: be_handler(the_name, the_pri, 0)
{
	new_list(&this->handlers);
	new_list(&this->event_queue);
}

be_looper::~be_looper()
{
}

void be_looper::add_handler(struct be_handler *bh)
{
	struct be_handler *curr;
	int done = 0;

	curr = (struct be_handler *)this->handlers.head;
	while( curr->succ && !done ) {
		if( bh->pri > curr->pri ) {
			insert_node(curr->pred, bh);
			done = 1;
		}
		curr = (struct be_handler *)curr->succ;
	}
	if( !done )
		add_tail(&this->handlers, bh);
	bh->set_parent(this);
}

struct be_handler *be_looper::find_handler(const char *the_name)
{
	struct be_handler *bh, *retval = 0;
	
	bh = (struct be_handler *)this->handlers.head;
	while( bh->succ && !retval ) {
		if( !strcmp(the_name, bh->name) )
			retval = bh;
		bh = (struct be_handler *)bh->succ;
	}
	return( retval );
}

void be_looper::queue_event(struct be_event *be)
{
	add_tail(&this->event_queue, &be->link);
}

struct be_event *be_looper::vmake_event(int id, va_list /*args*/)
{
	struct be_event *be;
	
	be = new be_event;
	be->id = id;
	return( be );
}

struct be_event *be_looper::make_event(int id, ...)
{
	struct be_event *retval;
	va_list args;
	
	va_start(args, id);
	retval = this->vmake_event(id, args);
	va_end(args);
	return( retval );
}

struct be_event *be_looper::handle(struct be_event *orig_be)
{
	struct be_handler *bh;
	struct be_event *be;
	
	/* Queue the event for processing */
	this->queue_event(orig_be);
	/*
	 * Handle any pending events, eventually we will get to
	 * the one passed in, and any added after it
	 */
	while( (be = (struct be_event *)rem_head(&this->event_queue)) ) {
		bh = (struct be_handler *)this->handlers.head;
		while( bh->succ && be ) {
			be = bh->handle(be);
			bh = (struct be_handler *)bh->succ;
		}
	}
	/* We return the original event since thats what the caller expects */
	return( orig_be );
}
