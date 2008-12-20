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

#ifndef _mom_c_be_state_hh_
#define _mom_c_be_state_hh_

#include <stdio.h>

#include <mom/meta.h>
#include <mom/cast.h>
#include <mom/pres_c.h>
#include <mom/compiler.h>
#include <mom/c/scml.hh>
#include <mom/c/be/be_looper.hh>
#include <mom/c/be/be_flags.hh>
#include <mom/c/be/be_file.hh>
#include <mom/c/be/presentation_impl.hh>

class be_state;

struct be_pres_impl {
	struct h_entry entry;
	struct presentation_collection *(*handler)(struct be_state *);
};

#define BE_PRES_IMPL_INIT(name, func) {{0,name}, func}

/*
 * be_state
 *
 * This class is basically a dumping ground for all of the objects needed
 * by a back end.
 */

class be_state : public be_looper {

public:
	be_state();
	virtual ~be_state();
	
	void set_name(const char *name);
	const char *get_name();
	
	/* Set/get various values */
	
	struct scml_scope *get_scml_root();
	
	struct be_flags *get_cli_args();
	
	pres_c_1 *get_pres();
	
	void set_implied_channel(data_channel_index channel);
	data_channel_index get_implied_channel();
	
	void set_pres_collection(struct presentation_collection *the_pc);
	struct presentation_collection *get_pres_collection();
	
	void set_scml_defs_stream(struct scml_stream_pos *ssp);
	struct scml_stream_pos *get_scml_defs_stream();
	
	struct dl_list *get_mu_stub_list();
	
	/*
	 * Add/Find a be_file in the be_state, internally this is just a
	 * be_looper with any be_files added to it.  So you just pass
	 * the looper file events to get them generated.
	 */
	void add_file(struct be_file *file);
	struct be_file *find_file(const char *name);
	struct be_looper *get_file_looper();
	
	/*
	 * Add/Find a pres_impl handler, the handler produces a pres
	 * collection for doing presentation implementation specific changes
	 */
	void add_pres_impl(struct be_pres_impl *bpi);
	struct be_pres_impl *find_pres_impl(const char *name);
	
	/* Parse the command line arguments */
	int args(int argc, char **argv);
	
	virtual void begin();
	virtual void end();
	
protected:
	be_flags cli_args;
	struct be_looper *files;
	pres_c_1 pres;
	data_channel_index implied_channel;
	struct scml_scope *root_scope;
	struct scml_stream_pos *scml_defs_stream;
	struct h_table *pres_impls;
	struct presentation_collection *pres_coll;
	struct dl_list mu_stubs;
	
	virtual struct be_event *vmake_event(int id, va_list args);
	virtual be_flags get_default_be_flags();
	be_flags be_args(int argc, char **argv, const be_flags &def_flgs,
			 const char *info = 0);
};

/* Event IDs for the be_state */
enum {
	BESE_NONE,
	BESE_INIT,
	BESE_CLI_ARGS,
	BESE_SHUTDOWN,
	BESE_MAX
};

/* An event with a pointer to the state the event occured in */
struct be_state_event : public be_event {
	struct be_state *state;
};

struct be_state_cli_args_event : public be_state_event {
	int argc;
	char **argv;
};

#endif /* _mom_c_be_state_hh */
