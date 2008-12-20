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
#include <mom/c/libpres_c.h>

tag_list *create_tag_list(int count)
{
	tag_list *retval;

	retval = mustmalloc(sizeof(tag_list));
	retval->parent = 0;
	retval->items.items_val = 0;
	retval->items.items_len = 0;
	if( count ) {
		retval->items.items_val = mustmalloc(sizeof(tag_item) * count);
		retval->items.items_len = count;
		memset( retval->items.items_val, 0, sizeof(tag_item) * count);
	}
	return( retval );
}

void delete_tag_list(tag_list *tl)
{
	free(tl->items.items_val);
	free(tl);
}

tag_list *copy_tag_list(tag_list *tl)
{
	tag_list *retval;
	unsigned int lpc;
	
	retval = mustmalloc(sizeof(tag_list));
	retval->parent = tl->parent;
	retval->items.items_len = tl->items.items_len;
	retval->items.items_val = mustmalloc(sizeof(tag_item) *
					     retval->items.items_len);
	for( lpc = 0; lpc < tl->items.items_len; lpc++ ) {
		retval->items.items_val[lpc] =
			copy_tag_item(&tl->items.items_val[lpc]);
	}
	return( retval );
}

void concat_tag_list(tag_list *tl1, tag_list *tl2)
{
	unsigned int lpc;
	
	for( lpc = 0; lpc < tl2->items.items_len; lpc++ ) {
		*add_tag(tl1, tl2->items.items_val[lpc].tag, TAG_NONE) =
			tl2->items.items_val[lpc];
	}
	delete_tag_list(tl2);
}

void relink_tag_list(tag_list *tl)
{
	unsigned int lpc;
	tag_item *ti;
	
	for( lpc = 0; lpc < tl->items.items_len; lpc++ ) {
		ti = &tl->items.items_val[lpc];
		switch( ti->data.kind ) {
		case TAG_TAG_LIST:
			ti->data.tag_data_u.tl->parent = tl;
			relink_tag_list(ti->data.tag_data_u.tl);
			break;
		case TAG_TAG_LIST_ARRAY: {
			unsigned int i;
			
			for( i = 0; i < ti->data.tag_data_u.tl_a.tl_a_len;
			     i++ ) {
				ti->data.tag_data_u.tl_a.tl_a_val[i]->
					parent = tl;
				relink_tag_list(ti->data.tag_data_u.tl_a.
						tl_a_val[i]);
			}
			break;
		}
		default:
			break;
		}
	}
}

tag_item *create_tag_item(const char *tag, tag_data *data)
{
	tag_item *retval;
	
	retval = (tag_item *)mustmalloc(sizeof(tag_item));
	retval->tag = ir_strlit(tag);
	retval->data = *data;
	return( retval );
}

void delete_tag_item(tag_item *ti)
{
	free(ti);
}

tag_item copy_tag_item(tag_item *ti)
{
	tag_item retval;
	
	retval.tag = ti->tag;
	retval.data = copy_tag_data(&ti->data);
	return( retval );
}

tag_item *add_tag(tag_list *tl, const char *tag, tag_data_kind kind, ...)
{
	int lpc, done = 0;
	tag_list *parent;
	va_list arg_addr;
	tag_item *retval;
	
	va_start( arg_addr, kind );
	parent = tl->parent;
	tl->parent = 0;
	assert( !find_tag(tl, tag) );
	tl->parent = parent;
	tl->items.items_len++;
	tl->items.items_val = mustrealloc(tl->items.items_val,
					  sizeof(tag_item) *
					  tl->items.items_len);
	for( lpc = tl->items.items_len - 1; (lpc > 0) && !done; lpc-- ) {
		if (flick_strcasecmp(tag,
				     tl->items.items_val[lpc - 1].tag)
		    >= 0)
			done = 1;
		else
			tl->items.items_val[lpc] =
				tl->items.items_val[lpc - 1];
	}
	if( done )
		lpc++;
	retval = &tl->items.items_val[lpc];
	retval->tag = ir_strlit(tag);
	if( kind & TAG_ARRAY ) {
		retval->data = create_tag_data(kind, va_arg(arg_addr, int));
	} else {
		retval->data.kind = kind;
		switch( kind ) {
		case TAG_NONE:
		case TAG_ANY:
			break;
		case TAG_TAG_LIST:
			retval->data.tag_data_u.tl = va_arg(arg_addr,
							    tag_list *);
			break;
		case TAG_BOOL:
			retval->data.tag_data_u.b = va_arg(arg_addr, int);
			break;
		case TAG_STRING:
			retval->data.tag_data_u.str = va_arg(arg_addr, char *);
			break;
		case TAG_INTEGER:
			retval->data.tag_data_u.i = va_arg(arg_addr, int);
			break;
		case TAG_FLOAT:
			retval->data.tag_data_u.f = va_arg(arg_addr, float);
			break;
		case TAG_REF:
			retval->data.tag_data_u.ref = va_arg(arg_addr, char *);
			break;
		case TAG_OBJECT:
			retval->data.tag_data_u.obj = va_arg(arg_addr,
							     tag_object_array);
		case TAG_CAST_SCOPED_NAME:
			retval->data.tag_data_u.scname =
				va_arg(arg_addr, cast_scoped_name);
			break;
		case TAG_CAST_DEF:
			retval->data.tag_data_u.cdef = va_arg(arg_addr,
							      cast_def_t);
			break;
		case TAG_CAST_TYPE:
			retval->data.tag_data_u.ctype = va_arg(arg_addr,
							       cast_type);
			break;
		case TAG_CAST_EXPR:
			retval->data.tag_data_u.cexpr = va_arg(arg_addr,
							       cast_expr);
			break;
		case TAG_CAST_STMT:
			retval->data.tag_data_u.cstmt = va_arg(arg_addr,
							       cast_stmt);
			break;
		case TAG_CAST_INIT:
			retval->data.tag_data_u.cinit = va_arg(arg_addr,
							       cast_init);
			break;
		default:
			panic("add_tag() doesn't recognize kind %d", kind);
			break;
		}
	}
	va_end( arg_addr );
	return( retval );
}

void rem_tag(tag_list *tl, const char *tag)
{
	unsigned int lpc;
	int kill = 0;
	
	for( lpc = 0; lpc < tl->items.items_len; lpc++ ) {
		if( kill ) {
			tl->items.items_val[lpc - 1] =
				tl->items.items_val[lpc];
		} else if( !strcmp(tag, tl->items.items_val[lpc].tag) ) {
			kill = 1;
		}
	}
	if( kill ) {
		tl->items.items_len--;
	}
}

tag_item *find_tag(tag_list *tl, const char *tag)
{
	tag_item *retval = 0;
	int start, mid, end;
	int cmp_val;
	
	while( tl && tag && !retval ) {
		start = 0;
		end = tl->items.items_len - 1;
		while( (start <= end) && !retval ) {
			mid = start + ((end - start) / 2);
			cmp_val = flick_strcasecmp(tag,
						   (tl->items.items_val[mid].
						    tag));
			if( cmp_val > 0 ) {
				start = mid + 1;
			}
			else if( cmp_val < 0 ) {
				end = mid - 1;
			}
			else
				retval = &tl->items.items_val[mid];
		}
		tl = tl->parent;
	}
	return( retval );
}

tag_data create_tag_data(tag_data_kind kind, int length)
{
	tag_data retval;
	
	memset( &retval, 0, sizeof( tag_data ) );
	retval.kind = kind;
	if( kind & TAG_ARRAY ) {
		if( length )
			retval.tag_data_u.str_a.str_a_val =
				(char **)mustmalloc(tag_data_kind_size(kind) *
						    length);
		else
			retval.tag_data_u.str_a.str_a_val = 0;
		retval.tag_data_u.str_a.str_a_len = length;
		if( kind == TAG_STRING_ARRAY ) {
			int lpc;
			
			for( lpc = 0; lpc < length; lpc++ ) {
				retval.tag_data_u.str_a.str_a_val[lpc] = "";
			}
		}
	} else if( kind == TAG_STRING ) {
		retval.tag_data_u.str =	"";
	}
	return( retval );
}

tag_data copy_tag_data(tag_data *td)
{
	tag_data retval;
	unsigned int len;
	unsigned int lpc;
	
	len = tag_data_length(td);
	retval = create_tag_data(td->kind, len);
	for( lpc = 0; lpc < len; lpc++ ) {
		if( td->kind == TAG_TAG_LIST_ARRAY ) {
			union tag_data_u data;
			
			data.tl = copy_tag_list(get_tag_data(td, lpc).tl);
			set_tag_data(&retval, lpc, data);
		}
		else
			set_tag_data(&retval, lpc,
				     get_tag_data(td, lpc));
	}
	return( retval );
}

union tag_data_u get_tag_data(tag_data *td, unsigned int index)
{
	union tag_data_u retval;
	
	if( (td->kind & TAG_ARRAY) ) {
		/* assert(index >= 0); `index' is unsigned! */
		assert(index < td->tag_data_u.tl_a.tl_a_len);
	}
	switch( td->kind ) {
	case TAG_NONE:
	case TAG_ANY:
		break;
	case TAG_TAG_LIST:
		retval.tl = td->tag_data_u.tl;
		break;
	case TAG_BOOL:
		retval.b = td->tag_data_u.b;
		break;
	case TAG_STRING:
		retval.str = td->tag_data_u.str;
		break;
	case TAG_INTEGER:
		retval.i = td->tag_data_u.i;
		break;
	case TAG_FLOAT:
		retval.f = td->tag_data_u.f;
		break;
	case TAG_REF:
		retval.ref = td->tag_data_u.ref;
		break;
	case TAG_OBJECT:
		retval.obj = td->tag_data_u.obj;
		break;
	case TAG_CAST_SCOPED_NAME:
		retval.scname = td->tag_data_u.scname;
		break;
	case TAG_CAST_DEF:
		retval.cdef = td->tag_data_u.cdef;
		break;
	case TAG_CAST_TYPE:
		retval.ctype = td->tag_data_u.ctype;
		break;
	case TAG_CAST_EXPR:
		retval.cexpr = td->tag_data_u.cexpr;
		break;
	case TAG_CAST_STMT:
		retval.cstmt = td->tag_data_u.cstmt;
		break;
	case TAG_CAST_INIT:
		retval.cinit = td->tag_data_u.cinit;
		break;
		
	case TAG_ANY_ARRAY:
		break;
	case TAG_TAG_LIST_ARRAY:
		retval.tl = td->tag_data_u.tl_a.tl_a_val[index];
		break;
	case TAG_BOOL_ARRAY:
		retval.b = td->tag_data_u.b_a.b_a_val[index];
		break;
	case TAG_STRING_ARRAY:
		retval.str = td->tag_data_u.str_a.str_a_val[index];
		break;
	case TAG_INTEGER_ARRAY:
		retval.i = td->tag_data_u.i_a.i_a_val[index];
		break;
	case TAG_FLOAT_ARRAY:
		retval.f = td->tag_data_u.f_a.f_a_val[index];
		break;
	case TAG_REF_ARRAY:
		retval.ref = td->tag_data_u.ref_a.ref_a_val[index];
		break;
	case TAG_OBJECT_ARRAY:
		retval.obj = td->tag_data_u.obj_a.obj_a_val[index];
		break;
	case TAG_CAST_SCOPED_NAME_ARRAY:
		retval.scname = td->tag_data_u.scname_a.scname_a_val[index];
		break;
	case TAG_CAST_DEF_ARRAY:
		retval.cdef = td->tag_data_u.cdef_a.cdef_a_val[index];
		break;
	case TAG_CAST_TYPE_ARRAY:
		retval.ctype = td->tag_data_u.ctype_a.ctype_a_val[index];
		break;
	case TAG_CAST_EXPR_ARRAY:
		retval.cexpr = td->tag_data_u.cexpr_a.cexpr_a_val[index];
		break;
	case TAG_CAST_STMT_ARRAY:
		retval.cstmt = td->tag_data_u.cstmt_a.cstmt_a_val[index];
		break;
	case TAG_CAST_INIT_ARRAY:
		retval.cinit = td->tag_data_u.cinit_a.cinit_a_val[index];
		break;
	}
	return( retval );
}

void set_tag_data(tag_data *td, int index, union tag_data_u data)
{
	switch( td->kind ) {
	case TAG_NONE:
	case TAG_ANY:
		break;
	case TAG_TAG_LIST:
		td->tag_data_u.tl = data.tl;
		break;
	case TAG_BOOL:
		td->tag_data_u.b = data.b;
		break;
	case TAG_STRING:
		td->tag_data_u.str = ir_strlit(data.str);
		break;
	case TAG_INTEGER:
		td->tag_data_u.i = data.i;
		break;
	case TAG_FLOAT:
		td->tag_data_u.f = data.f;
		break;
	case TAG_REF:
		td->tag_data_u.ref = ir_strlit(data.ref);
		break;
	case TAG_OBJECT:
		td->tag_data_u.obj = data.obj;
		break;
	case TAG_CAST_SCOPED_NAME:
		td->tag_data_u.scname = data.scname;
		break;
	case TAG_CAST_DEF:
		td->tag_data_u.cdef = data.cdef;
		break;
	case TAG_CAST_TYPE:
		td->tag_data_u.ctype = data.ctype;
		break;
	case TAG_CAST_EXPR:
		td->tag_data_u.cexpr = data.cexpr;
		break;
	case TAG_CAST_STMT:
		td->tag_data_u.cstmt = data.cstmt;
		break;
	case TAG_CAST_INIT:
		td->tag_data_u.cinit = data.cinit;
		break;
		
	case TAG_ANY_ARRAY:
		break;
	case TAG_TAG_LIST_ARRAY:
		td->tag_data_u.tl_a.tl_a_val[index] = data.tl;
		break;
	case TAG_BOOL_ARRAY:
		td->tag_data_u.b_a.b_a_val[index] = data.b;
		break;
	case TAG_STRING_ARRAY:
		td->tag_data_u.str_a.str_a_val[index] = ir_strlit(data.str);
		break;
	case TAG_INTEGER_ARRAY:
		td->tag_data_u.i_a.i_a_val[index] = data.i;
		break;
	case TAG_FLOAT_ARRAY:
		td->tag_data_u.f_a.f_a_val[index] = data.f;
		break;
	case TAG_REF_ARRAY:
		td->tag_data_u.ref_a.ref_a_val[index] = ir_strlit(data.ref);
		break;
	case TAG_OBJECT_ARRAY:
		td->tag_data_u.obj_a.obj_a_val[index] = data.obj;
		break;
	case TAG_CAST_SCOPED_NAME_ARRAY:
		td->tag_data_u.scname_a.scname_a_val[index] = data.scname;
		break;
	case TAG_CAST_DEF_ARRAY:
		td->tag_data_u.cdef_a.cdef_a_val[index] = data.cdef;
		break;
	case TAG_CAST_TYPE_ARRAY:
		td->tag_data_u.ctype_a.ctype_a_val[index] = data.ctype;
		break;
	case TAG_CAST_EXPR_ARRAY:
		td->tag_data_u.cexpr_a.cexpr_a_val[index] = data.cexpr;
		break;
	case TAG_CAST_STMT_ARRAY:
		td->tag_data_u.cstmt_a.cstmt_a_val[index] = data.cstmt;
		break;
	case TAG_CAST_INIT_ARRAY:
		td->tag_data_u.cinit_a.cinit_a_val[index] = data.cinit;
		break;
	}
}

int tag_data_kind_size(tag_data_kind kind)
{
	int retval;
	
	switch( kind & ~TAG_ARRAY ) {
	case TAG_NONE:
	case TAG_ANY:
		retval = 0;
		break;
	case TAG_TAG_LIST:
		retval = sizeof( tag_list_ptr );
		break;
	case TAG_BOOL:
		retval = sizeof( char );
		break;
	case TAG_STRING:
		retval = sizeof( char * );
		break;
	case TAG_INTEGER:
		retval = sizeof( int );
		break;
	case TAG_FLOAT:
		retval = sizeof( float );
		break;
	case TAG_REF:
		retval = sizeof( char * );
		break;
	case TAG_OBJECT:
		retval = sizeof( tag_object_array );
		break;
	case TAG_CAST_SCOPED_NAME:
		retval = sizeof( cast_scoped_name );
		break;
	case TAG_CAST_DEF:
		retval = sizeof( cast_def_t );
		break;
	case TAG_CAST_TYPE:
		retval = sizeof( cast_type );
		break;
	case TAG_CAST_EXPR:
		retval = sizeof( cast_expr );
		break;
	case TAG_CAST_STMT:
		retval = sizeof( cast_stmt );
		break;
	case TAG_CAST_INIT:
		retval = sizeof( cast_init );
		break;
	default:
		retval = 0;
		break;
	}
	return( retval );
}

int append_tag_data(tag_data *td, union tag_data_u data)
{
	int retval, elem_size;
	
	retval = td->tag_data_u.str_a.str_a_len++;
	elem_size = tag_data_kind_size(td->kind);
	td->tag_data_u.str_a.str_a_val =
		(char **)mustrealloc(td->tag_data_u.str_a.str_a_val,
				     (retval + 1) * elem_size );
	set_tag_data(td, retval, data);
	return( retval );
}

unsigned int tag_data_length(tag_data *td)
{
	unsigned int retval;
	
	if( td->kind & TAG_ARRAY )
		retval = td->tag_data_u.str_a.str_a_len;
	else
		retval = 1;
	return( retval );
}

tag_data_kind get_base_tag_data_kind(tag_data_kind kind)
{
	return( kind & ~TAG_ARRAY );
}

char *ptr_to_tag_ref(const char *type, void *ptr)
{
	return( flick_asprintf("ptr:(%s)=%p", type, ptr) );
}

void *tag_ref_to_ptr(const char *type, const char *ref)
{
	void *retval = 0;
	const char *str;
	int type_len;
	
	if( !strncmp( "ptr:(", ref, 5 ) ) {
		str = ref + 5;
		type_len = strlen(type);
		if( !strncmp(type, str, type_len) &&
		    ((*(str + type_len)) == ')') &&
		    ((*(str + type_len + 1)) == '=') ) {
			str += type_len + 2;
			sscanf(str, "%p", &retval);
		}
	}
	return( retval );
}

int cmp_tag_ref_class(char *ref_class, char *ref)
{
	int retval = 0;
	int len;
	
	len = strlen(ref_class);
	if( !strncmp(ref_class, ref, len) &&
	    (ref[len] == ':') ) {
		retval = 1;
	}
	return( retval );
}

static void tag_print_indent(int size)
{
	int lpc;
	
	for( lpc = 0; lpc < size; lpc++ ) {
		w_printf(" ");
	}
}

void print_tag_data(int indent, tag_data_kind kind, union tag_data_u data)
{
	switch( kind ) {
	case TAG_NONE:
	case TAG_ANY:
		break;
	case TAG_TAG_LIST: {
		unsigned int lpc;
		tag_list *tl;
		
		tl = data.tl;
		w_printf("{\n");
		for( lpc = 0; lpc < tl->items.items_len; lpc++ ) {
			print_tag(indent+1,&tl->items.items_val[lpc]);
			w_printf("\n");
		}
		tag_print_indent(indent);
		w_printf("}");
		break;
	}
	case TAG_BOOL:
		tag_print_indent(indent);
		w_printf("%s", data.b ? "true" : "false");
		break;
	case TAG_STRING:
		tag_print_indent(indent);
		w_printf("%s", data.str ? data.str : "(null)");
		break;
	case TAG_INTEGER:
		tag_print_indent(indent);
		w_printf("%d", data.i);
		break;
	case TAG_FLOAT:
		tag_print_indent(indent);
		w_printf("%f", data.f);
		break;
	case TAG_REF:
		tag_print_indent(indent);
		w_printf("%s", data.ref ? data.ref : "(null)");
		break;
	case TAG_OBJECT: {
		unsigned int lpc;
		
		w_printf("[");
		for( lpc = 0; lpc < data.obj.tag_object_array_len; lpc++ ) {
			if( !(lpc % 16) ) {
				w_printf("\n");
				w_indent(indent);
			}
			if( !(lpc % 8) )
				w_printf("  ");
			w_printf("%x", data.obj.tag_object_array_val[lpc]);
		}
		if( lpc % 16 )
			w_printf("\n");
		w_i_printf(indent, "]");
		break;
	}
	case TAG_CAST_SCOPED_NAME:
		tag_print_indent(indent);
		cast_w_scoped_name(&(data.scname));
		break;
	case TAG_CAST_DEF:
		indent = (indent + 1) & ~1;
		cast_w_def(data.cdef, indent / 2);
		break;
	case TAG_CAST_TYPE:
		tag_print_indent(indent);
		indent = (indent + 1) & ~1;
		cast_w_type(null_scope_name, data.ctype, indent);
		break;
	case TAG_CAST_EXPR:
		indent = (indent + 1) & ~1;
		cast_w_expr(data.cexpr, indent / 2);
		break;
	case TAG_CAST_STMT:
		indent = (indent + 1) & ~1;
		cast_w_stmt(data.cstmt, indent / 2);
		break;
	case TAG_CAST_INIT:
		indent = (indent + 1) & ~1;
		cast_w_init(data.cinit, indent / 2);
		break;
	default:
		break;
	}
}

struct tag_kind_map {
	tag_data_kind kind;
	char *name;
};

struct tag_kind_map tag_kind_names[] = {
	{TAG_NONE, "TAG_NONE"},
	{TAG_ANY, "TAG_ANY"},
	{TAG_TAG_LIST, "TAG_TAG_LIST"},
	{TAG_BOOL, "TAG_BOOL"},
	{TAG_STRING, "TAG_STRING"},
	{TAG_INTEGER, "TAG_INTEGER"},
	{TAG_FLOAT, "TAG_FLOAT"},
	{TAG_REF, "TAG_REF"},
	{TAG_CAST_SCOPED_NAME, "TAG_CAST_SCOPED_NAME"},
	{TAG_CAST_DEF, "TAG_CAST_DEF"},
	{TAG_CAST_TYPE, "TAG_CAST_TYPE"},
	{TAG_CAST_EXPR, "TAG_CAST_EXPR"},
	{TAG_CAST_STMT, "TAG_CAST_STMT"},
	{TAG_CAST_INIT, "TAG_CAST_INIT"},
	{TAG_TAG_LIST_ARRAY, "TAG_TAG_LIST_ARRAY"},
	{TAG_BOOL_ARRAY, "TAG_BOOL_ARRAY"},
	{TAG_STRING_ARRAY, "TAG_STRING_ARRAY"},
	{TAG_INTEGER_ARRAY, "TAG_INTEGER_ARRAY"},
	{TAG_FLOAT_ARRAY, "TAG_FLOAT_ARRAY"},
	{TAG_REF_ARRAY, "TAG_REF_ARRAY"},
	{TAG_CAST_SCOPED_NAME_ARRAY, "TAG_CAST_SCOPED_NAME_ARRAY"},
	{TAG_CAST_DEF_ARRAY, "TAG_CAST_DEF_ARRAY"},
	{TAG_CAST_TYPE_ARRAY, "TAG_CAST_TYPE_ARRAY"},
	{TAG_CAST_EXPR_ARRAY, "TAG_CAST_EXPR_ARRAY"},
	{TAG_CAST_STMT_ARRAY, "TAG_CAST_STMT_ARRAY"},
	{TAG_CAST_INIT_ARRAY, "TAG_CAST_INIT_ARRAY"},
	{0, 0}
};

char *map_tag_data_kind(tag_data_kind kind)
{
	char *retval = 0;
	int lpc;
	
	for( lpc = 0; tag_kind_names[lpc].name && !retval; lpc++ ) {
		if( kind == tag_kind_names[lpc].kind )
			retval = tag_kind_names[lpc].name;
	}
	if( !retval )
		retval = "<UNKNOWN>";
	return( retval );
}

void print_tag(int indent, tag_item *ti)
{
	w_i_printf(indent, "%s:%s=", ti->tag,
		   map_tag_data_kind(ti->data.kind));
	if( ti->data.kind & TAG_ARRAY ) {
		unsigned int len;
		unsigned int lpc;
		int base_kind;
		
		len = tag_data_length(&ti->data);
		base_kind = get_base_tag_data_kind(ti->data.kind);
		w_printf("[\n");
		for( lpc = 0; lpc < len; lpc++ ) {
			w_i_printf(indent+1, "#%d=", lpc);
			print_tag_data(indent+1, base_kind,
				       get_tag_data(&ti->data, lpc));
			w_printf("\n");
		}
		w_i_printf(indent, "]");
	} else {
		print_tag_data(indent, ti->data.kind,
			       get_tag_data(&ti->data, 0));
	}
}

/* End of file. */

