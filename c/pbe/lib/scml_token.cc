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

#include <mom/c/scml.hh>

void scml_token::strip()
{
	switch( this->kind ) {
	case SCML_TERM_TAG:
		switch( this->value.ti->data.kind ) {
		case TAG_BOOL:
			this->kind = SCML_TERM_BOOL;
			this->value.b = this->value.ti->data.tag_data_u.b;
			break;
		case TAG_INTEGER:
			this->kind = SCML_TERM_INT;
			this->value.i = this->value.ti->data.tag_data_u.i;
			break;
		case TAG_FLOAT:
			this->kind = SCML_TERM_FLOAT;
			this->value.f = this->value.ti->data.tag_data_u.f;
			break;
		case TAG_CAST_SCOPED_NAME:
		case TAG_CAST_DEF:
		case TAG_CAST_TYPE:
		case TAG_CAST_EXPR:
		case TAG_CAST_STMT:
		case TAG_CAST_INIT:
		case TAG_STRING: {
			struct scml_string *ss;
			
			this->kind = SCML_TERM_STRING;
			ss = new scml_string;
			*ss->add_component() = this->value.ti->data;
			this->value.str = ss;
			break;
		}
		case TAG_TAG_LIST:
			this->kind = SCML_TERM_TAG_LIST;
			this->value.tl = this->value.ti->data.tag_data_u.tl;
			break;
		case TAG_REF: {
			struct scml_string *ss;
			
			if( (ss = scml_string::ptr(this->value.ti->
						   data.tag_data_u.ref)) ) {
				this->kind = SCML_TERM_STRING;
				this->value.str = ss;
			}
			break;
		}
		default:
			break;
		}
		break;
	default:
		break;
	}
}

void scml_token::promote(int required_kind)
{
	switch( required_kind ) {
	case SCML_TERM_INT:
		switch( this->kind ) {
		case SCML_TERM_INT:
			break;
		default:
			this->kind = SCML_ERROR;
			break;
		}
		break;
	case SCML_TERM_FLOAT:
		switch( this->kind ) {
		case SCML_TERM_FLOAT:
			break;
		case SCML_TERM_INT:
			this->kind = SCML_TERM_FLOAT;
			this->value.f = this->value.i;
			break;
		default:
			this->kind = SCML_ERROR;
			break;
		}
		break;
	case SCML_TERM_TAG: {
		tag_data td;
		
		switch( this->kind ) {
		case SCML_TERM_BOOL:
			this->kind = SCML_TERM_BOOL;
			td = create_tag_data(TAG_BOOL, 1);
			td.tag_data_u.b = this->value.i;
			break;
		case SCML_TERM_STRING:
			this->kind = SCML_TERM_TAG;
			td = create_tag_data(TAG_REF, 1);
			td.tag_data_u.ref = this->value.str->tag_ref();
			this->value.ti = create_tag_item("", &td);
			break;
		case SCML_TERM_INT:
			this->kind = SCML_TERM_TAG;
			td = create_tag_data(TAG_INTEGER, 1);
			td.tag_data_u.i = this->value.i;
			this->value.ti = create_tag_item("", &td);
			break;
		case SCML_TERM_FLOAT:
			this->kind = SCML_TERM_TAG;
			td = create_tag_data(TAG_FLOAT, 1);
			td.tag_data_u.f = this->value.f;
			this->value.ti = create_tag_item("", &td);
			break;
		case SCML_TERM_TAG:
			break;
		default:
			this->kind = SCML_ERROR;
			break;
		}
		break;
	}
	case SCML_TERM_STRING: {
		struct scml_string *ss;
		tag_data *td;
		
		this->strip();
		switch( this->kind ) {
		case SCML_TERM_BOOL:
			this->kind = SCML_TERM_STRING;
			ss = new scml_string;
			td = ss->add_component();
			td->kind = TAG_STRING;
			td->tag_data_u.str = flick_asprintf("%s",
							    this->value.i ?
							    "true" : "false");
			this->value.str = ss;
			break;
		case SCML_TERM_INT:
			this->kind = SCML_TERM_STRING;
			ss = new scml_string;
			td = ss->add_component();
			td->kind = TAG_STRING;
			td->tag_data_u.str = flick_asprintf("%d",
							    this->value.i);
			this->value.str = ss;
			break;
		case SCML_TERM_FLOAT:
			this->kind = SCML_TERM_STRING;
			ss = new scml_string;
			td = ss->add_component();
			td->kind = TAG_STRING;
			td->tag_data_u.str = flick_asprintf("%f",
							    this->value.f);
			this->value.str = ss;
			break;
		case SCML_TERM_STRING:
			break;
		default:
			this->kind = SCML_ERROR;
			break;
		}
		break;
	}
	case SCML_TERM_TAG_LIST:
		switch( this->kind ) {
		case SCML_TERM_TAG_LIST:
			break;
		default:
			this->kind = SCML_ERROR;
			break;
		}
		break;
	default:
		this->kind = SCML_ERROR;
		break;
	}
}

const char *scml_kind_map[SCML_NT_MAX + 1] = {
	"SCML_NONE",
	"SCML_IGNORE",
	"SCML_ERROR",
	"SCML_DONE",
	"SCML_COL_POS",
	"SCML_ROW_POS",
	
	"SCML_TERM",
	
	"SCML_TERM_BOOL",
	"SCML_TERM_INT",
	"SCML_TERM_FLOAT",
	"SCML_TERM_STRING",
	"SCML_TERM_TAG",
	"SCML_TERM_TAG_LIST",
	
	"SCML_TERM_TEXT",
	"SCML_TERM_ESCAPE",
	"SCML_TERM_ID",
	"SCML_TERM_VERBATIM",
	
	"SCML_TERM_LPAREN",
	"SCML_TERM_RPAREN",
	"SCML_TERM_LBRACE",
	"SCML_TERM_RBRACE",
	"SCML_TERM_LCURLY",
	"SCML_TERM_RCURLY",
	"SCML_TERM_SLASH",
	"SCML_TERM_LT",
	"SCML_TERM_GT",
	
	"SCML_TERM_MAX",
	"SCML_NT",
	
	"SCML_NT_COMMAND",
	"SCML_NT_SET",
	
	"SCML_NT_ASSIGN",
	"SCML_NT_PLUS",
	"SCML_NT_MINUS",
	"SCML_NT_DIV",
	"SCML_NT_MOD",
	"SCML_NT_MULT",
	"SCML_NT_COND",
	"SCML_NT_COLON",
	"SCML_NT_COMMA",
	"SCML_NT_EQUAL",
	"SCML_NT_NOT_EQUAL",
	"SCML_NT_LT",
	"SCML_NT_GT",
	"SCML_NT_LE",
	"SCML_NT_GE",
	"SCML_NT_AND",
	"SCML_NT_LAND",
	"SCML_NT_OR",
	"SCML_NT_LOR",
	"SCML_NT_XOR",
	"SCML_NT_NOT",
	"SCML_NT_DOT",
	"SCML_NT_SCOPE_RES",
	"SCML_NT_NAME",
	"SCML_NT_EXPR",
	"SCML_NT_TAG",
	"SCML_NT_SEL",
	"SCML_NT_INIT",
	
	"SCML_NT_MAX"
};

void scml_token::print()
{
	printf(":%s", scml_kind_map[this->kind]);
	switch( this->kind ) {
	case SCML_NONE:
	case SCML_DONE:
		break;
	case SCML_IGNORE:
		printf("=ignore");
		break;
	case SCML_ERROR:
		printf("=error");
		break;
	case SCML_COL_POS:
		printf("=%d", this->value.i);
		break;
	case SCML_ROW_POS:
		printf("=%d", this->value.i);
		break;
	case SCML_TERM_BOOL:
		printf("=%s", this->value.b ? "true" : "false");
		break;
	case SCML_TERM_INT:
		printf("=%d", this->value.i);
		break;
	case SCML_TERM_FLOAT:
		printf("=%f", this->value.f);
		break;
	case SCML_TERM_STRING:
		printf("=\"");
		this->value.str->print(0);
		printf("\"");
		break;
	case SCML_TERM_TEXT:
		printf("=\'%s\'", this->value.text);
		break;
	case SCML_TERM_ESCAPE:
		printf("=%s", this->value.escape);
		break;
	case SCML_TERM_ID:
		printf("=%s", this->value.id);
		break;
	case SCML_TERM_VERBATIM:
		printf("=%s", this->value.text);
		break;
	case SCML_TERM_TAG:
		print_tag(0, this->value.ti);
		break;
	case SCML_TERM_TAG_LIST: {
		tag_item ti;
		
		ti.tag = ir_strlit("");
		ti.data.kind = TAG_TAG_LIST;
		ti.data.tag_data_u.tl = this->value.tl;
		printf("={\n");
		print_tag(0, &ti);
		printf("}\n");
		break;
	}
	case SCML_NT_COMMAND:
		if( this->value.children ) {
			int lpc;
			
			printf("=<");
			for( lpc = 0;
			     this->value.children[lpc].kind != SCML_NONE;
			     lpc++ ) {
				if( lpc > 0 )
					printf(" ");
				this->value.children[lpc].print();
			}
			printf(">");
		}
		break;
	case SCML_NT_SET:
		if( this->value.children ) {
			this->value.children[0].print();
			printf(" = ");
			this->value.children[1].print();
		}
		break;
	case SCML_NT_PLUS:
		if( this->value.children ) {
			printf("(");
			this->value.children[0].print();
			printf(" + ");
			this->value.children[1].print();
			printf(")");
		}
		break;
	case SCML_NT_MINUS:
		if( this->value.children ) {
			printf("(");
			this->value.children[0].print();
			printf(" - ");
			this->value.children[1].print();
			printf(")");
		}
		break;
	case SCML_NT_DIV:
		if( this->value.children ) {
			this->value.children[0].print();
			printf(" / ");
			this->value.children[1].print();
		}
		break;
	case SCML_NT_MOD:
		if( this->value.children ) {
			this->value.children[0].print();
			printf(" %% ");
			this->value.children[1].print();
		}
		break;
	case SCML_NT_MULT:
		if( this->value.children ) {
			printf("(");
			this->value.children[0].print();
			printf(" * ");
			this->value.children[1].print();
			printf(")");
		}
		break;
	case SCML_NT_COLON:
		if( this->value.children ) {
			this->value.children[0].print();
			printf(" : ");
			this->value.children[1].print();
		}
		break;
	case SCML_NT_EQUAL:
		if( this->value.children ) {
			this->value.children[0].print();
			printf(" == ");
			this->value.children[1].print();
		}
		break;
	case SCML_NT_NOT_EQUAL:
		if( this->value.children ) {
			this->value.children[0].print();
			printf(" != ");
			this->value.children[1].print();
		}
		break;
	case SCML_NT_LT:
		if( this->value.children ) {
			this->value.children[0].print();
			printf(" < ");
			this->value.children[1].print();
		}
		break;
	case SCML_NT_GT:
		if( this->value.children ) {
			this->value.children[0].print();
			printf(" > ");
			this->value.children[1].print();
		}
		break;
	case SCML_NT_LE:
		if( this->value.children ) {
			this->value.children[0].print();
			printf(" <= ");
			this->value.children[1].print();
		}
		break;
	case SCML_NT_GE:
		if( this->value.children ) {
			this->value.children[0].print();
			printf(" >= ");
			this->value.children[1].print();
		}
		break;
	case SCML_NT_AND:
		if( this->value.children ) {
			this->value.children[0].print();
			printf(" & ");
			this->value.children[1].print();
		}
		break;
	case SCML_NT_LAND:
		if( this->value.children ) {
			this->value.children[0].print();
			printf(" && ");
			this->value.children[1].print();
		}
		break;
	case SCML_NT_OR:
		if( this->value.children ) {
			this->value.children[0].print();
			printf(" | ");
			this->value.children[1].print();
		}
		break;
	case SCML_NT_LOR:
		if( this->value.children ) {
			this->value.children[0].print();
			printf(" || ");
			this->value.children[1].print();
		}
		break;
	case SCML_NT_NOT:
		if( this->value.children ) {
			printf("! ");
			this->value.children[0].print();
		}
		break;
	case SCML_NT_DOT:
		if( this->value.children ) {
			this->value.children[0].print();
			printf(" . ");
			this->value.children[1].print();
		}
		break;
	case SCML_NT_SCOPE_RES:
		if( this->value.children ) {
			this->value.children[0].print();
			printf(" :: ");
			this->value.children[1].print();
		}
		break;
	case SCML_NT_EXPR:
		if( this->value.children ) {
			this->value.children[0].print();
		}
		break;
	case SCML_NT_TAG:
		if( this->value.children ) {
			this->value.children[0].print();
			printf(" : ");
			this->value.children[1].print();
		}
		break;
	case SCML_NT_NAME:
		this->value.children[0].print();
		break;
	case SCML_NT_SEL:
		if( this->value.children ) {
			this->value.children[0].print();
			printf(" [ ");
			this->value.children[1].print();
			printf(" ] ");
		}
		break;
	case SCML_NT_INIT:
		if( this->value.children ) {
			int lpc;
			
			printf(" { ");
			for( lpc = 0;
			     this->value.children[lpc].kind != SCML_NONE;
			     lpc++ ) {
				if( lpc > 0 )
					printf(" ");
				this->value.children[lpc].print();
			}
			printf( " } ");
		}
		break;
	default:
		printf("=<>");
		break;
	}
}

int scml_token::is_expr()
{
	int retval = 0;
	
	switch( this->kind ) {
	case SCML_TERM_INT:
	case SCML_TERM_BOOL:
	case SCML_TERM_FLOAT:
	case SCML_TERM_STRING:
	case SCML_TERM_ID:
	case SCML_NT_EXPR:
	case SCML_NT_NAME:
	case SCML_NT_TAG:
	case SCML_NT_SEL:
	case SCML_NT_INIT:
		retval = this->kind;
		break;
	default:
		break;
	}
	return( retval );
}

int scml_token::is_value()
{
	int retval = 0;
	
	switch( this->kind ) {
	case SCML_TERM_INT:
	case SCML_TERM_BOOL:
	case SCML_TERM_FLOAT:
	case SCML_TERM_STRING:
	case SCML_TERM_TAG:
	case SCML_TERM_TAG_LIST:
	case SCML_TERM_TEXT:
	case SCML_TERM_ESCAPE:
	case SCML_TERM_ID:
	case SCML_TERM_VERBATIM:
	case SCML_NT_COMMAND:
	case SCML_NT_EXPR:
	case SCML_NT_TAG:
	case SCML_NT_SEL:
	case SCML_NT_NAME:
	case SCML_NT_INIT:
		retval = this->kind;
		break;
	default:
		break;
	}
	return( retval );
}

int scml_token::is_operator()
{
	int retval = 0;
	
	switch( this->kind ) {
	case SCML_NT_ASSIGN:
	case SCML_NT_PLUS:
	case SCML_NT_MINUS:
	case SCML_NT_DIV:
	case SCML_NT_MOD:
	case SCML_NT_MULT:
	case SCML_NT_COLON:
	case SCML_NT_EQUAL:
	case SCML_NT_NOT_EQUAL:
	case SCML_NT_LT:
	case SCML_NT_GT:
	case SCML_NT_LE:
	case SCML_NT_GE:
	case SCML_NT_AND:
	case SCML_NT_LAND:
	case SCML_NT_OR:
	case SCML_NT_LOR:
	case SCML_NT_NOT:
	case SCML_NT_DOT:
	case SCML_NT_SCOPE_RES:
		retval = this->kind;
		break;
	default:
		break;
	}
	return( retval );
}

int scml_token::is_container()
{
	int retval = 0;
	
	switch( this->kind ) {
	case SCML_TERM_LPAREN:
	case SCML_TERM_RPAREN:
		retval = SCML_NT_EXPR;
		break;
	case SCML_TERM_LBRACE:
	case SCML_TERM_RBRACE:
		retval = SCML_NT_SEL;
		break;
	case SCML_TERM_LCURLY:
	case SCML_TERM_RCURLY:
		retval = SCML_NT_INIT;
		break;
	case SCML_TERM_LT:
	case SCML_TERM_GT:
		retval = SCML_NT_COMMAND;
		break;
	default:
		break;
	}
	return( retval );
}

struct scml_prec {
	int op;
	int prec;
};

struct scml_prec scml_op_prec[] = {
	{SCML_NT_ASSIGN, 0},
	{SCML_NT_COLON, 1},
	{SCML_NT_SCOPE_RES, 1},
	{SCML_NT_LOR, 2},
	{SCML_NT_LAND, 3},
	{SCML_NT_OR, 4},
	{SCML_NT_AND, 5},
	{SCML_NT_EQUAL, 6},
	{SCML_NT_NOT_EQUAL, 6},
	{SCML_NT_LT, 7},
	{SCML_NT_GT, 7},
	{SCML_NT_LE, 7},
	{SCML_NT_GE, 7},
	{SCML_NT_PLUS, 8},
	{SCML_NT_MINUS, 8},
	{SCML_NT_MULT, 9},
	{SCML_NT_DIV, 9},
	{SCML_NT_MOD, 9},
	{SCML_TERM_LBRACE, 10},
	{SCML_TERM_RBRACE, 10},
	{SCML_NT_NOT, 12},
	{SCML_NT_COMMA, 12},
	{SCML_NT_DOT, 13},
	{SCML_TERM_LPAREN, 13},
	{SCML_TERM_RPAREN, 13},
	{SCML_TERM_LCURLY, 13},
	{SCML_TERM_RCURLY, 13},
	{SCML_NONE, 0}
};

int scml_token::get_prec()
{
	int retval = -1;
	int lpc;
	
	for( lpc = 0; (scml_op_prec[lpc].op != SCML_NONE) && (retval == -1);
	     lpc++ ) {
		if( this->kind == scml_op_prec[lpc].op )
			retval = scml_op_prec[lpc].prec;
	}
	return( retval );
}

char *scml_token::get_identifier()
{
	char *retval = 0;
	
	switch( this->kind ) {
	case SCML_TERM_ID:
		retval = flick_asprintf("%s", this->value.id);
		break;
	case SCML_TERM_ESCAPE:
		retval = flick_asprintf("%s", this->value.escape);
		break;
	case SCML_NT_NAME:
		retval = this->value.children[0].get_identifier();
		break;
	case SCML_NT_SCOPE_RES: {
		char *left, *right;
		
		left = this->value.children[0].get_identifier();
		right = this->value.children[1].get_identifier();
		retval = flick_asprintf("%s::%s", left, right);
		free(left);
		free(right);
		break;
	}
	default:
		break;
	}
	return( retval );
}

scml_token_sequence::scml_token_sequence()
{
	this->stream_pos = 0;
	this->indent = 0;
	this->val = 0;
	this->len = 0;
}

scml_token_sequence::~scml_token_sequence()
{
}

char *scml_token_sequence::tag_ref()
{
	return( ptr_to_tag_ref("struct scml_token_sequence *", this) );
}

struct scml_token_sequence *scml_token_sequence::ptr(char *ref)
{
	return( (struct scml_token_sequence *)
		tag_ref_to_ptr("struct scml_token_sequence *", ref) );
}

void scml_token_sequence::print()
{
	int lpc;
	
	printf("sequence=\"");
	for( lpc = 0; lpc < this->len; lpc++ ) {
		this->val[lpc].print();
	}
	printf("\"");
}

void scml_token_sequence::set_value(struct scml_token *st)
{
	this->val = st;
}

struct scml_token *scml_token_sequence::get_value()
{
	return( this->val );
}

void scml_token_sequence::set_length(int the_len)
{
	this->len = the_len;
}

int scml_token_sequence::get_length()
{
	return( this->len );
}

void scml_token_sequence::set_stream_pos(struct scml_stream_pos *ssp)
{
	this->stream_pos = ssp;
}

struct scml_stream_pos *scml_token_sequence::get_stream_pos()
{
	return( this->stream_pos );
}

void scml_token_sequence::set_indent(int offset)
{
	this->indent = offset;
}

int scml_token_sequence::get_indent()
{
	return( this->indent );
}

int scml_token_stack::default_max = 10;
int scml_token_stack::default_inc = 10;

scml_token_stack::scml_token_stack()
{
	this->val = (struct scml_token *)mustmalloc(default_max *
						    sizeof(struct scml_token));
	this->top = -1;
	this->max = default_max;
}

scml_token_stack::~scml_token_stack()
{
	free( this->val );
}

void scml_token_stack::push(struct scml_token &st)
{
	this->top++;
	if( this->top >= this->max ) {
		this->max += default_inc;
		this->val = (struct scml_token *)
			mustrealloc(this->val,
				    this->max * sizeof(struct scml_token));
	}
	this->val[this->top] = st;
}

void scml_token_stack::pop()
{
	this->top--;
}

struct scml_token *scml_token_stack::index(int offset)
{
	if( offset > 0 )
		panic("Less than zero offset used in index");
	else if( (this->top + offset) < 0 )
		panic("Too large an offset used in index");
	return( &(this->val[this->top + offset]) );
}

int scml_token_stack::count()
{
	return( this->top + 1 );
}

void scml_token_stack::print()
{
	int lpc;
	
	for( lpc = this->top; lpc >= 0; lpc-- ) {
		this->val[lpc].print();
		printf("\n");
	}
}
