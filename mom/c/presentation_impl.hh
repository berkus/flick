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

#ifndef PRESENTATION_IMPL_H
#define PRESENTATION_IMPL_H

#include <mom/pres_c.h>
#include <mom/compiler.h>
#include <mom/c/scml.hh>

class presentation_collection;

/* This holds information about a specific implementation of an IDL type
   and/or presentation only type.  It is used in conjunction with
   presentation_collection to produce code that is specific to some
   presentation and implementation. */
class presentation_impl {

public:
	presentation_impl();
	~presentation_impl();
	
	/* Set/get the idl type to match, if NULL then it matches everything */
	void set_idl_type(const char *type);
	const char *get_idl_type();
	
	/* Set/get the pres type to match, if NULL then it matches all */
	void set_pres_type(const char *type);
	const char *get_pres_type();
	
	/* Set/get the collection we are a part of */
	void set_collection(struct presentation_collection *pc);
	struct presentation_collection *get_collection();
	
	/* Set/get a function to run over the cast_scope of a type */
	void set_scope_impl(void (*impl)(cast_scope *scope, tag_list *tl));
	void (*get_scope_impl())(cast_scope *scope, tag_list *tl);
	
	/* Set/get a function to do miscellaneous operations on a type */
	void set_misc_impl(void (*impl)(pres_c_1 *pres, tag_list *));
	void (*get_misc_impl())(pres_c_1 *pres, tag_list *tl);
	
	/* Write out the list of pres/model functions specified in tl */
	void write_pres_funcs(pres_c_1 *pres, struct scml_scope *pres_scope,
			      tag_list *tl);
	void write_model_funcs(pres_c_1 *pres, struct scml_scope *pres_scope,
			       tag_list *tl);
	
	/* Implement the given tag_list with the stuff
	   we've been initialized with */
	void implement(pres_c_1 *pres, tag_list *tl);
	
	struct list_node link;
private:
	struct presentation_collection *pc;
	const char *idl_type;
	const char *pres_type;
	void (*scope_impl)(cast_scope *scope, tag_list *tl);
	void (*misc_impl)(pres_c_1 *pres, tag_list *tl);
	
};

class presentation_collection {

public:
	presentation_collection();
	~presentation_collection();
	
	/* Set/get the SCML scope for the collection, this is needed
	   in order to find and execute SCML code */
	void set_scml_scope(struct scml_scope *scope);
	struct scml_scope *get_scml_scope();
	
	/* Set/get the SCML stream for the collection */
	void set_scml_stream_pos(struct scml_stream_pos *ssp);
	struct scml_stream_pos *get_scml_stream_pos();
	
	/* Add/find an implementation */
	void add_impl(struct presentation_impl *pi);
	struct presentation_impl *find_impl(char *idl_type, char *pres_type);
	
	/* Implement a presentation */
	void implement(pres_c_1 *pres);
	
private:
	struct dl_list impls;
	struct scml_scope *scope;
	struct scml_stream_pos *stream_pos;
	
};

#endif

/* End of file. */

