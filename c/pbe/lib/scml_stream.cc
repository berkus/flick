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

#include <mom/c/scml.hh>

scml_stream::scml_stream()
{
	this->flags = SSF_INPUT;
	this->include_directory_list = 0;
	this->desc = 0;
	this->data = 0;
	this->len = 0;
}

scml_stream::~scml_stream()
{
	if( this->data )
		free(this->data);
}

char *scml_stream::tag_ref()
{
	return( ptr_to_tag_ref("struct scml_stream *", this) );
}

struct scml_stream *scml_stream::ptr(char *ref)
{
	return( (struct scml_stream *)
		tag_ref_to_ptr("struct scml_stream *", ref) );
}

void scml_stream::set_flags(unsigned int the_flags)
{
	this->flags = the_flags;
}

unsigned int scml_stream::get_flags()
{
	return( this->flags );
}

void scml_stream::set_data(char *str)
{
	this->data = str;
	if( str )
		this->len = strlen(str);
	else
		this->len = 0;
}

char *scml_stream::get_data()
{
	return( this->data );
}

void scml_stream::set_desc(const char *the_desc)
{
	this->desc = the_desc;
}

const char *scml_stream::get_desc()
{
	return( this->desc );
}

void scml_stream::set_include_directory_list(const char * const *dir_list)
{
	this->include_directory_list = dir_list;
}

const char * const *scml_stream::get_include_directory_list()
{
	return( this->include_directory_list );
}

int scml_stream::get_length()
{
	return( this->len );
}

void scml_stream::set_file(FILE *the_file)
{
	this->file = the_file;
	if( this->flags & SSF_INPUT ) {
		if( fseek(this->file, 0, SEEK_END) == 0 ) {
			this->len = ftell(this->file);
			fseek(this->file, 0, SEEK_SET);
			this->data = (char *)mustmalloc( this->len + 1 );
			fread(this->data, this->len, 1, this->file);
			this->data[this->len] = 0;
		} else {
			scml_alert(0, SAF_ERROR|SAF_IO,
				   "Error accessing file");
		}
	}
}

FILE *scml_stream::get_file()
{
	return( this->file );
}

scml_stream_pos::scml_stream_pos()
{
	this->stream = 0;
	this->flags = 0;
	this->state = SSPS_NONE;
	this->cursor = 0;
	this->row = 0;
	this->column = 0;
}

scml_stream_pos::~scml_stream_pos()
{
}

void scml_stream_pos::set_stream(struct scml_stream *the_stream)
{
	this->stream = the_stream;
	if( the_stream ) {
		this->state = SSPS_COL_POS;
		this->cursor = the_stream->get_data();
		this->row = 1;
		this->column = 0;
	}
}

struct scml_stream *scml_stream_pos::get_stream()
{
	return( this->stream );
}

void scml_stream_pos::set_flags(unsigned int the_flags)
{
	this->flags = the_flags;
}

unsigned int scml_stream_pos::get_flags()
{
	return( this->flags );
}

void scml_stream_pos::set_state(int the_state)
{
	this->state = the_state;
}

int scml_stream_pos::get_state()
{
	return( this->state );
}

void scml_stream_pos::set_cursor(char *the_cursor)
{
	this->cursor = the_cursor;
}

char *scml_stream_pos::get_cursor()
{
	return( this->cursor );
}

void scml_stream_pos::set_row(int the_row)
{
	this->row = the_row;
}

int scml_stream_pos::get_row()
{
	return( this->row );
}

void scml_stream_pos::set_column(int the_column)
{
	this->column = the_column;
}

int scml_stream_pos::get_column()
{
	return( this->column );
}

char *scml_stream_pos::munge_string(char *str)
{
	char *retval;
	int len, r_pos, s_pos;
	
	len = strlen(str);
	retval = (char *)mustmalloc(len + 1);
	for( r_pos = 0, s_pos = 0; str[s_pos]; r_pos++, s_pos++ ) {
		if( str[s_pos] == '\\' ) {
			switch( str[s_pos + 1] ) {
			case 'n':
				retval[r_pos] = '\n';
				break;
			case 't':
				retval[r_pos] = '\t';
				break;
			case 'x': {
				int c_val;
				
				sscanf(&str[s_pos + 2], "%2x", &c_val);
				retval[r_pos] = c_val;
				s_pos += 2;
				break;
			}
			default:
				retval[r_pos] = str[s_pos + 1];
				break;
			}
			s_pos++;
		}
		else {
			retval[r_pos] = str[s_pos];
		}
	}
	retval[r_pos] = 0;
	return( retval );
}

int scml_stream_pos::scan(int kind, const char *str)
{
	int saved_row = 0, saved_col = 0;
	int str_pos = 0;
	int retval = 0;
	
	/*
	 * Record the last position so we can give the user a range
	 * of text to look at on an error
	 */
	this->last_row = this->row;
	this->last_column = this->column;
	for( ; *this->cursor && !retval; this->cursor++ ) {
		/* Track the cursor position */
		switch( *this->cursor ) {
		case '\n':
			this->row++;
			this->column = -1;
			break;
		case '\t':
			this->column += 7;
			break;
		default:
			break;
		}
		if( kind & SSPF_SCAN_SEQUENCE ) {
			/* Scan for a sequence of characters. */
			if( (*this->cursor == str[str_pos]) ) {
				if( str_pos == 0 ) {
					saved_row = this->row;
					saved_col = this->column;
				}
				str_pos++;
				if( str[str_pos] == 0 )
					retval = 1;
			} else {
				this->cursor -= str_pos;
				if( str_pos ) {
					this->row = saved_row;
					this->column = saved_col;
				}
				str_pos = 0;
			}
		} else if( kind & SSPF_SCAN_SET ) {
			int found = 0;
			
			/* Scan for a character in the set */
			for( str_pos = 0;
			     str[str_pos] && !found;
			     str_pos++ ) {
				if( *this->cursor == str[str_pos] )
					found = 1;
			}
			if( (found && (kind & SSPF_SCAN_IN)) ||
			    (!found && (kind & SSPF_SCAN_NOT_IN)) )
				retval = 1;
		}
		this->column++;
	}
	if( (*this->cursor == 0) && (this->flags & SSPF_IGNORE_EOF) ) {
		retval = 1;
	}
	if( (*this->cursor == 0) && (this->flags & SSPF_IGNORE_EOF) )
		this->flags |= SSPF_EOF;
	if( !retval ) {
		this->last_row = -1;
		this->last_column = -1;
	}
	return( retval );
}

int scml_stream_pos::get_last_row()
{
	return( this->last_row );
}

int scml_stream_pos::get_last_column()
{
	return( this->last_column );
}

int scml_stream_pos::get_number(struct scml_token *st)
{
	int retval = 1;
	char *start;
	
	if( (*(this->cursor - 1) == '0') && ((*this->cursor == 'x') ||
	     (*this->cursor == 'X')) ) {
		/* Its a hex number */
		this->cursor++;
		this->column++;
		start = this->cursor;
		if( this->scan(SSPF_SCAN_NOT_IN|SSPF_SCAN_SET,
			       "0123456789AaBbCcDdEeFf") &&
		    sscanf(start, "%x", &st->value.i) == 1 ) {
			st->kind = SCML_TERM_INT;
		} else {
			scml_alert(this, SAF_ERROR|SAF_LEXICAL,
				   "Not a hex number or"
				   "unexpected end of file");
			retval = 0;
		}
	} else {
		/* Its either an int or a float */
		start = this->cursor - 1;
		if( this->scan(SSPF_SCAN_NOT_IN|SSPF_SCAN_SET,
			       "0123456789") ) {
			/*
			 * We back up here because the scan will leave us at
			 * one character past the one that stopped the scan.
			 * Otherwise we could skip something interesting.
			 */
			this->cursor--;
			this->column--;
			if( *this->cursor == '\n' )
				this->row--;
			if( (*this->cursor == '.') ) {
				/* Try and get a float */
				this->cursor++;
				if( sscanf(start, "%f", &st->value.f) == 1 )
					st->kind = SCML_TERM_FLOAT;
				if( !this->scan(SSPF_SCAN_NOT_IN|SSPF_SCAN_SET,
						"0123456789") ) {
					scml_alert(this, SAF_ERROR|SAF_LEXICAL,
						   "Malformed float number");
					retval = 0;
				}
			} else if( sscanf(start, "%d", &st->value.i) == 1 ) {
				/* Get an int */
				st->kind = SCML_TERM_INT;
			} else {
				scml_alert(this, SAF_ERROR|SAF_LEXICAL,
					   "Not a number");
				retval = 0;
			}
		} else {
  			scml_alert(this, SAF_ERROR|SAF_LEXICAL,
				   "Not a number or unexpected end of file");
  			retval = 0;
		}
	}
	return( retval );
}

const char *scml_id_set
	= "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";

struct scml_token scml_stream_pos::get_token()
{
	struct scml_token retval;
	
	if( (this->flags & SSPF_IGNORE_EOF) &&
	    (this->flags & SSPF_EOF) )
		this->state = SSPS_NONE;
	retval.kind = SCML_ERROR;
	switch( this->state ) {
	case SSPS_NONE:
	case SSPS_ERROR:
		retval.kind = SCML_DONE;
		break;
	case SSPS_COL_POS:
		/* Record the column position */
		retval.kind = SCML_COL_POS;
		retval.value.i = this->column;
		this->state = SSPS_ROW_POS;
		break;
	case SSPS_ROW_POS:
		/* Record the row position */
		retval.kind = SCML_ROW_POS;
		retval.value.i = this->row;
		this->state = SSPS_TEXT;
		break;
	case SSPS_ECOL_POS:
		retval.kind = SCML_COL_POS;
		retval.value.i = this->column - 1;
		this->state = SSPS_EROW_POS;
		break;
	case SSPS_EROW_POS:
		retval.kind = SCML_ROW_POS;
		retval.value.i = this->row;
		this->state = SSPS_ESCAPE;
		break;
	case SSPS_CCOL_POS:
		retval.kind = SCML_COL_POS;
		retval.value.i = this->column - 1;
		this->state = SSPS_CROW_POS;
		break;
	case SSPS_CROW_POS:
		retval.kind = SCML_ROW_POS;
		retval.value.i = this->row;
		this->state = SSPS_COMMAND;
		break;
	case SSPS_TEXT: {
		int done = 0;
		
		retval.kind = SCML_TERM_TEXT;
		retval.value.text = this->cursor;
		this->state = SSPS_TEXT;
		while( !done ) {
			/* Scan for a command, escape, or end of the input */
			if( this->scan(SSPF_SCAN_IN|SSPF_SCAN_SET, "<&") ) {
				if( (*(this->cursor - 1) == '&') ) {
					this->state = SSPS_ECOL_POS;
					*(this->cursor - 1) = 0;
					done = 1;
				} else if( !isspace(*this->cursor) ) {
					this->state = SSPS_CCOL_POS;
					*(this->cursor - 1) = 0;
					done = 1;
				}
			} else {
				done = 1;
				if( *retval.value.text == 0 ) {
					retval.kind = SCML_DONE;
				}
			}
		}
		break;
	}
	case SSPS_ESCAPE:
		retval.value.escape = this->cursor;
		if( this->scan(SSPF_SCAN_SEQUENCE, ";") ) {
			retval.kind = SCML_TERM_ESCAPE;
			*(this->cursor - 1) = 0;
			this->state = SSPS_COL_POS;
		} else {
			if( this->flags & SSPF_SINGLE_STATE )
				retval.kind = SCML_DONE;
			else
				scml_alert(this, SAF_ERROR|SAF_LEXICAL,
					   "Escape sequence not terminated");
		}
		break;
	case SSPS_COMMAND: {
		/*
		 * Handle the special commands here and then let
		 * SSPS_EXPR take care of everything else
		 */
		switch( *this->cursor ) {
		case '!':
			retval.value.text = "";
			if( this->scan(SSPF_SCAN_SEQUENCE, "-->") ) {
				retval.kind = SCML_IGNORE;
				this->state = SSPS_COL_POS;
			} else {
				scml_alert(this, SAF_ERROR|SAF_LEXICAL,
					   "Comment not terminated");
			}
			break;
		case '|':
			this->cursor++;
			this->column++;
			retval.value.text = this->cursor;
			if( this->scan(SSPF_SCAN_SEQUENCE, "|>") ) {
				retval.kind = SCML_TERM_VERBATIM;
				*(this->cursor - 2) = 0;
				this->state = SSPS_COL_POS;
			} else {
				scml_alert(this, SAF_ERROR|SAF_LEXICAL,
					   "Verbatim not terminated");
			}
			break;
		case '(':
		case '/':
		default:
			retval.kind = SCML_TERM_LT;
			this->state = SSPS_EXPR;
			break;
		}
		break;
	}
	case SSPS_EXPR: {
		/* Scan through white space until we hit something */
		if( !this->scan(SSPF_SCAN_NOT_IN|SSPF_SCAN_SET, " \n\t") ) {
			if( this->flags & SSPF_SINGLE_STATE ) {
				retval.kind = SCML_DONE;
			} else {
				scml_alert(this, SAF_ERROR|SAF_LEXICAL,
					   "Unexpected end of file in "
					   "expression");
			}
			break;
		}
		this->state = SSPS_EXPR;
		switch( *(this->cursor - 1) ) {
		case '>':
			if( *this->cursor == '>' ) {
				retval.kind = SCML_NT_GT;
				this->cursor++;
				this->column++;
			} else if( *this->cursor == '=' ) {
				retval.kind = SCML_NT_GE;
				this->cursor++;
				this->column++;
			} else {
				this->state = SSPS_COL_POS;
				retval.kind = SCML_TERM_GT;
			}
			break;
		case '<':
			if( *this->cursor == '<' ) {
				retval.kind = SCML_NT_LT;
				this->cursor++;
				this->column++;
			} else if( *this->cursor == '=' ) {
				retval.kind = SCML_NT_LE;
				this->cursor++;
				this->column++;
			} else {
				scml_alert(this, SAF_ERROR|SAF_LEXICAL,
					   "'<' is not a valid symbol in "
					   "a command, use '<<' or '<='");
			}
			break;
		case '+':
			retval.kind = SCML_NT_PLUS;
			break;
		case '-':
			retval.kind = SCML_NT_MINUS;
			break;
		case '/':
			retval.kind = SCML_NT_DIV;
			break;
		case '%':
			retval.kind = SCML_NT_MOD;
			break;
		case '*':
			retval.kind = SCML_NT_MULT;
			break;
		case '?':
			retval.kind = SCML_NT_COND;
			break;
		case ':':
			if( *this->cursor == ':' ) {
				retval.kind = SCML_NT_SCOPE_RES;
				this->cursor++;
				this->column++;
			} else
				retval.kind = SCML_NT_COLON;
			break;
		case ',':
			retval.kind = SCML_NT_COMMA;
			break;
		case '=':
			if( *this->cursor == '=' ) {
				retval.kind = SCML_NT_EQUAL;
				this->cursor++;
				this->column++;
			}
			else
				retval.kind = SCML_NT_ASSIGN;
			break;
		case '!':
			if( *this->cursor == '=' ) {
				retval.kind = SCML_NT_NOT_EQUAL;
				this->cursor++;
				this->column++;
			}
			else
				retval.kind = SCML_NT_NOT;
			break;
		case '&':
			if( *this->cursor == '&' ) {
				retval.kind = SCML_NT_LAND;
				this->cursor++;
				this->column++;
			}
			else
				retval.kind = SCML_NT_AND;
			break;
		case '|':
			if( *this->cursor == '|' ) {
				retval.kind = SCML_NT_LOR;
				this->cursor++;
				this->column++;
			}
			else
				retval.kind = SCML_NT_OR;
			break;
		case '.':
			retval.kind = SCML_NT_DOT;
			break;
		case '(':
			retval.kind = SCML_TERM_LPAREN;
			break;
		case ')':
			retval.kind = SCML_TERM_RPAREN;
			break;
		case '[':
			retval.kind = SCML_TERM_LBRACE;
			break;
		case ']':
			retval.kind = SCML_TERM_RBRACE;
			break;
		case '{':
			retval.kind = SCML_TERM_LCURLY;
			break;
		case '}':
			retval.kind = SCML_TERM_RCURLY;
			break;
		case '\'': {
			int no_error;
			char *start;
			
			/* This is an id with special characters in it */
			start = this->cursor;
			while( (no_error = this->scan(SSPF_SCAN_SEQUENCE,
						      "\'" )) &&
			       (*(this->cursor - 2) == '\\') &&
			       (*(this->cursor - 3) != '\\') );
			if( no_error ) {
				*(this->cursor - 1) = 0;
				retval.kind = SCML_TERM_ID;
				retval.value.id = this->munge_string(start);
			} else {
				scml_alert(this, SAF_ERROR|SAF_LEXICAL,
					   "This is a not a valid id string");
			}
			break;
		}
		case '\"': {
			int no_error;
			char *start;
			
			/* Grab the string */
			start = this->cursor;
			while( (no_error = this->scan(SSPF_SCAN_SEQUENCE,
						      "\"" )) &&
			       (*(this->cursor - 2) == '\\') &&
			       (*(this->cursor - 3) != '\\') );
			if( no_error ) {
				tag_data *td;
				
				*(this->cursor - 1) = 0;
				retval.kind = SCML_TERM_STRING;
				retval.value.str = new scml_string;
				td = retval.value.str->add_component();
				td->kind = TAG_STRING;
				td->tag_data_u.str = this->munge_string(start);
			} else {
				scml_alert(this, SAF_ERROR|SAF_LEXICAL,
					   "This is a not a valid string");
			}
			break;
		}
		default:
			/* Its an ID or a number */
			if( isdigit( *(this->cursor - 1) ) ) {
				this->get_number(&retval);
			} else if( isalpha( *(this->cursor - 1) ) ||
				   (*(this->cursor - 1) == '_') ) {
				char *start = (this->cursor - 1);
				int len;
				
				if( this->scan(SSPF_SCAN_NOT_IN|SSPF_SCAN_SET,
					       scml_id_set) ) {
					/*
					 * We back up here because the scan
					 * will leave us at one character
					 * past the one that stopped the scan.
					 * Otherwise we could skip something
					 * interesting.
					 */
					this->cursor--;
					this->column--;
					if( *this->cursor == '\n' )
						this->row--;
					len = this->cursor - start;
					/* Test for special IDs */
					if( !strncmp("true", start, len) ) {
						retval.kind = SCML_TERM_BOOL;
						retval.value.b = 1;
					} else if( !strncmp("false",
							    start, len) ) {
						retval.kind = SCML_TERM_BOOL;
						retval.value.b = 0;
					} else {
						char *id;
						
						id = (char *)
							mustmalloc(len + 1);
						strncpy(id, start, len);
						id[len] = 0;
						
						retval.kind = SCML_TERM_ID;
						retval.value.id = id;
					}
				} else {
					scml_alert(this, SAF_ERROR|SAF_LEXICAL,
						   "Unexpeceted end of file "
						   "in identifier");
				}
			} else {
				scml_alert(this, SAF_ERROR|SAF_LEXICAL,
					   "Invalid character (0x%x) in input",
					   (int)(*(this->cursor - 1)));
			}
			break;
		}
		break;
	}
	default:
		panic("Unknown state in scml_stream_pos::get_token()");
		break;
	}
	if( retval.kind == SCML_ERROR )
		this->state = SSPS_ERROR;
	return( retval );
}

/* End of file. */

