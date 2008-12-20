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

#ifndef _mom_c_be_file_hh_
#define _mom_c_be_file_hh_

#include <stdio.h>

#include <mom/c/scml.hh>
#include <mom/c/be/be_looper.hh>

class be_state;

/*
 * be_file
 *
 * This class is used for tracking and generating the various files in a
 * back end.
 */

enum {
	BEFB_INPUT,
	BEFB_OUTPUT
};

enum {
	BEFF_INPUT = (1L << BEFB_INPUT),
	BEFF_OUTPUT = (1L << BEFB_OUTPUT)
};

/* Event IDs for be_file */
enum {
	BEFE_NONE,
	BEFE_INIT,
	BEFE_BODY,
	BEFE_SHUTDOWN,
	BEFE_MAX
};

/* We subclass from be_looper and use handlers for outputing any data */
class be_file : public be_looper {

public:
	be_file();
	virtual ~be_file();
	
	void set_name(const char *the_name);
	const char *get_name();
	
	void set_flags(int the_flags);
	int get_flags();
	
	void set_path(const char *the_path);
	const char *get_path();
	
	void set_state(struct be_state *the_state);
	struct be_state *get_state();
	
	void set_file(FILE *file);
	FILE *get_file();
	
	void export_to_scml();
	
	virtual struct be_event *handle(struct be_event *);
	
	int open();
	void close();
	
private:
	struct be_state *state;
	int flags;
	const char *path;
	struct scml_stream *stream;
};

extern struct be_handler be_file_prologue;
extern struct be_handler be_file_header_defs;
extern struct be_handler be_file_pre_stubs;
extern struct be_handler be_file_gen_stubs;
extern struct be_handler be_file_epilogue;

#endif /* _mom_c_be_file_hh */
