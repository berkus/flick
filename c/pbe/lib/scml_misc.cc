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
#include <ctype.h>

#include <mom/c/scml.hh>

static const char* scml_std_include_dirs_array[]
	= {
		".",
		SCML_INSTALL_DIR,	/* Set by Makefile. */
		0			/* Terminating null string. */
	};

const char **scml_std_include_dirs()
{
	return scml_std_include_dirs_array;
}

void scml_alert(struct scml_stream_pos *ssp,
		int alert_flags,
		const char *format, ...)
{
	const char *alert_type = 0;
	const char *alert_class = 0;
	va_list args;
	
	va_start( args, format );
	switch( alert_flags & SAF_ALERT_CLASS ) {
	case SAF_INTERNAL:
		alert_class = "Internal";
		break;
	case SAF_GENERAL:
		alert_class = "General";
		break;
	case SAF_IO:
		alert_class = "I/O";
		break;
	case SAF_LEXICAL:
		alert_class = "Lexical";
		break;
	case SAF_PARSE:
		alert_class = "Parse";
		break;
	case SAF_TYPE:
		alert_class = "Type";
		break;
	case SAF_RUNTIME:
		alert_class = "Runtime";
		break;
	default:
		panic("Invalid alert class 0x%x passed to scml_alert()",
		      alert_flags & SAF_ALERT_CLASS);
		break;
	}
	switch( alert_flags & SAF_ALERT_TYPE ) {
	case SAF_WARNING:
		alert_type = "Warning";
		break;
	case SAF_ERROR:
		alert_type = "Error";
		break;
	default:
		panic("Invalid alert type 0x%x passed to scml_alert()",
		      alert_flags & SAF_ALERT_TYPE);
		break;
	}
	if( ssp ) {
		fprintf(stderr, "%s %s %s[",
			alert_class, alert_type,
			ssp->get_stream()->get_desc());
		if( ssp->get_last_row() == -1 ) {
			fprintf(stderr, "%d:%d] - ",
				ssp->get_row(), ssp->get_column());
		} else {
			/*
			 * This will print out a range to make it easier
			 * for the user to find the problem
			 */
			fprintf(stderr, "%d:%d - %d:%d] - ",
				ssp->get_last_row(), ssp->get_last_column(),
				ssp->get_row(), ssp->get_column());
		}
	}
	else {
		fprintf(stderr, "%s %s - ", alert_class, alert_type);
	}
	vfprintf(stderr, format, args);
	fprintf(stderr, "\n");
	va_end( args );
}

int scml_hash_name(const char *name, int table_size)
{
	int h;
	
	for( h = 0; *name; name++ )
		h = (64 * h + tolower(*name)) % table_size;
	return( h );
}

scml_string::scml_string()
{
	this->components.val = 0;
	this->components.len = 0;
}

scml_string::~scml_string()
{
	free( this->components.val );
}

char *scml_string::tag_ref()
{
	return( ptr_to_tag_ref("struct scml_string *", this) );
}

struct scml_string *scml_string::ptr(char *ref)
{
	return( (struct scml_string *)
		tag_ref_to_ptr("struct scml_string *", ref) );
}

char *scml_string::tag_string(tag_item *ti)
{
	char *retval = 0;
	
	switch( ti->data.kind ) {
	case TAG_STRING:
		retval = ti->data.tag_data_u.str;
		break;
	case TAG_REF:
		retval = scml_string::ptr(ti->data.tag_data_u.ref)->
			make_chars();
		break;
	default:
		break;
	}
	return( retval );
}

tag_data *scml_string::add_component()
{
	tag_data *retval;
	int i;
	
	i = this->components.len++;
	this->components.val = (tag_data *)mustrealloc(this->components.val,
						       sizeof( tag_data ) *
						       this->components.len);
	this->components.val[i].kind = TAG_NONE;
	retval = &this->components.val[i];
	return( retval );
}

void scml_string::concat(struct scml_string *ss)
{
	int start, lpc;
	
	start = this->components.len;
	this->components.len += ss->components.len;
	this->components.val = (tag_data *)mustrealloc(this->components.val,
						       sizeof( tag_data ) *
						       this->components.len);
	for( lpc = 0; lpc < ss->components.len; lpc++ ) {
		this->components.val[start + lpc] = ss->components.val[lpc];
	}
}

int scml_string::cmp(struct scml_string *ss)
{
	int my_pos = 0, ss_pos = 0, done = 0;
	const char *my_str;
	const char *ss_str;
	int retval = 0;
	
	my_str = "";
	ss_str = "";
	while( !done ) {
		/* Find the next non-empty string in the scml_strings */
		while( !(*my_str) && (my_pos < this->components.len) &&
		       (this->components.val[my_pos].kind == TAG_STRING) ) {
			my_str = this->components.val[my_pos].tag_data_u.str;
			my_pos++;
		}
		while( !(*ss_str) && (ss_pos < ss->components.len) &&
		       (ss->components.val[ss_pos].kind == TAG_STRING) ) {
			ss_str = ss->components.val[ss_pos].tag_data_u.str;
			ss_pos++;
		}
		/* Compare the character strings */
		while( *my_str && *ss_str && (*my_str == *ss_str) ) {
			my_str++;
			ss_str++;
		}
		if( *my_str < *ss_str )
			retval = -1;
		else if( *my_str > *ss_str )
			retval = 1;
		/*
		 * Only quit if the characters mismatch or there is nothing
		 * left in the strings
		 */
		if( retval || (!*my_str && !*ss_str) ) {
			done = 1;
		}
	}
	return( retval );
}

char *scml_string::make_chars()
{
	char *retval = 0, *curr;
	int lpc, poss = 1, str_size = 0;
	
	/* Get the size of the string to be produced */
	for( lpc = 0; poss && (lpc < this->components.len); lpc++ ) {
		switch( this->components.val[lpc].kind ) {
		case TAG_BOOL:
			str_size += this->components.val[lpc].
				tag_data_u.b ? 4 : 5;
			break;
		case TAG_STRING:
			str_size += strlen(this->components.val[lpc].
					    tag_data_u.str);
			break;
		case TAG_INTEGER: {
			int i;
			
			i = this->components.val[lpc].tag_data_u.i;
			str_size++;
			while( i ) {
				i /= 10;
				str_size++;
			}
			break;
		}
		case TAG_FLOAT: {
			char f[30];
			
			sprintf(f, "%f", this->components.val[lpc].
				tag_data_u.f);
			str_size += strlen(f);
			break;
		}
		default:
			poss = 0;
			break;
		}
	}
	if( poss ) {
		/* Construct the string */
		retval = (char *)mustmalloc(str_size + 1);
		curr = retval;
		for( lpc = 0; lpc < this->components.len; lpc++ ) {
			switch( this->components.val[lpc].kind ) {
			case TAG_BOOL:
				strcpy(curr,
				       this->components.val[lpc].
				       tag_data_u.b ? "true" : "false");
				break;
			case TAG_STRING:
				strcpy(curr,
				       this->components.val[lpc].
				       tag_data_u.str);
				break;
			case TAG_INTEGER:
				sprintf(curr, "%d", this->components.
					val[lpc].tag_data_u.i);
				break;
			case TAG_FLOAT:
				sprintf(curr, "%f", this->components.
					val[lpc].tag_data_u.f);
				break;
			default:
				break;
			}
			curr += strlen(curr);
		}
	}
	return( retval );
}

void scml_string::print(int indent)
{
	union tag_data_u data;
	int lpc;
	
	/* Simply walk through the components and use the tag printer */
	for( lpc = 0; lpc < this->components.len; lpc++ ) {
		data = get_tag_data(&this->components.val[lpc], 0);
		print_tag_data(indent, this->components.val[lpc].kind, data);
		indent = 0;
	}
}

int scml_parse_cmdline_defines(struct scml_scope *root_scope,
			       flag_value_seq *defs)
{
	const char *str, *var_value = 0;
	struct scml_token_sequence *sts;
	struct scml_stream_pos *ssp;
	struct scml_stream *ss;
	struct scml_parser *sp;
	unsigned int lpc;
	const char *arg;
	int retval = 0;
	char *var_name;
	int name_len;
	tag_data td;
	
	for( lpc = 0; lpc < defs->len; lpc++ ) {
		arg = defs->values[lpc].string;
		td = create_tag_data(TAG_BOOL, 0);
		td.tag_data_u.b = 1;
		for( str = arg; *str && (*str != '='); str++ );
		if( *str == '=' ) {
			name_len = str - arg;
			var_name = (char *)mustmalloc(name_len + 1);
			strncpy(var_name, arg, name_len);
			var_name[name_len] = 0;
			
			var_value = str + 1;
			ss = new scml_stream;
			ss->set_data(muststrdup(var_value));
			ss->set_desc(flick_asprintf(
				"Command line define `%s'=`%s'",
				var_name, var_value));
			ssp = new scml_stream_pos;
			ssp->set_stream(ss);
			ssp->set_flags(ssp->get_flags() |
				       SSPF_IGNORE_EOF |
				       SSPF_SINGLE_STATE);
			ssp->set_state(SSPS_EXPR);
			sp = new scml_parser;
			sp->set_stream_pos(ssp);
			if( (sts = sp->parse()) &&
			    (sts->get_length() == 1) ) {
				struct scml_context *sc;
				struct scml_token st;
				
				sc = new scml_context;
				sc->set_stream_pos(ssp);
				sc->set_scope(root_scope);
				st = sc->eval_token(sts->get_value());
				switch( st.kind ) {
				case SCML_TERM_BOOL:
					td.kind = TAG_BOOL;
					td.tag_data_u.b = st.value.b;
					retval = 1;
					break;
				case SCML_TERM_INT:
					td.kind = TAG_INTEGER;
					td.tag_data_u.i = st.value.i;
					retval = 1;
					break;
				case SCML_TERM_FLOAT:
					td.kind = TAG_FLOAT;
					td.tag_data_u.f = st.value.f;
					retval = 1;
					break;
				case SCML_TERM_STRING:
					td.kind = TAG_REF;
					td.tag_data_u.ref = st.value.str->
						tag_ref();
					retval = 1;
					break;
				case SCML_TERM_TAG:
					td = st.value.ti->data;
					retval = 1;
					break;
				case SCML_TERM_TAG_LIST:
					td.kind = TAG_TAG_LIST;
					td.tag_data_u.tl = st.value.tl;
					retval = 1;
					break;
				default:
					break;
				}
				delete sc;
			}
			delete sp;
			delete ssp;
			delete ss;
		} else {
			var_name = muststrdup(arg);
			name_len = str - arg;
			retval = 1;
		}
		add_tag(root_scope->get_values(), var_name, TAG_NONE)->
			data = td;
	}
	return( retval );
}

int scml_execute_str(struct scml_scope *root_scope,
		     char *code_desc, char *code, tag_list *tl)
{
	struct scml_token_sequence *sts;
	struct scml_stream_pos *ssp;
	struct scml_parser *sp;
	struct scml_stream *ss;
	int retval = 0;
	
	ss = new scml_stream;
	ss->set_desc(code_desc);
	ss->set_data(muststrdup(code));
	ssp = new scml_stream_pos;
	ssp->set_stream(ss);
	sp = new scml_parser();
	sp->set_stream_pos(ssp);
	if( (sts = sp->parse()) ) {
		struct scml_context *sc;
		
		sc = new scml_context;
		sc->set_stream_pos(ssp);
		sc->set_scope(root_scope);
		sc->get_lvalues()->parent = tl;
		sc->format_sequence(sts);
		delete sc;
		retval = 1;
	}
	delete sp;
	delete ssp;
	delete ss;
	return( retval );
}

int scml_code_cast_handler(int indent, cast_handler *ch)
{
	struct scml_token_sequence *sts;
	struct scml_scope *root_scope;
	struct scml_stream_pos *ssp;
	struct scml_stream *ss;
	struct scml_parser *sp;
	int retval = 0;
	tag_list *tl;
	
	ss = new scml_stream;
	ss->set_desc(ch->args.args_val[0]);
	ss->set_data(muststrdup(ch->args.args_val[1]));
	root_scope = (struct scml_scope *)tag_ref_to_ptr("struct scml_scope *",
							 ch->args.args_val[2]);
	tl = (tag_list *)tag_ref_to_ptr("tag_list *", ch->args.args_val[3]);
	ssp = new scml_stream_pos;
	ssp->set_stream(ss);
	sp = new scml_parser();
	sp->set_stream_pos(ssp);
	if( (sts = sp->parse()) ) {
		struct scml_context *sc;
		
		sc = new scml_context;
		sc->set_stream_pos(ssp);
		sc->set_scope(root_scope);
		sc->set_indent_size(indent * 2);
		sc->get_lvalues()->parent = tl;
		sc->format_sequence(sts);
		delete sc;
		retval = 1;
	}
	delete sp;
	delete ssp;
	delete ss;
	return( retval );
}

struct scml_stream_pos *scml_execute_defs_file(struct scml_scope *root_scope,
					       const char **include_dirs,
					       const char *file_desc,
					       const char *file_name)
{
	struct scml_stream_pos *retval = 0;
	char *found_filename;
	FILE *file;
	
	file = fopen_search(file_name,
			    include_dirs,
			    "r",
			    &found_filename);
	if( file ) {
		struct scml_token_sequence *sts;
		struct scml_stream_pos *ssp;
		struct scml_stream *ss;
		struct scml_parser *sp;
		
		ss = new scml_stream;
		ss->set_desc(file_desc);
		ss->set_file(file);
		ss->set_include_directory_list(include_dirs);
		ssp = new scml_stream_pos;
		ssp->set_stream(ss);
		sp = new scml_parser;
		sp->set_stream_pos(ssp);
		if( (sts = sp->parse()) ) {
			struct scml_context *sc;
			
			sc = new scml_context;
			sc->set_flags(sc->get_flags() |
				      SCF_IGNORE_WHITE_SPACE);
			sc->set_stream_pos(ssp);
			sc->set_scope(root_scope);
			if( sc->format_sequence(sts) ) {
				retval = ssp;
			} else {
				panic("There was an error in executing the"
				      " file `%s'", file_desc);
			}
			delete sc;
		} else {
			panic("Couldn't parse the input file `%s'", file_desc);
		}
		fclose(file);
		delete sp;
	}
	return( retval );
}
