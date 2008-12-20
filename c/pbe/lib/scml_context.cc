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
#include <ctype.h>
#include <string.h>
#include <assert.h>

#include <mom/c/libcast.h>
#include <mom/c/scml.hh>

#define min(x,y) ((x < y) ? x : y)
#define max(x,y) ((x > y) ? x : y)

struct scml_handler_table *scml_context::sht = 0;

scml_context::scml_context()
{
	this->flags = 0;
	this->stream_pos = 0;
	this->scope = 0;
	this->lvalues = create_tag_list(0);
	this->rvalues = this->lvalues;
	this->offset_size = 0;
	this->indent_size = 0;
	this->ws_queue = 0;
}

scml_context::~scml_context()
{
}

void scml_context::set_flags(int the_flags)
{
	this->flags = the_flags;
}

int scml_context::get_flags()
{
	return( this->flags );
}

void scml_context::set_parent(struct scml_context *sc)
{
	this->parent = sc;
}

struct scml_context *scml_context::get_parent()
{
	return( this->parent );
}

void scml_context::set_stream_pos(struct scml_stream_pos *ssp)
{
	this->stream_pos = ssp;
}

struct scml_stream_pos *scml_context::get_stream_pos()
{
	return( this->stream_pos );
}

void scml_context::set_cmd_def(struct scml_cmd_definition *the_def)
{
	this->def = the_def;
}

struct scml_cmd_definition *scml_context::get_cmd_def()
{
	return( this->def );
}

void scml_context::set_lvalues(tag_list *tl)
{
	this->lvalues = tl;
}

tag_list *scml_context::get_lvalues()
{
	return( this->lvalues );
}

void scml_context::set_rvalues(tag_list *tl)
{
	this->rvalues = tl;
}

tag_list *scml_context::get_rvalues()
{
	return( this->rvalues );
}

void scml_context::set_scope(struct scml_scope *the_scope)
{
	this->scope = the_scope;
}

struct scml_scope *scml_context::get_scope()
{
	return( this->scope );
}

void scml_context::set_offset_size(int size)
{
	this->offset_size = size;
}

int scml_context::get_offset_size()
{
	return( this->offset_size );
}

void scml_context::set_indent_size(int size)
{
	this->indent_size = size;
}

int scml_context::get_indent_size()
{
	return( this->indent_size );
}

void scml_context::set_ws_queue(int size)
{
	this->ws_queue = size;
}

int scml_context::get_ws_queue()
{
	return( this->ws_queue );
}

void scml_context::locate_scope(struct scml_token *st,
				struct scml_scope **inout_scope,
				const char **out_id)
{
	switch( st->kind ) {
	case SCML_TERM_ID:
		*out_id = st->value.id;
		break;
	case SCML_NT_SCOPE_RES:
		if( (st->value.children[1].kind == SCML_TERM_ID) ) {
			struct scml_scope *child_scope;
			
			*out_id = 0;
			this->locate_scope(&st->value.children[0],
					   inout_scope,
					   out_id);
			if( *out_id && *inout_scope &&
			    (child_scope = (*inout_scope)->
			     find_child(*out_id)) ) {
				*out_id = st->value.children[1].value.id;
				*inout_scope = child_scope;
			}
		} else {
			scml_alert(this->stream_pos, SAF_ERROR|SAF_RUNTIME,
				   "Non-ID(%s) used in scope resolution",
				   scml_kind_map[st->value.children[1].kind]);
			*inout_scope = 0;
			*out_id = 0;
		}
		break;
	case SCML_NT_NAME:
		this->locate_scope(&st->value.children[0],
				   inout_scope,
				   out_id);
		break;
	default:
		scml_alert(this->stream_pos, SAF_ERROR|SAF_RUNTIME,
			   "Non-ID(%s) used in scope resolution",
			   scml_kind_map[st->value.children[1].kind]);
		*inout_scope = 0;
		*out_id = 0;
		break;
	}
}

void scml_context::print_indent(int size)
{
	int lpc;
	
	for( lpc = 0; lpc < size; lpc++ ) {
		w_printf(" ");
	}
}

int scml_context::print_token(struct scml_token *st)
{
	int retval = 1;
	
	switch( st->kind ) {
	case SCML_COL_POS:
	case SCML_ROW_POS:
		break;
	case SCML_TERM_BOOL:
		this->print_indent(this->ws_queue);
		this->ws_queue = 0;
		w_printf("%s", st->value.b ? "true" : "false");
		break;
	case SCML_TERM_INT:
		this->print_indent(this->ws_queue);
		this->ws_queue = 0;
		w_printf("%d", st->value.i);
		break;
	case SCML_TERM_FLOAT:
		this->print_indent(this->ws_queue);
		this->ws_queue = 0;
		w_printf("%f", st->value.f);
		break;
	case SCML_TERM_STRING:
		st->value.str->print(this->ws_queue);
		this->ws_queue = 0;
		break;
	case SCML_TERM_TAG:
		/*
		 * If its a tag ref we need to figure out what its
		 * referring to, in order to print it
		 */
		if( (st->value.ti->data.kind == TAG_REF) ) {
			struct scml_token_sequence *ref_sts;
			struct scml_string *ref_ss;
			
			if( (ref_sts = scml_token_sequence::
			     ptr(st->value.ti->data.
				 tag_data_u.ref)) ) {
				int old_offset, old_indent;
				
				/*
				 * Its scml code so we need to adjust ourself
				 * to the formatting of this sequence and
				 * print it out.
				 */
				old_offset = this->offset_size;
				this->offset_size = ref_sts->get_indent();
				old_indent = this->indent_size;
				if( this->flags & SCF_PREFORMATTED )
					this->indent_size = this->stream_pos->
						get_column() - old_offset;
				retval = this->format_sequence(ref_sts);
				this->indent_size = old_indent;
				this->offset_size = old_offset;
			} else if( (ref_ss = scml_string::
				    ptr(st->value.ti->data.tag_data_u.ref)) ) {
				/* An encapsulated string */
				ref_ss->print(this->ws_queue);
				this->ws_queue = 0;
			} else {
				scml_alert(this->stream_pos,
					   SAF_WARNING|SAF_INTERNAL,
					   "Can't handle ref tag (%s)",
					   st->value.ti->data.tag_data_u.ref);
			}
		} else {
			union tag_data_u data;
			
			/* Just a regular tag, print it */
			data = get_tag_data(&st->value.ti->data, 0);
			print_tag_data(this->ws_queue,
				       st->value.ti->data.kind,
				       data);
			this->ws_queue = 0;
		}
		break;
	case SCML_TERM_TEXT: {
		int row, col;
		const char *curr;
		
		/* Track the row/column when printing (for errors) */
		row = this->stream_pos->get_row();
		col = this->stream_pos->get_column();
		for( curr = st->value.text; *curr; curr++ ) {
			/*
			 * Check for tabs that cross over the indentation
			 * of the sequence.  We'll need to fill it out with
			 * spaces rather than printing the tab.
			 */
			if( (this->flags & SCF_PREFORMATTED) &&
			    (*curr == '\t') &&
			    (col < this->offset_size) &&
			    ((col + 8) > this->offset_size) ) {
				this->ws_queue = (col + 8) - this->offset_size;
			} else if( isspace(*curr) &&
				   (!(this->flags & SCF_PREFORMATTED) ||
				    (*curr != '\n')) ) {
				if( !(this->flags & SCF_IGNORE_WHITE_SPACE) &&
				    ((this->flags & SCF_PREFORMATTED) ||
				     (this->flags & SCF_PRINTING_BODY)) &&
				    (col >= this->offset_size) ) {
					if( this->flags & SCF_PREFORMATTED )
						this->ws_queue +=
							(*curr == '\t') ? 8 :
						1;
					else if( !this->ws_queue )
						this->ws_queue = 1;
				}
			} else if( !(this->flags & SCF_PREFORMATTED) ||
				   (col >= this->offset_size) ||
				   (*curr == '\n') ) {
				/* Print any queued white space */
				if( this->ws_queue ) {
					if( *curr != '\n' )
						this->print_indent(this->
								   ws_queue);
					this->ws_queue = 0;
				}
				/*
				 * Record that we are in the body of the
				 * sequence.
				 */
				this->flags |= SCF_PRINTING_BODY;
				w_printf("%c", *curr);
				/*
				 * If this is a newline and we have some
				 * extra indentation we'll need to print
				 * that first
				 */
				if( (this->flags & SCF_PREFORMATTED) &&
				    (*curr == '\n') &&
				    !(this->flags & SCF_IGNORE_INDENT) ) {
					this->print_indent(this->indent_size);
				}
			}
			switch( *curr ) {
			case '\n':
				row++;
				col = 0;
				break;
			case '\t':
				col += 8;
				break;
			default:
				col++;
				break;
			}
		}
		/* There was some left over white space */
		this->stream_pos->set_row(row);
		this->stream_pos->set_column(col);
		break;
	}
	case SCML_TERM_ESCAPE: {
		struct scml_escape *se = 0;
		struct scml_scope *curr;
		
		curr = this->scope;
		while( curr ) {
			if( (se = curr->get_escape_table()->
			     find_escape(st->value.escape)) ) {
				this->print_indent(this->ws_queue);
				this->ws_queue = 0;
				w_printf("%s", se->value);
				curr = 0;
			} else
				curr = curr->get_parent();
		}
		if( !se ) {
			scml_alert(this->stream_pos,
				   SAF_ERROR|SAF_RUNTIME,
				   "Couldn't find escape '%s'",
				   st->value.escape);
			retval = 0;
		}
		break;
	}
	case SCML_TERM_VERBATIM:
		this->print_indent(this->ws_queue);
		this->ws_queue = 0;
		w_printf("%s", st->value.text);
		break;
	case SCML_ERROR:
		retval = 0;
		scml_alert(this->stream_pos, SAF_ERROR|SAF_RUNTIME,
			   "There was an error in execution",
			   scml_kind_map[st->kind]);
		break;
	default:
		retval = 0;
		scml_alert(this->stream_pos, SAF_WARNING|SAF_INTERNAL,
			   "Token (%s) not handled",
			   scml_kind_map[st->kind]);
		break;
	}
	switch( st->kind ) {
	case SCML_COL_POS:
	case SCML_ROW_POS:
		break;
	default:
		/* We're printing out the body of the text */
		this->flags |= SCF_PRINTING_BODY;
		break;
	}
	return( retval );
}

int scml_context::equalize_tokens(struct scml_token *dest,
				  struct scml_token *src,
				  int count)
{
	int retval = SCML_ERROR;
	int min_kind = SCML_NT_MAX, max_kind = 0, common_kind;
	int lpc;
	
	/* Strip everything and figure out what the range of types is */
	for( lpc = 0; lpc < count; lpc++ ) {
		dest[lpc] = this->eval_token(&src[lpc]);
		dest[lpc].strip();
		min_kind = min(min_kind, dest[lpc].kind);
		max_kind = max(max_kind, dest[lpc].kind);
	}
	if( (min_kind != max_kind) ) {
		/*
		 * We don't have all the same kind of type so we promote
		 * them all to the most accomodating type.
		 */
		common_kind = max_kind;
		min_kind = SCML_NT_MAX;
		max_kind = 0;
		for( lpc = 0; lpc < count; lpc++ ) {
			dest[lpc].promote(common_kind);
			min_kind = min(min_kind, dest[lpc].kind);
			max_kind = max(max_kind, dest[lpc].kind);
		}
		if( min_kind == max_kind )
			retval = dest[0].kind;
	} else
		retval = dest[0].kind;
	return( retval );
}

struct scml_token scml_context::eval_token(struct scml_token *st)
{
	struct scml_token args[3];
	struct scml_token retval;
	
	retval.kind = SCML_ERROR;
	switch( st->kind ) {
	case SCML_NONE:
	case SCML_IGNORE:
	case SCML_DONE:
		retval.kind = SCML_NONE;
		break;
	case SCML_ERROR:
		break;
	case SCML_COL_POS:
		this->stream_pos->set_column(st->value.i);
		retval = *st;
		break;
	case SCML_ROW_POS:
		this->stream_pos->set_row(st->value.i);
		retval = *st;
		break;
	case SCML_TERM_BOOL:
	case SCML_TERM_INT:
	case SCML_TERM_FLOAT:
	case SCML_TERM_STRING:
	case SCML_TERM_TAG:
	case SCML_TERM_VERBATIM:
	case SCML_TERM_TEXT:
	case SCML_TERM_ESCAPE:
	case SCML_TERM_TAG_LIST:
	case SCML_TERM_SLASH:
	case SCML_NT_COMMAND:
		/* These dont require any evaluation */
		retval = *st;
		break;
	case SCML_NT_SET: {
		union tag_data_u data;
		tag_item *ti = 0;
		int index;
		
		/* Get the lvalue of a token, the tag and its array index */
		this->token_lvalue(&st->value.children[0], &ti, &index);
		if( ti ) {
			/* Evaluate the right side */
			args[0] = this->eval_token(&st->value.children[1]);
			args[0].strip();
			retval.value.ti = ti;
			/*
			 * Resolve any differences in types between the
			 * right and left sides.
			 */
			switch( args[0].kind ) {
			case SCML_TERM_BOOL:
				if( (ti->data.kind == TAG_ANY) ||
				    (ti->data.kind == TAG_ANY_ARRAY) ) {
					ti->data.kind = (tag_data_kind)
						((ti->data.kind & ~TAG_ANY) |
						 TAG_BOOL);
				}
				if( (ti->data.kind == TAG_BOOL) ||
				    (ti->data.kind == TAG_BOOL_ARRAY) ) {
					retval.kind = SCML_TERM_TAG;
					data.b = args[0].value.b;
					set_tag_data(&ti->data, index, data);
				}
				break;
			case SCML_TERM_INT:
				if( (ti->data.kind == TAG_ANY) ||
				    (ti->data.kind == TAG_ANY_ARRAY) ) {
					ti->data.kind = (tag_data_kind)
						((ti->data.kind & ~TAG_ANY) |
						 TAG_INTEGER);
				}
				if( (ti->data.kind == TAG_INTEGER) ||
				    (ti->data.kind == TAG_INTEGER_ARRAY) ) {
					retval.kind = SCML_TERM_TAG;
					data.i = args[0].value.i;
					set_tag_data(&ti->data, index, data);
				}
				break;
			case SCML_TERM_FLOAT:
				if( (ti->data.kind == TAG_ANY) ||
				    (ti->data.kind == TAG_ANY_ARRAY) ) {
					ti->data.kind = (tag_data_kind)
						((ti->data.kind & ~TAG_ANY) |
						 TAG_FLOAT);
				}
				if( (ti->data.kind == TAG_FLOAT) ||
				    (ti->data.kind == TAG_FLOAT_ARRAY) ) {
					retval.kind = SCML_TERM_TAG;
					data.f = args[0].value.f;
					set_tag_data(&ti->data, index, data);
				}
				break;
			case SCML_TERM_STRING:
				if( (ti->data.kind == TAG_ANY) ||
				    (ti->data.kind == TAG_ANY_ARRAY) ) {
					ti->data.kind = (tag_data_kind)
						((ti->data.kind & ~TAG_ANY) |
						 TAG_REF);
				}
				if( (ti->data.kind == TAG_STRING) ||
				    (ti->data.kind == TAG_STRING_ARRAY) ) {
					if( (data.str = args[0].value.str->
					     make_chars()) ) {
						retval.kind = SCML_TERM_TAG;
						set_tag_data(&ti->data, index,
							     data);
					}
				}
				if( (ti->data.kind == TAG_REF) ||
				    (ti->data.kind == TAG_REF_ARRAY) ) {
					retval.kind = SCML_TERM_TAG;
					data.ref = args[0].value.str->
						tag_ref();
					set_tag_data(&ti->data, index, data);
				}
				break;
			case SCML_TERM_TAG:
				if( (ti->data.kind == TAG_ANY) ||
				    (ti->data.kind ==
				     args[0].value.ti->data.kind) ) {
					retval.kind = SCML_TERM_TAG;
					ti->data = args[0].value.ti->data;
				}
				break;
			case SCML_TERM_TAG_LIST:
				if( (ti->data.kind == TAG_ANY) ||
				    (ti->data.kind == TAG_ANY_ARRAY) ) {
					ti->data.kind = (tag_data_kind)
						((ti->data.kind & ~TAG_ANY) |
						 TAG_TAG_LIST);
				}
				if( (ti->data.kind == TAG_TAG_LIST) ||
				    (ti->data.kind == TAG_TAG_LIST_ARRAY) ) {
					retval.kind = SCML_TERM_TAG;
					data.tl = args[0].value.tl;
					set_tag_data(&ti->data, index, data);
				}
				break;
			default:
				retval.kind = SCML_ERROR;
				break;
			}
			if( retval.kind == SCML_ERROR ) {
				scml_alert(this->stream_pos,
					   SAF_ERROR|SAF_TYPE,
					   "Incompatible types in assignment");
				args[0].print();
				print_tag(0, ti);
			}
		}
		break;
	}
	case SCML_NT_PLUS:
		retval.kind = this->equalize_tokens(args, st->value.children,
						    2);
		switch( retval.kind ) {
		case SCML_TERM_INT:
			retval.value.i = args[0].value.i + args[1].value.i;
			break;
		case SCML_TERM_FLOAT:
			retval.value.f = args[0].value.f + args[1].value.f;
			break;
		case SCML_TERM_STRING:
			retval.value.str = new scml_string;
			retval.value.str->concat(args[0].value.str);
			retval.value.str->concat(args[1].value.str);
			break;
		default:
			scml_alert(this->stream_pos, SAF_ERROR|SAF_TYPE,
				   "Types for '+' are inconsistent");
			args[0].print();
			args[1].print();
			retval.kind = SCML_ERROR;
			break;
		}
		break;
	case SCML_NT_MINUS:
		retval.kind = this->equalize_tokens(args, st->value.children,
						    2);
		switch( retval.kind ) {
		case SCML_TERM_INT:
			retval.value.i = args[0].value.i - args[1].value.i;
			break;
		case SCML_TERM_FLOAT:
			retval.value.f = args[0].value.f - args[1].value.f;
			break;
		default:
			scml_alert(this->stream_pos, SAF_ERROR|SAF_TYPE,
				   "Types for '-' are inconsistent");
			retval.kind = SCML_ERROR;
			break;
		}
		break;
	case SCML_NT_DIV:
		retval.kind = this->equalize_tokens(args, st->value.children,
						    2);
		switch( retval.kind ) {
		case SCML_TERM_INT:
			retval.value.i = args[0].value.i / args[1].value.i;
			break;
		case SCML_TERM_FLOAT:
			retval.value.f = args[0].value.f / args[1].value.f;
			break;
		default:
			retval.kind = SCML_ERROR;
			break;
		}
		break;
	case SCML_NT_MOD:
		retval.kind = this->equalize_tokens(args, st->value.children,
						    2);
		switch( retval.kind ) {
		case SCML_TERM_INT:
			retval.value.i = args[0].value.i % args[1].value.i;
			break;
		default:
			retval.kind = SCML_ERROR;
			break;
		}
		break;
	case SCML_NT_MULT:
		retval.kind = this->equalize_tokens(args, st->value.children,
						    2);
		switch( retval.kind ) {
		case SCML_TERM_INT:
			retval.value.i = args[0].value.i * args[1].value.i;
			break;
		case SCML_TERM_FLOAT:
			retval.value.f = args[0].value.f * args[1].value.f;
			break;
		default:
			retval.kind = SCML_ERROR;
			break;
		}
		break;
	case SCML_NT_COND:
		args[0] = this->eval_token(st->value.children);
		args[0].strip();
		retval.kind = this->equalize_tokens(&args[1],
						    &st->value.children[1],
						    2);
		if( (retval.kind != SCML_ERROR) &&
		    ((args[0].kind == SCML_TERM_INT) ||
		     (args[0].kind == SCML_TERM_BOOL)) ) {
			if( args[0].value.i )
				retval = args[1];
			else
				retval = args[2];
		}
		break;
	case SCML_NT_EQUAL: {
		int cmp_kind;
		
		cmp_kind = this->equalize_tokens(args,
						 st->value.children,
						 2);
		retval.kind = SCML_TERM_BOOL;
		switch( cmp_kind ) {
		case SCML_TERM_STRING:
			retval.value.i = !args[0].value.str->
				cmp(args[1].value.str);
			break;
		case SCML_TERM_BOOL:
		case SCML_TERM_INT:
			retval.value.i = args[0].value.i == args[1].value.i;
			break;
		case SCML_TERM_FLOAT:
			retval.value.i = args[0].value.f == args[1].value.f;
			break;
		default:
			retval.kind = SCML_ERROR;
			break;
		}
		break;
	}
	case SCML_NT_NOT_EQUAL: {
		int cmp_kind;
		
		cmp_kind = this->equalize_tokens(args,
						 st->value.children,
						 2);
		retval.kind = SCML_TERM_BOOL;
		switch( cmp_kind ) {
		case SCML_TERM_STRING:
			retval.value.i = args[0].value.str->
				cmp(args[1].value.str);
			break;
		case SCML_TERM_BOOL:
		case SCML_TERM_INT:
			retval.value.i = args[0].value.i != args[1].value.i;
			break;
		case SCML_TERM_FLOAT:
			retval.value.i = args[0].value.f != args[1].value.f;
			break;
		default:
			retval.kind = SCML_ERROR;
			break;
		}
		break;
	}
	case SCML_NT_LT: {
		int cmp_kind;
		
		cmp_kind = this->equalize_tokens(args,
						 st->value.children,
						 2);
		retval.kind = SCML_TERM_BOOL;
		switch( cmp_kind ) {
		case SCML_TERM_STRING:
			retval.value.i = (args[0].value.str->
					  cmp(args[1].value.str) < 0);
			break;
		case SCML_TERM_INT:
			retval.value.i = args[0].value.i < args[1].value.i;
			break;
		case SCML_TERM_FLOAT:
			retval.value.i = args[0].value.f < args[1].value.f;
			break;
		default:
			retval.kind = SCML_ERROR;
			break;
		}
		break;
	}
	case SCML_NT_GT: {
		int cmp_kind;
		
		cmp_kind = this->equalize_tokens(args,
						 st->value.children,
						 2);
		retval.kind = SCML_TERM_BOOL;
		switch( cmp_kind ) {
		case SCML_TERM_STRING:
			retval.value.i = (args[0].value.str->
					  cmp(args[1].value.str) > 0);
			break;
		case SCML_TERM_INT:
			retval.value.i = args[0].value.i > args[1].value.i;
			break;
		case SCML_TERM_FLOAT:
			retval.value.i = args[0].value.f > args[1].value.f;
			break;
		default:
			retval.kind = SCML_ERROR;
			break;
		}
		break;
	}
	case SCML_NT_LE: {
		int cmp_kind;
		
		cmp_kind = this->equalize_tokens(args,
						 st->value.children,
						 2);
		retval.kind = SCML_TERM_BOOL;
		switch( cmp_kind ) {
		case SCML_TERM_STRING:
			retval.value.i = (args[0].value.str->
					  cmp(args[1].value.str) <= 0);
			break;
		case SCML_TERM_INT:
			retval.value.i = args[0].value.i <= args[1].value.i;
			break;
		case SCML_TERM_FLOAT:
			retval.value.i = args[0].value.f <= args[1].value.f;
			break;
		default:
			retval.kind = SCML_ERROR;
			break;
		}
		break;
	}
	case SCML_NT_GE: {
		int cmp_kind;
		
		cmp_kind = this->equalize_tokens(args,
						 st->value.children,
						 2);
		retval.kind = SCML_TERM_BOOL;
		switch( cmp_kind ) {
		case SCML_TERM_STRING:
			retval.value.i = (args[0].value.str->
					  cmp(args[1].value.str) >= 0);
			break;
		case SCML_TERM_INT:
			retval.value.i = args[0].value.i >= args[1].value.i;
			break;
		case SCML_TERM_FLOAT:
			retval.value.i = args[0].value.f >= args[1].value.f;
			break;
		default:
			retval.kind = SCML_ERROR;
			break;
		}
		break;
	}
	case SCML_NT_AND:
		break;
	case SCML_NT_LAND:
		retval.kind = this->equalize_tokens(args,
						    st->value.children,
						    2);
		switch( retval.kind ) {
		case SCML_TERM_INT:
		case SCML_TERM_BOOL:
			retval.kind = SCML_TERM_BOOL;
			retval.value.i = args[0].value.i && args[1].value.i;
			break;
		default:
			scml_alert(this->stream_pos, SAF_ERROR|SAF_TYPE,
				   "Types for '&&' are inconsistent");
			args[0].print();
			args[1].print();
			retval.kind = SCML_ERROR;
			break;
		}
		break;
	case SCML_NT_OR:
		retval.kind = this->equalize_tokens(args,
						    st->value.children,
						    2);
		if( retval.kind == SCML_TERM_TAG_LIST ) {
			retval.value.tl = copy_tag_list(args[0].value.tl);
			concat_tag_list(retval.value.tl, args[1].value.tl);
		} else {
			scml_alert(this->stream_pos, SAF_ERROR|SAF_GENERAL,
				   "Couldn't get tag lists for union");
		}
		break;
	case SCML_NT_LOR:
		retval.kind = this->equalize_tokens(args,
						    st->value.children,
						    2);
		switch( retval.kind ) {
		case SCML_TERM_INT:
		case SCML_TERM_BOOL:
			retval.kind = SCML_TERM_BOOL;
			retval.value.i = args[0].value.i || args[1].value.i;
			break;
		default:
			retval.kind = SCML_ERROR;
			break;
		}
		break;
		break;
	case SCML_NT_NOT:
		retval.kind = this->equalize_tokens(args,
						    st->value.children,
						    1);
		switch( retval.kind ) {
		case SCML_TERM_INT:
		case SCML_TERM_BOOL:
			retval.kind = SCML_TERM_BOOL;
			retval.value.i = !args[0].value.i;
			break;
		default:
			retval.kind = SCML_ERROR;
			break;
		}
		break;
	case SCML_NT_EXPR:
		retval = this->eval_token(&st->value.children[0]);
		break;
	case SCML_NT_DOT: {
		args[0] = this->eval_token(&st->value.children[0]);
		args[0].strip();
		switch( args[0].kind ) {
		case SCML_TERM_TAG_LIST: {
			tag_item *ti;
			
			if( (st->value.children[1].kind == SCML_TERM_ID) &&
			    (ti = find_tag(args[0].value.tl,
					   st->value.children[1].value.id)) ) {
				retval.kind = SCML_TERM_TAG;
				retval.value.ti = ti;
			} else {
				scml_alert(this->stream_pos,
					   SAF_ERROR|SAF_GENERAL,
					   "Couldn't find tag");
				args[0].print();
				st->value.children[1].print();
			}
			break;
		}
		default:
			retval.kind = SCML_ERROR;
			break;
		}
		break;
	}
	case SCML_NT_SEL: {
		/* Evaluate the tag and its array index */
		args[0] = this->eval_token(&st->value.children[0]);
		args[1] = this->eval_token(&st->value.children[1]);
		args[1].strip();
		if( (args[0].kind == SCML_TERM_TAG) &&
		    (args[1].kind == SCML_TERM_INT) &&
		    (args[1].value.i <
		     (int) tag_data_length(&args[0].value.ti->data)) ) {
			tag_item *ti;
			tag_data td;
			
			/*
			 * We create a temporary tag to hold the value.  We
			 * have to do this since scml tokens don't encode
			 * every tag type so we just make a fake one so
			 * we can handle everything.  Of course it gets
			 * leaked which kinda sucks.
			 */
			td = create_tag_data((tag_data_kind)
					     (args[0].value.ti->data.kind &
					      ~TAG_ARRAY), 1);
			set_tag_data(&td, 0,
				     get_tag_data(&args[0].value.ti->data,
						  args[1].value.i));
			ti = create_tag_item("", &td);
			retval.kind = SCML_TERM_TAG;
			retval.value.ti = ti;
		} else {
			if( args[0].kind != SCML_TERM_TAG ) {
				scml_alert(this->stream_pos,
					   SAF_ERROR|SAF_RUNTIME,
					   "Couldn't get a tag for selection");
			} else if( args[1].kind != SCML_TERM_INT ) {
				scml_alert(this->stream_pos,
					   SAF_ERROR|SAF_RUNTIME,
					   "Selector isn't an integer");
			} else {
				scml_alert(this->stream_pos,
					   SAF_ERROR|SAF_RUNTIME,
					   "Selector %d is out of range",
					   args[1].value.i);
			}
		}
		break;
	}
	case SCML_NT_INIT: {
		union tag_data_u data;
		int error = 0, lpc;
		tag_data td;
		
		td = create_tag_data(TAG_ANY, 0);
		/*
		 * Walk through all the children of this token.  Each child
		 * evaluates to a tag so we just copy it to the tag list
		 */
		for( lpc = 0;
		     !error && (st->value.children[lpc].kind != SCML_NONE);
		     lpc++ ) {
			args[0] = this->eval_token(&st->value.children[lpc]);
			switch( args[0].kind ) {
			case SCML_TERM_BOOL:
				if( td.kind == TAG_ANY ) {
					td.kind = TAG_BOOL_ARRAY;
					retval.kind = SCML_TERM_TAG;
				}
				if( td.kind == TAG_BOOL_ARRAY ) {
					data.b = args[0].value.b;
					append_tag_data(&td, data);
				} else
					error = 1;
				break;
			case SCML_TERM_INT:
				if( td.kind == TAG_ANY ) {
					td.kind = TAG_INTEGER_ARRAY;
					retval.kind = SCML_TERM_TAG;
				}
				if( td.kind == TAG_INTEGER_ARRAY ) {
					data.i = args[0].value.i;
					append_tag_data(&td, data);
				} else
					error = 1;
				break;
			case SCML_TERM_FLOAT:
				if( td.kind == TAG_ANY ) {
					td.kind = TAG_FLOAT_ARRAY;
					retval.kind = SCML_TERM_TAG;
				}
				if( td.kind == TAG_FLOAT_ARRAY ) {
					data.f = args[0].value.f;
					append_tag_data(&td, data);
				} else
					error = 1;
				break;
			case SCML_TERM_STRING:
				if( td.kind == TAG_ANY ) {
					td.kind = TAG_REF_ARRAY;
					retval.kind = SCML_TERM_TAG;
				}
				if( td.kind == TAG_REF_ARRAY ) {
					data.ref = args[0].value.str->
						tag_ref();
					append_tag_data(&td, data);
				} else
					error = 1;
				break;
			case SCML_TERM_TAG:
				if( td.kind == TAG_ANY ) {
					td.kind = TAG_TAG_LIST;
					retval.kind = SCML_TERM_TAG_LIST;
					retval.value.tl = create_tag_list(0);
				}
				if( td.kind == TAG_TAG_LIST ) {
					*(add_tag(retval.value.tl,
						  args[0].value.ti->tag,
						  TAG_NONE)) =
						*(args[0].value.ti);
				} else
					error = 1;
				break;
			case SCML_TERM_TAG_LIST:
				if( td.kind == TAG_ANY ) {
					td.kind = TAG_TAG_LIST_ARRAY;
					retval.kind = SCML_TERM_TAG;
				}
				if( td.kind == TAG_TAG_LIST_ARRAY ) {
					data.tl = args[0].value.tl;
					append_tag_data(&td, data);
				} else
					error = 1;
				break;
			default:
				error = 1;
				break;
			}
		}
		if( lpc == 0 ) {
			retval.kind = SCML_TERM_TAG_LIST;
			retval.value.tl = create_tag_list(0);
		}
		if( error ) {
			retval.kind = SCML_ERROR;
			scml_alert(this->stream_pos,
				   SAF_ERROR|SAF_GENERAL,
				   "Cannot add incompatible type",
				   scml_kind_map[args[0].kind]);
		}
		if( retval.kind == SCML_TERM_TAG ) {
			retval.value.ti = create_tag_item("", &td);
		}
		break;
	}
	case SCML_NT_TAG: {
		tag_data_kind kind;
		union tag_data_u data;
		int size, lpc;
		tag_item *ti;
		tag_data td;
		
		/* This will construct a new tag with a specific type */
		this->tag_type(&st->value.children[1], &kind, &size);
		if( (st->value.children[0].kind == SCML_TERM_ID) &&
		    (kind != -1) ) {
			td = create_tag_data(kind, size);
			ti = create_tag_item(st->value.children[0].value.id,
					     &td);
			if( kind & TAG_ARRAY ) {
				/* Be nice and clear the memory */
				memset(&data, 0, sizeof( union tag_data_u ));
				for( lpc = 0; lpc < size; lpc++ ) {
					set_tag_data(&ti->data, lpc, data);
				}
			}
			retval.kind = SCML_TERM_TAG;
			retval.value.ti = ti;
		} else {
			if( kind == -1 ) {
				scml_alert(this->stream_pos,
					   SAF_ERROR|SAF_TYPE,
					   "Invalid type given for tag");
			}
		}
		break;
	}
	case SCML_TERM_ID:
		if( (retval.value.ti = find_tag(this->rvalues,
						st->value.id)) ||
		    (retval.value.ti = find_tag(this->scope->get_values(),
						st->value.id)) ) {
			retval.kind = SCML_TERM_TAG;
		} else {
			scml_alert(this->stream_pos, SAF_ERROR|SAF_GENERAL,
				   "Couldn't find tag \"%s\"",
				   st->value.id);
		}
		break;
	case SCML_NT_NAME: {
		struct scml_scope *var_scope;
		const char *id;
		
		var_scope = this->scope;
		while( var_scope->get_parent() )
			var_scope = var_scope->get_parent();
		this->locate_scope(st, &var_scope, &id);
		if( var_scope &&
		    (retval.value.ti = find_tag(var_scope->get_values(),
						id)) ) {
			retval.kind = SCML_TERM_TAG;
		} else {
			if( !var_scope ) {
				scml_alert(this->stream_pos,
					   SAF_ERROR|SAF_GENERAL,
					   "Couldn't locate scope");
			} else if( !retval.value.ti ) {
				scml_alert(this->stream_pos,
					   SAF_ERROR|SAF_GENERAL,
					   "Couldn't locate id (%s) in scope"
					   " %s", id, var_scope->get_name());
			}
		}
		break;
	}
	default:
		break;
	}
	return( retval );
}

void scml_context::token_lvalue(struct scml_token *st, tag_item **out_ti,
				int *out_index)
{
	switch( st->kind ) {
	case SCML_TERM_TAG:
		*out_ti = st->value.ti;
		*out_index = 0;
		break;
	case SCML_TERM_ID: {
		tag_item *ti;
		
		/* If its in the list of lvalues than just return that */
		if( (ti = find_tag(this->lvalues, st->value.id)) ) {
			*out_ti = ti;
		} else if( this->def &&
			   ((ti = find_tag(this->def->get_opt_params(),
					   st->value.id)) ||
			    (ti = find_tag(this->def->get_req_params(),
					   st->value.id))) ) {
			/*
			 * If its part of the optional or required parameters
			 * then we can safely add it to the lvalues.  This
			 * is used in parameter passing so that the parameter
			 * lists are preserved and we still get access to the
			 * variables.
			 */
			*out_ti = add_tag(this->lvalues, st->value.id,
					  TAG_NONE);
			(*out_ti)->data = ti->data;
		} else if( (ti = find_tag(this->scope->get_values(),
					  st->value.id)) ) {
			/* Its in the current scope */
			*out_ti = ti;
		} else {
			*out_ti = 0;
			scml_alert(this->stream_pos, SAF_ERROR|SAF_GENERAL,
				   "Couldn't find tag lvalue %s",
				   st->value.id);
		}
		*out_index = 0;
		break;
	}
	case SCML_NT_SEL: {
		struct scml_token t_index;
		
		/* Its an array index, return the same tag with an index */
		this->token_lvalue(&st->value.children[0], out_ti, out_index);
		t_index = this->eval_token(&st->value.children[1]);
		t_index.strip();
		if( t_index.kind == SCML_TERM_INT ) {
			*out_index = t_index.value.i;
		}
		else {
			*out_ti = 0;
			*out_index = 0;
		}
		break;
	}
	case SCML_NT_DOT:
		/* Its a tag within a tag list */
		if( st->value.children[1].kind == SCML_TERM_ID ) {
			this->token_lvalue(&st->value.children[0],
					   out_ti, out_index);
			if( *out_ti &&
			    (((*out_ti)->data.kind == TAG_TAG_LIST) ||
			     ((*out_ti)->data.kind == TAG_TAG_LIST_ARRAY)) ) {
				if( ((*out_ti) = find_tag(
					get_tag_data(&(*out_ti)->data,
						     *out_index).tl,
					st->value.children[1].
					value.id)) ) {
					*out_index = 0;
				}
			} else {
				*out_ti = 0;
			}
		}
		break;
	case SCML_NT_EXPR:
		this->token_lvalue(&st->value.children[0], out_ti, out_index);
		break;
	case SCML_NT_TAG: {
		struct scml_token t_tag;
		
		t_tag = this->eval_token(st);
		if( t_tag.kind == SCML_TERM_TAG ) {
			*out_ti = t_tag.value.ti;
			*out_index = 0;
		}
		else
			*out_ti = 0;
		break;
	}
	default:
		*out_ti = 0;
		break;
	}
}

int scml_context::handle_params(struct scml_token *cmd)
{
	int retval = 1;
	tag_item *ti;
	tag_list *tl;
	unsigned int lpc;
	
	/*
	 * Handle_params has to build the lvalues tag list from the parameters
	 * but it also has to have access to the parents values.  This
	 * inbetween state is accomplished by using the parents values as
	 * our rvalues and then token_lvalue takes care of building our lvalues
	 * from the parameter lists.
	 */
	this->rvalues = this->parent->lvalues;
	if( cmd->value.children[0].kind == SCML_TERM_SLASH )
		lpc = 2;
	else
		lpc = 1;
	for( ; cmd->value.children[lpc].kind != SCML_NONE; lpc++ ) {
		if( this->eval_token(&cmd->value.children[lpc]).kind ==
		    SCML_ERROR ) {
			retval = 0;
			scml_alert(this->stream_pos, SAF_ERROR|SAF_RUNTIME,
				   "Invalid arg");
			cmd->value.children[lpc].print();
		}
	}
	/* Add the optional paramters with their default values */
	tl = this->def->get_opt_params();
	for( lpc = 0; lpc < tl->items.items_len; lpc++ ) {
		if( !(ti = find_tag(this->lvalues,
				    tl->items.items_val[lpc].tag)) ) {
			add_tag(this->lvalues, tl->items.items_val[lpc].tag,
				TAG_NONE)->data =
				copy_tag_data(&tl->items.items_val[lpc].data);
		}
	}
	/* Make sure all required parameters were specified */
	tl = this->def->get_req_params();
	for( lpc = 0; lpc < tl->items.items_len; lpc++ ) {
		if( !(ti = find_tag(this->lvalues,
				    tl->items.items_val[lpc].tag)) ) {
			retval = 0;
			scml_alert(this->stream_pos, SAF_ERROR|SAF_GENERAL,
				   "Missing required parameter %s",
				   tl->items.items_val[lpc].tag);
		}
	}
	this->rvalues = this->lvalues;
	return( retval );
}

struct scml_cmd_definition *scml_context::find_cmd(
	struct scml_scope **cmd_scope,
	struct scml_token *st)
{
	struct scml_cmd_definition *retval = 0;
	
	switch( st->kind ) {
	case SCML_TERM_ID:
		retval = this->scope->find_cmd_definition(cmd_scope,
							  st->value.id);
		break;
	case SCML_NT_NAME: {
		struct scml_scope *new_cmd_scope;
		const char *id = 0;
		
		new_cmd_scope = this->scope;
		while( new_cmd_scope->get_parent() )
			new_cmd_scope = new_cmd_scope->get_parent();
		this->locate_scope(st, &new_cmd_scope, &id);
		if( new_cmd_scope )
			retval = new_cmd_scope->find_cmd_definition(cmd_scope,
								    id);
		break;
	}
	default:
		break;
	}
	return( retval );
}

struct scml_token_sequence *scml_context::get_contents(
	struct scml_cmd_definition *cmd_def,
	struct scml_token_sequence *sts,
	int start)
{
	struct scml_token_sequence *retval = 0;
	struct scml_cmd_definition *sub_cmd;
	struct scml_scope *cmd_scope;
	int depth = 1, lpc, len;
	struct scml_token *st;
	tag_list *tl, *tl_sl;
	tag_item *ti;
	char *ident;
	
	assert(cmd_def);
	/* These tag lists are used to track begin/end commands */
	tl = create_tag_list(0);
	tl_sl = create_tag_list(0);
	add_tag(tl, sts->get_value()[start - 1].value.children[0].
		get_identifier(), TAG_INTEGER, 1);
	/* Walk through the sequence trying to find the terminating command */
	for( lpc = start; (lpc < sts->get_length()) && depth; lpc++ ) {
		switch( sts->get_value()[lpc].kind ) {
		case SCML_NT_COMMAND: {
			int slash = 0;
			
			st = &sts->get_value()[lpc];
			switch( st->value.children[0].kind ) {
			case SCML_NT_EXPR:
				sub_cmd = 0;
				break;
			case SCML_TERM_SLASH:
				/* Record any terminating commands */
				slash = 1;
				sub_cmd = this->find_cmd(&cmd_scope,
							 &st->value.
							 children[1]);
				if( (ident = st->value.children[1].
				     get_identifier()) ) {
					if( (ti = find_tag(tl_sl, ident)) ) {
						ti->data.tag_data_u.i++;
						free(ident);
					} else {
						add_tag(tl_sl, ident,
							TAG_INTEGER, 1);
					}
				}
				break;
			case SCML_TERM_ID:
			case SCML_NT_NAME:
				/* Record any commands */
				sub_cmd = this->find_cmd(&cmd_scope,
							 &st->value.
							 children[0]);
				if( (ident = st->value.children[0].
				     get_identifier()) ) {
					if( (ti = find_tag(tl, ident)) ) {
						ti->data.tag_data_u.i++;
						free(ident);
					} else {
						add_tag(tl, ident,
							TAG_INTEGER, 1);
					}
				}
				break;
			default:
				sub_cmd = 0;
				break;
			}
			/*
			 * See if this command has been terminated,
			 * or if it was nested
			 */
			if( sub_cmd &&
			    (sub_cmd->get_flags() & SCDF_BRACKETED) &&
			    (cmd_def == sub_cmd) ) {
				if( slash )
					depth--;
				else
					depth++;
			}
			break;
		}
		default:
			break;
		}
	}
	if( depth ) {
		scml_alert(this->stream_pos, SAF_ERROR|SAF_RUNTIME,
			   "This is a malformed bracketed tag %d", depth);
	}
	/* Make sure all the terminators have corresponding initiators */
	len = lpc - start - 1;
	for( lpc = 0; lpc < (signed int)tl_sl->items.items_len; lpc++ ) {
		if( !find_tag(tl, tl_sl->items.items_val[lpc].tag) ) {
			scml_alert(this->stream_pos, SAF_ERROR|SAF_RUNTIME,
				   "Terminators without initiators for "
				   "command '%s'",
				   tl_sl->items.items_val[lpc].tag);
			depth = 1;
		}
	}
	/*
	 * Make sure every initiator is paired with a terminator.
	 * Note - Because some of the initiators and terminators could be
	 * defined within this sequence we can't be sure whether a command
	 * has a terminator.  We can only guess by checking to see if
	 * its terminator was ever used.  So we aren't able to check here
	 * whether a terminator was needed but never paired with its
	 * command.  However, this isn't a problem since the lack of a
	 * terminator will appear when you try and execute this sequence.
	 */
	for( lpc = 0; lpc < (signed int)tl->items.items_len; lpc++ ) {
		ti = find_tag(tl_sl, tl->items.items_val[lpc].tag);
		if( ti &&
		    (tl->items.items_val[lpc].data.tag_data_u.i >
		     ti->data.tag_data_u.i) ) {
			scml_alert(this->stream_pos, SAF_ERROR|SAF_RUNTIME,
				   "Not enough terminators for command '%s'",
				   ti->tag);
			depth = 1;
		} else if( ti &&
			   (tl->items.items_val[lpc].data.tag_data_u.i <
			    ti->data.tag_data_u.i) ) {
			scml_alert(this->stream_pos, SAF_ERROR|SAF_RUNTIME,
				   "Too many terminators for command '%s'",
				   ti->tag);
			depth = 1;
		}
		free(tl->items.items_val[lpc].tag);
	}
	delete_tag_list(tl);
	delete_tag_list(tl_sl);
	if( depth ) {
		scml_alert(this->stream_pos, SAF_ERROR|SAF_RUNTIME,
			   "This is a malformed bracketed tag %d", depth);
	} else {
		/* Build a new sequence corresponding to this subsequence */
		retval = new scml_token_sequence();
		retval->set_indent(this->get_stream_pos()->get_column());
		retval->set_stream_pos(sts->get_stream_pos());
		retval->set_value(&(sts->get_value()[start]));
		retval->set_length(len);
	}
	return( retval );
}

struct scml_token_sequence *scml_context::partition_sequence(
	struct scml_cmd_definition *cmd_def,
	struct scml_cmd_definition *partition,
	struct scml_token_sequence *sts)
{
	struct scml_token_sequence *retval = 0;
	int lpc, poss_pivot = -1, pivot = -1;
	struct scml_cmd_definition *sub_cmd;
	struct scml_scope *cmd_scope;
	struct ptr_stack *cmd_stack;
	struct scml_token *st;
	
	cmd_stack = create_ptr_stack();
	push_ptr(cmd_stack, cmd_def);
	/*
	 * get_contents has already done the error checking for us so we
	 * just need to find the subcommand and partition around it
	 */
	for( lpc = 0;
	     (lpc < sts->get_length()) && !empty_ptr_stack(cmd_stack) &&
		     (pivot == -1);
	     lpc++ ) {
		switch( sts->get_value()[lpc].kind ) {
		case SCML_NT_COMMAND: {
			int slash = 0;
			
			st = &sts->get_value()[lpc];
			switch( st->value.children[0].kind ) {
			case SCML_NT_EXPR:
				sub_cmd = 0;
				break;
			case SCML_TERM_SLASH:
				slash = 1;
				sub_cmd = this->find_cmd(&cmd_scope,
							 &st->value.
							 children[1]);
				if( sub_cmd != cmd_def )
					poss_pivot = -1;
				if( (top_ptr(cmd_stack) == sub_cmd) )
					pop_ptr(cmd_stack);
				else {
					while( top_ptr(cmd_stack) == 0 )
						pop_ptr(cmd_stack);
					if( top_ptr(cmd_stack) == sub_cmd )
						pop_ptr(cmd_stack);
				}
				break;
			default:
				sub_cmd = this->find_cmd(&cmd_scope,
							 &st->value.
							 children[0]);
				if( !sub_cmd ||
				    (sub_cmd->get_flags() & SCDF_BRACKETED) )
					push_ptr(cmd_stack, sub_cmd);
				break;
			}
			if( sub_cmd && (sub_cmd == partition) ) {
				poss_pivot = lpc;
				if( (ptr_stack_length(cmd_stack) == 1) ) {
					pivot = lpc;
				}
			}
			break;
		}
		default:
			break;
		}
	}
	delete_ptr_stack(cmd_stack);
	if( (poss_pivot != -1) && (pivot == -1) )
		pivot = poss_pivot;
	if( pivot != -1 ) {
		int old_len;
		
		/*
		 * Found the pivot command so adjust the current sequence to
		 * enclose the first part, and make a new sequence to enclose
		 * the second.
		 */
		old_len = sts->get_length();
		sts->set_length(pivot - 1);
		retval = new scml_token_sequence();
		retval->set_indent(sts->get_indent());
		retval->set_stream_pos(sts->get_stream_pos());
		retval->set_value(&(sts->get_value()[pivot + 1]));
		retval->set_length(old_len - (pivot + 1));
	}
	return( retval );
}

int scml_context::define(struct scml_context *sc)
{
	tag_item *kind_ti, *name_ti, *value_ti;
	char *kind_str = 0, *name_str;
	int retval = 0;
	tag_list *tl;
	
	/* Grab our parameters */
	tl = sc->get_lvalues();
	name_ti = find_tag(tl, "name");
	kind_ti = find_tag(tl, "kind");
	value_ti = find_tag(tl, "value");
	if( name_ti && kind_ti && value_ti ) {
		name_str = scml_string::tag_string(name_ti);
		kind_str = scml_string::tag_string(kind_ti);
		if( !strcmp(kind_str, "variable") ) {
			tag_list *dest_tl;
			
			dest_tl = this->lvalues;
			add_tag(dest_tl, name_str, TAG_NONE)->data =
				value_ti->data;
			retval = 1;
		} else if( !strcmp(kind_str, "static-variable") ) {
			tag_list *dest_tl, *par;
			
			dest_tl = this->scope->get_values();
			par = dest_tl->parent;
			dest_tl->parent = 0;
			add_tag(dest_tl, name_str, TAG_NONE)->data =
				value_ti->data;
			dest_tl->parent = par;
			retval = 1;
		} else if( !strcmp(kind_str, "escape" ) ) {
			struct scml_escape *se;
			
			se = new scml_escape;
			se->name = name_str;
			if( (se->value = scml_string::tag_string(value_ti)) )
				this->scope->get_escape_table()->
					add_escape(se);
			else
				delete se;
			retval = 1;
		} else if( !strcmp(kind_str, "tag") ) {
			tag_item *oparams_ti, *rparams_ti;
			struct scml_handler *sh;
			char *value_str;
			
			/* Build a new command with the given parameters */
			value_str = scml_string::tag_string(value_ti);
			oparams_ti = find_tag(tl, "oparams");
			rparams_ti = find_tag(tl, "rparams");
			sh = sht->find_handler(value_str);
			if( value_str && oparams_ti && rparams_ti && sh ) {
				struct scml_cmd_definition *scd;
				
				scd = new scml_cmd_definition;
				scd->set_name(name_str);
				scd->set_handler(sh);
				scd->set_opt_params(oparams_ti->data.
						    tag_data_u.tl);
				scd->set_req_params(rparams_ti->data.
						    tag_data_u.tl);
				this->scope->add_cmd_definition(scd);
				retval = 1;
			} else {
				if( !sh ) {
					scml_alert(this->stream_pos,
						   SAF_ERROR|SAF_RUNTIME,
						   "Couldn't find handler"
						   " function (%s)",
						   value_str);
				}
			}
		} else if( !strcmp(kind_str, "bracket-tag") ) {
			tag_item *oparams_ti, *rparams_ti;
			struct scml_handler *sh;
			char *value_str;
			
			/* Build a bracketed command with the parameters */
			value_str = scml_string::tag_string(value_ti);
			oparams_ti = find_tag(tl, "oparams");
			rparams_ti = find_tag(tl, "rparams");
			sh = sht->find_handler(value_str);
			if( value_str && oparams_ti && rparams_ti && sh ) {
				struct scml_cmd_definition *scd;
				
				scd = new scml_cmd_definition;
				scd->set_name(name_str);
				scd->set_handler(sh);
				scd->set_flags(scd->get_flags() |
					       SCDF_BRACKETED);
				scd->set_opt_params(oparams_ti->data.
						    tag_data_u.tl);
				scd->set_req_params(rparams_ti->data.
						    tag_data_u.tl);
				this->scope->add_cmd_definition(scd);
				retval = 1;
			} else {
				if( !sh ) {
					scml_alert(this->stream_pos,
						   SAF_ERROR|SAF_RUNTIME,
						   "Couldn't find handler"
						   " function (%s)",
						   value_str);
				}
			}
		} else {
			scml_alert(this->stream_pos, SAF_ERROR|SAF_GENERAL,
				   "Unknown kind(%s) in define",
				   kind_str);
		}
	}
	return( retval );
}

int scml_context::undefine(struct scml_context *sc)
{
	tag_item *name_ti;
	char *name_str;
	int retval = 0;
	
	name_ti = find_tag(sc->get_rvalues(), "name");
	if( (name_str = scml_string::tag_string(name_ti)) ) {
		struct scml_escape *se;
		tag_item *ti;
		
		if( find_tag(this->lvalues, name_str) ) {
			rem_tag(this->lvalues, name_str);
		} else if( (ti = find_tag(this->scope->get_values(),
					  name_str)) ) {
			struct scml_cmd_definition *scd;
			
			if( (ti->data.kind == TAG_REF) &&
			    (scd = scml_cmd_definition::
			     ptr(ti->data.tag_data_u.ref)) ) {
				delete scd;
			}
			rem_tag(this->scope->get_values(), name_str);
		} else if( (se = this->scope->get_escape_table()->
			    find_escape(name_str)) ) {
			this->scope->get_escape_table()->rem_escape(name_str);
			delete se;
		}
		retval = 1;
	}
	return( retval );
}

int scml_context::rename(struct scml_context *sc)
{
	char *name_str, *to_str;
	int retval = 0;
	tag_item *ti;
	
	ti = find_tag(sc->get_rvalues(), "name");
	name_str = scml_string::tag_string(ti);
	ti = find_tag(sc->get_rvalues(), "to");
	to_str = scml_string::tag_string(ti);
	if( name_str && to_str ) {
		struct scml_escape *se;
		
		if( (ti = find_tag(this->lvalues, name_str)) ) {
			add_tag(this->lvalues, to_str, TAG_NONE)->data =
				ti->data;
			rem_tag(this->lvalues, name_str);
		} else if( (ti = find_tag(this->scope->get_values(),
					  name_str)) ) {
			tag_data td;
			
			td = ti->data;
			rem_tag(this->scope->get_values(), name_str);
			add_tag(this->scope->get_values(), to_str,
				TAG_NONE)->data = td;
		} else if( (se = this->scope->get_escape_table()->
			    find_escape(name_str)) ) {
			this->scope->get_escape_table()->rem_escape(name_str);
			se->name = to_str;
			this->scope->get_escape_table()->add_escape(se);
		}
		retval = 1;
	}
	return( retval );
}

int scml_context::ifdef(struct scml_context *sc,
			struct scml_token_sequence *sub,
			int defined)
{
	int found = 0, retval = 0;
	tag_item *name_ti;
	char *name_str;
	
	name_ti = find_tag(sc->get_rvalues(), "name");
	if( (name_str = scml_string::tag_string(name_ti)) ) {
		struct scml_token_sequence *else_sts;
		struct scml_scope *cmd_scope;
		
		if( find_tag(this->get_rvalues(), name_str) )
			found = 1;
		else if( find_tag(this->scope->get_values(), name_str) )
			found = 1;
		else if( this->scope->get_escape_table()->
			 find_escape(name_str) )
			found = 1;
		else_sts = this->
			partition_sequence(sc->get_cmd_def(),
					   this->get_scope()->
					   find_cmd_definition(&cmd_scope,
							       "else"),
					   sub);
		if( (found && defined) || (!found && !defined) ) {
			retval = this->format_sequence(sub);
		} else if( else_sts ) {
			retval = this->format_sequence(else_sts);
		} else
			retval = 1;
	} else {
		scml_alert(this->stream_pos, SAF_ERROR|SAF_RUNTIME,
			   "Couldn't get string from 'name' argument");
	}
	return( retval );
}

int scml_context::exec_token(struct scml_token_sequence *sts, int *index,
			     struct scml_token *st)
{
	int retval = 1;
	
	switch( st->kind ) {
	case SCML_NT_COMMAND: {
		struct scml_cmd_definition *scd = 0;
		struct scml_scope *cmd_scope;
		
		switch( st->value.children[0].kind ) {
		case SCML_NT_EXPR: {
			struct scml_token expr;
			
			/* A simple expression, just print it */
			expr = this->eval_token(&st->value.children[0]);
			retval = this->print_token(&expr);
			break;
		}
		case SCML_TERM_SLASH:
			if( !(scd = this->find_cmd(&cmd_scope,
						   &st->value.children[1])) ) {
				scml_alert(this->stream_pos,
					   SAF_ERROR|SAF_RUNTIME,
					   "Couldn't locate command name %s",
					   st->value.children[1].
					   get_identifier());
				retval = 0;
			}
			break;
		case SCML_TERM_ID:
		case SCML_NT_NAME:
			if( !(scd = this->find_cmd(&cmd_scope,
						   &st->value.children[0])) ) {
				scml_alert(this->stream_pos,
					   SAF_ERROR|SAF_RUNTIME,
					   "Couldn't locate command name %s",
					   st->value.children[0].
					   get_identifier());
				retval = 0;
			}
			break;
		default:
			scml_alert(this->stream_pos,
				   SAF_ERROR|SAF_RUNTIME,
				   "Invalid command");
			retval = 0;
			break;
		}
		/* We have to execute a command */
		if( scd ) {
			struct scml_token_sequence *sub = 0;
			struct scml_context *sc;
			
			/* Construct a sub context to execute the command in */
			sc = new scml_context;
			sc->set_flags(this->flags & SCF_IGNORE_WHITE_SPACE);
			sc->set_parent(this);
			sc->set_stream_pos(this->stream_pos);
			sc->set_scope(cmd_scope);
			sc->set_offset_size(this->offset_size);
			sc->set_indent_size(this->indent_size);
			sc->set_ws_queue(this->ws_queue);
			sc->set_cmd_def(scd);
			/* Check for built in commands first */
			if( !strcmp( "define", scd->get_name() ) ) {
				if( sc->handle_params(st) )
					retval = this->define(sc);
				else
					scml_alert(this->stream_pos,
						   SAF_ERROR|SAF_RUNTIME,
						   "Bad args to %s",
						   scd->get_name());
			} else if( !strcmp( "undef", scd->get_name() ) ) {
				if( sc->handle_params(st) )
					retval = this->undefine(sc);
				else
					scml_alert(this->stream_pos,
						   SAF_ERROR|SAF_RUNTIME,
						   "Bad args to %s",
						   scd->get_name());
			} else if( !strcmp( "rename", scd->get_name() ) ) {
				if( sc->handle_params(st) )
					retval = this->rename(sc);
				else
					scml_alert(this->stream_pos,
						   SAF_ERROR|SAF_RUNTIME,
						   "Bad args to %s",
						   scd->get_name());
			} else if( !strcmp( "ifdef", scd->get_name() ) ) {
				if( sc->handle_params(st) &&
				    (sub = this->
				     get_contents(scd, sts,
						  (*index) + 1)) ) {
					(*index) += sub->get_length() + 1;
					retval = this->ifdef(sc, sub, 1);
				}
				else
					scml_alert(this->stream_pos,
						   SAF_ERROR|SAF_RUNTIME,
						   "Bad args to %s",
						   scd->get_name());
			} else if( !strcmp( "ifndef", scd->get_name() ) ) {
				if( sc->handle_params(st) &&
				    (sub = this->
				     get_contents(scd, sts,
						  (*index) + 1)) ) {
					(*index) += sub->get_length() + 1;
					retval = this->ifdef(sc, sub, 0);
				}
				else
					scml_alert(this->stream_pos,
						   SAF_ERROR|SAF_RUNTIME,
						   "Bad args to %s",
						   scd->get_name());
			} else {
				if( scd->get_flags() & SCDF_BRACKETED ) {
					/*
					 * Its bracketed so get the contents
					 * and add a "contents" tag to the
					 * context so the command can use it
					 */
					if( (sub = this->
					     get_contents(scd, sts,
							  (*index) + 1)) ) {
						add_tag(sc->rvalues,
							"contents",
							TAG_REF,
							sub->tag_ref());
						/*
						 * We have to skip over the
						 * contents
						 */
						(*index) += sub->
							get_length() + 1;
						retval = scd->execute(st, sc);
					} else {
						scml_alert(this->stream_pos,
							   SAF_ERROR|
							   SAF_RUNTIME,
							   "Non terminated "
							   "command '%s'",
							   scd->get_name());
						retval = 0;
					}
				} else {
					/* Its a plain command just execute */
					retval = scd->execute(st, sc);
				}
				if( !retval ) {
					scml_alert(this->stream_pos,
						   SAF_ERROR|
						   SAF_RUNTIME,
						   "Command '%s' failed to"
						   " execute",
						   scd->get_name());
				}
			}
			if( sc->flags & SCF_PREFORMATTED )
				this->ws_queue = sc->get_ws_queue();
			else
				this->ws_queue = 0;
			delete sc;
		}
		(*index)++;
		break;
	}
	default:
		retval = this->print_token(st);
		(*index)++;
		break;
	}
	return( retval );
}

int scml_context::format_sequence(struct scml_token_sequence *sts)
{
	struct scml_stream_pos *old_ssp;
	struct scml_token curr;
	int lpc, retval = 1;
	
	old_ssp = this->stream_pos;
	this->stream_pos = sts->get_stream_pos();
	for( lpc = 0; (lpc < sts->get_length()) && retval; ) {
		/* Evaluate and execute the tokens */
		curr = this->eval_token(&(sts->get_value()[lpc]));
		retval = this->exec_token(sts, &lpc, &curr);
	}
	this->stream_pos = old_ssp;
	if( !retval ) {
		printf("Stack Trace -\n");
		this->print(0);
		panic("Too many errors, bailing out");
	}
	return( retval );
}

struct scml_type_map {
	const char *name;
	tag_data_kind tag_type;
};

struct scml_type_map scml_tag_types[] = {
	{"any", TAG_ANY},
	{"int", TAG_INTEGER},
	{"float", TAG_FLOAT},
	{"string", TAG_REF},
	{"tag_list", TAG_TAG_LIST},
	{"bool", TAG_BOOL},
	{"scml", TAG_REF},
	{"cmd", TAG_REF},
	{"internal", TAG_REF},
	{"stream", TAG_REF},
	{0, (tag_data_kind) 0}
};

void scml_context::tag_type(struct scml_token *t_type,
			    tag_data_kind *out_kind, int *out_size)
{
	switch( t_type->kind ) {
	case SCML_NT_SEL: {
		struct scml_token st;
		
		this->tag_type(&t_type->value.children[0], out_kind, out_size);
		if( *out_kind != -1 ) {
			st = this->eval_token(&t_type->value.children[1]);
			st.strip();
			if( st.kind == SCML_TERM_INT ) {
				*out_kind = (tag_data_kind)
					(*out_kind | TAG_ARRAY);
				*out_size = st.value.i;
			} else {
				scml_alert(this->stream_pos,
					   SAF_ERROR|SAF_RUNTIME,
					   "Couldn't get integer for "
					   "array size");
				*out_kind = (tag_data_kind)(-1);
			}
		}
		break;
	}
	case SCML_TERM_ID: {
		int lpc, match = -1;
		
		for( lpc = 0; scml_tag_types[lpc].name && (match == -1);
		     lpc++ ) {
			if( !strcmp( scml_tag_types[lpc].name,
				     t_type->value.id ) )
				match = lpc;
		}
		if( match != -1 ) {
			*out_kind = scml_tag_types[match].tag_type;
		} else {
			*out_kind = (tag_data_kind)(-1);
		}
		break;
	}
	default:
		break;
	}
}

int scml_context::exec_cmd(const char *cmd_name, tag_list *locals, ...)
{
	struct scml_cmd_definition *scd;
	struct scml_stream_pos *ssp = 0;
	struct scml_scope *cmd_scope;
	struct scml_stream *ss = 0;
	tag_data_kind tag_kind;
	struct scml_context *child;
	struct scml_token cmd, id;
	union tag_data_u data;
	va_list arg_addr;
	char *arg_name;
	int retval = 0;
	tag_item *ti;
	
	va_start(arg_addr, locals);
	if( (scd = this->scope->find_cmd_definition(&cmd_scope, cmd_name)) ) {
		/* Build up a fake command to execute */
		id.kind = SCML_TERM_ID;
		id.value.id = cmd_name;
		cmd.kind = SCML_NT_COMMAND;
		cmd.value.children = new scml_token[2];
		cmd.value.children[0] = id;
		cmd.value.children[1].kind = SCML_NONE;
		
		if( !this->stream_pos ) {
			if( scd->get_token_sequence() )
				this->stream_pos = scd->get_token_sequence()->
					get_stream_pos();
			else {
				ss = new scml_stream;
				ss->set_desc(cmd_name);
				ssp = new scml_stream_pos;
				ssp->set_stream(ss);
				this->stream_pos = ssp;
			}
		}
		/*
		 * Construct a child context to execute in.  This is
		 * necessary since we need to replicate the usual
		 * environment for scml code.  A context is executing
		 * the root code and then spawns a child to execute
		 * commands and what not.  And since some of the
		 * commands expect this environment, so who are we
		 * to deny them?
		 */
		child = new scml_context;
		child->set_parent(this);
		child->set_stream_pos(this->stream_pos);
		child->set_cmd_def(scd);
		child->set_scope(cmd_scope);
		/*
		 * Set the parents of the tag lists to be the passed in
		 * locals so that they can reference data in them without
		 * modify the actual tag list by defining variables.
		 */
		child->get_lvalues()->parent = locals;
		this->lvalues->parent = locals;
		/*
		 * Walk through the var args list as if it were the parameter
		 * list in a regular call.
		 */
		arg_name = va_arg(arg_addr, char *);
		while( arg_name ) {
			tag_kind = va_arg(arg_addr, tag_data_kind);
			data = va_arg(arg_addr, union tag_data_u);
			ti = add_tag(child->get_lvalues(),
				     arg_name,
				     TAG_NONE);
			ti->data.kind = tag_kind;
			set_tag_data(&ti->data, 0, data);
			arg_name = va_arg(arg_addr, char *);
		}
		retval = scd->execute(&cmd, child);
		child->get_lvalues()->parent = 0;
		this->lvalues->parent = 0;
		delete child;
	}
	if( ssp )
		delete ssp;
	if( ss )
		delete ss;
	va_end(arg_addr);
	return( retval );
}

void scml_context::set_handler_table(struct scml_handler_table *handlers)
{
	sht = handlers;
}

struct scml_handler_table *scml_context::get_handler_table()
{
	return( sht );
}

void scml_context::print(int level)
{
	union tag_data_u data;

	w_set_fh(stdout);
	printf("SCML Context(%d)\n", level);
	if( this->scope->get_name() )
		printf("  Current Scope: %s\n", this->scope->get_name());
	else
		printf("  Root Scope\n");
	data.tl = this->lvalues;
	printf("  lvalues: ");
	print_tag_data(4, TAG_TAG_LIST, data);
	printf("\n");
	data.tl = this->rvalues;
	printf("  rvalues: ");
	print_tag_data(4, TAG_TAG_LIST, data);
	printf("\n");
	if( this->parent )
		this->parent->print(level + 1);
}

scml_handler_table::scml_handler_table()
{
	int lpc;
	
	for( lpc = 0; lpc < SCML_HANDLER_TABLE_SIZE; lpc++ ) {
		this->table[lpc] = 0;
	}
}

scml_handler_table::~scml_handler_table()
{
}

void scml_handler_table::add_handler(struct scml_handler *sh)
{
	int h;
	
	h = scml_hash_name(sh->name, SCML_HANDLER_TABLE_SIZE);
	sh->next = this->table[h];
	this->table[h] = sh;
}

struct scml_handler *scml_handler_table::find_handler(const char *name)
{
	struct scml_handler *retval = 0, *curr;
	int h;
	
	if( name ) {
		h = scml_hash_name(name, SCML_HANDLER_TABLE_SIZE);
		curr = this->table[h];
		while( curr && !retval ) {
			if( !strcmp( name, curr->name ) )
				retval = curr;
			curr = curr->next;
		}
	}
	return( retval );
}

int c_defvar_handler(struct scml_token *st, struct scml_context *sc)
{
	struct scml_token expr;
	int lpc, retval = 1;
	
	/*
	 * We diverge from regular syntax and just process the parameters
	 * ourselves, turning them into variables for the parent context.
	 */
	for( lpc = 1; st->value.children[lpc].kind != SCML_NONE; lpc++ ) {
		expr = sc->get_parent()->eval_token(&st->value.children[lpc]);
		if( expr.kind == SCML_TERM_TAG ) {
			if( find_tag(sc->get_parent()->get_lvalues(),
				     expr.value.ti->tag) ) {
				scml_alert(sc->get_stream_pos(),
					   SAF_ERROR|SAF_GENERAL,
					   "Tag \"%s\" has already been "
					   "defined", expr.value.ti->tag);
				retval = 0;
			} else {
				add_tag(sc->get_parent()->get_lvalues(),
					expr.value.ti->tag,
					TAG_NONE)->data = expr.value.ti->data;
				delete_tag_item(expr.value.ti);
			}
		} else {
			scml_alert(sc->get_stream_pos(), SAF_ERROR|SAF_GENERAL,
				   "Expression used in defvar doesn't"
				   "result in a tag");
			retval = 0;
		}
	}
	return( retval );
}

int c_ignore_handler(struct scml_token *st, struct scml_context *sc)
{
	int lpc;
	
	/* Similar to defvar except we don't change anything */
	for( lpc = 1; st->value.children[lpc].kind != SCML_NONE; lpc++ ) {
		sc->get_parent()->eval_token(&st->value.children[lpc]);
	}
	return( 1 );
}

int c_scope_handler(struct scml_token *st, struct scml_context *sc)
{
	int retval = 0;
	
	if( sc->handle_params(st) ) {
		struct scml_token_sequence *sts;
		struct scml_scope *scope;
		char *sc_name;
		tag_item *ti;
		
		if( (sc_name = scml_string::
		     tag_string(find_tag(sc->get_rvalues(), "name"))) ) {
			if( strlen(sc_name) ) {
				if( !(scope = sc->get_parent()->
				      get_scope()->find_child(sc_name)) ) {
					/*
					 * Construct a new scope if one isn't
					 * already made.
					 */
					scope = new scml_scope;
					scope->set_name(sc_name);
					scope->set_escape_table(
						new scml_escape_table);
					sc->get_parent()->get_scope()->
						add_child(scope);
				}
			} else {
				/* Empty string means goto the root scope */
				scope = sc->get_parent()->get_scope();
				while( scope->get_parent() )
					scope = scope->get_parent();
			}
			/*
			 * We need to change the parent to this scope and then
			 * have it execute our sequence since we're just
			 * changing the lexical scope and not changing
			 * the execution context.  (i.e. we still need
			 * to get access to any local variables and what not)
			 */
			sc->get_parent()->set_scope(scope);
			if( sc->get_parent()->get_flags() &
			    SCF_PREFORMATTED ) {
				sc->print_indent(sc->get_ws_queue());
				sc->set_ws_queue(0);
				sc->get_parent()->set_ws_queue(0);
			}
			sc->get_parent()->
				set_flags(sc->get_parent()->get_flags() &
					  ~(SCF_PRINTING_BODY));
			ti = find_tag(sc->get_rvalues(), "contents");
			if( ti && (ti->data.kind == TAG_REF) &&
			    (sts = scml_token_sequence::
			     ptr(ti->data.tag_data_u.ref)) ) {
				retval = sc->get_parent()->
					format_sequence(sts);
			}
			sc->set_ws_queue(sc->get_parent()->get_ws_queue());
			sc->get_parent()->set_scope(scope->get_parent());
		} else {
			scml_alert(sc->get_stream_pos(), SAF_ERROR|SAF_RUNTIME,
				   "Invalid string passed in name tag"
				   "for scope");
		}
	}
	return( retval );
}

/*
 * This is the handler for the macro command, it will construct the macro
 * from its parameters and contents.
 */
int c_macro_handler(struct scml_token *st, struct scml_context *sc)
{
	struct scml_cmd_definition *scd;
	struct scml_scope *cmd_scope;
	char *name_str;
	int retval = 0;
	
	if( !sc->handle_params(st) )
		return( retval );
	if( (name_str = scml_string::
	     tag_string(find_tag(sc->get_rvalues(), "name"))) &&
	    !sc->get_parent()->get_scope()->
	    find_cmd_definition(&cmd_scope, name_str) &&
	    find_tag(sc->get_rvalues(), "contents")) {
		/* Build the new command from the given parameters */
		scd = new scml_cmd_definition;
		scd->set_name(name_str);
		if( find_tag(sc->get_rvalues(), "close")->data.tag_data_u.b ) {
			scd->set_flags(scd->get_flags() | SCDF_BRACKETED);
		}
		/*
		 * We use a special handler to actually execute the
		 * macro since all commands need a handler to actually
		 * do something.
		 */
		scd->set_handler(scml_context::get_handler_table()->
				 find_handler("c-macro-executer"));
		scd->set_token_sequence(scml_token_sequence::
					ptr(find_tag(sc->get_rvalues(),
						     "contents")->
					    data.tag_data_u.ref));
		scd->set_opt_params(find_tag(sc->get_rvalues(),
					     "oparams")->
				    data.tag_data_u.tl);
		scd->set_req_params(find_tag(sc->get_rvalues(),
					     "rparams")->
				    data.tag_data_u.tl);
		sc->get_parent()->get_scope()->add_cmd_definition(scd);
		retval = 1;
	} else if( !name_str ) {
		scml_alert(sc->get_stream_pos(), SAF_ERROR|SAF_RUNTIME,
			   "Invalid string passed in name tag for macro");
	} else {
		scml_alert(sc->get_stream_pos(), SAF_ERROR|SAF_RUNTIME,
			   "A macro named %s has already been defined",
			   name_str);
	}
	return( retval );
}

/*
 * This is the handler for executing user macros.  This is different from
 * the above handler since it only constructed the command for the macro
 * this does the actual execution.
 */
int c_macro_executer(struct scml_token *st, struct scml_context *sc)
{
	int retval = 0;
	
	if( sc->handle_params(st) ) {
		struct scml_token_sequence *sts;
		int old_offset, old_indent;
		
		/* Setup the context with values from the sequence */
		sts = sc->get_cmd_def()->get_token_sequence();
		old_offset = sc->get_offset_size();
		sc->set_offset_size(sts->get_indent());
		old_indent = sc->get_indent_size();
		/*
		 * If the parent was preformatted than it will have
		 * indenting information that we need to duplicate.  This
		 * will record the position of the macro call so that
		 * it can be used when printing to get the same initial
		 * indentation as well as any indentation in the macro.
		 */
		if( sc->get_parent()->get_flags() & SCF_PREFORMATTED )
			sc->set_indent_size(sc->get_stream_pos()->
					    get_column() - old_offset);
		/*
		 * Finally execute the macro, note that the format_sequence
		 * that called up has already gotten our "contents" variable
		 * set up.
		 */
		if( !(retval = sc->format_sequence(sts)) )
			scml_alert(sc->get_stream_pos(), SAF_ERROR|SAF_RUNTIME,
				   "There was an error in the macro body");
		sc->set_indent_size(old_indent);
		sc->set_offset_size(old_offset);
	}
	return( retval );
}

/* This handles the if command */
int c_if_handler(struct scml_token *st, struct scml_context *sc)
{
	struct scml_token expr;
	int retval = 0;
	
	/*
	 * Similar to defvar we cheat and don't follow the syntax,
	 * the first child in the token is expected to just evaluate
	 * to an int or bool, rather than setting a parameter.
	 */
	expr = sc->get_parent()->eval_token(&st->value.children[1]);
	expr.strip();
	if( ((expr.kind == SCML_TERM_INT) || (expr.kind == SCML_TERM_BOOL)) ) {
		struct scml_token_sequence *sts, *else_sts;
		struct scml_scope *cmd_scope;
		tag_item *ti;
		
		ti = find_tag(sc->get_rvalues(), "contents");
		if( ti && (ti->data.kind == TAG_REF) &&
		    (sts = scml_token_sequence::
		     ptr(ti->data.tag_data_u.ref)) ) {
			/* Try and partition around an <else> */
			sc->set_flags(sc->get_parent()->get_flags());
			else_sts = sc->partition_sequence(
				sc->get_cmd_def(),
				sc->get_scope()->
				find_cmd_definition(&cmd_scope, "else"),
				sts);
			if( sc->get_parent()->get_flags() &
			    SCF_PREFORMATTED ) {
				sc->print_indent(sc->get_ws_queue());
				sc->set_ws_queue(0);
				sc->get_parent()->set_ws_queue(0);
			}
			sc->get_parent()->
				set_flags(sc->get_parent()->get_flags() &
					  ~SCF_PRINTING_BODY);
			/* Finally, do the test and execute the code. */
			if( expr.value.i ) {
				retval = sc->get_parent()->
					format_sequence(sts);
			} else if( else_sts ) {
				retval = sc->get_parent()->
					format_sequence(else_sts);
			} else
				retval = 1;
			sc->set_ws_queue(sc->get_parent()->get_ws_queue());
		}
	} else {
		scml_alert(sc->get_stream_pos(), SAF_ERROR|SAF_RUNTIME,
			   "Non integer or boolean used for if");
		expr.print();
	}
	return( retval );
}

/*
 * This handles the else command.  Since its sole purpose in live is to be
 * used as something to partition around we just throw an error.
 */
int c_else_handler(struct scml_token */*st*/, struct scml_context *sc)
{
	scml_alert(sc->get_stream_pos(), SAF_ERROR|SAF_RUNTIME,
		   "Else should never be called by itself, it needs"
		   " to be contained in an if block");
	return( 0 );
}

/*
 * This is the handler for 'for'.  It just a simple loop construct that
 * can iterate over a tag array.
 */
int c_for_handler(struct scml_token *st, struct scml_context *sc)
{
	int retval = 0;
	
	if( sc->handle_params(st) ) {
		struct scml_token_sequence *sts;
		union tag_data_u data;
		char *iter_name, *each_str;
		tag_item *tag, *iter_ti, *ti;
		int len;
		
		/* Grab the iterator and tag to iterate over */
		iter_name = scml_string::
			tag_string(find_tag(sc->get_rvalues(), "iter"));
		if( (each_str = scml_string::
		     tag_string(find_tag(sc->get_rvalues(), "each"))) &&
		    iter_name ) {
			tag = find_tag(sc->get_parent()->get_rvalues(),
				       each_str);
			if( !tag && strlen(each_str) ) {
				scml_alert(sc->get_stream_pos(),
					   SAF_ERROR|SAF_RUNTIME,
					   "Couldn't find tag '%s' for "
					   "'for' loop", each_str);
				return 0;
			}
			/* We can iterate over an array or a specific length */
			len = find_tag(sc->get_rvalues(), "length")->
				data.tag_data_u.i;
			if( tag ) {
				len = tag_data_length(&tag->data);
			}
			/* Add the iterator tag */
			if( !(iter_ti = find_tag(sc->get_parent()->
						 get_lvalues(),
						 iter_name)) ) {
				iter_ti = add_tag(sc->get_parent()->
						  get_lvalues(),
						  iter_name, TAG_INTEGER, 0);
			}
			ti = find_tag(sc->get_rvalues(), "contents");
			if( iter_ti->data.kind != TAG_INTEGER ) {
				scml_alert(sc->get_stream_pos(),
					   SAF_ERROR|SAF_RUNTIME,
					   "Iterator '%s' already exists"
					   " and is not an integer",
					   iter_name);
			} else if( ti && (ti->data.kind == TAG_REF) &&
				   (sts = scml_token_sequence::
				    ptr(ti->data.tag_data_u.ref)) ) {
				int lpc;
				
				/* Finally, Loop over the contents */
				retval = 1;
				for( lpc = 0; (lpc < len) && retval; lpc++ ) {
					data.i = lpc;
					set_tag_data(&find_tag(sc->
							       get_parent()->
							       get_rvalues(),
							       iter_name)->
						     data,
						     0,
						     data);
					retval = sc->get_parent()->
						format_sequence(sts);
				}
			}
		} else {
			scml_alert(sc->get_stream_pos(), SAF_ERROR|SAF_RUNTIME,
				   "Invalid string passed in tags for 'for'");
		}
	}
	return( retval );
}

/*
 * This is the handler for the preformatting tag, it just sets the flag
 * for preformatting in the context
 */
int c_pre_handler(struct scml_token *st, struct scml_context *sc)
{
	int retval = 0;
	
	if( sc->handle_params(st) ) {
		struct scml_token_sequence *sts;
		tag_item *ti;
		int do_indent;
		
		/* Clear any queued white space */
		sc->set_ws_queue(0);
		sc->get_parent()->set_ws_queue(0);
		sc->get_parent()->set_flags((sc->get_flags() |
					     SCF_PREFORMATTED));
		/*
		 * Check if we're supposed to do extra indenting based
		 * on where a command is called
		 */
		do_indent = find_tag(sc->get_rvalues(), "indented")->data.
			tag_data_u.b;
		if( !do_indent )
			sc->get_parent()->set_flags(sc->get_parent()->
						    get_flags() |
						    SCF_IGNORE_INDENT);
		ti = find_tag(sc->get_rvalues(), "contents");
		if( ti && (ti->data.kind == TAG_REF) &&
		    (sts = scml_token_sequence::
		     ptr(ti->data.tag_data_u.ref)) ) {
			int old_offset;
			
			/* Set the parent to our sequences offset */
			old_offset = sc->get_parent()->get_offset_size();
			sc->get_parent()->set_offset_size(sts->get_indent());
			if( !(retval = sc->get_parent()->
			      format_sequence(sts)) )
				scml_alert(sc->get_stream_pos(),
					   SAF_ERROR|SAF_RUNTIME,
					   "There was an error in the "
					   "preformat body");
			sc->set_ws_queue(sc->get_parent()->get_ws_queue());
			sc->get_parent()->set_offset_size(old_offset);
		}
		sc->get_parent()->set_flags(sc->get_flags());
	}
	return( retval );
}

/*
 * This is the handler for aliascmd, it simply creates a new command definition
 * that is an alias for another.
 */
int c_aliascmd_handler(struct scml_token *st, struct scml_context *sc)
{
	int retval = 0;
	
	if( sc->handle_params(st) ) {
		struct scml_cmd_definition *scd = 0;
		struct scml_scope *cmd_scope;
		char *name_str;
		
		name_str = scml_string::tag_string(find_tag(sc->get_rvalues(),
							    "name"));
		if( name_str &&
		    (scd = scml_cmd_definition::ptr(find_tag(sc->get_rvalues(),
							     "handler")->
						    data.tag_data_u.ref)) &&
		    !sc->get_parent()->get_scope()->
		    find_cmd_definition(&cmd_scope, name_str) ) {
			struct scml_cmd_definition *new_scd;
			
			/* Make a new command def based on the specified one */
			new_scd = new scml_cmd_definition;
			new_scd->set_name(name_str);
			new_scd->set_flags(scd->get_flags());
			new_scd->set_handler(scd->get_handler());
			new_scd->set_token_sequence(scd->get_token_sequence());
			new_scd->set_opt_params(scd->get_opt_params());
			new_scd->set_req_params(scd->get_req_params());
			sc->get_parent()->get_scope()->
				add_cmd_definition(new_scd);
			retval = 1;
		} else {
			if( !scd ) {
				scml_alert(sc->get_stream_pos(),
					   SAF_ERROR|SAF_RUNTIME,
					   "Couldn't resolve cmd_name value"
					   " to a command");
			}
			if( !name_str ) {
				scml_alert(sc->get_stream_pos(),
					   SAF_ERROR|SAF_RUNTIME,
					   "Invalid value for 'name' tag");
			}
		}
	}
	return( retval );
}

/* This is handler for include, it allows you to include other scml files. */
int c_include_handler(struct scml_token *st, struct scml_context *sc)
{
	char *filename;
	int retval = 0;
	
	if( !sc->handle_params(st) )
		return 0;
	if( (filename = scml_string::tag_string(find_tag(sc->get_rvalues(),
							 "file"))) ) {
		const char * const *include_dir_list;
		FILE *file;
		char *found_filename;
		
		/* Try and open the file in a set of include dirs */
		include_dir_list = sc->get_stream_pos()->get_stream()->
				   get_include_directory_list();
		file = fopen_search(filename, include_dir_list, "r",
				    &found_filename);
		if (file) {
			struct scml_token_sequence *sts;
			struct scml_stream_pos *ssp;
			struct scml_scope *scope;
			struct scml_stream *ss;
			struct scml_parser *sp;
			
			/* Read in, parse, and execute the file */
			scope = sc->get_parent()->get_scope();
			ss = new scml_stream();
			ss->set_file(file);
			ss->set_desc(found_filename);
			ss->set_include_directory_list(include_dir_list);
			ssp = new scml_stream_pos();
			ssp->set_stream(ss);
			sp = new scml_parser;
			sp->set_stream_pos(ssp);
			if( (sts = sp->parse()) ) {
				struct scml_context *new_sc;
				
				new_sc = new scml_context;
				new_sc->set_flags(new_sc->get_flags() |
						  SCF_IGNORE_WHITE_SPACE);
				new_sc->set_stream_pos(ssp);
				new_sc->set_scope(scope);
				if( new_sc->format_sequence(sts) ) {
					retval = 1;
				} else {
					scml_alert(new_sc->get_stream_pos(),
						   SAF_ERROR|SAF_RUNTIME,
						   "There was an error in"
						   "executing the include "
						   "file '%s'", filename);
				}
			} else {
				scml_alert(ssp,
					   SAF_ERROR|SAF_RUNTIME,
					   "There was an error parsing "
					   "include file '%s'", filename);
			}
		} else {
			scml_alert(sc->get_stream_pos(),
				   SAF_ERROR|SAF_RUNTIME,
				   "Cannot open include file '%s'",
				   filename);
		}
	} else {
		scml_alert(sc->get_stream_pos(),
			   SAF_ERROR|SAF_RUNTIME,
			   "Unable to get string from 'file' argument");
	}
	return( retval );
}

int c_retarget_handler(struct scml_token *st, struct scml_context *sc)
{
	int retval = 0;
	
	if( sc->handle_params(st) ) {
		struct scml_stream *ss;
		tag_item *ti;
		
		ti = find_tag(sc->get_rvalues(), "output");
		if( (ss = scml_stream::ptr(ti->data.tag_data_u.ref)) ) {
			if( ss->get_file() ) {
				sc->get_parent()->set_flags(
					sc->get_parent()->get_flags() &
					~SCF_IGNORE_WHITE_SPACE);
				w_set_fh(ss->get_file());
			} else {
				sc->get_parent()->set_flags(
					sc->get_parent()->get_flags() |
					SCF_IGNORE_WHITE_SPACE);
				w_set_fh(stdout);
			}
			retval = 1;
		} else {
			scml_alert(sc->get_stream_pos(),
				   SAF_ERROR|SAF_RUNTIME,
				   "Cannot get output stream");
		}
	}
	return( retval );
}

int c_create_stream_handler(struct scml_token *st, struct scml_context *sc)
{
	int retval = 0;
	
	if( sc->handle_params(st) ) {
		char *name_str, *path_str;
		tag_item *ti;
		
		ti = find_tag(sc->get_rvalues(), "name");
		name_str = scml_string::tag_string(ti);
		ti = find_tag(sc->get_rvalues(), "path");
		path_str = scml_string::tag_string(ti);
		if( name_str && path_str ) {
			struct scml_stream *ss;
			FILE *file = NULL;
			
			if( strlen(path_str) ) {
				file = fopen(path_str, "w");
				if( !file )
					panic("Cannot create file %s",
					      path_str);
			}
			ss = new scml_stream;
			ss->set_flags((ss->get_flags() | SSF_OUTPUT) &
				      ~SSF_INPUT);
			ss->set_desc(path_str);
			ss->set_file(file);
			add_tag(sc->get_scope()->get_values(), name_str,
				TAG_REF, ss->tag_ref());
			retval = 1;
		}
	}
	return( retval );
}

struct cast_handler_entry scml_code_cast_handler_entry;

int init_scml()
{
	struct scml_handler_table *sht;
	struct scml_handler *sh;
	
	scml_code_cast_handler_entry.he.name = "scml-code-cast-handler";
	scml_code_cast_handler_entry.c_func = scml_code_cast_handler;
	add_entry(cast_handler_table, &scml_code_cast_handler_entry.he);
	
	/* Construct a handler table for the set of built in handlers */
	sht = new scml_handler_table;
	
	sh = new scml_handler;
	sh->name = "c-ignore-handler";
	sh->kind = SHK_C_FUNCTION;
	sh->function.c_func = c_ignore_handler;
	sht->add_handler(sh);
	
	sh = new scml_handler;
	sh->name = "c-macro-handler";
	sh->kind = SHK_C_FUNCTION;
	sh->function.c_func = c_macro_handler;
	sht->add_handler(sh);
	
	sh = new scml_handler;
	sh->name = "c-macro-executer";
	sh->kind = SHK_C_FUNCTION;
	sh->function.c_func = c_macro_executer;
	sht->add_handler(sh);
	
	sh = new scml_handler;
	sh->name = "c-scope-handler";
	sh->kind = SHK_C_FUNCTION;
	sh->function.c_func = c_scope_handler;
	sht->add_handler(sh);
	
	sh = new scml_handler;
	sh->name = "c-if-handler";
	sh->kind = SHK_C_FUNCTION;
	sh->function.c_func = c_if_handler;
	sht->add_handler(sh);
	
	sh = new scml_handler;
	sh->name = "c-defvar-handler";
	sh->kind = SHK_C_FUNCTION;
	sh->function.c_func = c_defvar_handler;
	sht->add_handler(sh);
	
	sh = new scml_handler;
	sh->name = "c-else-handler";
	sh->kind = SHK_C_FUNCTION;
	sh->function.c_func = c_else_handler;
	sht->add_handler(sh);
	
	sh = new scml_handler;
	sh->name = "c-for-handler";
	sh->kind = SHK_C_FUNCTION;
	sh->function.c_func = c_for_handler;
	sht->add_handler(sh);
	
	sh = new scml_handler;
	sh->name = "c-pre-handler";
	sh->kind = SHK_C_FUNCTION;
	sh->function.c_func = c_pre_handler;
	sht->add_handler(sh);
	
	sh = new scml_handler;
	sh->name = "c-aliascmd-handler";
	sh->kind = SHK_C_FUNCTION;
	sh->function.c_func = c_aliascmd_handler;
	sht->add_handler(sh);
	
	sh = new scml_handler;
	sh->name = "c-include-handler";
	sh->kind = SHK_C_FUNCTION;
	sh->function.c_func = c_include_handler;
	sht->add_handler(sh);
	
	sh = new scml_handler;
	sh->name = "c-retarget-handler";
	sh->kind = SHK_C_FUNCTION;
	sh->function.c_func = c_retarget_handler;
	sht->add_handler(sh);
	
	sh = new scml_handler;
	sh->name = "c-create-stream-handler";
	sh->kind = SHK_C_FUNCTION;
	sh->function.c_func = c_create_stream_handler;
	sht->add_handler(sh);
	
	scml_context::set_handler_table(sht);
	
	return( 1 );
}

/* End of file. */

