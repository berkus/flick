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

#include <string.h>

#include <mom/libmeta.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>
#include <mom/c/pbe.hh>
#include <mom/c/be/presentation_impl.hh>

/* This file contains all functions and data needed in order to generate
   a TAO implementation for a CORBA C++ presentation. */

enum {
	TTIK_NONE,
	TTIK_ROOT,
	TTIK_CONTAINER,
	TTIK_BYTES,
	TTIK_NAMED,
	TTIK_XLATE,
	TTIK_NUMBER,
	TTIK_CHAR,
	TTIK_INDIRECT
};

class tao_typecode_init {

public:
	tao_typecode_init();
	~tao_typecode_init();
	
	void encode_string(const char *str);
	void encode_name(const char *name);
	void encode_typecode(struct tao_typecode_init *tti);
	void encode_xlate(const char *macro, int number);
	void encode_number(int number);
	void encode_char(char letter);
	void encode_indirect(struct tao_typecode_init *parent);
	
	void print(int indent);
	int length();
	
	struct tao_typecode_init *find_parent(int pres_index);
	
	void set_kind(int kind);
	int get_kind();
	
	void set_parent(struct tao_typecode_init *tti);
	struct tao_typecode_init *get_parent();
	
	void set_comment(const char *comment);
	const char *get_comment();
	
	void set_pres_index(int pres_index);
	int get_pres_index();
	
	struct list_node link;
	unsigned long kind;
	struct tao_typecode_init *parent;
	union {
		struct {
			struct dl_list children;
			int pres_index;
		} container;
		struct {
			const char *bytes_val;
			int bytes_len;
		} bytes;
		struct {
			const char *macro;
			int number;
		} xlate;
		const char *named;
		int number;
		char letter;
	} data_u;
	const char *comment;
};

tao_typecode_init::tao_typecode_init()
{
	this->kind = TTIK_NONE;
	this->parent = 0;
	this->comment = 0;
}

tao_typecode_init::~tao_typecode_init()
{
}

void tao_typecode_init::encode_string(const char *str)
{
	this->kind = TTIK_BYTES;
	this->data_u.bytes.bytes_val = str;
	this->data_u.bytes.bytes_len = strlen( str ) + 1;
}

void tao_typecode_init::encode_name(const char *name)
{
	this->kind = TTIK_NAMED;
	this->data_u.named = name;
}

void tao_typecode_init::encode_typecode(struct tao_typecode_init *tti)
{
	if( this->kind == TTIK_NONE ) {
		this->kind = TTIK_CONTAINER;
		new_list( &this->data_u.container.children );
	}
	tti->set_parent( this );
	add_tail( &this->data_u.container.children, &tti->link );
}

void tao_typecode_init::encode_xlate(const char *macro, int number)
{
	this->kind = TTIK_XLATE;
	this->data_u.xlate.macro = macro;
	this->data_u.xlate.number = number;
}

void tao_typecode_init::encode_number(int number)
{
	this->kind = TTIK_NUMBER;
	this->data_u.number = number;
}

void tao_typecode_init::encode_char(char letter)
{
	this->kind = TTIK_CHAR;
	this->data_u.letter = letter;
}

void tao_typecode_init::encode_indirect(struct tao_typecode_init *parent_tti)
{
	this->kind = TTIK_INDIRECT;
	this->data_u.number = -parent_tti->length() - 4;
}

void tao_typecode_init::print(int indent)
{
	switch( this->kind ) {
	case TTIK_CONTAINER:
		w_i_printf(indent,
			   "%d /* encapsulation length */,\n",
			   this->length() - 4);
	case TTIK_ROOT: {
		struct tao_typecode_init *tti;
		
		tti = (struct tao_typecode_init *)this->data_u.container.
			children.head;
		while( tti->link.succ ) {
			tti->print( indent + 1 );
			tti = (struct tao_typecode_init *)tti->link.succ;
			if( tti->link.succ )
				w_printf(",\n");
		}
		break;
	}
	case TTIK_BYTES: {
		int lpc;
		
		w_i_printf(indent, "%d, ", this->data_u.bytes.bytes_len);
		for( lpc = 0;
		     lpc < (this->data_u.bytes.bytes_len);
		     lpc++ ) {
			if( lpc == 0 )
				w_printf("ACE_NTOHL(0x");
			else if( (lpc % 4) == 0 )
				w_printf("), ACE_NTOHL(0x");
			w_printf("%02x", this->data_u.bytes.bytes_val[lpc]);
		}
		for( ; (lpc % 4); lpc++ ) {
			w_printf("00");
		}
		w_printf( ")" );
		break;
	}
	case TTIK_NAMED:
		w_i_printf(indent, "%s", this->data_u.named);
		break;
	case TTIK_CHAR:
		w_i_printf(indent, "\'%c\'", this->data_u.letter);
		break;
	case TTIK_XLATE:
		w_i_printf(indent, "%s(0x%04x)",
			   this->data_u.xlate.macro,
			   this->data_u.xlate.number);
		break;
	case TTIK_NUMBER:
		w_i_printf(indent, "%d", this->data_u.number);
		break;
	case TTIK_INDIRECT:
		w_i_printf(indent, "-1, %d", this->data_u.number);
		break;
	default:
		break;
	}
	if( this->comment ) {
		w_printf(" /* %s", this->comment);
		switch( this->kind ) {
		case TTIK_BYTES:
			w_printf(" = %s", this->data_u.bytes.bytes_val);
			break;
		default:
			break;
		}
		w_printf(" */");
	}
}

int tao_typecode_init::length()
{
	int retval = 0;
	
	switch( this->kind ) {
	case TTIK_ROOT:
	case TTIK_CONTAINER: {
		struct tao_typecode_init *tti;
		
		tti = (struct tao_typecode_init *)this->data_u.container.
			children.head;
		while( tti->link.succ ) {
			retval += tti->length();
			tti = (struct tao_typecode_init *)tti->link.succ;
		}
		retval += 4;
		break;
	}
	case TTIK_BYTES:
		retval = ((this->data_u.bytes.bytes_len + 3) & ~3) + 4;
		break;
	case TTIK_CHAR:
	case TTIK_XLATE:
	case TTIK_NUMBER:
	case TTIK_NAMED:
		retval = 4;
		break;
	case TTIK_INDIRECT:
		retval = 8;
		break;
	}
	return( retval );
}

struct tao_typecode_init *tao_typecode_init::find_parent(int pres_index)
{
	struct tao_typecode_init *curr, *retval = 0;
	
	curr = this;
	while( curr ) {
		if( pres_index == curr->data_u.container.pres_index )
			retval = curr;
		curr = curr->parent;
	}
	return( retval );
}

void tao_typecode_init::set_kind(int the_kind)
{
	switch( the_kind ) {
	case TTIK_ROOT:
	case TTIK_CONTAINER:
		if( this->kind == TTIK_NONE ) {
			this->data_u.container.pres_index = -1;
			new_list( &this->data_u.container.children );
		}
		break;
	case TTIK_BYTES:
		this->data_u.bytes.bytes_val = 0;
		this->data_u.bytes.bytes_len = 0;
		break;
	case TTIK_NAMED:
		this->data_u.named = 0;
		break;
	}
	this->kind = the_kind;
}

int tao_typecode_init::get_kind()
{
	return( this->kind );
}

void tao_typecode_init::set_parent(struct tao_typecode_init *tti)
{
	this->parent = tti;
}

struct tao_typecode_init *tao_typecode_init::get_parent()
{
	return( this->parent );
}

void tao_typecode_init::set_comment(const char *the_comment)
{
	this->comment = the_comment;
}

const char *tao_typecode_init::get_comment()
{
	return( this->comment );
}

void tao_typecode_init::set_pres_index(int pres_index)
{
	this->data_u.container.pres_index = pres_index;
}

int tao_typecode_init::get_pres_index()
{
	return( this->data_u.container.pres_index );
}

/* This function will add any variables for the poa tie */
void tao_impl_tie_data(cast_scope *scope, tag_list */*tl*/)
{
	int cdef;
	
	/* T *ptr_ */
	cdef = cast_add_def(scope,
			    cast_new_scoped_name("ptr_", NULL),
			    CAST_SC_NONE,
			    CAST_VAR_DECL,
			    PASSTHRU_DATA_CHANNEL,
			    CAST_PROT_PRIVATE);
	scope->cast_scope_val[cdef].u.cast_def_u_u.var_type =
		cast_new_pointer_type(cast_new_type_name("T"));
	/* PortableServer::POA_var poa_ */
	cdef = cast_add_def(scope,
			    cast_new_scoped_name("poa_", NULL),
			    CAST_SC_NONE,
			    CAST_VAR_DECL,
			    PASSTHRU_DATA_CHANNEL,
			    CAST_PROT_PRIVATE);
	scope->cast_scope_val[cdef].u.cast_def_u_u.var_type =
		cast_new_type_scoped_name(
			cast_new_scoped_name("PortableServer",
					     "POA_var", NULL));
	/* CORBA::Boolean rel_ */
	cdef = cast_add_def(scope,
			    cast_new_scoped_name("rel_", NULL),
			    CAST_SC_NONE,
			    CAST_VAR_DECL,
			    PASSTHRU_DATA_CHANNEL,
			    CAST_PROT_PRIVATE);
	scope->cast_scope_val[cdef].u.cast_def_u_u.var_type =
		cast_new_type_scoped_name(
			cast_new_scoped_name("CORBA", "Boolean", NULL));
}

/* This function will add any variables to the _var types */
void tao_impl_var_data(cast_scope *scope, tag_list *tl)
{
	int cdef;
	
	cdef = cast_add_def(scope,
			    cast_new_scoped_name("ptr_", NULL),
			    CAST_SC_NONE,
			    CAST_VAR_DECL,
			    PASSTHRU_DATA_CHANNEL,
			    CAST_PROT_PRIVATE);
	scope->cast_scope_val[cdef].u.cast_def_u_u.var_type =
		find_tag(tl, "pointer")->data.tag_data_u.ctype;
}

/* This function will add any variables to the _forany types */
void tao_impl_forany_data(cast_scope *scope, tag_list *tl)
{
	cast_scoped_name scn;
	cast_type type;
	int cdef;
	
	cdef = cast_add_def(scope,
			    cast_new_scoped_name("ptr_", NULL),
			    CAST_SC_NONE,
			    CAST_VAR_DECL,
			    PASSTHRU_DATA_CHANNEL,
			    CAST_PROT_PRIVATE);
	scope->cast_scope_val[cdef].u.cast_def_u_u.var_type =
		find_tag(tl, "pointer")->data.tag_data_u.ctype;
	cdef = cast_add_def(scope,
			    cast_new_scoped_name("nocopy_", NULL),
			    CAST_SC_NONE,
			    CAST_VAR_DECL,
			    PASSTHRU_DATA_CHANNEL,
			    CAST_PROT_PRIVATE);
	scn = cast_new_scoped_name("CORBA", "Boolean", NULL);
	type = cast_new_type_scoped_name(scn);
	scope->cast_scope_val[cdef].u.cast_def_u_u.var_type = type;
}

/* This function will add any variables to _out types */
void tao_impl_out_data(cast_scope *scope, tag_list *tl)
{
	cast_type type;
	int cdef;
	
	type = find_tag(tl, "pointer")->data.tag_data_u.ctype;
	type = cast_new_reference_type(type);
	cdef = cast_add_def(scope,
			    cast_new_scoped_name("ptr_", NULL),
			    CAST_SC_NONE,
			    CAST_VAR_DECL,
			    PASSTHRU_DATA_CHANNEL,
			    CAST_PROT_PRIVATE);
	scope->cast_scope_val[cdef].u.cast_def_u_u.var_type = type;
}

void tao_impl_managed_struct_preprocess(pres_c_1 *pres, tag_list *tl)
{
	cast_scoped_name scn;
	cast_scope *scope;
	const char *slot_name;
	cast_type type;
	tag_item *ti, *member_ti, *pt_ti;
	int cdef;
	
	scn = find_tag(tl, "definition")->data.tag_data_u.ctype->
		cast_type_u_u.name;
	pt_ti = find_tag(pres->pres_attrs, "pres_type");
	scope = &pres->cast;
	cdef = cast_find_def(&scope, scn, CAST_TYPE|CAST_TYPEDEF);
	type = scope->cast_scope_val[cdef].u.cast_def_u_u.type;
	while( cdef != -1 ) {
		type = scope->cast_scope_val[cdef].u.cast_def_u_u.type;
		switch( type->kind ) {
		case CAST_TYPE_TEMPLATE:
			type = type->cast_type_u_u.template_type.def;
			break;
		default:
			break;
		}
		if( type->kind == CAST_TYPE_AGGREGATE ) {
			scope = &type->cast_type_u_u.agg_type.scope;
			cdef = -1;
		} else {
			cdef = cast_find_def_pos(&scope, cdef + 1, scn,
						 CAST_TYPEDEF|
						 CAST_TYPE);
			if( cdef == -1 )
				scope = 0;
		}
	}
	member_ti = find_tag(tl, "member");
	if( (ti = find_tag(tl, "managed-strings")) ) {
		cast_type string_type;
		int len, lpc;
		
		string_type = cast_new_type_name("TAO_String_Manager");
		len = tag_data_length(&ti->data);
		for( lpc = 0; lpc < len; lpc++ ) {
			slot_name =
				find_tag(get_tag_data(&member_ti->data,
						      get_tag_data(&ti->data,
								   lpc).i).tl,
					 "name")->data.tag_data_u.str;
			scn = cast_new_scoped_name(slot_name, NULL);
			cdef = cast_find_def(&scope, scn,
					     CAST_VAR_DECL|CAST_VAR_DEF);
			switch(scope->cast_scope_val[cdef].u.kind) {
			case CAST_VAR_DECL:
				scope->cast_scope_val[cdef].u.cast_def_u_u.
					var_type = string_type;
				break;
			case CAST_VAR_DEF:
				scope->cast_scope_val[cdef].u.cast_def_u_u.
					var_def.type = string_type;
				break;
			default:
				break;
			}
		}
	}
	if( (ti = find_tag(tl, "managed-objects")) ) {
		cast_scoped_name field_name = null_scope_name;
		cast_type obj_type, var_type, field_type;
		tag_list *member_tl, *obj_tl;
		int len, lpc;
		
		len = tag_data_length(&ti->data);
		for( lpc = 0; lpc < len; lpc++ ) {
			member_tl = get_tag_data(&member_ti->data,
						 get_tag_data(&ti->data,
							      lpc).i).tl;
			slot_name = find_tag(member_tl, "name")->data.
				tag_data_u.str;
			scn = cast_new_scoped_name(slot_name, NULL);
			cdef = cast_find_def(&scope, scn,
					     CAST_VAR_DECL|CAST_VAR_DEF);
			obj_tl = get_tag_data(&pt_ti->data,
					      find_tag(member_tl,
						       "pres_index")->
					      data.tag_data_u.i).tl;
			obj_type = find_tag(obj_tl,
					    "_global_definition")->data.
				tag_data_u.ctype;
			var_type = find_tag(obj_tl,
					    "_global_smart_pointer")->data.
				tag_data_u.ctype;
			cast_add_scope_name(
				&field_name,
				"TAO_Object_Field_T",
				cast_set_template_arg_array(
					0,
					cast_new_template_arg_type(obj_type),
					cast_new_template_arg_type(var_type),
					NULL)
				);
			field_type = cast_new_type_scoped_name(field_name);
			switch(scope->cast_scope_val[cdef].u.kind) {
			case CAST_VAR_DECL:
				scope->cast_scope_val[cdef].u.cast_def_u_u.
					var_type = field_type;
				break;
			case CAST_VAR_DEF:
				scope->cast_scope_val[cdef].u.cast_def_u_u.
					var_def.type = field_type;
				break;
			default:
				break;
			}
		}
	}
}

void tao_impl_managed_array_preprocess(pres_c_1 *pres, tag_list *tl)
{
	tag_item *ti, *pt_ti;
	
	pt_ti = find_tag(pres->pres_attrs, "pres_type");
	if( (ti = find_tag(tl, "managed")) ) {
		cast_type type, slice_type;
		cast_scoped_name scn;
		tag_item *slice_ti;
		cast_scope *scope;
		int cdef;
		
		scn = find_tag(tl, "definition")->data.tag_data_u.ctype->
			cast_type_u_u.name;
		scope = &pres->cast;
		cdef = cast_find_def(&scope, scn, CAST_TYPE|CAST_TYPEDEF);
		type = scope->cast_scope_val[cdef].u.cast_def_u_u.type;
		while( type->cast_type_u_u.array_type.element_type->
		       kind == CAST_TYPE_ARRAY )
			type = type->cast_type_u_u.array_type.element_type;
		if( !strcmp(ti->data.tag_data_u.str, "string") ) {
			slice_type = cast_new_type_name("TAO_String_Manager");
		} else if( !strcmp(ti->data.tag_data_u.str, "object") ) {
			cast_scoped_name slice_name = null_scope_name;
			cast_type obj_type = type->cast_type_u_u.
				array_type.element_type;
			cast_type var_type;
			tag_list *obj_tl;
			
			obj_tl = get_tag_data(&pt_ti->data,
					      find_tag(tl,
						       "slice_pres_index")->
					      data.tag_data_u.i).tl;
			var_type = find_tag(obj_tl,
					    "_global_smart_pointer")->data.
				tag_data_u.ctype;
			cast_add_scope_name(
				&slice_name,
				"TAO_Object_Field_T",
				cast_set_template_arg_array(
					0,
					cast_new_template_arg_type(obj_type),
					cast_new_template_arg_type(var_type),
					NULL)
				);
			slice_type = cast_new_type_scoped_name(slice_name);
		} else
			slice_type = 0;
		type->cast_type_u_u.array_type.element_type = slice_type;
		
		slice_ti = find_tag(tl, "array_slice");
		scn = slice_ti->data.tag_data_u.ctype->cast_type_u_u.name;
		scope = &pres->cast;
		cdef = cast_find_def(&scope, scn, CAST_TYPE|CAST_TYPEDEF);
		scope->cast_scope_val[cdef].u.cast_def_u_u.type = slice_type;
		slice_ti->data.tag_data_u.ctype = slice_type;
		slice_ti = find_tag(tl, "pointer");
		slice_ti->data.tag_data_u.ctype =
			cast_new_pointer_type(slice_type);
	}
}

void tao_impl_managed_union_preprocess(pres_c_1 *pres, tag_list *tl)
{
	tag_item *ti, *member, *pt_ti;
	cast_scoped_name scn;
	int cdef, lpc, len;
	cast_scope *scope;
	tag_list *mem_tl;
	cast_type type;
	
	if( !(member = find_tag(tl, "member")) )
		return;
	
	pt_ti = find_tag(pres->pres_attrs, "pres_type");
	scn = find_tag(tl, "definition")->data.tag_data_u.ctype->
		cast_type_u_u.name;
	scn = cast_copy_scoped_name(&scn);
	scope = &pres->cast;
	cdef = cast_find_def(&scope, scn, CAST_TYPE|CAST_TYPEDEF);
	type = scope->cast_scope_val[cdef].u.cast_def_u_u.var_def.type;
	cast_class_add_parent(type, CAST_PARENT_PUBLIC,
			      cast_new_scoped_name("TAO_Base_Union", NULL));
	cast_add_scope_name(&scn,
			    find_tag(tl, "union_name")->data.tag_data_u.str,
			    null_template_arg_array);
	scope = &pres->cast;
	cdef = cast_find_def(&scope, scn, CAST_VAR_DEF);
	scope = &scope->cast_scope_val[cdef].u.cast_def_u_u.var_def.
		type->cast_type_u_u.agg_type.scope;
	len = tag_data_length(&member->data);
	for( lpc = 0; lpc < len; lpc++ ) {
		mem_tl = get_tag_data(&member->data, lpc).tl;
		if( !(ti = find_tag(mem_tl, "managed")) )
			continue;
		if( !strcmp(ti->data.tag_data_u.str, "object") ) {
			cast_scoped_name field_name = null_scope_name;
			cast_type obj_type, var_type;
			tag_list *obj_tl;
			char *slot_name;
			
			obj_tl = get_tag_data(&pt_ti->data,
					      find_tag(mem_tl, "pres_index")->
					      data.tag_data_u.i).tl;
			slot_name = find_tag(mem_tl, "name")->data.
				tag_data_u.str;
			obj_type = find_tag(obj_tl,
					    "_global_definition")->data.
				tag_data_u.ctype;
			var_type = find_tag(obj_tl,
					    "_global_smart_pointer")->data.
				tag_data_u.ctype;;
			scn = cast_new_scoped_name(slot_name, NULL);
			cdef = cast_find_def(&scope, scn, CAST_VAR_DEF);
			cast_add_scope_name(
				&field_name,
				"TAO_Object_Field_T",
				cast_set_template_arg_array(
					0,
					cast_new_template_arg_type(obj_type),
					cast_new_template_arg_type(var_type),
					NULL)
				);
			obj_type = cast_new_type_scoped_name(field_name);
			obj_type = cast_new_pointer_type(obj_type);
			scope->cast_scope_val[cdef].u.cast_def_u_u.var_def.
				type = obj_type;
		}
	}
}

void tao_impl_sequence_preprocess(pres_c_1 *pres, tag_list *tl)
{
	int slice_pres_index;
	tag_list *slice_tl;
	char *slice_type;
	
	slice_pres_index = find_tag(tl, "slice_pres_index")->data.
		tag_data_u.i;
	slice_tl = get_tag_data(&find_tag(pres->pres_attrs, "pres_type")->data,
				slice_pres_index).tl;
	slice_type = find_tag(slice_tl, "idl_type")->data.tag_data_u.str;
	if( !strcmp(slice_type, "string") &&
	    !find_tag(find_tag(tl, "main")->data.tag_data_u.tl,
		      "T &operator[](ulong)") ) {
		data_channel_index decl_channel, impl_channel;
		cast_scoped_name scn;
		cast_scope *scope;
		cast_type type;
		int cdef;
		
		scn = find_tag(tl, "_global_definition")->data.tag_data_u.
			ctype->cast_type_u_u.name;
		scope = &pres->cast;
		cdef = cast_find_def(&scope, scn, CAST_TYPE|CAST_TYPEDEF);
		decl_channel = scope->cast_scope_val[cdef].channel;
		impl_channel = meta_find_channel(&pres->meta_data,
						 pres->meta_data.channels.
						 channels_val[decl_channel].
						 input,
						 pres->meta_data.channels.
						 channels_val[decl_channel].id,
						 DATA_CHANNEL_IMPL);
		type = scope->cast_scope_val[cdef].u.cast_def_u_u.type;
		while( cdef != -1 ) {
			type = scope->cast_scope_val[cdef].u.cast_def_u_u.type;
			switch( type->kind ) {
			case CAST_TYPE_TEMPLATE:
				type = type->cast_type_u_u.template_type.def;
				break;
			default:
				break;
			}
			if( type->kind == CAST_TYPE_AGGREGATE ) {
				scope = &type->cast_type_u_u.agg_type.scope;
				cdef = -1;
			} else {
				cdef = cast_find_def_pos(&scope, cdef + 1, scn,
							 CAST_TYPEDEF|
							 CAST_TYPE);
				if( cdef == -1 )
					scope = 0;
			}
		}
		if( scope ) {
			tag_list *main_tl;
			cast_type type2;
			
			main_tl = find_tag(tl, "main")->data.tag_data_u.tl;
			type = cast_new_type_name(
				"TAO_SeqElem_String_Manager");
			type2 = cast_new_type_scoped_name(
				cast_new_scoped_name("CORBA", "ULong", NULL));
			pres_function(pres, main_tl,
				      scn,
				      cast_new_scoped_name("operator[]", NULL),
				      PFA_Scope, scope,
				      PFA_DeclChannel, decl_channel,
				      PFA_ImplChannel, impl_channel,
				      PFA_FunctionKind, "T &operator[](ulong)",
				      PFA_ReturnType, type,
				      PFA_Parameter, type2, "i", NULL,
				      PFA_Spec, CAST_FUNC_CONST,
				      PFA_Protection, CAST_PROT_PUBLIC,
				      PFA_TAG_DONE);
			relink_tag_list(main_tl);
		}
		scn = find_tag(tl, "_global_smart_pointer")->data.tag_data_u.
			ctype->cast_type_u_u.name;
		scope = &pres->cast;
		cdef = cast_find_def(&scope, scn, CAST_TYPE|CAST_TYPEDEF);
		type = scope->cast_scope_val[cdef].u.cast_def_u_u.type;
		while( cdef != -1 ) {
			type = scope->cast_scope_val[cdef].u.cast_def_u_u.type;
			switch( type->kind ) {
			case CAST_TYPE_TEMPLATE:
				type = type->cast_type_u_u.template_type.def;
				break;
			default:
				break;
			}
			if( type->kind == CAST_TYPE_AGGREGATE ) {
				scope = &type->cast_type_u_u.agg_type.scope;
				cdef = -1;
			} else {
				cdef = cast_find_def_pos(&scope, cdef + 1, scn,
							 CAST_TYPEDEF|
							 CAST_TYPE);
				if( cdef == -1 )
					scope = 0;
			}
		}
		if( scope ) {
			tag_list *main_tl;
			cast_type type2;
			
			main_tl = find_tag(tl, "var")->data.tag_data_u.tl;
			type = cast_new_type_name(
				"TAO_SeqElem_String_Manager");
			type2 = cast_new_type_scoped_name(
				cast_new_scoped_name("CORBA", "ULong", NULL));
			pres_function(pres, main_tl,
				      scn,
				      cast_new_scoped_name("operator[]", NULL),
				      PFA_Scope, scope,
				      PFA_DeclChannel, decl_channel,
				      PFA_ImplChannel, impl_channel,
				      PFA_FunctionKind,
				        "T_slice operator[](ulong)",
				      PFA_ReturnType, type,
				      PFA_Parameter, type2, "i", NULL,
				      PFA_Protection, CAST_PROT_PUBLIC,
				      PFA_TAG_DONE);
			relink_tag_list(main_tl);
		}
		scn = find_tag(tl, "_global_out_pointer")->data.tag_data_u.
			ctype->cast_type_u_u.name;
		scope = &pres->cast;
		cdef = cast_find_def(&scope, scn, CAST_TYPE|CAST_TYPEDEF);
		type = scope->cast_scope_val[cdef].u.cast_def_u_u.type;
		while( cdef != -1 ) {
			type = scope->cast_scope_val[cdef].u.cast_def_u_u.type;
			switch( type->kind ) {
			case CAST_TYPE_TEMPLATE:
				type = type->cast_type_u_u.template_type.def;
				break;
			default:
				break;
			}
			if( type->kind == CAST_TYPE_AGGREGATE ) {
				scope = &type->cast_type_u_u.agg_type.scope;
				cdef = -1;
			} else {
				cdef = cast_find_def_pos(&scope, cdef + 1, scn,
							 CAST_TYPEDEF|
							 CAST_TYPE);
				if( cdef == -1 )
					scope = 0;
			}
		}
		if( scope ) {
			tag_list *main_tl;
			cast_type type2;
			
			main_tl = find_tag(tl, "out")->data.tag_data_u.tl;
			type = cast_new_type_name(
				"TAO_SeqElem_String_Manager");
			type2 = cast_new_type_scoped_name(
				cast_new_scoped_name("CORBA", "ULong", NULL));
			pres_function(pres, main_tl,
				      scn,
				      cast_new_scoped_name("operator[]", NULL),
				      PFA_Scope, scope,
				      PFA_DeclChannel, decl_channel,
				      PFA_ImplChannel, impl_channel,
				      PFA_FunctionKind,
				        "T_slice operator[](ulong)",
				      PFA_ReturnType, type,
				      PFA_Parameter, type2, "i", NULL,
				      PFA_Protection, CAST_PROT_PUBLIC,
				      PFA_TAG_DONE);
			relink_tag_list(main_tl);
		}
	}
}

const char *corba_idl_type_to_tk_map[] = {
	"null", "CORBA::tk_null",
	"void", "CORBA::tk_void",
	"Short", "CORBA::tk_short",
	"Long", "CORBA::tk_long",
	"LongLong", "CORBA::tk_longlong",
	"UShort", "CORBA::tk_ushort",
	"ULong", "CORBA::tk_ulong",
	"ULongLong", "CORBA::tk_ulonglong",
	"Float", "CORBA::tk_float",
	"Double", "CORBA::tk_double",
	"LongDouble", "CORBA::tk_longdouble",
	"Fixed", "CORBA::tk_fixed",
	"Boolean", "CORBA::tk_boolean",
	"Char", "CORBA::tk_char",
	"WChar", "CORBA::tk_wchar",
	"Octet", "CORBA::tk_octet",
	"Any", "CORBA::tk_any",
	"TypeCode", "CORBA::tk_TypeCode",
	"Principal", "CORBA::tk_Principal",
	"interface", "CORBA::tk_objref",
	"struct", "CORBA::tk_struct",
	"union", "CORBA::tk_union",
	"enum", "CORBA::tk_enum",
	"string", "CORBA::tk_string",
	"wstring", "CORBA::tk_wstring",
	"sequence", "CORBA::tk_sequence",
	"array", "CORBA::tk_array",
	"typedef", "CORBA::tk_alias",
	"exception", "CORBA::tk_except",
	0
};

/* Convert our idl type names to CORBA::tk_* names */
const char *corba_idl_type_to_tk(const char *idl_type)
{
	const char *retval = 0;
	int lpc;
	
	for(lpc = 0; !retval && corba_idl_type_to_tk_map[lpc]; lpc += 2) {
		if( !strcmp(corba_idl_type_to_tk_map[lpc], idl_type) ) {
			retval = corba_idl_type_to_tk_map[lpc + 1];
		}
	}
	return( retval );
}

const char *tao_discrim_translator_map[] = {
	"Short", "ACE_IDL_NSTOHL",
	"UShort", "ACE_IDL_NSTOHL",
	"Long", "",
	"ULong", "",
	"Char", "",
	"enum", "",
	"Boolean", "",
	0
};

const char *tao_discrim_translator(const char *idl_type)
{
	const char *retval = 0;
	int lpc;
	
	for(lpc = 0; !retval && tao_discrim_translator_map[lpc]; lpc += 2) {
		if( !strcmp(tao_discrim_translator_map[lpc], idl_type) ) {
			retval = tao_discrim_translator_map[lpc + 1];
		}
	}
	if( !retval )
		panic("Couldn't match idl_type");
	return( retval );
}

/* Create the basic type code stuff */
void tao_impl_tc_base(struct tao_typecode_init *root_tti,
		      tag_list *tl, int pt_index)
{
	struct tao_typecode_init *tti;
	tag_item *ti;
	
	tti = new tao_typecode_init;
	tti->encode_name("TAO_ENCAP_BYTE_ORDER");
	tti->set_comment("byte order");
	root_tti->encode_typecode(tti);
	if( (ti = find_tag(tl, "id")) ) {
		tti = new tao_typecode_init;
		tti->encode_string(ti->data.tag_data_u.str);
		tti->set_comment("repository ID");
		root_tti->encode_typecode(tti);
	}
	if( (ti = find_tag(tl, "name")) ) {
		char *name = ti->data.tag_data_u.str;
		
		tti = new tao_typecode_init;
		tti->encode_string(name);
		tti->set_comment("name");
		root_tti->encode_typecode(tti);
	}
	root_tti->set_pres_index(pt_index);
}

struct tao_typecode_init *tao_impl_tc(pres_c_1 *pres,
				      struct tao_typecode_init *root_tti,
				      tag_list *tl,
				      tag_list *ref_tl,
				      int pt_index);

/* Add a member typecode */
void tao_impl_tc_add_member(pres_c_1 *pres,
			    struct tao_typecode_init *root_tti,
			    tag_list *tl,
			    tag_list *ref_tl,
			    int pt_index)
{
	struct tao_typecode_init *tti, *orig = 0;
	const char *idl_type;
	
	tti = new tao_typecode_init;
	if( find_tag(tl, "typedef_ref") && !find_tag(tl, "anonymous") )
		idl_type = "typedef";
	else
		idl_type = find_tag(tl, "idl_type")->data.tag_data_u.str;
	tti->encode_name(corba_idl_type_to_tk(idl_type));
	root_tti->encode_typecode(tti);
	if( (pt_index != -1) &&
	    (orig = root_tti->find_parent(pt_index)) ) {
		/* Its a recursive type, do indirect */
		tti->encode_indirect(orig);
		root_tti->encode_typecode(tti);
	} else {
		tao_impl_tc(pres, root_tti, tl, ref_tl, pt_index);
	}
}

/* Do the main body of a type code for a type. */
struct tao_typecode_init *tao_impl_tc(pres_c_1 *pres,
				      struct tao_typecode_init *root_tti,
				      tag_list *tl,
				      tag_list *ref_tl,
				      int pt_index)
{
	struct tao_typecode_init *my_tti = 0;
	tag_item *pres_type;
	const char *idl_type;
	int lpc;
	
	if( find_tag(tl, "typedef_ref") && !find_tag(tl, "anonymous") )
		idl_type = "typedef";
	else
		idl_type = find_tag(tl, "idl_type")->data.tag_data_u.str;
	pres_type = find_tag(pres->pres_attrs, "pres_type");
	if( !strcmp( idl_type, "interface" ) ) {
		my_tti = new tao_typecode_init;
		my_tti->set_parent(root_tti);
		tao_impl_tc_base(my_tti, tl, pt_index);
	} else if( !strcmp( idl_type, "struct" ) ||
		 !strcmp( idl_type, "exception" ) ) {
		struct tao_typecode_init *tti;
		tag_list *member_tl, *pt_tl;
		tag_item *member_ti;
		int member_count;
		int mem_index;
		
		my_tti = new tao_typecode_init;
		my_tti->set_parent(root_tti);
		tao_impl_tc_base(my_tti, tl, pt_index);
		if( (member_ti = find_tag(tl, "member")) ) {
			member_count = tag_data_length(&member_ti->data);
		} else
			member_count = 0;
		tti = new tao_typecode_init;
		tti->encode_number(member_count);
		tti->set_comment("member count");
		my_tti->encode_typecode(tti);
		for( lpc = 0; lpc < member_count; lpc++ ) {
			member_tl = get_tag_data(&member_ti->data, lpc).tl;
			mem_index = find_tag(member_tl, "pres_index")->data.
				tag_data_u.i;
			pt_tl = get_tag_data(&pres_type->data, mem_index).tl;
			tti = new tao_typecode_init;
			tti->encode_string(find_tag(member_tl, "name")->
					   data.tag_data_u.str);
			tti->set_comment("name");
			my_tti->encode_typecode(tti);
			tao_impl_tc_add_member(pres, my_tti, pt_tl,
					       0, mem_index);
		}
	} else if( !strcmp( idl_type, "union" ) ) {
		struct tao_typecode_init *tti;
		tag_list *member_tl, *pt_tl;
		char *disc_type;
		const char *xlator;
		tag_item *member_ti;
		int member_count;
		cast_expr cexpr;
		int mem_index;
		
		my_tti = new tao_typecode_init;
		my_tti->set_parent(root_tti);
		tao_impl_tc_base(my_tti, tl, pt_index);
		mem_index = find_tag(tl, "discrim_pres_index")->data.
			tag_data_u.i;
		pt_tl = get_tag_data(&pres_type->data, mem_index).tl;
		disc_type = find_tag(pt_tl, "idl_type")->data.tag_data_u.str;
		xlator = tao_discrim_translator(disc_type);
		tao_impl_tc_add_member(pres, my_tti, pt_tl, 0, mem_index);
		tti = new tao_typecode_init;
		tti->encode_number(-1);
		tti->set_comment("default used index");
		my_tti->encode_typecode(tti);
		member_ti = find_tag(tl, "member");
		member_count = tag_data_length(&member_ti->data);
		tti = new tao_typecode_init;
		tti->encode_number(member_count);
		tti->set_comment("member count");
		my_tti->encode_typecode(tti);
		for( lpc = 0; lpc < member_count; lpc++ ) {
			member_tl = get_tag_data(&member_ti->data, lpc).tl;
			mem_index = find_tag(member_tl, "pres_index")->data.
				tag_data_u.i;
			pt_tl = get_tag_data(&pres_type->data, mem_index).tl;
			tti = new tao_typecode_init;
			cexpr = find_tag(member_tl, "disc")->
				data.tag_data_u.cexpr;
			switch( cexpr->cast_expr_u_u.lit_prim.u.kind ) {
			case CAST_PRIM_CHAR:
				tti->encode_char(cexpr->cast_expr_u_u.
						 lit_prim.u.
						 cast_lit_prim_u_u.c);
				break;
			case CAST_PRIM_INT:
				tti->encode_xlate(xlator,
						  cexpr->cast_expr_u_u.
						  lit_prim.u.
						  cast_lit_prim_u_u.i);
				break;
			default:
				panic( "Can't handle discrim values that"
				       "aren't scalar" );
				break;
			}
			tti->set_comment("union case label (evaluated)");
			my_tti->encode_typecode(tti);
			tti = new tao_typecode_init;
			tti->encode_string(find_tag(member_tl, "name")->
					   data.tag_data_u.str);
			tti->set_comment("name");
			my_tti->encode_typecode(tti);
			tao_impl_tc_add_member(pres, my_tti, pt_tl, 0,
					       mem_index);
		}
	} else if( !strcmp( idl_type, "enum" ) ) {
		struct tao_typecode_init *tti;
		tag_item *member_ti;
		int member_count;
		int lpc;
		
		my_tti = new tao_typecode_init;
		my_tti->set_parent(root_tti);
		tao_impl_tc_base(my_tti, tl, pt_index);
		member_ti = find_tag(tl, "member");
		member_count = tag_data_length(&member_ti->data);
		tti = new tao_typecode_init;
		tti->encode_number( member_count );
		tti->set_comment( "member count" );
		my_tti->encode_typecode(tti);
		for( lpc = 0; lpc < member_count; lpc++ ) {
			tti = new tao_typecode_init;
			tti->encode_string( get_tag_data(&member_ti->data,
							 lpc).str );
			tti->set_comment( "name" );
			my_tti->encode_typecode(tti);
		}
	} else if( !strcmp( idl_type, "string" ) ) {
		struct tao_typecode_init *tti;
		tag_item *ti;
		
		tti = new tao_typecode_init;
		if( (ti = find_tag(ref_tl, "max_len")) ) {
			tti->encode_number(ti->data.tag_data_u.i);
		}
		else
			tti->encode_number(0);
		root_tti->encode_typecode(tti);
	} else if( !strcmp( idl_type, "sequence" ) ) {
		struct tao_typecode_init *tti;
		tag_list *pt_tl;
		tag_item *ti;
		int mem_index;
		
		my_tti = new tao_typecode_init;
		my_tti->set_parent(root_tti);
		my_tti->set_pres_index(pt_index);
		tti = new tao_typecode_init;
		tti->encode_name("TAO_ENCAP_BYTE_ORDER");
		tti->set_comment("byte order");
		my_tti->encode_typecode(tti);
		mem_index = find_tag(tl, "slice_pres_index")->data.
			tag_data_u.i;
		pt_tl = get_tag_data(&pres_type->data, mem_index).tl;
		tao_impl_tc_add_member(pres, my_tti, pt_tl, 0, mem_index);
		tti = new tao_typecode_init;
		if( (ti = find_tag(tl, "max_len")) ) {
			tti->encode_number(ti->data.tag_data_u.i);
		}
		else
			tti->encode_number(0);
		my_tti->encode_typecode(tti);
	} else if( !strcmp( idl_type, "array" ) ) {
		struct tao_typecode_init *tti;
		unsigned int lpc;
		int mem_index;
		tag_list *pt_tl;
		tag_item *ti;
		
		my_tti = new tao_typecode_init;
		my_tti->set_parent(root_tti);
		tao_impl_tc_base(my_tti, tl, pt_index);
		mem_index = find_tag(tl, "slice_pres_index")->data.
			tag_data_u.i;
		pt_tl = get_tag_data(&pres_type->data, mem_index).tl;
		tao_impl_tc_add_member(pres, my_tti, pt_tl, 0, mem_index);
		ti = find_tag(tl, "len");
		for( lpc = 0; lpc < tag_data_length(&ti->data); lpc++ ) {
			tti = new tao_typecode_init;
			tti->encode_number(get_tag_data(&ti->data, lpc).i);
			my_tti->encode_typecode(tti);
		}
	} else if( !strcmp( idl_type, "typedef" ) ) {
		tag_list *pt_tl;
		int mem_index;
		
		my_tti = new tao_typecode_init;
		my_tti->set_parent(root_tti);
		tao_impl_tc_base(my_tti, tl, pt_index);
		mem_index = find_tag(tl, "typedef_ref")->data.tag_data_u.i;
		pt_tl = get_tag_data(&pres_type->data, mem_index).tl;
		tao_impl_tc_add_member(pres, my_tti, pt_tl, tl, mem_index);
	}
	if( root_tti && my_tti ) {
		root_tti->encode_typecode(my_tti);
	}
	else if( my_tti ) {
		root_tti = my_tti;
		root_tti->set_kind( TTIK_ROOT );
	}
	return( root_tti );
}

/* This will create and write out the type code for a type */
void tao_impl_write_tc(pres_c_1 *pres, tag_list *tl)
{
	char *oc_name, *tc_tao_name, *tc_name;
	struct tao_typecode_init *root_tti;
	data_channel_index decl_channel = PASSTHRU_DATA_CHANNEL;
	cast_scoped_name scn, scn_cp;
	char *name;
	const char *idl_type;
	cast_scope *scope;
	unsigned int lpc;
	cast_type ctype;
	tag_item *ti;
	int name_len;
	int pt_index;
	int cdef;
	
	ti = find_tag(pres->pres_attrs, "pres_type");
	for( pt_index = 0; get_tag_data(&ti->data, pt_index).tl != tl;
	     pt_index++ );
	if( !(ti = find_tag(tl, "name")) )
		return;
	ctype = find_tag(tl, "definition")->data.tag_data_u.ctype;
	scn = ctype->cast_type_u_u.name;
	scope = &pres->cast;
	cdef = cast_find_def(&scope, scn, CAST_TYPE|CAST_TYPEDEF);
	if( (pres->meta_data.channels.
	     channels_val[scope->cast_scope_val[cdef].channel].flags &
	     DATA_CHANNEL_SQUELCHED) )
		return;
	root_tti = tao_impl_tc(pres, 0, tl, 0, pt_index);
	name = ti->data.tag_data_u.str;
	name_len = strlen( name );
	oc_name = (char *)mustmalloc(sizeof("_oc") + 1 + name_len + 1);
	strcpy(oc_name, "_oc");
	scn = find_tag(tl, "default_scope")->data.tag_data_u.scname;
	for( lpc = 0; lpc < scn.cast_scoped_name_len; lpc++ ) {
		oc_name = (char *)mustrealloc(oc_name,
					      strlen(oc_name) +
					      1 +
					      strlen(scn.
						     cast_scoped_name_val[lpc].
						     name) +
					      1 +
					      name_len +
					      1);
		strcat(oc_name, "_");
		strcat(oc_name, scn.cast_scoped_name_val[lpc].name);
	}
	strcat(oc_name, "_");
	strcat(oc_name, name);
	w_printf("\nstatic const CORBA::Long %s[] =\n", oc_name);
	w_printf("{\n");
	root_tti->print(1);
	w_printf("\n};\n\n");
	tc_tao_name = (char *)mustmalloc(sizeof("_tc_TAO_tc") +
					 1 + name_len + 1);
	strcpy(tc_tao_name, "_tc_TAO_tc");
	for( lpc = 0; lpc < scn.cast_scoped_name_len; lpc++ ) {
		tc_tao_name =
			(char *)mustrealloc(tc_tao_name,
					    strlen(tc_tao_name) +
					    1 +
					    strlen(scn.
						   cast_scoped_name_val[lpc].
						   name) +
					    1 +
					    name_len +
					    1);
		strcat(tc_tao_name, "_");
		strcat(tc_tao_name, scn.cast_scoped_name_val[lpc].name);
	}
	strcat(tc_tao_name, "_");
	strcat(tc_tao_name, name);
	tc_name = flick_asprintf("_tc_%s", name);
	add_tag(tl, "tc_data", TAG_STRING, oc_name);
	add_tag(tl, "tc_def", TAG_STRING, tc_tao_name);
	scn_cp = cast_copy_scoped_name(&scn);
	cast_add_scope_name(&scn_cp,
			    tc_name,
			    null_template_arg_array);
	add_tag(tl, "tc_ref", TAG_CAST_SCOPED_NAME, scn_cp);
	if( find_tag(tl, "typedef_ref") )
		idl_type = "typedef";
	else
		idl_type = find_tag(tl, "idl_type")->data.tag_data_u.str;
	add_tag(tl, "tk_type", TAG_STRING, corba_idl_type_to_tk(idl_type));
	w_printf("static CORBA::TypeCode %s (%s, sizeof(%s), "
		 "(char *) &%s, 0, sizeof(",
		 tc_tao_name, find_tag(tl, "tk_type")->data.tag_data_u.str,
		 oc_name, oc_name);
	cast_w_type(null_scope_name,
		    find_tag(tl, "definition")->data.tag_data_u.ctype,
		    0);
	w_printf("));\n\n");
	scope = &pres->cast;
	cdef = cast_find_def(&scope,
			     scn,
			     CAST_NAMESPACE|CAST_TYPE|CAST_TYPEDEF);
	while( (cdef != -1) &&
		((scope->cast_scope_val[cdef].u.kind &
		  (CAST_TYPE|CAST_TYPEDEF))
		 && (scope->cast_scope_val[cdef].u.cast_def_u_u.type->kind !=
		     CAST_TYPE_AGGREGATE)) ) {
		cdef = cast_find_def_pos(&scope,
					 cdef + 1,
					 scn,
					 CAST_NAMESPACE|
					 CAST_TYPEDEF|
					 CAST_TYPE);
	}
	if( cdef >= 0 ) {
		decl_channel = scope->cast_scope_val[cdef].channel;
		if( (scope->cast_scope_val[cdef].u.kind == CAST_NAMESPACE) ) {
			w_printf("TAO_NAMESPACE_TYPE(CORBA::TypeCode_ptr)\n");
			for( lpc = 0; lpc < scn.cast_scoped_name_len; lpc++ ) {
				w_printf("TAO_NAMESPACE_BEGIN (%s)\n",
					 scn.cast_scoped_name_val[lpc].name);
			}
			w_printf("TAO_NAMESPACE_DEFINE (CORBA::TypeCode_ptr, "
				 "%s, &%s)\n",
				 tc_name, tc_tao_name);
			for( lpc = 0; lpc < scn.cast_scoped_name_len; lpc++ ) {
				w_printf("TAO_NAMESPACE_END\n");
			}
		}
		else {
			w_printf("CORBA::TypeCode_ptr ");
			cast_w_scoped_name(&scn_cp);
			w_printf(" = &%s;\n\n", tc_tao_name);
		}
	} else {
		w_printf("CORBA::TypeCode_ptr ");
		cast_w_scoped_name(&scn_cp);
		w_printf(" = &%s;\n\n", tc_tao_name);
	}
	ctype = cast_new_type_scoped_name(cast_new_scoped_name("CORBA",
							       "TypeCode_ptr",
							       NULL));
	if( cdef >= 0 ) {
		if( scope->cast_scope_val[cdef].u.kind == CAST_NAMESPACE ) {
			scope = scope->cast_scope_val[cdef].u.cast_def_u_u.
				new_namespace;
			cdef = cast_add_def(scope,
					    null_scope_name,
					    CAST_SC_NONE,
					    CAST_DIRECT_CODE,
					    decl_channel,
					    CAST_PROT_NONE);
			scope->cast_scope_val[cdef].u.cast_def_u_u.direct.
				code_string
				= ir_strlit("TAO_NAMESPACE_STORAGE_CLASS");
			cdef = cast_add_def(scope,
					    cast_new_scoped_name(tc_name,
								 NULL),
					    CAST_SC_NONE,
					    CAST_VAR_DECL,
					    decl_channel,
					    CAST_PROT_NONE);
			scope->cast_scope_val[cdef].u.cast_def_u_u.var_type =
				ctype;
		}
		else {
			scope = &scope->cast_scope_val[cdef].u.cast_def_u_u.
				type->cast_type_u_u.agg_type.scope;
			cdef = cast_add_def(scope,
					    cast_new_scoped_name(tc_name,
								 NULL),
					    CAST_SC_STATIC,
					    CAST_VAR_DECL,
					    decl_channel,
					    CAST_PROT_PUBLIC);
			scope->cast_scope_val[cdef].u.cast_def_u_u.var_type =
				ctype;
		}
	}
	else {
		scope = &pres->cast;
		cdef = cast_add_def(scope,
				    cast_new_scoped_name(tc_name, NULL),
				    CAST_SC_EXTERN,
				    CAST_VAR_DECL,
				    decl_channel,
				    CAST_PROT_NONE);
		scope->cast_scope_val[cdef].u.cast_def_u_u.var_type = ctype;
	}
}

void tao_impl_cxx_def(tag_list *tl, const char *type, const char *macro_name)
{
	struct scml_scope *cmd_scope, *ss;
	struct scml_context sc;
	
	ss = the_state->get_scml_root();
	if( (ss = ss->find_child(type)) ) {
		sc.set_scope(ss);
		sc.set_stream_pos(the_state->get_scml_defs_stream());
		if( ss->find_cmd_definition(&cmd_scope, macro_name) ) {
			sc.exec_cmd(macro_name, tl, 0);
			w_printf("\n");
		} else {
			panic(("Couldn't find macro \"%s\" for C++"
			       " definition"), macro_name);
		}
	}
}

/* This will create the _tao_collocated class and
   add a constructor to the main interface class. */
void tao_impl_interface_preprocess(pres_c_1 *pres, tag_list *pt_tl)
{
	cast_scoped_name sc_col_name, poa_scname, scn, sc_full_col_name;
	cast_scoped_name stub_factory_pointer_name;
	cast_type col_type, type, type2, type3, ftype;
	tag_list *main_tl, *tl, *ftl, *op_tl;
	cast_func_type cfunc;
	tag_item *op_ti, *parm_ti, *parm_type_ti, *mf_ti;
	union tag_data_u data;
	cast_scope *def_scope = 0;
	cast_scope *scope;
	data_channel_index decl_channel, impl_channel;
	tag_data td;
	unsigned int lpc;
	unsigned int lpc2;
	char *name;
	char *col_name;
	char *opname;
	char *stub_factory_name;
	int param_idx;
	int cdef, cdef2;
	
	name = find_tag(pt_tl, "name")->data.tag_data_u.str;
	
	col_name = flick_asprintf("_tao_thru_poa_collocated_%s", name);
	sc_col_name = cast_new_scoped_name(col_name, NULL);
	col_type = cast_new_class_type(0);
	col_type->cast_type_u_u.agg_type.name = sc_col_name;
	main_tl = find_tag(pt_tl, "main")->data.tag_data_u.tl;
	cast_class_add_parent(col_type,
			      CAST_PARENT_PUBLIC|CAST_PARENT_VIRTUAL,
			      find_tag(main_tl, "name")->
			      data.tag_data_u.scname);
	
	scn = find_tag(pt_tl, "default_scope")->data.tag_data_u.scname;
	stub_factory_pointer_name = cast_copy_scoped_name(&scn);
	scn = cast_copy_scoped_name(&scn);
	cast_add_scope_name(&scn,
			    find_tag(pt_tl, "name")->data.tag_data_u.str,
			    null_template_arg_array);
	scope = &pres->cast;
	cdef = cast_find_def(&scope, scn, CAST_TYPE|CAST_TYPEDEF);
	type = scope->cast_scope_val[cdef].u.cast_def_u_u.type;
	decl_channel = scope->cast_scope_val[cdef].channel;
	impl_channel = meta_find_channel(&pres->meta_data,
					 pres->meta_data.channels.
					 channels_val[decl_channel].input,
					 pres->meta_data.channels.
					 channels_val[decl_channel].id,
					 DATA_CHANNEL_IMPL);
	stub_factory_name = flick_asprintf("_TAO_collocation_%s_Stub"
					   "_Factory_function_pointer",
					   name);
	ftype = cast_new_function_type(find_tag(pt_tl, "pointer")->
				       data.tag_data_u.ctype, 1);
	ftype->cast_type_u_u.func_type.params.params_val[0].spec = 0;
	type = cast_new_type_scoped_name(
		cast_new_scoped_name("CORBA", "Object_ptr", NULL));
	ftype->cast_type_u_u.func_type.params.params_val[0].type = type;
	ftype->cast_type_u_u.func_type.params.params_val[0].name =
		ir_strlit("obj");
	ftype->cast_type_u_u.func_type.params.params_val[0].default_value = 0;
	cast_add_scope_name(&stub_factory_pointer_name,
			    stub_factory_name,
			    null_template_arg_array);
	add_tag(pt_tl, "stub_factory_pointer", TAG_CAST_SCOPED_NAME,
		stub_factory_pointer_name);
	if( !(pres->meta_data.channels.channels_val[decl_channel].flags &
	      DATA_CHANNEL_SQUELCHED) ) {
		cdef2 = cast_add_def(
			scope,
			cast_new_scoped_name(stub_factory_name, NULL),
			CAST_SC_EXTERN,
			CAST_VAR_DECL,
			decl_channel,
			CAST_PROT_NONE);
		scope->cast_scope_val[cdef2].u.cast_def_u_u.var_type =
			cast_new_pointer_type(ftype);
		tao_impl_cxx_def(pt_tl, "interface",
				 "Object Field Template Instantiator");
		tao_impl_cxx_def(pt_tl, "interface",
				 "Stub Factory function pointer");
	}
	while( cdef != -1 ) {
		type = scope->cast_scope_val[cdef].u.cast_def_u_u.type;
		switch( type->kind ) {
		case CAST_TYPE_TEMPLATE:
			type = type->cast_type_u_u.template_type.def;
			break;
		default:
			break;
		}
		if( type->kind == CAST_TYPE_AGGREGATE ) {
			scope = &type->cast_type_u_u.agg_type.scope;
			cdef = -1;
		} else {
			cdef = cast_find_def_pos(&scope, cdef + 1, scn,
						 CAST_TYPEDEF|CAST_TYPE);
			if( cdef == -1 )
				scope = 0;
		}
	}
	if( scope ) {
		cast_init cinit;
		
		cinit = cast_new_init_expr(cast_new_expr_lit_int(0,0));
		cast_class_add_parent(type,
				      CAST_PARENT_VIRTUAL|CAST_PARENT_PUBLIC,
				      cast_new_scoped_name("ACE_CORBA_1 "
							   "(Object)", NULL));
		type = cast_new_type_name("TAO_Stub");
		type = cast_new_pointer_type(type);
		type2 = cast_new_type_name("TAO_ServantBase");
		type2 = cast_new_pointer_type(type2);
		type3 = cast_new_type_scoped_name(
			cast_new_scoped_name("CORBA", "Boolean", NULL));
		pres_function(pres, main_tl,
			      scn,
			      cast_new_scoped_name(find_tag(pt_tl, "name")->
						   data.tag_data_u.str, NULL),
			      PFA_Scope, scope,
			      PFA_DeclChannel, decl_channel,
			      PFA_ImplChannel, impl_channel,
			      PFA_FunctionKind,
			        "T(TAO_Stub *, TAO_ServantBase *, bool)",
			      PFA_ReturnType, cast_new_type(CAST_TYPE_NULL),
			      PFA_Parameter, type, "objref", NULL,
			      PFA_Parameter, type2, "_tao_servant", cinit,
			      PFA_Parameter, type3, "_tao_collocated", cinit,
			      PFA_Constructor,
			      PFA_Protection, CAST_PROT_PROTECTED,
			      PFA_TAG_DONE);
	}
	poa_scname = find_tag(pt_tl, "poa_scope")->data.tag_data_u.scname;
	sc_full_col_name = cast_copy_scoped_name(&poa_scname);
	cast_add_scope_name(&sc_full_col_name,
			    col_name,
			    null_template_arg_array);
	scope = &pres->cast;
	if( cast_scoped_name_is_empty( &poa_scname ) )
		def_scope = scope;
	else {
		cdef = cast_find_def(&scope,
				     poa_scname,
				     CAST_TYPE|CAST_NAMESPACE|CAST_TYPEDEF);
		switch(scope->cast_scope_val[cdef].u.kind) {
		case CAST_TYPEDEF:
		case CAST_TYPE:
			def_scope = &scope->cast_scope_val[cdef].
				u.cast_def_u_u.type->
				cast_type_u_u.agg_type.scope;
			break;
		case CAST_NAMESPACE:
			def_scope = scope->cast_scope_val[cdef].u.
				cast_def_u_u.new_namespace;
			break;
		default:
			break;
		}
	}
	tl = find_tag(pt_tl, "poa")->data.tag_data_u.tl;
	scn = find_tag(tl, "name")->data.tag_data_u.scname;
	scope = &pres->cast;
	cdef = cast_find_def(&scope,
			     scn,
			     CAST_TYPE|CAST_TYPEDEF);
	decl_channel = scope->cast_scope_val[cdef].channel;
	impl_channel = meta_find_channel(&pres->meta_data,
					 pres->meta_data.channels.
					 channels_val[decl_channel].input,
					 pres->meta_data.channels.
					 channels_val[decl_channel].id,
					 DATA_CHANNEL_IMPL);
	
	type = cast_new_type_scoped_name(
		cast_new_scoped_name("CORBA", "Object_ptr", NULL));
	pres_function(pres, tl,
		      find_tag(pt_tl, "poa_scope")->data.tag_data_u.scname,
		      cast_new_scoped_name(flick_asprintf("_TAO_collocation_POA_%s_Stub_Factory", name), NULL),
		      PFA_Scope, scope,
		      PFA_DeclChannel, decl_channel,
		      PFA_ImplChannel, impl_channel,
		      PFA_FunctionKind,
		      "T_ptr stub_factory(CORBA::Object_ptr obj)",
		      PFA_ReturnType, find_tag(pt_tl, "pointer")->
		      data.tag_data_u.ctype,
		      PFA_Parameter, type, "obj", NULL,
		      PFA_GlobalFunction,
		      PFA_TAG_DONE);
	pres_function(pres, tl,
		      find_tag(pt_tl, "poa_scope")->data.tag_data_u.scname,
		      cast_new_scoped_name(flick_asprintf("_TAO_collocation_POA_%s_Stub_Factory_Initializer", name), NULL),
		      PFA_Scope, scope,
		      PFA_DeclChannel, decl_channel,
		      PFA_ImplChannel, impl_channel,
		      PFA_FunctionKind,
		      "int stub_factory_initializer(long dummy)",
		      PFA_ReturnType, cast_new_prim_type(CAST_PRIM_INT, 0),
		      PFA_Parameter, cast_new_prim_type(CAST_PRIM_INT, 0),
		      "dummy", NULL,
		      PFA_GlobalFunction,
		      PFA_TAG_DONE);
	if( !(pres->meta_data.channels.channels_val[decl_channel].flags &
	      DATA_CHANNEL_SQUELCHED) )
		tao_impl_cxx_def(pt_tl, "interface",
				 "Stub Factory initializer");
	tl = create_tag_list(0);
	add_tag(pt_tl, "poa_collocated", TAG_TAG_LIST, tl);
	add_tag(tl, "name", TAG_CAST_SCOPED_NAME, sc_full_col_name);
	cdef = cast_add_def(def_scope,
			    sc_col_name,
			    CAST_SC_NONE,
			    CAST_TYPE,
			    decl_channel,
			    CAST_PROT_NONE);
	def_scope->cast_scope_val[cdef].u.cast_def_u_u.type = col_type;
	scope = &col_type->cast_type_u_u.agg_type.scope;
	
	type = cast_new_pointer_type(cast_new_type_name("TAO_Stub"));
	pres_function(pres, tl,
		      sc_full_col_name,
		      sc_col_name,
		      PFA_Scope, scope,
		      PFA_DeclChannel, decl_channel,
		      PFA_ImplChannel, impl_channel,
		      PFA_Protection, CAST_PROT_PUBLIC,
		      PFA_FunctionKind, "T(POA_ptr, obj)",
		      PFA_ReturnType, cast_new_type(CAST_TYPE_NULL),
		      PFA_Parameter, type, "stub", NULL,
		      PFA_Constructor,
		      PFA_TAG_DONE);
	
	if( !(op_ti = find_tag(pt_tl, "operation")) ) {
		relink_tag_list(pt_tl);
		return;
	}
	if( !(mf_ti = find_tag(tl, "model_func")) )
		mf_ti = add_tag(tl, "model_func", TAG_TAG_LIST_ARRAY, 0);
	for( lpc = 0; lpc < tag_data_length(&op_ti->data); lpc++ ) {
		op_tl = get_tag_data(&op_ti->data, lpc).tl;
		ftl = create_tag_list(0);
		opname = find_tag(op_tl, "name")->data.tag_data_u.str;
		cast_init_function_type(&cfunc, 0);
		cdef = cast_add_def(scope,
				    cast_new_scoped_name(opname, NULL),
				    CAST_SC_NONE,
				    CAST_FUNC_DECL,
				    decl_channel,
				    CAST_PROT_PUBLIC);
		cfunc.return_type = find_tag(op_tl, "return_type")->
			data.tag_data_u.ctype;
		cfunc.spec = CAST_FUNC_VIRTUAL;
		add_tag(ftl, "return", TAG_INTEGER,
			(cfunc.return_type->kind != CAST_TYPE_VOID));
		add_tag(ftl, "return_type", TAG_CAST_TYPE,
			cfunc.return_type);
		parm_ti = find_tag(op_tl, "parameter");
		parm_type_ti = find_tag(op_tl, "parameter_type");
		td = create_tag_data(TAG_STRING_ARRAY, 0);
		for( lpc2 = 0;
		     lpc2 < tag_data_length(&parm_ti->data);
		     lpc2++ ) {
			param_idx = cast_func_add_param(&cfunc);
			cfunc.params.params_val[param_idx].name =
				ir_strlit(get_tag_data(&parm_ti->data, lpc2).
					  str);
			cfunc.params.params_val[param_idx].type =
				get_tag_data(&parm_type_ti->data, lpc2).ctype;
			data.str = cfunc.params.params_val[param_idx].name;
			append_tag_data(&td, data);
		}
		add_tag(ftl, "parameter", TAG_STRING_ARRAY, 0)->data = td;
		scope->cast_scope_val[cdef].u.cast_def_u_u.func_type = cfunc;
		scn = cast_copy_scoped_name(&sc_full_col_name);
		cast_add_scope_name(&scn,
				    opname,
				    null_template_arg_array);
		cdef = cast_add_def(&pres->stubs_cast,
				    scn,
				    CAST_SC_NONE,
				    CAST_FUNC_DECL,
				    decl_channel,
				    CAST_PROT_PUBLIC);
		cfunc.spec &= ~CAST_FUNC_VIRTUAL;
		pres->stubs_cast.cast_scope_val[cdef].u.
			cast_def_u_u.func_type = cfunc;
		data.tl = ftl;
		append_tag_data(&mf_ti->data, data);
		add_tag(ftl, "name", TAG_STRING, opname);
		add_tag(ftl, "kind", TAG_STRING, "col_func");
		add_tag(ftl, "c_func", TAG_INTEGER, cdef);
	}
	ftl = create_tag_list(0);
	opname = ir_strlit("_non_existent");
	cast_init_function_type(&cfunc, 0);
	cdef = cast_add_def(scope,
			    cast_new_scoped_name(opname, NULL),
			    CAST_SC_NONE,
			    CAST_FUNC_DECL,
			    decl_channel,
			    CAST_PROT_PUBLIC);
	cfunc.return_type = cast_new_type_scoped_name(
		cast_new_scoped_name("CORBA", "Boolean", NULL));
	cfunc.spec = CAST_FUNC_VIRTUAL;
	add_tag(ftl, "return", TAG_INTEGER,
		(cfunc.return_type->kind != CAST_TYPE_VOID));
	add_tag(ftl, "return_type", TAG_CAST_TYPE, cfunc.return_type);
	param_idx = cast_func_add_param(&cfunc);
	cfunc.params.params_val[param_idx].name =
		ir_strlit("_ev");
	cfunc.params.params_val[param_idx].type =
		cast_new_reference_type(
			cast_new_type_scoped_name(
				cast_new_scoped_name("CORBA",
						     "Environment",
						     NULL)));
	cfunc.params.params_val[param_idx].spec = 0;
	cfunc.params.params_val[param_idx].default_value = 0;
	td = create_tag_data(TAG_STRING_ARRAY, 0);
	data.str = cfunc.params.params_val[param_idx].name;
	append_tag_data(&td, data);
	add_tag(ftl, "parameter", TAG_STRING_ARRAY, 0)->data = td;
	scope->cast_scope_val[cdef].u.cast_def_u_u.func_type = cfunc;
	scn = cast_copy_scoped_name(&sc_full_col_name);
	cast_add_scope_name(&scn,
			    opname,
			    null_template_arg_array);
	cdef = cast_add_def(&pres->stubs_cast,
			    scn,
			    CAST_SC_NONE,
			    CAST_FUNC_DECL,
			    decl_channel,
			    CAST_PROT_PUBLIC);
	cfunc.spec &= ~CAST_FUNC_VIRTUAL;
	pres->stubs_cast.cast_scope_val[cdef].u.
		cast_def_u_u.func_type = cfunc;
	data.tl = ftl;
	append_tag_data(&mf_ti->data, data);
	add_tag(ftl, "name", TAG_STRING, opname);
	add_tag(ftl, "kind", TAG_STRING, "col_func");
	add_tag(ftl, "c_func", TAG_INTEGER, cdef);
	relink_tag_list(pt_tl);
}

int tao_string_slot_mapping_handler(mu_state *must, cast_expr cexpr,
				    cast_type /* ctype */, mint_ref itype,
				    pres_c_mapping_xlate *xmap)
{
	const char *the_selector = "";
	
	switch( must->current_param_dir ) {
	case PRES_C_DIRECTION_UNKNOWN:
		the_selector = "inout";
		break;
	case PRES_C_DIRECTION_IN:
		if( must->op & MUST_ENCODE )
			the_selector = "in";
		if( must->op & MUST_DECODE )
			the_selector = "inout";
		break;
	case PRES_C_DIRECTION_INOUT:
		the_selector = "inout";
		break;
	case PRES_C_DIRECTION_OUT:
		the_selector = "inout";
		break;
	case PRES_C_DIRECTION_RETURN:
		if( must->op & MUST_ENCODE )
			the_selector = "in";
		if( must->op & MUST_DECODE )
			the_selector = "inout";
		break;
	default:
		panic("Unknown direction %d while handling translation",
		      must->current_param_dir);
		break;
	}
	cexpr = cast_new_expr_call_0(
		cast_new_expr_sel(cexpr,
				  cast_new_scoped_name(the_selector, NULL)));
	must->mu_mapping(cexpr,
			 xmap->internal_ctype,
			 itype,
			 xmap->internal_mapping);
	return( 1 );
}

/* This is the primary impl generator */
struct presentation_collection *make_tao_impl(struct be_state *state)
{
	struct scml_scope *root_scope;
	struct scml_stream_pos *ssp;
	presentation_collection *pc;
	presentation_impl *pi;
	
	root_scope = state->get_scml_root();
	ssp = state->get_scml_defs_stream();
	
	cast_namespace_str = "TAO_NAMESPACE";
	
	pc = new presentation_collection;
	pc->set_scml_stream_pos(ssp);
	pc->set_scml_scope(root_scope);
	
	/* Primitives */
	pi = new presentation_impl;
	pi->set_idl_type("Short");
	pi->set_misc_impl(tao_impl_write_tc);
	pc->add_impl(pi);
	
	pi = new presentation_impl;
	pi->set_idl_type("Long");
	pi->set_misc_impl(tao_impl_write_tc);
	pc->add_impl(pi);
	
	pi = new presentation_impl;
	pi->set_idl_type("LongLong");
	pi->set_misc_impl(tao_impl_write_tc);
	pc->add_impl(pi);
	
	pi = new presentation_impl;
	pi->set_idl_type("UShort");
	pi->set_misc_impl(tao_impl_write_tc);
	pc->add_impl(pi);
	
	pi = new presentation_impl;
	pi->set_idl_type("ULong");
	pi->set_misc_impl(tao_impl_write_tc);
	pc->add_impl(pi);
	
	pi = new presentation_impl;
	pi->set_idl_type("ULongLong");
	pi->set_misc_impl(tao_impl_write_tc);
	pc->add_impl(pi);
	
	pi = new presentation_impl;
	pi->set_idl_type("Float");
	pi->set_misc_impl(tao_impl_write_tc);
	pc->add_impl(pi);
	
	pi = new presentation_impl;
	pi->set_idl_type("Double");
	pi->set_misc_impl(tao_impl_write_tc);
	pc->add_impl(pi);
	
	pi = new presentation_impl;
	pi->set_idl_type("LongDouble");
	pi->set_misc_impl(tao_impl_write_tc);
	pc->add_impl(pi);
	
	pi = new presentation_impl;
	pi->set_idl_type("Fixed");
	pi->set_misc_impl(tao_impl_write_tc);
	pc->add_impl(pi);
	
	pi = new presentation_impl;
	pi->set_idl_type("Boolean");
	pi->set_misc_impl(tao_impl_write_tc);
	pc->add_impl(pi);
	
	pi = new presentation_impl;
	pi->set_idl_type("Char");
	pi->set_misc_impl(tao_impl_write_tc);
	pc->add_impl(pi);
	
	pi = new presentation_impl;
	pi->set_idl_type("WChar");
	pi->set_misc_impl(tao_impl_write_tc);
	pc->add_impl(pi);
	
	pi = new presentation_impl;
	pi->set_idl_type("Octet");
	pi->set_misc_impl(tao_impl_write_tc);
	pc->add_impl(pi);
	
	pi = new presentation_impl;
	pi->set_idl_type("string");
	pi->set_misc_impl(tao_impl_write_tc);
	pc->add_impl(pi);
	
	/* Complex */
	pi = new presentation_impl;
	pi->set_idl_type("struct");
	pi->set_misc_impl(tao_impl_write_tc);
	pc->add_impl(pi);
	
	pi = new presentation_impl;
	pi->set_idl_type("struct");
	pi->set_misc_impl(tao_impl_managed_struct_preprocess);
	pc->add_impl(pi);
	
	pi = new presentation_impl;
	pi->set_idl_type("union");
	pi->set_misc_impl(tao_impl_write_tc);
	pc->add_impl(pi);
	
	pi = new presentation_impl;
	pi->set_idl_type("union");
	pi->set_misc_impl(tao_impl_managed_union_preprocess);
	pc->add_impl(pi);
	
	pi = new presentation_impl;
	pi->set_idl_type("enum");
	pi->set_misc_impl(tao_impl_write_tc);
	pc->add_impl(pi);
	
	pi = new presentation_impl;
	pi->set_idl_type("sequence");
	pi->set_misc_impl(tao_impl_write_tc);
	pc->add_impl(pi);
	
	pi = new presentation_impl;
	pi->set_idl_type("array");
	pi->set_misc_impl(tao_impl_write_tc);
	pc->add_impl(pi);
	
	pi = new presentation_impl;
	pi->set_idl_type("array");
	pi->set_misc_impl(tao_impl_managed_array_preprocess);
	pc->add_impl(pi);
	
	pi = new presentation_impl;
	pi->set_idl_type("exception");
	pi->set_misc_impl(tao_impl_write_tc);
	pc->add_impl(pi);
	
	pi = new presentation_impl;
	pi->set_idl_type("exception");
	pi->set_misc_impl(tao_impl_managed_struct_preprocess);
	pc->add_impl(pi);
	
	pi = new presentation_impl;
	pi->set_idl_type("interface");
	pi->set_misc_impl(tao_impl_write_tc);
	pc->add_impl(pi);
	
	pi = new presentation_impl;
	pi->set_idl_type("interface");
	pi->set_misc_impl(tao_impl_interface_preprocess);
	pc->add_impl(pi);
	
	pi = new presentation_impl;
	pi->set_idl_type("exception");
	pi->set_pres_type("main");
	pc->add_impl(pi);
	
	pi = new presentation_impl;
	pi->set_idl_type("sequence");
	pi->set_pres_type("main");
	pi->set_misc_impl(tao_impl_sequence_preprocess);
	pc->add_impl(pi);
	
	pi = new presentation_impl;
	pi->set_idl_type("interface");
	pi->set_pres_type("main");
	pc->add_impl(pi);
	
	pi = new presentation_impl;
	pi->set_idl_type("interface");
	pi->set_pres_type("poa");
	pc->add_impl(pi);
	
	pi = new presentation_impl;
	pi->set_idl_type("interface");
	pi->set_pres_type("poa_tie");
	pi->set_scope_impl(tao_impl_tie_data);
	pc->add_impl(pi);
	
	pi = new presentation_impl;
	pi->set_idl_type("interface");
	pi->set_pres_type("poa_collocated");
	pc->add_impl(pi);
	
	pi = new presentation_impl;
	pi->set_idl_type("union");
	pi->set_pres_type("main");
	pc->add_impl(pi);
	
	pi = new presentation_impl;
	pi->set_idl_type("array");
	pi->set_pres_type("main");
	pc->add_impl(pi);
	
	pi = new presentation_impl;
	pi->set_idl_type("struct");
	pi->set_pres_type("main");
	pc->add_impl(pi);
	
	pi = new presentation_impl;
	pi->set_pres_type("var");
	pi->set_scope_impl(tao_impl_var_data);
	pc->add_impl(pi);
	
	pi = new presentation_impl;
	pi->set_idl_type("array");
	pi->set_pres_type("forany");
	pi->set_scope_impl(tao_impl_forany_data);
	pc->add_impl(pi);
	
	pi = new presentation_impl;
	pi->set_pres_type("out");
	pi->set_scope_impl(tao_impl_out_data);
	pc->add_impl(pi);
	
	struct translation_handler_entry *the = new translation_handler_entry;
	
	the->entry.name = "string_slot_xlator";
	the->handler = tao_string_slot_mapping_handler;
	add_entry(mu_state::translation_handlers, &the->entry);
	
	return( pc );
}

struct be_pres_impl be_tao_impl = BE_PRES_IMPL_INIT("tao_cxx", make_tao_impl);

/* End of file. */

