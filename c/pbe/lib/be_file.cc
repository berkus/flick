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
#include <mom/c/pbe.hh>

be_file::be_file()
{
	this->name = 0;
	this->state = 0;
	this->flags = 0;
	this->path = 0;
	this->stream = new scml_stream;
	this->stream->set_flags((this->stream->get_flags() | SSF_OUTPUT) &
				~SSF_INPUT);
}

be_file::~be_file()
{
	delete this->stream;
}

void be_file::set_name(const char *the_name)
{
	this->name = the_name;
}

const char *be_file::get_name()
{
	return( this->name );
}

void be_file::set_flags(int the_flags)
{
	this->flags = the_flags;
}

int be_file::get_flags()
{
	return( this->flags );
}

void be_file::set_path(const char *the_path)
{
	this->path = the_path;
}

const char *be_file::get_path()
{
	return( this->path );
}

void be_file::set_state(struct be_state *the_state)
{
	this->state = the_state;
}

struct be_state *be_file::get_state()
{
	return( this->state );
}

void be_file::set_file(FILE *file)
{
	this->stream->set_file(file);
}

FILE *be_file::get_file()
{
	return( this->stream->get_file() );
}

void be_file::export_to_scml()
{
	tag_list *tl;
	char *c_id;
	
	tl = create_tag_list(0);
	this->stream->set_desc(flick_asprintf("%s stream", this->name));
	add_tag(tl, "stream", TAG_REF, this->stream->tag_ref());
	c_id = muststrdup(file_part(this->path));
	filename_to_c_id(c_id);
	add_tag(tl, "c_id", TAG_STRING, c_id);
	add_tag(tl, "path", TAG_STRING, this->path);
	add_tag(this->state->get_scml_root()->get_values(),
		flick_asprintf("%s_file", this->name),
		TAG_TAG_LIST, tl);
}

int be_file::open()
{
	int retval = 0;
	FILE *file;
	
	if( (file = fopen(this->path,
			  (this->flags & BEFF_INPUT) ? "rb" : "w")) ) {
		this->stream->set_file(file);
		retval = 1;
	}
	return( retval );
}

struct be_event *be_file::handle(struct be_event *event)
{
	w_set_fh(this->get_file());
	return( be_looper::handle(event) );
}

void be_file::close()
{
	if( this->stream->get_file() ) {
		fclose(this->stream->get_file());
	}
}
