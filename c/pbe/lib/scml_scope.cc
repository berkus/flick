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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mom/c/scml.hh>

scml_scope::scml_scope()
{
	int lpc;
	
	this->next = 0;
	this->parent = 0;
	this->name = 0;
	this->values = create_tag_list(0);
	this->setable = 0;
	for( lpc = 0; lpc < SCML_SCOPE_TABLE_SIZE; lpc++ ) {
		this->sc_table[lpc] = 0;
	}
}

scml_scope::~scml_scope()
{
}

struct scml_scope *scml_scope::get_parent()
{
	return( this->parent );
}

void scml_scope::set_name(const char *the_name)
{
	this->name = the_name;
}

const char *scml_scope::get_name()
{
	return( this->name );
}

tag_list *scml_scope::get_values()
{
	return( this->values );
}

void scml_scope::set_escape_table(struct scml_escape_table *the_setable)
{
	this->setable = the_setable;
}

struct scml_escape_table *scml_scope::get_escape_table()
{
	return( this->setable );
}

void scml_scope::add_cmd_definition(struct scml_cmd_definition *scd)
{
	add_tag(this->values, scd->get_name(), TAG_REF, scd->tag_ref());
}

void scml_scope::rem_cmd_definition(struct scml_cmd_definition *scd)
{
	rem_tag(this->values, scd->get_name());
}

struct scml_cmd_definition *scml_scope::find_cmd_definition(
	struct scml_scope **scope,
	const char *cmd_name)
{
	struct scml_cmd_definition *retval = 0;
	tag_item *ti;
	tag_list *tl;
	
	*scope = this;
	while( *scope && !retval ) {
		tl = (*scope)->values->parent;
		(*scope)->values->parent = 0;
		if( (ti = find_tag((*scope)->values, cmd_name)) &&
		    (ti->data.kind == TAG_REF) ) {
			retval = scml_cmd_definition::ptr(ti->data.
							  tag_data_u.ref);
		}
		(*scope)->values->parent = tl;
		if( !retval )
			*scope = (*scope)->parent;
	}
	return( retval );
}

void scml_scope::add_child(struct scml_scope *ss)
{
	int h;
	
	h = scml_hash_name(ss->name, SCML_SCOPE_TABLE_SIZE);
	ss->next = this->sc_table[h];
	this->sc_table[h] = ss;
	ss->parent = this;
	ss->get_values()->parent = this->values;
}

struct scml_scope *scml_scope::find_child(const char *child_name)
{
	struct scml_scope *retval = 0, *ss;
	int h;
	
	if( child_name ) {
		h = scml_hash_name(child_name, SCML_SCOPE_TABLE_SIZE);
		ss = this->sc_table[h];
		while( ss && !retval ) {
			if( !strcmp( ss->name, child_name ) )
				retval = ss;
			ss = ss->next;
		}
	}
	return( retval );
}

void scml_scope::print()
{
	struct scml_scope *ss;
	int lpc;
	
	printf("scml_scope %s at %p\n", this->name, this);
	printf("parent: %p\n", this->parent);
	if( this->parent )
		printf("parent_name: %s\n", this->parent->name);
	printf("children:\n");
	for( lpc = 0; lpc < SCML_SCOPE_TABLE_SIZE; lpc++ ) {
		if( this->sc_table[lpc] ) {
			ss = this->sc_table[lpc];
			while( ss ) {
				printf("  %s\n", ss->name);
				ss = ss->next;
			}
		}
	}
}

struct scml_scope *scml_scope::make_root_scope()
{
	struct scml_cmd_definition *scd;
	struct scml_scope *retval;
	
	retval = new scml_scope;
	
	/* Construct the root scope with all the built-in definitions */
	scd = new scml_cmd_definition();
	scd->set_name("define");
	add_tag(scd->get_req_params(), "name", TAG_REF, 0);
	add_tag(scd->get_req_params(), "kind", TAG_REF, 0);
	add_tag(scd->get_req_params(), "value", TAG_ANY);
	add_tag(scd->get_opt_params(), "oparams",
		TAG_TAG_LIST, create_tag_list(0));
	add_tag(scd->get_opt_params(), "rparams",
		TAG_TAG_LIST, create_tag_list(0));
	retval->add_cmd_definition(scd);
	
	scd = new scml_cmd_definition();
	scd->set_name("undef");
	add_tag(scd->get_req_params(), "name", TAG_REF, 0);
	retval->add_cmd_definition(scd);
	
	scd = new scml_cmd_definition();
	scd->set_name("rename");
	add_tag(scd->get_req_params(), "name", TAG_REF, 0);
	add_tag(scd->get_req_params(), "to", TAG_REF, 0);
	retval->add_cmd_definition(scd);
	
	scd = new scml_cmd_definition();
	scd->set_name("ifdef");
	scd->set_flags(SCDF_BRACKETED);
	add_tag(scd->get_req_params(), "name", TAG_REF, 0);
	retval->add_cmd_definition(scd);
	
	scd = new scml_cmd_definition();
	scd->set_name("ifndef");
	scd->set_flags(SCDF_BRACKETED);
	add_tag(scd->get_req_params(), "name", TAG_REF, 0);
	retval->add_cmd_definition(scd);
	
	scd = new scml_cmd_definition();
	scd->set_name("else");
	scd->set_handler(scml_context::get_handler_table()->
			 find_handler("c-else-handler"));
	add_tag(scd->get_req_params(), "name", TAG_REF, 0);
	retval->add_cmd_definition(scd);
	
	scd = new scml_cmd_definition();
	scd->set_name("include");
	scd->set_handler(scml_context::get_handler_table()->
			 find_handler("c-include-handler"));
	add_tag(scd->get_req_params(), "file", TAG_REF, 0);
	retval->add_cmd_definition(scd);
	
	retval->set_escape_table(new scml_escape_table);
	return( retval );
}

scml_cmd_definition::scml_cmd_definition()
{
	this->name = 0;
	this->opt_params = create_tag_list(0);
	this->req_params = create_tag_list(0);
	this->flags = 0;
	this->sh = 0;
	this->sts = 0;
}

scml_cmd_definition::~scml_cmd_definition()
{
}

char *scml_cmd_definition::tag_ref()
{
	return( ptr_to_tag_ref("struct scml_cmd_definition *", this) );
}

struct scml_cmd_definition *scml_cmd_definition::ptr(char *ref)
{
	return( (struct scml_cmd_definition *)
		tag_ref_to_ptr("struct scml_cmd_definition *", ref) );
}

void scml_cmd_definition::set_name(const char *the_name)
{
	this->name = the_name;
}

const char *scml_cmd_definition::get_name()
{
	return( this->name );
}

void scml_cmd_definition::set_opt_params(tag_list *tl)
{
	if( tl != this->opt_params )
		delete_tag_list(this->opt_params);
	this->opt_params = tl;
}

tag_list *scml_cmd_definition::get_opt_params()
{
	return( this->opt_params );
}

void scml_cmd_definition::set_req_params(tag_list *tl)
{
	if( tl != this->req_params )
		delete_tag_list(this->req_params);
	this->req_params = tl;
}

tag_list *scml_cmd_definition::get_req_params()
{
	return( this->req_params );
}

void scml_cmd_definition::set_flags(int the_flags)
{
	this->flags = the_flags;
}

int scml_cmd_definition::get_flags()
{
	return( this->flags );
}

void scml_cmd_definition::set_handler(struct scml_handler *the_sh)
{
	this->sh = the_sh;
}

struct scml_handler *scml_cmd_definition::get_handler()
{
	return( this->sh );
}

void scml_cmd_definition::set_token_sequence(
	struct scml_token_sequence *the_sts)
{
	this->sts = the_sts;
}

struct scml_token_sequence *scml_cmd_definition::get_token_sequence()
{
	return( this->sts );
}

int scml_cmd_definition::execute(struct scml_token *st,
				 struct scml_context *sc)
{
	return( this->sh->function.c_func(st, sc) );
}

scml_escape_table::scml_escape_table()
{
	int lpc;
	
	for( lpc = 0; lpc < SCML_ESCAPE_TABLE_SIZE; lpc++ ) {
		this->table[lpc] = 0;
	}
}

scml_escape_table::~scml_escape_table()
{
}

void scml_escape_table::add_escape(struct scml_escape *se)
{
	int h;
	
	h = scml_hash_name(se->name, SCML_ESCAPE_TABLE_SIZE);
	se->next = this->table[h];
	this->table[h] = se;
}

void scml_escape_table::rem_escape(const char *name)
{
	struct scml_escape *curr, **last;
	int h;
	
	h = scml_hash_name(name, SCML_ESCAPE_TABLE_SIZE);
	curr = this->table[h];
	last = &this->table[h];
	while( curr && strcmp(name, curr->name) ) {
		last = &curr->next;
		curr = curr->next;
	}
	if( curr ) {
		*last = curr->next;
	}
}

struct scml_escape *scml_escape_table::find_escape(const char *name)
{
	struct scml_escape *retval = 0, *se;
	int h;
	
	if( name ) {
		h = scml_hash_name(name, SCML_ESCAPE_TABLE_SIZE);
		se = this->table[h];
		while( se && !retval ) {
			if( !strcmp( se->name, name ) )
				retval = se;
			se = se->next;
		}
	}
	return( retval );
}

/* End of file. */

