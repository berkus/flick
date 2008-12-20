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
#include <ctype.h>
#include <string.h>

#include <mom/compiler.h>
#include <mom/cast.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>
#include <mom/c/be/presentation_impl.hh>

presentation_impl::presentation_impl()
{
	this->idl_type = 0;
	this->pres_type = 0;
	this->scope_impl = 0;
	this->misc_impl = 0;
	this->pc = 0;
}

presentation_impl::~presentation_impl()
{
}

void presentation_impl::set_idl_type(const char *type)
{
	this->idl_type = type;
}

const char *presentation_impl::get_idl_type()
{
	return( this->idl_type );
}

void presentation_impl::set_pres_type(const char *type)
{
	this->pres_type = type;
}

const char *presentation_impl::get_pres_type()
{
	return( this->pres_type );
}

void presentation_impl::set_scope_impl(void (*impl)(cast_scope *scope,
						    tag_list *tl))
{
	this->scope_impl = impl;
}

void (*presentation_impl::get_scope_impl())(cast_scope *scope, tag_list *tl)
{
	return( this->scope_impl );
}

void presentation_impl::set_misc_impl(void (*impl)(pres_c_1 *pres, tag_list *))
{
	this->misc_impl = impl;
}

void (*presentation_impl::get_misc_impl())(pres_c_1 *pres, tag_list *)
{
	return( this->misc_impl );
}

void presentation_impl::set_collection(struct presentation_collection *the_pc)
{
	this->pc = the_pc;
}

struct presentation_collection *presentation_impl::get_collection()
{
	return( this->pc );
}

void presentation_impl::write_pres_funcs(pres_c_1 *pres,
					 struct scml_scope *pres_scope,
					 tag_list *tl)
{
	struct scml_cmd_definition *cmd_def;
	struct scml_scope *cmd_scope;
	struct scml_context *sc;
	tag_list *func_tl;
	cast_def *cfunc;
	tag_item *ti;
	int lpc, len;
	
	ti = find_tag(tl, "pres_func");
	if( !ti )
		return;
	len = tag_data_length(&ti->data);
	/* Create an execution context */
	sc = new scml_context;
	sc->set_scope(pres_scope);
	sc->set_stream_pos(this->pc->get_scml_stream_pos());
	/* Walk through the array of pres_funcs */
	for( lpc = 0; lpc < len; lpc++ ) {
		func_tl = find_tag(tl, get_tag_data(&ti->data, lpc).str)->
			data.tag_data_u.tl;
		cfunc = &pres->stubs_cast.cast_scope_val[find_tag(func_tl,
								  "c_func")->
							data.tag_data_u.i];
		/* output only if they have a C decl and a definition */
		cmd_def = pres_scope->find_cmd_definition(
			&cmd_scope, get_tag_data(&ti->data, lpc).str);
		if( !(pres->meta_data.channels.channels_val[cfunc->channel].
		      flags & DATA_CHANNEL_SQUELCHED) && cmd_def ) {
			/* Execute the definition */
			sc->exec_cmd(get_tag_data(&ti->data, lpc).str,
				     func_tl,
				     NULL);
		} else if( !(pres->meta_data.channels.
			     channels_val[cfunc->channel].flags &
			     DATA_CHANNEL_SQUELCHED) &&
			   (cfunc->protection != CAST_PROT_PRIVATE) ) {
			fprintf(stderr,
				"Warning '%s' function in %s::%s "
				"not handled\n",
				get_tag_data(&ti->data, lpc).str,
				pres_scope->get_parent()->get_name(),
				pres_scope->get_name());
		}
	}
	delete sc;
}

void presentation_impl::write_model_funcs(pres_c_1 *pres,
					  struct scml_scope *pres_scope,
					  tag_list *tl)
{
	struct scml_scope *cmd_scope;
	struct scml_context *sc;
	tag_list *func_tl;
	cast_def *cfunc;
	char *kind_str;
	tag_item *ti;
	int lpc, len;
	
	ti = find_tag(tl, "model_func");
	if( !ti )
		return;
	len = tag_data_length(&ti->data);
	sc = new scml_context;
	sc->set_scope(pres_scope);
	sc->set_stream_pos(this->pc->get_scml_stream_pos());
	for( lpc = 0; lpc < len; lpc++ ) {
		func_tl = get_tag_data(&ti->data, lpc).tl;
		cfunc = &pres->stubs_cast.cast_scope_val[find_tag(func_tl,
								  "c_func")->
							data.tag_data_u.i];
		kind_str = find_tag(func_tl, "kind")->data.tag_data_u.str;
		if( !(pres->meta_data.channels.channels_val[cfunc->channel].
		      flags & DATA_CHANNEL_SQUELCHED) &&
		    pres_scope->find_cmd_definition(&cmd_scope, kind_str) ) {
			sc->exec_cmd(kind_str, func_tl, NULL);
		} else if( !(pres->meta_data.channels.
			     channels_val[cfunc->channel].flags &
			     DATA_CHANNEL_SQUELCHED) ) {
			fprintf(stderr,
				"Warning '%s' function model in %s::%s "
				"not handled\n",
				get_tag_data(&ti->data, lpc).str,
				pres_scope->get_parent()->get_name(),
				pres_scope->get_name());
		}
	}
}

void presentation_impl::implement(pres_c_1 *pres, tag_list *tl)
{
	if( this->scope_impl ) {
		cast_scoped_name scname;
		cast_scope *scope;
		cast_type ctype;
		tag_item *ti;
		int cdef;
		
		/* This is an operation on a type's scope */
		if( (ti = find_tag(tl, "name")) &&
		    (ti->data.kind == TAG_CAST_SCOPED_NAME) ) {
			/* Find the scope */
			scname = ti->data.tag_data_u.scname;
			scope = &pres->cast;
			cdef = cast_find_def(&scope, scname,
					     CAST_TYPEDEF|CAST_TYPE);
			while( cdef != -1 ) {
				ctype = scope->cast_scope_val[cdef].u.
					cast_def_u_u.type;
				switch( ctype->kind ) {
				case CAST_TYPE_TEMPLATE:
					ctype = ctype->cast_type_u_u.
						template_type.def;
					break;
				default:
					break;
				}
				if( ctype->kind == CAST_TYPE_AGGREGATE ) {
					scope = &ctype->cast_type_u_u.
						agg_type.scope;
					/* Let the function add to it */
					this->scope_impl(scope, tl);
					cdef = -1;
				}
				else {
					cdef = cast_find_def_pos(&scope,
								 cdef + 1,
								 scname,
								 CAST_TYPEDEF|
								 CAST_TYPE);
				}
			}
		}
	}
	if( this->misc_impl )
		this->misc_impl(pres, tl);
	if( this->pc->get_scml_scope() && this->pres_type ) {
		struct scml_scope *idl_scope, *pres_scope;
		
		/* Find the matching IDL scope in the SCML */
		if( (idl_scope =
		     this->pc->get_scml_scope()->
		     find_child(find_tag(tl, "idl_type")->
				data.tag_data_u.str)) ) {
			/* Find the matching pres scope in the SCML */
			if( (pres_scope = idl_scope->
			     find_child(this->pres_type)) ) {
				this->write_pres_funcs(pres, pres_scope, tl);
				this->write_model_funcs(pres, pres_scope, tl);
			} else {
				fprintf(stderr,
					"Couldn't find pres_type '%s' in"
					" scope '%s'\n",
					this->pres_type,
					idl_scope->get_name());
			}
		} else {
			fprintf(stderr, "Couldn't find idl_type '%s' in"
				" scope\n", find_tag(tl, "idl_type")->
				data.tag_data_u.str);
		}
	}
}

presentation_collection::presentation_collection()
{
	new_list(&this->impls);
}

presentation_collection::~presentation_collection()
{
}

void presentation_collection::set_scml_scope(struct scml_scope *the_scope)
{
	this->scope = the_scope;
}

struct scml_scope *presentation_collection::get_scml_scope()
{
	return( this->scope );
}

void presentation_collection::set_scml_stream_pos(struct scml_stream_pos *ssp)
{
	this->stream_pos = ssp;
}

struct scml_stream_pos *presentation_collection::get_scml_stream_pos()
{
	return( this->stream_pos );
}

void presentation_collection::add_impl(struct presentation_impl *pi)
{
	add_tail(&this->impls, &pi->link);
	pi->set_collection(this);
}

struct presentation_impl *presentation_collection::find_impl(char *idl_type,
							     char *pres_type)
{
	struct presentation_impl *retval = 0, *curr;
	
	curr = (struct presentation_impl *)this->impls.head;
	while( curr->link.succ ) {
		if( (!idl_type || !curr->get_idl_type() ||
		     !strcmp(idl_type, curr->get_idl_type())) &&
		    (!pres_type || !curr->get_pres_type() ||
		     !strcmp(pres_type, curr->get_pres_type())) ) {
			retval = curr;
		}
		curr = (struct presentation_impl *)curr->link.succ;
	}
	return( retval );
}

void presentation_collection::implement(pres_c_1 *pres)
{
	struct presentation_impl *pi;
	tag_list *tl, *sub_tl;
	int lpc, len;
	char *idl_type;
	tag_item *pres_ti, *ti;
	
	tl = pres->pres_attrs;
	if( !(pres_ti = find_tag(tl, "pres_type")) )
		return;
	
	/* Fix the parent links of the tag_list's in tl */
	relink_tag_list(tl);
	/* Typedefs, detected by the typedef_ref tag, need to have their parent
	   links redirected to the pres_type tag_list specified by the numbe
	   in the typedef_ref */
	len = tag_data_length(&pres_ti->data);
	for( lpc = 0; lpc < len; lpc++ ) {
		sub_tl = get_tag_data(&pres_ti->data, lpc).tl;
		if( (ti = find_tag(sub_tl, "typedef_ref")) ) {
			sub_tl->parent =
				get_tag_data(&pres_ti->data,
					     ti->data.tag_data_u.i).tl;
		}
	}
	
	/* Walk through the array of pres_types and process them through
	   the list of pres_impls */
	for( lpc = 0; lpc < len; lpc++ ) {
		sub_tl = get_tag_data(&pres_ti->data, lpc).tl;
		ti = find_tag(sub_tl, "idl_type");
		if( find_tag(sub_tl, "main") && ti &&
		    !find_tag(sub_tl, "_pc_implemented") &&
		    (ti->data.kind == TAG_STRING) ) {
			idl_type = ti->data.tag_data_u.str;
			pi = (struct presentation_impl *)this->impls.head;
			while( pi->link.succ ) {
				if( (!pi->get_idl_type() ||
				     !strcmp(idl_type, pi->get_idl_type())) &&
				    (!pi->get_pres_type() ||
				     (ti = find_tag(sub_tl,
						    pi->get_pres_type()))) ) {
					/* If we have a pres type than pass
					   its tag_list down, otherwise
					   use the type's root tag_list */
					if( pi->get_pres_type() )
						pi->implement(pres,
							      ti->data.
							      tag_data_u.tl);
					else
						pi->implement(pres, sub_tl);
				}
				pi = (struct presentation_impl *)pi->link.succ;
			}
			add_tag(sub_tl, "_pc_implemented", TAG_NONE);
		}
	}
}

