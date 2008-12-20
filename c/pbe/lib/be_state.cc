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
#include <mom/c/libcast.h>
#include <mom/c/pbe.hh>

be_state::be_state()
{
	this->name = 0;
	this->files = new be_looper("file looper", 0);
	this->implied_channel = -1;
	this->root_scope = scml_scope::make_root_scope();
	this->scml_defs_stream = 0;
	pres_c_1_init(&this->pres);
	this->pres_impls = create_hash_table(3);
	this->pres_coll = 0;
	new_list(&this->mu_stubs);
	cast_meta = &this->pres.meta_data;
	cast_language = CAST_C;
}

be_state::~be_state()
{
	delete this->root_scope;
}

void be_state::set_name(const char *the_name)
{
	this->name = the_name;
}

const char *be_state::get_name()
{
	return( this->name );
}

struct scml_scope *be_state::get_scml_root()
{
	return( this->root_scope );
}

struct be_flags *be_state::get_cli_args()
{
	return( &this->cli_args );
}

pres_c_1 *be_state::get_pres()
{
	return( &this->pres );
}

void be_state::set_implied_channel(data_channel_index channel)
{
	this->implied_channel = channel;
}

data_channel_index be_state::get_implied_channel()
{
	return( this->implied_channel );
}

void be_state::add_file(struct be_file *file)
{
	this->files->add_handler(file);
}

struct be_file *be_state::find_file(const char *the_name)
{
	return( (struct be_file *)this->files->find_handler(the_name) );
}

struct be_looper *be_state::get_file_looper()
{
	return( this->files );
}

void be_state::add_pres_impl(struct be_pres_impl *bpi)
{
	add_entry(this->pres_impls, &bpi->entry);
}

struct be_pres_impl *be_state::find_pres_impl(const char *the_name)
{
	struct be_pres_impl *retval;
	
	retval = (struct be_pres_impl *)find_entry(this->pres_impls, the_name);
	return( retval );
}

void be_state::set_pres_collection(struct presentation_collection *the_pc)
{
	this->pres_coll = the_pc;
}

struct presentation_collection *be_state::get_pres_collection()
{
	return( this->pres_coll );
}

struct dl_list *be_state::get_mu_stub_list()
{
	return( &this->mu_stubs );
}

void be_state::set_scml_defs_stream(struct scml_stream_pos *ssp)
{
	this->scml_defs_stream = ssp;
}

struct scml_stream_pos *be_state::get_scml_defs_stream()
{
	return( this->scml_defs_stream );
}

int be_state::args(int argc, char **argv)
{
	int retval = 1;
	
	this->cli_args = this->get_default_be_flags();
	this->cli_args = this->be_args(argc, argv, this->cli_args);
	return( retval );
}

void be_state::begin()
{
	mu_msg_span::set_be_name(this->name);
}

void be_state::end()
{
}

struct be_event *be_state::vmake_event(int id, va_list args)
{
	struct be_event *retval = 0;
	
	switch( id ) {
	case BESE_NONE:
	case BESE_MAX:
		break;
	case BESE_INIT: {
		struct be_state_event *bse;
		
		bse = new be_state_event;
		bse->id = id;
		bse->state = this;
		retval = bse;
		break;
	}
	case BESE_CLI_ARGS: {
		struct be_state_cli_args_event *bse;
		
		bse = new be_state_cli_args_event;
		bse->id = id;
		bse->state = this;
		bse->argc = va_arg(args, int);
		bse->argv = va_arg(args, char **);
		retval = bse;
		break;
	}
	case BESE_SHUTDOWN: {
		struct be_state_event *bse;
		
		bse = new be_state_event;
		bse->id = id;
		bse->state = this;
		retval = bse;
		break;
	}
	default:
		retval = this->be_looper::vmake_event(id, args);
		break;
	}
	return( retval );
}
