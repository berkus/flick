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

#ifndef SCML_HH
#define SCML_HH

/*
 * SCML - Source Code Markup Language
 *
 * SCML, is a formatting language for generating generic source code based
 * on values produced by Flick's presentation generator.  For example,
 * the CORBA C++ mapping requires a great deal of C++ infrastructure to be
 * produced from its IDL.  The majority of this infrastructure is
 * straightforward and doesn't require anything more than simple formatting
 * of data to produce the proper C++ code.  Since the formatting of this
 * data doesn't require any real complex operations to be performed a
 * different method than printf's could be make the process clearer and
 * easier to understand.  Therefore, a simple formatting language was
 * produced to fill this need and it worked well for a time.  However,
 * the resulting language code become unwieldy and the language wasn't
 * able to satisfy the needs of some possible uses.  Since this simple
 * language didn't work out, a more sophisticated language was developed.
 * The result is a language with the syntax of SGML, called SCML.  SCML
 * is capable of not only formatting data gotten from Flick, it can also
 * evaluate simple expression's, allowing it to modify parts of Flick's state.
 * The change of syntax and the extra abilities that SCML has seem to
 * make it a good choice for a simple/flexible formatting language.
 */

extern "C" {
#include <mom/compiler.h>
#include <mom/c/libpres_c.h>
}

enum {
	SSB_INPUT,
	SSB_OUTPUT
};

enum {
	SSF_INPUT = (1L << SSB_INPUT),
	SSF_OUTPUT = (1L << SSB_OUTPUT)
};

/* scml_stream manages the data stream that contains SCML code */
class scml_stream {

public:
	scml_stream();
	~scml_stream();
	
	/* Convert an scml_stream to/from a TAG_REF */
	char *tag_ref();
	static struct scml_stream *ptr(char *ref);
	
	/* Set/get the flags */
	void set_flags(unsigned int the_flags);
	unsigned int get_flags();
	
	/* Set/get a string as the stream */
	void set_data(char *str);
	char *get_data();
	
	/* Set/get a description of the stream (e.g. file name) */
	void set_desc(const char *desc);
	const char *get_desc();
	
	/* Set/get the list of directories in which to find include files. */
	void set_include_directory_list(const char * const *dir_list);
	const char * const *get_include_directory_list();
	
	/* Get the length of the stream */
	int get_length();
	
	/* Set a file as the stream, this currently just reads
	   the whole file into memory for processing. */
	void set_file(FILE *the_file);
	FILE *get_file();
	
private:
	unsigned int flags;
	const char * const *include_directory_list;
	const char *desc;
	FILE *file;
	char *data;
	int len;
};

/* scml_string is a bit of a hack to get around problems with CAST.
   Since the tags can hold CAST structures and SCML doesn't want
   to know about CAST we have to consider CAST stuff as "computed strings."
   This means that we have to wrap all of our strings, real or CAST,
   into a single structure so that the SCML guts can deal with it easily. */
class scml_string {

public:
	scml_string();
	~scml_string();
	
	/* Encode/decode this string as a TAG_REF string. */
	char *tag_ref();
	static struct scml_string *ptr(char *ref);
	
	/* Get a C string from a tag, if the tag is a TAG_STRING then
	   it just returns that, otherwise its tries to resolve a TAG_REF
	   to an scml_string and does a make_chars on that */
	static char *tag_string(tag_item *ti);
	
	/* Add a component to this string, its just a
	   tag_data for simplicity */
	tag_data *add_component();
	/* Concat another string onto this one */
	void concat(struct scml_string *ss);
	/* Compare two strings, same as strcmp() */
	int cmp(struct scml_string *ss);
	/* Tries to produce a C string from all of the components,
	   if any of them are CAST it won't work */
	char *make_chars();
	/* Print the string using w_printf */
	void print(int indent);
	
private:
	struct {
		tag_data *val;
		int len;
	} components;
	
};

/* These are all the tokens used in SCML.  SCML_TERM_* means its a terminal
   with or without a value.  SCML_NT_* means its a non-terminal */
enum {
	SCML_NONE,
	SCML_IGNORE,
	SCML_ERROR,
	SCML_DONE,
	SCML_COL_POS,        // i holds the column position in the stream
	SCML_ROW_POS,        // i holds the row position in the stream
	
	SCML_TERM,
	
	SCML_TERM_BOOL,      // b holds a boolean value
	SCML_TERM_INT,       // i holds an integer value
	SCML_TERM_FLOAT,     // f holds a float value
	SCML_TERM_STRING,    // str holds an scml_string
	SCML_TERM_TAG,       // ti holds a tag_item
	SCML_TERM_TAG_LIST,  // tl holds a tag_list
	
	SCML_TERM_TEXT,      // text holds a string of text
	SCML_TERM_ESCAPE,    // escape holds the name of an escape sequence
	SCML_TERM_ID,        // id holds an identifier string
	SCML_TERM_VERBATIM,  // text holds a string of text
	
	SCML_TERM_LPAREN,
	SCML_TERM_RPAREN,
	SCML_TERM_LBRACE,
	SCML_TERM_RBRACE,
	SCML_TERM_LCURLY,
	SCML_TERM_RCURLY,
	SCML_TERM_SLASH,
	SCML_TERM_LT,
	SCML_TERM_GT,
	
	SCML_TERM_MAX,
	SCML_NT,
	
	SCML_NT_COMMAND,    // children is an SCML_NONE terminated
	                    //  list of tokens in a command
	SCML_NT_SET,        // children[0] holds the lvalue
	                    // children[1] holds the rvalue
	
	SCML_NT_ASSIGN,
	SCML_NT_PLUS,       // children[0] holds the left expr
	                    // children[1] holds the right expr
	SCML_NT_MINUS,      // same as SCML_NT_PLUS
	SCML_NT_DIV,        // same as SCML_NT_PLUS
	SCML_NT_MOD,        // same as SCML_NT_PLUS
	SCML_NT_MULT,       // same as SCML_NT_PLUS
	SCML_NT_COND,
	SCML_NT_COLON,      // same as SCML_NT_PLUS
	SCML_NT_COMMA,
	SCML_NT_EQUAL,      // same as SCML_NT_PLUS
	SCML_NT_NOT_EQUAL,  // same as SCML_NT_PLUS
	SCML_NT_LT,         // same as SCML_NT_PLUS
	SCML_NT_GT,         // same as SCML_NT_PLUS
	SCML_NT_LE,         // same as SCML_NT_PLUS
	SCML_NT_GE,         // same as SCML_NT_PLUS
	SCML_NT_AND,        // same as SCML_NT_PLUS
	SCML_NT_LAND,       // same as SCML_NT_PLUS
	SCML_NT_OR,         // same as SCML_NT_PLUS
	SCML_NT_LOR,        // same as SCML_NT_PLUS
	SCML_NT_XOR,        // same as SCML_NT_PLUS
	SCML_NT_NOT,        // children[0] holds the expr
	SCML_NT_DOT,        // same as SCML_NT_PLUS
	SCML_NT_SCOPE_RES,  // same as SCML_NT_PLUS
	SCML_NT_NAME,       // children[0] holds a scope_res
	SCML_NT_EXPR,       // children[0] holds another expr
	SCML_NT_TAG,        // chidlren[0] holds the tag name
	                    // children[1] holds the tag type
	SCML_NT_SEL,        // children[0] holds the tag
	                    // children[1] holds the index into the tag
	SCML_NT_INIT,       // children is an SCML_NONE terminated
	                    //  list of tokens
	
	SCML_NT_MAX
};

/* Tokens are the basic structure used in the processing of SCML. */
class scml_token {

public:
	scml_token() { this->value.children = 0; }
	
	/* strip/promote might change the kind of a token to make
	   processing easier.  They work only on terminals and
	   are dependent on the type.  Basically you can promote
	   from a simpler type to a more complex type, and you
	   can only strip a tag since its a "container" type.  The
	   types are ordered from simplest to more complex:

	   SCML_TERM_BOOL
	   SCML_TERM_INT
	   SCML_TERM_FLOAT
	   SCML_TERM_STRING
	   
	   SCML_TERM_TAG       - a "container" type
	   
	   SCML_TERM_TAG_LIST  - a "structured container" type
	                         since this needs names for its
				 members and is basically different
				 from the other types, you can't
				 promote to it.
	   
	   The idea is that all of the complex types can hold
	   the simpler types, but the reverse isn't true and
	   some special function is required to select what
	   information to throw away when going to a simpler type.
	*/
	void strip();
	void promote(int required_kind);
	
	/* Simple tests on the what kind of token this is */
	int is_expr();
	int is_value();
	int is_operator();
	int is_container();
	
	/* Get the precedence value for a binary token */
	int get_prec();
	
	/* Get a string representation of this id token */
	char *get_identifier();
	
	/* Debug printf */
	void print();
	
	int kind;
	union {
		int b;
		int i;
		float f;
		const char *id;
		struct scml_string *str;
		const char *text;
		const char *escape;
		tag_item *ti;
		tag_list *tl;
		struct scml_token *children;
		struct scml_token *token;
	} value;
};

/* scml_token_sequence points to a sequence of tokens */
class scml_token_sequence {

public:
	scml_token_sequence();
	~scml_token_sequence();
	
	/* Encode/Decode this object as a TAG_REF, this allows SCML
	   code to be passed around by the user and runtime. */
	char *tag_ref();
	static struct scml_token_sequence *ptr(char *ref);
	
	/* Debug printf */
	void print();
	
	/* Set the start of the token sequence, it
	   doesn't not assume ownership of it */
	void set_value(struct scml_token *st);
	struct scml_token *get_value();
	
	/* Set/get the number of tokens in the value */
	void set_length(int len);
	int get_length();
	
	/* Set/get the stream this sequence of tokens was created from */
	void set_stream_pos(struct scml_stream_pos *ssp);
	struct scml_stream_pos *get_stream_pos();
	
	/* Set/get the column based indent of the sequence.  This is
	   here so that users can structure their SCML code with indents
	   and not have the indents end up in the generated text */
	void set_indent(int offset);
	int get_indent();
	
private:
	int indent;
	struct scml_stream_pos *stream_pos;
	struct scml_token *val;
	int len;
	
};

/* scml_token_stack is a simple stack used in when parsing tokens */
class scml_token_stack {

public:
	scml_token_stack();
	~scml_token_stack();
	
	/* Push/pop a token */
	void push(struct scml_token &st);
	void pop();
	
	/* This will return the token at the given
	   offset from the top of the stack */
	struct scml_token *index(int offset);
	
	/* Number of tokens on the stack */
	int count();
	
	/* Debug printf */
	void print();
	
private:
	struct scml_token *val;
	int top;
	int max;
	
	static int default_max;
	static int default_inc;
};

/*
 * SSPS - Scml_Stream_Pos State
 *
 * These are the states used while lexxing the stream.  They are needed
 * since lexxing SCML depends on whether we're in a regular text segment,
 * an escape sequence, or a command sequence.
 */
enum {
	SSPS_NONE,
	SSPS_ERROR,     // Error state
	SSPS_COL_POS,   // returns the current column position in the stream
	SSPS_ROW_POS,   // returns the current row position in the stream
	SSPS_TEXT,      // handles text segments
	SSPS_ECOL_POS,
	SSPS_EROW_POS,
	SSPS_ESCAPE,    // handles escapes
	SSPS_CCOL_POS,
	SSPS_CROW_POS,
	SSPS_COMMAND,   // handles command segments
	SSPS_EXPR       // handles expressions
};

enum {
	SSPB_SINGLE_STATE,
	SSPB_IGNORE_EOF,
	SSPB_EOF
};

enum {
	SSPF_SINGLE_STATE = (1L << SSPB_SINGLE_STATE),
	SSPF_IGNORE_EOF = (1L << SSPB_IGNORE_EOF),
	SSPF_EOF = (1L << SSPB_EOF)
};

/*
 * SSPF_SCAN - Scml_Stream_Pos Flags for the scan function
 *
 * SSPF_SCAN_SEQUENCE - Scan for a sequence of characters
 * SSPF_SCAN_SET - Scan for a set of characters
 *   SSPF_SCAN_IN - Used with set to specify if the scan should
 *                  stop when it hits a char in the set
 *   SSPF_SCAN_NOT_IN - the opposite of SSPF_SCAN_IN
 */
enum {
	SSPB_SCAN_SEQUENCE,
	SSPB_SCAN_SET,
	SSPB_SCAN_IN,
	SSPB_SCAN_NOT_IN
};

enum {
	SSPF_SCAN_SEQUENCE = (1L << SSPB_SCAN_SEQUENCE),
	SSPF_SCAN_SET = (1L << SSPB_SCAN_SET),
	SSPF_SCAN_IN = (1L << SSPB_SCAN_IN),
	SSPF_SCAN_NOT_IN = (1L << SSPB_SCAN_NOT_IN)
};

/* scml_stream_pos is the lexxer, it takes a stream
   and produces tokens from it */
class scml_stream_pos {

public:
	scml_stream_pos();
	~scml_stream_pos();
	
	/* Set/get the stream */
	void set_stream(struct scml_stream *ss);
	struct scml_stream *get_stream();
	
	/* Set/get the flags */
	void set_flags(unsigned int the_flags);
	unsigned int get_flags();
	
	/* Set/get the state */
	void set_state(int state);
	int get_state();
	
	/* Set/get the current position in the stream */
	void set_cursor(char *cursor);
	char *get_cursor();
	
	/* Set/get the current row number in the stream */
	void set_row(int row);
	int get_row();
	
	/* Set/get the current column number in the stream */
	void set_column(int column);
	int get_column();
	
	/* Get the position of where the last scan started */
	int get_last_row();
	int get_last_column();
	
	/* Handle any backslash escapes in a string */
	char *munge_string(char *str);
	
	/* Go through the current position in the stream and
	   set the token to any number it finds */
	int get_number(struct scml_token *st);
	
	/* Scan through the stream, it will stop when the given
	   test values become true, leaving the cursor at next
	   character after the trigger */
	int scan(int kind, const char *str);
	
	/* This is the main function, it will walk through the
	   stream and return any functions it finds. */
	struct scml_token get_token();
	
private:
	struct scml_stream *stream;
	unsigned int flags;
	int state;
	char *cursor;
	int row;
	int column;
	int last_row;
	int last_column;
	
};

/* scml_parser will parse an input stream which results in a token sequence */
class scml_parser {

public:
	scml_parser();
	~scml_parser();
	
	/* Set/get the stream position to use in parsing */
	void set_stream_pos(struct scml_stream_pos *ssp);
	struct scml_stream_pos *get_stream_pos();
	
	/* This will get all the tokens from the stream pos and
	   will form them into a token sequence that it returns */
	struct scml_token_sequence *parse();
	
private:
	struct scml_token collapse(struct scml_token_stack *values,
				   struct scml_token *oper);
	
	struct scml_stream_pos *ssp;
	
};

enum {
	SHK_C_FUNCTION
};

class scml_context;

/* A simple structure to hold a pointer to any
   handler functions in the system */
struct scml_handler {
	struct scml_handler *next;
	const char *name;
	int kind;
	union {
		int (*c_func)(struct scml_token *st, struct scml_context *sc);
	} function;
};

/* A hash table for holding all of the C based handlers in the system */
class scml_handler_table {

public:
	scml_handler_table();
	~scml_handler_table();
	
	/* Add/find a handler */
	void add_handler(struct scml_handler *sh);
	void rem_handler(const char *name);
	struct scml_handler *find_handler(const char *name);

private:
	enum { SCML_HANDLER_TABLE_SIZE = 31 };
	
	struct scml_handler *table[SCML_HANDLER_TABLE_SIZE];
	
};

/*
 * SCDF - Scml_Cmd_Definition Flags
 *
 * SCDF_BRACKETED - The command is bracketed (e.g. <b>Hello</b>)
 */
enum {
	SCDB_BRACKETED
};

enum {
	SCDF_BRACKETED = (1L << SCDB_BRACKETED)
};

/* scml_cmd_definition is used to track all the commands in a scope */
class scml_cmd_definition {

public:
	scml_cmd_definition();
	~scml_cmd_definition();
	
	/* Encode/decode a TAG_REF string */
	char *tag_ref();
	static struct scml_cmd_definition *ptr(char *ref);
	
	/* Set/get the name of the command */
	void set_name(const char *name);
	const char *get_name();
	
	/* Set/get the optional parameters */
	void set_opt_params(tag_list *tl);
	tag_list *get_opt_params();
	
	/* Set/get the required parameters */
	void set_req_params(tag_list *tl);
	tag_list *get_req_params();
	
	/* Set/get the flags */
	void set_flags(int flags);
	int get_flags();
	
	/* Set/get the handler function */
	void set_handler(struct scml_handler *sh);
	struct scml_handler *get_handler();
	
	/* Set/get a token sequence attached to this command */
	void set_token_sequence(struct scml_token_sequence *sts);
	struct scml_token_sequence *get_token_sequence();
	
	/* Execute this command with the given command
	   structure and in the given context */
	int execute(struct scml_token *st, struct scml_context *sc);
	
	struct scml_cmd_definition *next;
	
private:
	const char *name;
	tag_list *opt_params;
	tag_list *req_params;
	int flags;
	struct scml_handler *sh;
	struct scml_token_sequence *sts;
	
};

/* scml_escape is a simple structure pairing an escape
   sequence name with its textual value */
struct scml_escape {
	struct scml_escape *next;
	char *name;
	char *value;
};

/* scml_escape_table tracks all the escapes defined for a scope */
class scml_escape_table {

public:
	scml_escape_table();
	~scml_escape_table();
	
	/* Add/find an escape sequence in the table */
	void add_escape(struct scml_escape *se);
	void rem_escape(const char *name);
	struct scml_escape *find_escape(const char *name);
	
private:
	enum { SCML_ESCAPE_TABLE_SIZE = 31 };
	
	struct scml_escape *table[SCML_ESCAPE_TABLE_SIZE];
	
};

/* scml_scope corresponds to a lexical scope in the system */
class scml_scope {

public:
	scml_scope();
	~scml_scope();
	
	/* Get our parent scope */
	struct scml_scope *get_parent();
	
	/* Set/get the name of the scope */
	void set_name(const char *name);
	const char *get_name();
	
	/* Set/get the escape table for this scope */
	void set_escape_table(struct scml_escape_table *setable);
	struct scml_escape_table *get_escape_table();
	
	/* Add/find a child scope */
	void add_child(struct scml_scope *ss);
	struct scml_scope *find_child(const char *name);
	
	/* Add a command definition to the scope */
	void add_cmd_definition(struct scml_cmd_definition *scd);
	void rem_cmd_definition(struct scml_cmd_definition *scd);
	
	/* Find a command definition, the scope it was found
	   in is returned in scope */
	struct scml_cmd_definition *
	find_cmd_definition(struct scml_scope **scope, const char *name);
	
	/* Get the tag_list corresponding to static variables in this scope */
	tag_list *get_values();
	
	/* Debug printf */
	void print();
	
	/* Construct a root scope.  Basically just makes it
	   and adds the command definition for 'define' */
	static struct scml_scope *make_root_scope();
	
private:
	enum { SCML_SCOPE_TABLE_SIZE = 15 };
	
	struct scml_scope *next;
	struct scml_scope *parent;
	const char *name;
	tag_list *values;
	struct scml_escape_table *setable;
	struct scml_scope *sc_table[SCML_SCOPE_TABLE_SIZE];
	
};

/*
 * SCF - Scml_Context Flags
 *
 * SCF_PREFORMATTED - Any text to be output is already formatted
 * SCF_IGNORE_INDENT - Ignore any indentation within the set indent_size
 * SCF_IGNORE_WHITE_SPACE - Completely ignore any white space in the text
 * SCF_DEFERRED_WHITE_SPACE - Some white space has been encountered but
 *                            there's no non-space text to put it before
 * SCF_PRINTING_BODY - This indicates whether we've started printing
 *                     out anything in the sequence.
 */
enum {
	SCB_PREFORMATTED,
	SCB_IGNORE_OFFSET,
	SCB_IGNORE_INDENT,
	SCB_IGNORE_WHITE_SPACE,
	SCB_PRINTING_BODY
};

enum {
	SCF_PREFORMATTED = (1L << SCB_PREFORMATTED),
	SCF_IGNORE_OFFSET = (1L << SCB_IGNORE_OFFSET),
	SCF_IGNORE_INDENT = (1L << SCB_IGNORE_INDENT),
	SCF_IGNORE_WHITE_SPACE = (1L << SCB_IGNORE_WHITE_SPACE),
	SCF_PRINTING_BODY = (1L << SCB_PRINTING_BODY)
};

/* scml_context is the execution context, it basically
   takes a sequence and executes whatever is in it.
   XXX - This should be separated into a printing class
   and a simple execution class. */
class scml_context {

public:
	scml_context();
	~scml_context();
	
	/* Set/get the flags */
	void set_flags(int flags);
	int get_flags();
	
	/* Set/get the parent context */
	void set_parent(struct scml_context *parent);
	struct scml_context *get_parent();
	
	/* Set/get the stream pos for the token sequence */
	void set_stream_pos(struct scml_stream_pos *ss);
	struct scml_stream_pos *get_stream_pos();
	
	/* Set/get the command definition for what the context is executing */
	void set_cmd_def(struct scml_cmd_definition *def);
	struct scml_cmd_definition *get_cmd_def();
	
	/* Set/get the tag_list where the context can find lvalues */
	void set_lvalues(tag_list *tl);
	tag_list *get_lvalues();
	
	/* Set/get the tag_list where the context can find rvalues */
	void set_rvalues(tag_list *tl);
	tag_list *get_rvalues();
	
	/* Set/get the scope where the code is being executed */
	void set_scope(struct scml_scope *sc);
	struct scml_scope *get_scope();
	
	/* Set/get the indent size */
	void set_indent_size(int size);
	int get_indent_size();
	
	/* set/get the offset size */
	void set_offset_size(int size);
	int get_offset_size();
	
	void set_ws_queue(int size);
	int get_ws_queue();
	
	/* Print a token */
	int print_token(struct scml_token *st);
	void print_indent(int size);
	
	/* Given a token this will try to locate the
	   scope starting from out_scope */
	void locate_scope(struct scml_token *st,
			  struct scml_scope **inout_scope,
			  const char **out_id);
	/* Convert an array tokens to a single type */
	int equalize_tokens(struct scml_token *dest, struct scml_token *src,
			    int count);
	/* Evaluate a token and return the result */
	struct scml_token eval_token(struct scml_token *st);
	/* Given a token this will try to return the tag and any
	   index into the tag to be used for assignment */
	void token_lvalue(struct scml_token *st, tag_item **out_ti,
			  int *out_index);
	/* Given an SCML_NT_COMMAND token this will try to resolve
	   all sets of parameters and return true if successful */
	int handle_params(struct scml_token *cmd);
	/* Given a token this will figure out the type of a new tag */
	void tag_type(struct scml_token *t_type, tag_data_kind *out_kind,
		      int *out_size);
	/* Given a token this will find the corresponding command
	   definition and what scope its in (cmd_scope) */
	struct scml_cmd_definition *find_cmd(struct scml_scope **cmd_scope,
					     struct scml_token *st);
	/* This is the definition for the root command
	   'define', from which all other things come */
	int define(struct scml_context *sc);
	int undefine(struct scml_context *sc);
	int rename(struct scml_context *sc);
	/* ifdef is an SCML command which tests if something has or hasn't
	   been defined and executes its body depending on that */
	int ifdef(struct scml_context *sc, struct scml_token_sequence *sub,
		  int defined);
	/* Walks through a token sequence trying to find the terminating
	   token for the command and then returns the sub-sequence of
	   tokens between them, the "contents" */
	struct scml_token_sequence *get_contents(
		struct scml_cmd_definition *def,
		struct scml_token_sequence *sts,
		int start);
	struct scml_token_sequence *partition_sequence(
		struct scml_cmd_definition *def,
		struct scml_cmd_definition *partition,
		struct scml_token_sequence *sts);
	/* Execute the token returned by eval_token */
	int exec_token(struct scml_token_sequence *sts, int *index,
		       struct scml_token *st);
	/* Execute the given token sequence */
	int format_sequence(struct scml_token_sequence *sts);
	/* exec_cmd is a hook for internal code to execute the given command
	   with the given locals and a list of arguments for the command.
	   The arguments are passed as a string which is the name of the
	   parameter, a type tag and a tag value. */
	int exec_cmd(const char *cmd_name, tag_list *locals, ...);
	
	/* Set/get the handler table that 'define' uses resolve command
	   handler names to real C functions */
	static void set_handler_table(struct scml_handler_table *sht);
	static struct scml_handler_table *get_handler_table();
	
	void print(int level);
	
private:
	int flags;
	struct scml_context *parent;
	struct scml_stream_pos *stream_pos;
	struct scml_scope *scope;
	struct scml_cmd_definition *def;
	tag_list *lvalues;
	tag_list *rvalues;
	int indent_size;
	int offset_size;
	int ws_queue;
	
	static struct scml_handler_table *sht;
};

/*
 * SAF - Scml_Alert Flags
 *
 * SAF_WARNING - This is a warning
 * SAF_ERROR - This is an error
 *
 * SAF_INTERNAL - The problem is internal
 * SAF_GENERAL - The problem can't be classified
 * SAF_IO - The problem is with I/O
 * SAF_LEXICAL - The problem is with lexxing
 * SAF_PARSE - The problem is with parsing
 * SAF_TYPE - The problem is with bad/inconsistent types
 * SAF_RUNTIME - The problem is with the execution environment
 *
 * SAF_ALERT_TYPE and SAF_ALERT_CLASS are masks for what is an
 * an alert type and an alert class.  Basically, an alert type
 * is or'd with class to produce a description of the error.
 */
enum {
	SAB_WARNING,
	SAB_ERROR,
	
	SAB_INTERNAL,
	SAB_GENERAL,
	SAB_IO,
	SAB_LEXICAL,
	SAB_PARSE,
	SAB_TYPE,
	SAB_RUNTIME
};

enum {
	SAF_WARNING = (1L << SAB_WARNING),
	SAF_ERROR = (1L << SAB_ERROR),
	
	SAF_INTERNAL = (1L << SAB_INTERNAL),
	SAF_GENERAL = (1L << SAB_GENERAL),
	SAF_IO = (1L << SAB_IO),
	SAF_LEXICAL = (1L << SAB_LEXICAL),
	SAF_PARSE = (1L << SAB_PARSE),
	SAF_TYPE = (1L << SAB_TYPE),
	SAF_RUNTIME = (1L << SAB_RUNTIME),
	
	SAF_ALERT_TYPE = (SAF_WARNING|SAF_ERROR),
	SAF_ALERT_CLASS = (SAF_INTERNAL|
			   SAF_GENERAL|
			   SAF_IO|
			   SAF_LEXICAL|
			   SAF_PARSE|
			   SAF_TYPE|
			   SAF_RUNTIME)
};

/* scml_alert is used to generate any warnings/errors
   that have to do with SCML */
void scml_alert(struct scml_stream_pos *ssp,
		int alert_flags,
		const char *format, ...);

/* init_scml will handle any initialization needed by SCML */
int init_scml();

/* Return the set of built-in directories to search for SCML. */
const char **scml_std_include_dirs();

/* A simple string hash function */
int scml_hash_name(const char *name, int table_size);

/* Parse definitions given on the command line in the form `var=expr' */
int scml_parse_cmdline_defines(struct scml_scope *root_scope,
			       flag_value_seq *defs);

/* Execute a string of SCML code */
int scml_execute_str(struct scml_scope *root_scope,
		     char *code_desc, char *code, tag_list *tl);

/* Load a file used for defining various SCML objects */
struct scml_stream_pos *scml_execute_defs_file(struct scml_scope *root_scope,
					       const char **include_dirs,
					       const char *file_desc,
					       const char *file_name);

extern const char *scml_kind_map[SCML_NT_MAX + 1];

extern "C" {
	int scml_code_cast_handler(int indent, cast_handler *ch);
}

#endif /* SCML_HH */

/* End of file. */

