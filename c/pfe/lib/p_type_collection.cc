/*
 * Copyright (c) 1998, 1999 The University of Utah and
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Flick; see the file COPYING.  If not, write to
 * the Free Software Foundation, 59 Temple Place #330, Boston, MA 02111, USA.
 */

#include <assert.h>
#include <string.h>

#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include <mom/c/p_type_collection.hh>

/* class p_type_node */
p_type_node::p_type_node()
{
	this->flags = 0;
	this->name = "";
	this->format = "";
	this->type = 0;
	this->map = 0;
	this->channel = PASSTHRU_DATA_CHANNEL;
}

p_type_node::~p_type_node()
{
}

void p_type_node::set_flags(unsigned int the_flags)
{
	this->flags = the_flags;
}

unsigned int p_type_node::get_flags()
{
	return( this->flags );
}

void p_type_node::set_name(const char *the_name)
{
	this->name = the_name;
}

const char *p_type_node::get_name()
{
	return( this->name );
}

void p_type_node::set_format(const char *the_format)
{
	this->format = the_format;
}

const char *p_type_node::get_format()
{
	return( this->format );
}

void p_type_node::set_type(cast_type the_type)
{
	this->type = the_type;
}

cast_type p_type_node::get_type()
{
	return( this->type );
}

void p_type_node::set_mapping(pres_c_mapping the_map)
{
	this->map = the_map;
}

pres_c_mapping p_type_node::get_mapping()
{
	return( this->map );
}

void p_type_node::set_channel(data_channel_index the_channel)
{
	this->channel = the_channel;
}

data_channel_index p_type_node::get_channel()
{
	return( this->channel );
}

const char *p_type_node::format_name(struct p_scope_node *psn)
{
	const char *retval = 0;
	
	if( this->format ) {
		retval = flick_asprintf(this->format,
					psn->get_collection()->get_name());
		if( cast_scoped_name_is_empty(psn->get_scope_name()) )
			retval = flick_asprintf("%s%s", psn->get_prefix(),
						retval);
	} else {
		cast_type ctype = this->type;

		while( ctype ) {
			switch( ctype->kind ) {
			case CAST_TYPE_AGGREGATE:
				retval = ctype->cast_type_u_u.agg_type.name.
					cast_scoped_name_val[0].name;
				ctype = 0;
				break;
			case CAST_TYPE_TEMPLATE:
				ctype = ctype->cast_type_u_u.template_type.def;
				break;
			case CAST_TYPE_ENUM:
				retval = ctype->cast_type_u_u.enum_type.name.
					cast_scoped_name_val[0].name;
				ctype = 0;
				break;
			case CAST_TYPE_CLASS_NAME:
				retval = ctype->cast_type_u_u.name.
					cast_scoped_name_val[0].name;
				ctype = 0;
				break;
			default:
				if( cast_scoped_name_is_empty(
					psn->get_scope_name()) )
					retval = flick_asprintf("%s%s",
								psn->
								get_prefix(),
								retval);
				else
					retval = psn->
						 get_collection()->
						 get_name();
				ctype = 0;
				break;
			}
		}
	}
	return( retval );
}

void p_type_node::add_def(struct p_scope_node *psn)
{
	cast_def_kind def_kind;
	cast_scoped_name formatted_name;
	int cdef;
	
	if( this->flags & PTF_NO_DEF )
		return;
	formatted_name = cast_new_scoped_name(this->format_name(psn), NULL);
	switch( this->type->kind ) {
	case CAST_TYPE_ENUM:
		def_kind = (cast_language == CAST_CXX) ?
			CAST_TYPE : CAST_TYPEDEF;
		if( !this->type->cast_type_u_u.enum_type.name.
		    cast_scoped_name_len )
			this->type->cast_type_u_u.enum_type.name =
				formatted_name;
		break;
	case CAST_TYPE_AGGREGATE:
		def_kind = (cast_language == CAST_CXX) ?
			CAST_TYPE : CAST_TYPEDEF;
		if( !type->cast_type_u_u.agg_type.name.
		    cast_scoped_name_len )
			type->cast_type_u_u.agg_type.name = formatted_name;
		break;
	case CAST_TYPE_TEMPLATE:
	case CAST_TYPE_CLASS_NAME:
		def_kind = CAST_TYPE;
		break;
	case CAST_TYPE_FUNCTION:
		def_kind = CAST_FUNC_DECL;
		break;
	default:
		def_kind = CAST_TYPEDEF;
		break;
	}
	if( psn->get_scope() ) {
		cdef = cast_add_def(psn->get_scope(),
				    formatted_name,
				    CAST_SC_NONE,
				    def_kind,
				    psn->find(this->name)->channel,
				    psn->get_collection()->get_protection());
		switch( def_kind ) {
		case CAST_FUNC_DECL:
			psn->get_scope()->cast_scope_val[cdef].u.
				cast_def_u_u.func_type =
				this->type->cast_type_u_u.func_type;
			break;
		default:
			psn->get_scope()->cast_scope_val[cdef].u.
				cast_def_u_u.type = this->type;
			break;
		}
	}
}

struct p_type_node *p_type_node::ref_type(struct p_scope_node *psn)
{
	struct p_type_node *retval;
	cast_scoped_name scn;
	cast_type ctype;
	cast_type_kind type_kind = CAST_TYPE_NAME;
	
	retval = new p_type_node;
	retval->flags = this->flags;
	retval->name = this->name;
	retval->format = this->format;
	retval->map = pres_c_new_mapping(PRES_C_MAPPING_STUB);
	retval->channel = psn->get_channel();
	if( this->format && strlen(this->format) ) {
		switch( this->type->kind ) {
		case CAST_TYPE_FUNCTION:
			retval->type = this->type;
			break;
		default:
			if( !(this->flags & PTF_NAME_REF) ) {
				switch( this->type->kind ) {
				case CAST_TYPE_ENUM:
					type_kind = CAST_TYPE_ENUM_NAME;
					break;
				case CAST_TYPE_AGGREGATE:
					switch( this->type->cast_type_u_u.
						agg_type.kind ) {
					case CAST_AGGREGATE_STRUCT:
						type_kind =
							CAST_TYPE_STRUCT_NAME;
						break;
					case CAST_AGGREGATE_UNION:
						type_kind =
							CAST_TYPE_UNION_NAME;
						break;
					case CAST_AGGREGATE_CLASS:
						break;
					}
					break;
				default:
					break;
				}
			}
			scn = cast_copy_scoped_name(psn->get_scope_name());
			cast_add_scope_name(&scn,
					    this->format_name(psn),
					    null_template_arg_array);
			if( psn->get_collection()->get_tag_list() ) {
				ctype = cast_new_type( type_kind );
				ctype->cast_type_u_u.name =
					cast_copy_scoped_name(&scn);
				add_tag(psn->get_collection()->
					get_tag_list(),
					flick_asprintf("_global_%s",
						       retval->name),
					TAG_CAST_TYPE,
					ctype);
			}
			if( scn.cast_scoped_name_len > 1 ) {
				cast_prepend_scope_name(
					&scn,
					"",
					null_template_arg_array);
			}
			ctype = cast_new_type( type_kind );
			ctype->cast_type_u_u.name = scn;
			retval->type = ctype;
			break;
		}
	} else {
		retval->type = this->type;
	}
	if( psn->get_collection()->get_tag_list() ) {
		add_tag(psn->get_collection()->get_tag_list(),
			retval->name,
			TAG_CAST_TYPE,
			retval->type);
	}
	return( retval );
}

void p_type_node::remove(struct p_scope_node *psn)
{
	remove_node(&this->link);
	if( psn->get_collection()->get_tag_list() ) {
		rem_tag(psn->get_collection()->get_tag_list(), this->name);
		rem_tag(psn->get_collection()->get_tag_list(),
			flick_asprintf("_global_%s", this->name));
	}
}

/* class p_scope_node */
p_scope_node::p_scope_node()
{
	this->collection = 0;
	this->name = 0;
	this->scope_name = null_scope_name;
	this->scope = 0;
	new_list( &this->types );
	this->prefix = "";
	this->channel = PASSTHRU_DATA_CHANNEL;
}

p_scope_node::~p_scope_node()
{
}

void p_scope_node::set_collection(struct p_type_collection *ptc)
{
	this->collection = ptc;
	if( ptc )
		this->channel = ptc->get_channel();
}

struct p_type_collection *p_scope_node::get_collection()
{
	return( this->collection );
}

void p_scope_node::set_name(const char *the_name)
{
	this->name = the_name;
}

const char *p_scope_node::get_name()
{
	return( this->name );
}

void p_scope_node::set_prefix(const char *the_prefix)
{
	this->prefix = the_prefix;
}

const char *p_scope_node::get_prefix()
{
	return( this->prefix );
}

void p_scope_node::set_scope_name(cast_scoped_name the_name)
{
	this->scope_name = the_name;
}

cast_scoped_name *p_scope_node::get_scope_name()
{
	return( &this->scope_name );
}

void p_scope_node::set_scope(cast_scope *the_scope)
{
	this->scope = the_scope;
}

cast_scope *p_scope_node::get_scope()
{
	return( this->scope );
}

void p_scope_node::set_channel(data_channel_index the_channel)
{
	this->channel = the_channel;
}

data_channel_index p_scope_node::get_channel()
{
	return( this->channel );
}

void p_scope_node::add_type(struct p_type_node *ptn, int add_to_tail)
{
	if( add_to_tail ) {
		ptn->set_channel(this->channel);
		add_tail( &this->types, &ptn->link );
	}
	else {
		ptn->set_channel(this->channel);
		add_head( &this->types, &ptn->link );
	}
}

struct p_type_node *p_scope_node::find(const char *the_name)
{
	struct p_type_node *ptn, *retval = 0;
	
	ptn = (struct p_type_node *)this->types.head;
	while( ptn->link.succ && !retval ) {
		if( !strcmp( ptn->get_name(), the_name ) ) {
			retval = ptn;
		}
		ptn = (struct p_type_node *)ptn->link.succ;
	}
	return( retval );
}

void p_scope_node::ref_types(struct p_scope_node *psn)
{
	struct p_type_node *ptn, *ref_ptn;
	
	ptn = (struct p_type_node *)psn->types.head;
	while( ptn->link.succ ) {
		if( !(ptn->get_flags() & PTF_NO_REF) ) {
			ref_ptn = ptn->ref_type(this);
			add_tail( &this->types, &ref_ptn->link );
		}
		ptn = (struct p_type_node *)ptn->link.succ;
	}
}

void p_scope_node::add_defs()
{
	struct p_type_node *ptn, *ref_ptn;
	struct p_type_collection *ptc;
	struct p_scope_node *ref_psn;
	
	ptc = this->collection->get_collection_ref();
	if( ptc && (ref_psn = ptc->find_scope(this->name)) ) {
		ptn = (struct p_type_node *)this->types.head;
		while( ptn->link.succ ) {
			if( ptn->get_flags() & PTF_REF_ONLY ) {
				if( (!ptn->get_format() ||
				     strlen(ptn->get_format())) )
					ptn->add_def(this);
			} else if( (ref_ptn = ref_psn->
				    find(ptn->get_name())) ) {
				if( (!ref_ptn->get_format() ||
				     strlen(ref_ptn->get_format())) )
					ref_ptn->add_def(this);
			}
			ptn = (struct p_type_node *)ptn->link.succ;
		}
	} else {
		ptn = (struct p_type_node *)this->types.head;
		while( ptn->link.succ ) {
			if( (ptn->get_flags() & PTF_REF_ONLY) &&
			    (!ptn->get_format() ||
			     strlen(ptn->get_format())) )
				ptn->add_def(this);
			ptn = (struct p_type_node *)ptn->link.succ;
		}
	}
}


/* class p_type_collection */
p_type_collection::p_type_collection()
{
	new_list( &this->scopes );
	this->ptc_ref = 0;
	this->ref = aoi_ref_null;
	this->name = 0;
	this->id = 0;
	this->channel = PASSTHRU_DATA_CHANNEL;
	this->protection = CAST_PROT_NONE;
	this->attr_index = -1;
	this->tl = 0;
}

p_type_collection::~p_type_collection()
{
	struct p_scope_node *psn;
	
	while( (psn = (struct p_scope_node *)rem_head( &this->scopes )) ) {
		delete psn;
	}
}

void p_type_collection::set_id(const char *the_id)
{
	this->id = the_id;
	if( this->tl ) {
		add_tag(this->tl, "id", TAG_STRING, this->id);
	}
}

const char *p_type_collection::get_id()
{
	return this->id;
}

void p_type_collection::set_channel(data_channel_index the_channel)
{
	this->channel = the_channel;
}

data_channel_index p_type_collection::get_channel()
{
	return( this->channel );
}

void p_type_collection::set_attr_index(int idx)
{
	this->attr_index = idx;
}

int p_type_collection::get_attr_index()
{
	if( (this->attr_index == -1) && this->ptc_ref )
		return( this->ptc_ref->get_attr_index() );
	else
		return( this->attr_index );
}

void p_type_collection::set_tag_list(tag_list *the_tl)
{
	p_scope_node *psn;
	p_type_node *ptn;
	
	this->tl = the_tl;
	if( !(this->ptc_ref && this->ptc_ref->tl) ) {
		psn = (p_scope_node *)this->scopes.head;
		while( psn->link.succ ) {
			add_tag(tl,
				flick_asprintf("%s_scope",
					       psn->get_name()),
				TAG_CAST_SCOPED_NAME,
				*psn->get_scope_name());
			ptn = (struct p_type_node *)psn->types.head;
			while( ptn->link.succ ) {
				add_tag(tl, ptn->get_name(),
					TAG_CAST_TYPE,
					ptn->get_type());
				ptn = (struct p_type_node *)ptn->link.succ;
			}
			psn = (p_scope_node *)psn->link.succ;
		}
	}
}

tag_list *p_type_collection::get_tag_list()
{
	return( this->tl );
}

void p_type_collection::set_protection(cast_def_protection the_protection)
{
	this->protection = the_protection;
}

cast_def_protection p_type_collection::get_protection()
{
	return( this->protection );
}

void p_type_collection::set_ref(aoi_ref the_ref)
{
	this->ref = the_ref;
}

aoi_ref p_type_collection::get_ref()
{
	return( this->ref );
}

void p_type_collection::set_name(const char *the_name)
{
	this->name = the_name;
}

const char *p_type_collection::get_name()
{
	return( this->name );
}

void p_type_collection::set_collection_ref(struct p_type_collection *ptc)
{
	struct p_scope_node *psn, *my_psn;
	
	if( this->ptc_ref != ptc ) {
		this->ptc_ref = ptc;
		psn = (struct p_scope_node *)ptc->scopes.head;
		while( psn->link.succ ) {
			my_psn = this->find_scope(psn->get_name());
			my_psn->ref_types(psn);
			psn = (struct p_scope_node *)psn->link.succ;
		}
		if( this->tl && (ptc->get_attr_index() != -1) ) {
			add_tag(this->tl, "typedef_ref", TAG_INTEGER,
				ptc->get_attr_index());
		}
	}
}

struct p_type_collection *p_type_collection::get_collection_ref()
{
	return( this->ptc_ref );
}

struct p_type_node *p_type_collection::add_type(const char *scope_name,
						struct p_type_node *ptn,
						int add_to_tail,
						int add_to_ref)
{
	struct p_type_node *retval = 0;
	struct p_scope_node *psn;
	
	if( this->ptc_ref && add_to_ref ) {
		if( (psn = this->ptc_ref->find_scope(scope_name)) ) {
			assert(!psn->find(ptn->get_name()));
			psn->add_type( ptn, add_to_tail );
			psn = this->find_scope(scope_name);
			assert(!psn->find(ptn->get_name()));
			psn->add_type( retval = ptn->ref_type(psn),
				       add_to_tail );
		}
	} else {
		if( (psn = this->find_scope(scope_name)) ) {
			assert(!psn->find(ptn->get_name()));
			psn->add_type( ptn, add_to_tail );
			retval = ptn;
		}
	}
	return( retval );
}

void p_type_collection::define_types()
{
	if( this->ptc_ref ) {
		struct p_scope_node *psn;
		
		psn = (struct p_scope_node *)this->scopes.head;
		while( psn->link.succ ) {
			psn->add_defs();
			psn = (struct p_scope_node *)psn->link.succ;
		}
	}
}

void p_type_collection::add_scope(struct p_scope_node *psn)
{
	psn->set_collection( this );
	add_tail( &this->scopes, &psn->link );
}

struct p_scope_node *p_type_collection::find_scope(const char *the_name)
{
	struct p_scope_node *psn, *retval = 0;
	
	psn = (struct p_scope_node *)this->scopes.head;
	while( psn->link.succ && !retval ) {
		if( !strcmp( psn->get_name(), the_name ) ) {
			retval = psn;
		}
		psn = (struct p_scope_node *)psn->link.succ;
	}
	return( retval );
}

struct p_type_node *p_type_collection::find_type(const char *the_name)
{
	struct p_type_node *retval = 0;
	struct p_scope_node *psn;
	
	psn = (struct p_scope_node *)this->scopes.head;
	while( psn->link.succ && !retval ) {
		retval = psn->find(the_name);
		psn = (struct p_scope_node *)psn->link.succ;
	}
	return( retval );
}

p_type_collection *p_type_collection::find_collection(struct dl_list *list,
						      aoi_ref ref)
{
	struct p_type_collection *ptc, *retval = 0;
	
	ptc = (struct p_type_collection *)list->head;
	while( ptc->link.succ && !retval ) {
		if( ptc->ref == ref )
			retval = ptc;
		ptc = (struct p_type_collection *)ptc->link.succ;
	}
	return( retval );
}

p_type_collection *p_type_collection::find_collection(struct dl_list *list,
						      const char *name,
						      cast_scope *scope)
{
	struct p_type_collection *ptc, *retval = 0;
	
	ptc = (struct p_type_collection *)list->head;
	while( ptc->link.succ && !retval ) {
		if( !strcmp( name, ptc->name ) &&
		    (ptc->find_scope("default")->get_scope() == scope) )
			retval = ptc;
		ptc = (struct p_type_collection *)ptc->link.succ;
	}
	return( retval );
}

/* End of file. */

