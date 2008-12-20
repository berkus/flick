%{
/*
 * Copyright (c) 1995, 1996, 1997, 1998, 1999 The University of Utah and
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

/* scanner.ll - a scanner for CORBA IDL. */

#include "parser.hh"
#include "Errors.hh"
extern "C" {
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <math.h>
}
#include <mom/aoi.h>
#include <mom/compiler.h>
#include <mom/libmeta.h>

#define tokenAdvance    { tokenPos = nextPos; nextPos += yyleng;}
#define gotToken   	tokenAdvance
#define gotTokenAndWhite	{ tokenPos = nextPos;  \
			  for (char *s = yytext; *s != '\0'; s++)  \
				if (*s == '\t')   \
           				nextPos += 8 - (nextPos & 7); \
				else nextPos++;   \
			}

#define incNextPos	(nextPos++)

static unsigned int get_dec(char *);
static unsigned int get_hex(char *);
static unsigned int get_oct(char *);
static double get_float(char *);
static char get_chr_oct(char *);
static char get_chr_esc(char *);
int nextchar = 0;
int buflen;
extern aoi cur_aoi;
extern const char *infilename;
extern const char *root_filename;
extern io_file_index root_file;
extern io_file_index current_i_file;

#ifdef FLEX_SCANNER 

#undef YY_INPUT
#define YY_INPUT(buffer, result, max_size) \
        (result = my_yyinput(buffer, max_size))
static int my_yyinput(char*, int);
/*
 * We don't need `yyunput' from Flex, so we define `YY_NO_UNPUT' to remove its
 * definition and thus avoid a compile-time warning about not using that static
 * function.
 */
#define YY_NO_UNPUT

#else

#undef input
#undef unput
#define input() (my_input())
#define unput(c) (my_unput(c))
static char my_input();
static void my_unput(char);

#endif
%}

%%

";"		{gotToken; return SEMI;}
"{"		{gotToken; return LBRACE;}
"}"		{gotToken; return RBRACE;}
":"		{gotToken; return COLON;}
","		{gotToken; return COMMA;}
"::"		{gotToken; return SCOPE;}
"="		{gotToken; return EQUAL;}
"|"		{gotToken; return OR;}
"^"		{gotToken; return XOR;}
"&"		{gotToken; return AND;}
">>"		{gotToken; return RSHIFT;}
"<<"		{gotToken; return LSHIFT;}
"+"		{gotToken; return ADD;}
"-"		{gotToken; return SUB;}
"*"		{gotToken; return MUL;}
"/"		{gotToken; return DIV;}
"%"		{gotToken; return MOD;}
"~"		{gotToken; return NEG;}
"("		{gotToken; return LPAREN;}
")"		{gotToken; return RPAREN;}
"<"		{gotToken; return LT;}
">"		{gotToken; return GT;}
"["		{gotToken; return LBRACK;}
"]"		{gotToken; return RBRACK;}
TRUE		{gotToken; return BTRUE;}
FALSE		{gotToken; return BFALSE;}
const		{gotToken; return CONST;}
module		{gotToken; return MODULE;}
interface	{gotToken; return INTERFACE;}
typedef		{gotToken; return TYPEDEF;}
float		{gotToken; return FLOAT;}
double		{gotToken; return DOUBLE;}
long		{gotToken; return LONG;}
short		{gotToken; return SHORT;}
unsigned	{gotToken; return UNSIGNED;}
int		{gotToken; return INT;} /* invalid */
char		{gotToken; return CHAR;}
boolean		{gotToken; return BOOLEAN;}
octet		{gotToken; return OCTET;}
any		{gotToken; return ANY;}
struct		{gotToken; return STRUCT;}
union		{gotToken; return UNION;}
switch		{gotToken; return SWITCH;}
case		{gotToken; return CASE;}
default		{gotToken; return DEFAULT;}
enum		{gotToken; return ENUM;}
sequence	{gotToken; return SEQUENCE;}
string		{gotToken; return STRING;}
readonly	{gotToken; return READONLY;}
attribute	{gotToken; return ATTRIBUTE;}
exception	{gotToken; return EXCEPTION;}
oneway		{gotToken; return ONEWAY;}
void		{gotToken; return VOID;}
in		{gotToken; return IN;}
inout		{gotToken; return INOUT;}
out		{gotToken; return OUT;}
raises		{gotToken; return RAISES;}
context		{gotToken; InternalError("contexts are not yet supported");return CONTEXT;}
Object		{gotToken; return OBJECT;}

[a-zA-Z][a-zA-Z0-9_]*	{gotToken;
	char *z = (char *) malloc(strlen(yytext) + 1);
	strcpy(z, yytext);
	yylval.id = z;
	return ID;
}

[1-9][0-9]*	{gotToken; 
	yylval.integer = get_dec(yytext);
	return LIT_INT;
}

0[xX][a-fA-F0-9]+	{gotToken; 
	yylval.integer = get_hex(yytext+2);   /* strip off the "0x" */
	return LIT_INT;
}

0[0-7]*	{gotToken; 
	yylval.integer = get_oct(yytext);
	return LIT_INT;
}

[0-9]+"."[0-9]*([eE][+-]?[0-9]+)? {gotToken;
	yylval.real = get_float(yytext);
	return LIT_REAL;
}

[0-9]+[eE][+-]?[0-9]+ {gotToken;
	yylval.real = get_float(yytext);
	return LIT_REAL;
}

"\""(([^\"]*)((\\.)*))*"\"" {gotToken;
	int len = strlen(yytext) - 1;
	char *z = (char *) malloc(len);
	strncpy(z, yytext + 1, len - 1);
	z[len - 1] = 0;
	yylval.str = z;
	return LIT_STRING;
}

"'"."'" {gotToken;
	yylval.chr = yytext[1];
	return LIT_CHR;
}

"'"\\([0-3]?[0-7]?[0-7])"'" {gotToken;
	yylval.chr = get_chr_oct(yytext + 2);
	return LIT_CHR;
}

"'"\\."'"	{gotToken; 
	yylval.chr = get_chr_esc(yytext + 2);
	return LIT_CHR;
}

"'"\\"x"[a-fA-F0-9]?[a-fA-F0-9]"'"	{gotToken;
	yylval.chr = get_chr_esc(yytext+2); 
	return LIT_CHR;
}

"//".*	{gotToken; }

"/*" {  gotToken;
	int prevchar = 0, currchar = yyinput();
	incNextPos;
	for (;;) {
        	if (currchar == EOF)
 			break;
 		if (currchar == '\n') {
			++lineno;
			nextPos = 0;
			tokenPos = 0;
		}
		else if (currchar == '\t')
			nextPos += (8 - 1) - (nextPos-1) & 7;
		else if (prevchar == '/' && currchar == '*') {
			tokenPos = nextPos-2; 
                 	LexWarning(("comment start delimiter `/*' appears "
				    "within comment"));
 		}
		else if (prevchar == '*' && currchar == '/')
			break;
		prevchar = currchar;
		currchar = yyinput();
		incNextPos;
	}
}

^[ \t]*#.* {gotTokenAndWhite;
	/*
	 * Parse `#line' directives.  (The token `line' is optional.)
	 * (White space before the '#' is optional.)
	 */
	int i = 0;
	/* Skip over whitespace before the initial `#'. */
	for (;
	     ((i < yyleng)
	      && ((yytext[i] == ' ') || (yytext[i] == '\t')));
	     ++i)
		/* Do nothing. */ ;
	
	/* Skip over '#' and any whitespace. */
	for (++i;
	     ((i < yyleng)
	      && ((yytext[i] == ' ') || (yytext[i] == '\t')));
	     ++i)
		/* Do nothing. */ ;
	/* Skip over the (optional) token `line' and subsequent whitespace. */
	if ((i < yyleng)
	    && !strncmp(&(yytext[i]), "line", (sizeof("line") - 1))) {
		i += sizeof("line") - 1; /* `sizeof' counts terminating NUL */
		for (;
		     ((i < yyleng) &&
		      ((yytext[i] == ' ') || (yytext[i] == '\t')));
		     ++i)
			/* Do nothing. */ ;
	}
	
	/* Parse the line number and subsequent filename. */
	if ((i < yyleng)
	    && isdigit(yytext[i])) {
		int filename_start, filename_end;
		char *filename;
		int flag = 0;
		
		lineno = get_dec(yytext + i) - 1;
		filename_end = 0;
		
		for (filename_start = 0; (i < yyleng); ++i)
			if (yytext[i] == '\"') {
				filename_start = i + 1;
				break;
			}
		if (filename_start)
			for (i = filename_start; (i < yyleng); ++i)
				if (yytext[i] == '\"') {
					filename_end = i;
					break;
				}
		if (filename_start && filename_end) {
			int last_i_file;
			
			last_i_file = current_i_file;
			if (filename_end - filename_start > 0) {
				filename =
					(char *)
					mustmalloc(sizeof(char)
						   * ((filename_end
						       - filename_start)
						      + 1 /* for NUL */));
				strncpy(filename,
					&(yytext[filename_start]),
					(filename_end - filename_start));
				filename[filename_end - filename_start] = 0;
				infilename = filename;
			} else {
				/* If no filename given, it must be stdin. */
				infilename = "<stdin>";
			}
			for(i++;
			    ((i < yyleng) &&
			     (yytext[i] == ' ') || (yytext[i] == '\t'));
			    i++);
			if( isdigit(yytext[i]) ) {
				flag = get_dec(yytext + i);
			}
			
			/*
			 * Since the root filename was purged of its path, we
			 * have to make sure that we don't try to add the
			 * full-path version.  Also, if the root file includes
			 * another file in the same directory or a
			 * subdirectory, then we strip off the pathname of that
			 * file, too.
			 *
			 * XXX - This isn't the whole solution.  If the root
			 * file includes a file relative to the parent
			 * directory (".."), then the preprocessor may give us
			 * only the exact path of the file, not the relative
			 * path.  What we really want is the text inside the
			 * #include directive, rather than exact paths, perhaps
			 * with additional information as to which are system
			 * files and user files (e.g. whether included with <>
			 * or "").  Unfortunately, the preprocessor does away
			 * with the #include lines, and only gives us a
			 * relative or full pathname, although gcc is at least
			 * gracious enough to also indicate system include
			 * files.
			 */
			if (strcmp(infilename, root_filename) != 0) {
				const char *root_dir = dir_part(root_filename);
				int dir_len = strlen(root_dir);
				
				if ((dir_len > 0)
				    && (strncmp(infilename, root_dir, dir_len)
					== 0)) {
					current_i_file = meta_add_file(
						&cur_aoi.meta_data,
						infilename + dir_len + 1,
						IO_FILE_INPUT);
				} else {
					current_i_file = meta_add_file(
						&cur_aoi.meta_data,
						infilename,
						(IO_FILE_INPUT |
						 ((flag == 3) ?
						  IO_FILE_SYSTEM : 0)));
				}
			} else {
				current_i_file = root_file;
			}
			
			switch( flag ) {
			case 2:
				meta_include_file(&cur_aoi.meta_data,
						  current_i_file,
						  last_i_file);
				break;
			default:
				break;
			}
		}
	}
}

[ \t]*	{gotTokenAndWhite; }

\n	{ ++lineno;
	  tokenPos = nextPos = 0;
}

.	{gotToken; LexError("illegal character `%s'", yytext);}

%%

static int 
ishexdigit(char c)
{
	if (c >= '0' && c <= '9')
		return 1;
	if (c >= 'a' && c <= 'f')
		return 1;
	if (c >= 'A' && c <= 'F')
		return 1;
	return 0;
}

static unsigned int 
get_dec(char *s) 
{
	unsigned int res = 0, lastres = 0;
	while (*s >= '0' && *s <= '9' && lastres <= res) {
		lastres = res;
		res = res * 10 + *s - '0';
		s++;
	}
	if (lastres > res)
		LexError("decimal constant overflow");
	return res;
}

/* converts a hex number string to an int.  Note: `s' must not contain "0x" */
static unsigned int 
get_hex(char *s) 
{
	unsigned int res = 0, lastres = 0;
	while (((*s >= '0' && *s <= '9') ||
	        (*s >= 'a' && *s <= 'f') ||
	        (*s >= 'A' && *s <= 'F')) &&
	       (lastres <= res)) {
		lastres = res;
		if (*s >= '0' && *s <= '9') 
			res = res * 16 + *s - '0';
		else 
			res = res * 16 + (*s & 0x1f + 9);
		s++;
	}
	if (lastres > res)
		LexError("hexadecimal constant overflow");
	return res;
}
			
static unsigned int 
get_oct(char *s) 
{
	unsigned int res = 0, lastres = 0;
	while (*s >= '0' && *s < '8' && lastres <= res) {
		lastres = res;
		res = res * 8 + *s - '0';
		s++;
	}
	if (lastres > res)
		LexError("octal constant overflow");
	return res;
}
			
static double 
get_float(char *s)
{
	double div = 0;
	int exp = 0, sign = 1;
	int mantissa = 1;
	double res = 0;
	while (1) {
	  if (*s >= '0' && *s <= '9') {
	    if (mantissa) {
	      if (div)
		res += ((double)(*s - '0') / div);
	      else
		res = res * 10 + (*s - '0');
	      div *= 10;
	    } else 
	      exp = exp * 10 + (*s - '0');
	  } else if (*s == '.') {
	    div = 10;
	  } else if (*s == 'e' || *s == 'E') {
	    mantissa = 0;
	  } else if (*s == '-') {
	    sign = -1;
	  } else if (*s == '+') {
	    sign = 1;
	  } else {
	    res *= pow(10, sign * exp);
	    return res;
    	  }
	  s++;
	}
}

static char 
get_chr_oct(char *s) 
{
	int ret = 0;
	int pos = 0;
	while (s[pos] >= '0' && s[pos] <= '7' && pos < 3)
		ret = ret * 8 + s[pos++] - '0';
	return (char) ret;
}

static char 
get_chr_esc(char *s)
{
	switch (*s) {
	case 'n':
		return '\n';
	case 't':
		return '\t';
	case 'v':
		return '\v';
	case 'b':
		return '\b';
	case 'r':
		return '\r';
	case 'f':
		return '\f';
	case 'a':
		return '\a';
	case '\\':
		return '\\';
	case '\?':
		return '?';
	case '\'':
		return '\'';
	case '"':
		return '"';
	case 'x': {
		char str[3];
		int i;
		for (i = 0; 
		     s[i + 1] != '\0' && ishexdigit(s[i + 1]) && i < 2;
		     i++)
			str[i] = s[i+1];
		str[i] = 0;
		return get_hex(str);
	}
	break;
	default:
		if (s[1] >= '0' && s[1] <= '7') {
			int i;
			for (i = 1; i <= 3 && s[i] >= '0' && s[i] <= '7'; i++)
				{}
			char save = s[i];
			s[i] = '\0';
			char out = (char)get_oct(&s[1]);
			s[i] = save;
			return out;
		} else {
			LexError("invalid escape sequence `%s'", s);
			return s[1];
		}

		break;
	}
}

#ifdef FLEX_SCANNER

static int my_yyinput(char* dest, int size) {
  	if (!linebuf[nextchar]) {
    		if (fgets(linebuf, BUFSIZE, yyin)) {
	 	 	buflen = strlen(linebuf);
    			nextchar = 0;
		} else {
			dest[0] = '\0';
			return 0;
		}
  	}
  	size--; /* reserve room for NULL terminating char */
  	if (buflen - nextchar < size)
	    	size = buflen - nextchar;
  	strncpy(dest, &linebuf[nextchar], size);
  	dest[size] = 0; /* terminate with null */
  	nextchar += size;
  	return size;
}

#else

static char
my_input()
{
	if (!linebuf[nextchar]) {
    		fgets(linebuf, BUFSIZE, yyin);
    		nextchar = 0;
 	}
  	return linebuf[nextchar++];
}

static void my_unput(char c)
{
  	if (nextchar <= 0) 
    		InternalError("cannot unput characters from previous lines");
  	else
    		linebuf[--nextchar] = c;
}

#endif

/* End of file. */

