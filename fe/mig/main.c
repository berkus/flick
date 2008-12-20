/*
 * Copyright (c) 1995, 1996, 1997, 1999 The University of Utah and
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
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <rpc/types.h>
#include <rpc/xdr.h>

#include <mom/mint.h>
#include <mom/pres_c.h>
#include <mom/compiler.h>
#include <mom/c/libpres_c.h>

#include "lexxer.h"
#include "xlate_util.h"
#include "global.h"
#include "type.h"
#include "error.h"

char *progname;

static char *cpp_args = NULL;
static int cpp_args_size = 0;
static int cpp_args_length = 0;

extern void translate(); /* Translation of types, routines */
extern int yyparse();
void gen_error_mappings( pres_c_1 * );

extern int yydebug;
extern FILE *yyin;

extern char *ihead_name;
int make_ihead;


char *infilename;

FILE *fin;
FILE *fout;

pres_c_1 out_pres_c;

int gen_client = 1;
int gen_server = 1;

extern void make_iheader(void);

static FILE *f_open(const char *file_name, const char *mode)
{
	FILE *ret;
	if ( (ret = fopen(file_name,mode)) )
		return ret;
	else
		perror(file_name);

	return NULL;
}

static void add_to_cpp_args(char *arg);

/*
 * Open input file, run thorugh C preprocessor  
 */
static void open_input(char *infile)
{
	infilename = (infile == NULL) ? "<stdin>" : infile;
	
	yyin = call_c_preprocessor(infile, cpp_args);
	if (!yyin)
		panic("Can't open file `%s' for reading.", infilename);
}

/*
 * Print the standard usage to stderr
 */
static void print_usage(void)
{
    fprintf(stderr,"Flick MIG IDL front end usage:\n");
    fprintf(stderr,"\tflick_fe_mig [<options>] [<infile>]\n");
    fprintf(stderr,"If <infile> is unspecified, defaults to stdin.\n");
    fprintf(stderr,"Options:\n");
    fprintf(stderr,"\t -?, -h, --help: Print this message.\n");
    fprintf(stderr,"\t -v, -V, --version: Print program version number.\n");
    fprintf(stderr,"\t -c, --client: Generate client presentation only.\n");
    fprintf(stderr,"\t\t (Default is both)\n");
    fprintf(stderr,"\t -s, --server: Generate server presentation only.\n");
    fprintf(stderr,"\t\t (Default is both)\n");
    fprintf(stderr,"\t -o<file>, -o <file>, --output <file>: Output to <file>.\n");
    fprintf(stderr,"\t\t Defaults to <infile>.prc, stdout if <infile> unspecified.\n");

    fprintf(stderr,"\t-I<dir>: Specify include directories for cpp.\n");
    fprintf(stderr,"\t-D<sym>[=<value>]: Define <sym> to be <value> (Defaults to 1)\n");
    fprintf(stderr,"\t-U<sym>: Undefine symbol.\n");
    fprintf(stderr,"\t-Xcpp <option>: Pass <option> to cpp.\n");

    exit(1);
}

static void print_version(void)
{
    fprintf(stderr,"Flick MIG IDL front end, version %s.\n",FLICK_VERSION);
    exit(0);
}

/* 
 * Add this string as an argument for the C preprocessor.  We aggregrate all of
 * the arguments into a single string, which is later pulled apart by the
 * `call_c_preprocessor' function.  We should probably have an interface to the
 * library that takes an argv instead.
 */
static void add_to_cpp_args(char *arg)
{
	int arg_length;
	
	if (!arg)
		return;
	
	arg_length = strlen(arg);
	
	if ((cpp_args_length + 1 /* SPC */ + arg_length + 1 /* NUL */)
	    > cpp_args_size) {
		cpp_args_size = cpp_args_length + 1 + arg_length + 1
				+ 256 /* extra */;
		cpp_args = (char *) mustrealloc(cpp_args, cpp_args_size);
	}
	
	if (cpp_args_length <= 0) {
		strcpy(cpp_args, arg);
		cpp_args_length = arg_length;
	} else {
		strcat(cpp_args, " ");
		strcat(cpp_args, arg);
		cpp_args_length += 1 /* SPC */ + arg_length;
	}
}

int main(int argc, char **argv)
{
    int i;

    /* Which command line args did we see, didn't see. */
    int seen_in = 0;
    int seen_client = 0;
    int seen_server = 0;
    int seen_output = 0;

    char *inname = NULL;
    
    char *outname = NULL;
    
    ihead_name = NULL;
	
    progname = argv[0];
    
    set_program_name(progname); /* for fatal error messages */

    /* Parse the args */

    for (i = 1; i < argc; i++)
	{
	    if (argv[i][0] == '-')
		{
		    switch (argv[i][1])
			{
			case '-':

			    if (strcmp(argv[i],"--help") == 0)
				{
				    print_usage();
				    exit(0);
				}
			    
			    else if (strcmp(argv[i],"--version") == 0)
				{
				    print_version();
				    exit(0);
				}

			    else if (strcmp(argv[i],"--client") == 0)
				{
				    if (seen_client)
					    panic("Client option already seen.");
				    gen_server = 0;
				    
				    seen_client = 1;
				}
			    
			    else if (strcmp(argv[i],"--server") == 0)
				{
				    if (seen_server)
					    panic("Server option already seen.");
				    gen_client = 0;
				    
				    seen_server = 1;
				}
			    
			    else if (strcmp(argv[i],"--output") == 0)
				{
				    if (seen_output)
					    panic("Output option already seen.");
				    if( ++i == argc || argv[i][0] == '-')
					    panic("Output option must be followed by a filename.");
				    else
					outname = argv[i];
				}
			    else
				{
				    fprintf(stderr,
					    "Unknown option `%s'seen.\n\n",
					    argv[i]);
				    print_usage();
				    exit(1);
				}

			    break;
			    			    
			case '?':
			case 'h':
			
			    print_usage();
			    exit(0);
			    
			    break;

			case 'v':
			case 'V':
				
			    print_version();
			    exit(0);
			    
			    break;
			    
			case 'c':
			    if (seen_client)
				    panic("Client option already seen.");
			    
			    gen_server = 0;
			    
			    seen_client = 1;
			    
			    break;
			    
			case 's':
			    if (seen_server)
				    panic("Server option already seen.");
			    
			    gen_client = 0;
			    
			    seen_server = 1;
			    
			    break;
			    
			case 'o':

			    if (seen_output)
				    panic("Output option already seen.");
			    
			    if (strlen(argv[i]) > 2)
				outname = argv[i] + 2;
			    else
				{
				    if (++i == argc || argv[i][0] == '-')
					    panic("Output option must be followed by a filename.");
				    else
					outname = argv[i];
				}
				
			    seen_output = 1;
			    
			    break;

			case 'I':
			case 'D':
			case 'U':
			
			    add_to_cpp_args(argv[i]);
			    
			    break;

			case 'X':
			    if (strcmp(argv[i],"-Xcpp"))
				{
				    fprintf(stderr,
					    "Unknown option `%s' seen.\n\n",
					    argv[i]);
				    
				    print_usage();
				    exit(1);
				}
			    else
				{
				    add_to_cpp_args(argv[i+1]);
				    i++;
				}
								  
			    break;

			case 'i':

				if (strcmp(argv[i],"-iheader"))
				    {
				        fprintf(stderr,"Unknown option `%s' seen", argv[i]);
					print_usage();
					exit(1);
				    }

				if( ++i == argc || argv[i][0] == '-')
				    panic("Output option must be followed by a filename.");
				
				ihead_name = argv[i];
				make_ihead = 1;
				
				break;

			default:
			    
			    fprintf(stderr,"Unknown option `%s' seen.\n\n",
				    argv[i]);
				
			    print_usage();
			    exit(1);
			    break;
			}
		}
	    else
		{
		    if (seen_in)
			    panic("Input file already seen.");

		    inname = argv[i];
		    seen_in = 1;
		}
	}

    /* Open up I/O. */

    open_input(inname);

    if (outname)
	{
	    fout = f_open(outname,"wb");
	}
    else
	{
	    if (inname)
		{
		    outname = resuffix(inname,".prc");
		    fout = f_open(outname,"wb");
		}
	    else
		fout = stdout;
	}
    
    init_global();
    init_meta(&out_pres_c.meta_data);
    meta_add_channel(&out_pres_c.meta_data,
		     meta_add_file(&out_pres_c.meta_data, "(generated)",
				   IO_FILE_INPUT),
		     "");
    builtin_file = meta_add_file(&out_pres_c.meta_data, "(builtin)",
				 IO_FILE_BUILTIN);
    /*
     * We strip off any leading path on `inname'.  We don't want PRES_C
     * files to differ based on the path to the input file.
     */
    root_filename = (inname == NULL) ? "<stdin>" : muststrdup(inname);
    root_file = meta_add_file(&out_pres_c.meta_data,
			      file_part(root_filename),
			      IO_FILE_INPUT | IO_FILE_ROOT);
    init_type();
    LookNormal();
    yyparse();
    
    if (errors > 0)
	    fatal(errors==1 ? "%d error found" : "%d errors found", errors);

    if (make_ihead)
	    make_iheader();

    out_pres_c.a.defs.defs_len = 0;
    out_pres_c.a.defs.defs_val = 0;
    out_pres_c.pres_context = "mig"; /* Name of PG style. */
    out_pres_c.error_mappings.error_mappings_len = 0;
    out_pres_c.error_mappings.error_mappings_val = 0;
    init_channel_maps(&out_pres_c);
    for( i = 0; i < out_pres_c.meta_data.files.files_len; i++ ) {
	map_file_channels(&out_pres_c, i);
    }
    gen_error_mappings( &out_pres_c );
    
    build_init_cast();
    translate();
    pres_c_1_writefh(&out_pres_c,fout);

    return 0;
}
