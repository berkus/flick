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

/* aoid.cc - aoid, the AOI file ASCII-output pretty printer */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include <string.h>		// POSIX string funcs
#include <ctype.h>		// External functions
#include <assert.h>

#include <mom/libmeta.h>
#include <mom/compiler.h>
#include <mom/libaoi.h>

aoi the_aoi;
char *progname;
FILE *output_file;


/* The following is for printing the AOI tree
 * ------------------------------------------ 
 * (moved from mom/fe/corba/be_produce.cc to mom/util/aoid)
 */ 

static int indent;
const int ISPC = 2;
static void print_node(aoi_type node);

static void i_printf(const char *format, ...)
{
	int i;
	va_list vl;
	
	va_start(vl, format);
	for (i=0; i<indent; i++)
		fprintf(output_file, " ");
	fflush(output_file);
	vfprintf(output_file, format, vl);
	va_end(vl);
	fflush(output_file);
}

static const char *aoi_kind_str(aoi_kind kind)
{
	switch (kind) {
	case AOI_INDIRECT:
		return "AOI_INDIRECT";
	case AOI_INTEGER:
		return "AOI_INTEGER";
	case AOI_SCALAR:
		return "AOI_SCALAR";
	case AOI_FLOAT:
		return "AOI_FLOAT";
	case AOI_CHAR:
		return "AOI_CHAR";
	case AOI_ARRAY:
		return "AOI_ARRAY";
	case AOI_STRUCT:
		return "AOI_STRUCT";
	case AOI_UNION:
		return "AOI_UNION";
	case AOI_INTERFACE:
		return "AOI_INTERFACE";
	case AOI_EXCEPTION:
		return "AOI_EXCEPTION";
	case AOI_ENUM:
		return "AOI_ENUM";
	case AOI_VOID:
		return "AOI_VOID";
	case AOI_CONST:
		return "AOI_CONST";
	case AOI_NAMESPACE:
		return "AOI_NAMESPACE";
	case AOI_OPTIONAL:
		return "AOI_OPTIONAL";
	case AOI_FWD_INTRFC:
		return "AOI_FWD_INTRFC";
	case AOI_ANY:
		return "AOI_ANY";
	case AOI_TYPE_TAG:
		return "AOI_TYPE_TAG";
	case AOI_TYPED:
		return "AOI_TYPED";
	case AOI_ERROR:
		return "AOI_ERROR";
	default:
		return "(invalid AOI type)";
	}
}

static void print_op_flags(aoi_op_flags op_flags)
{
	int old_indent = indent;
	
	/*
	 * Output the proper indentation; we subtract 1 because we always
	 * output a leading space.
	 */
	indent += ISPC - 1;
	i_printf("");
	
	/* Disable indentation for the rest of our output. */
	indent = 0;
	
	/* One `if' for each possible flag. */
	if (op_flags & AOI_OP_FLAG_ONEWAY)
		i_printf(" oneway");
	if (op_flags & AOI_OP_FLAG_IDEMPOTENT)
		i_printf(" idempotent");
	
	/* If there are no flags, say so. */
	if (op_flags == AOI_OP_FLAG_NONE)
		i_printf(" none");
	
	i_printf("\n");
	
	indent = old_indent;
}

static void print_const(aoi_const con, int doindent);

static void print_attrib(aoi_attribute *att)
{
	indent += ISPC;
	i_printf("name = \"%s\" (%s)\n",
		att->name,
		att->readonly ? "readonly" : "readwrite");
	i_printf("type:\n"); 
	print_node(att->type);
	i_printf("read request:\n");
	print_const(att->read_request_code, 1);
	i_printf("read reply:\n");
	print_const(att->read_reply_code, 1);
	if (!att->readonly) {
		i_printf("write request:\n");
		print_const(att->write_request_code, 1);
		i_printf("write reply:\n");
		print_const(att->write_reply_code, 1);
	}
	indent -= ISPC;
}

static void print_parameter(aoi_parameter *param)
{
	indent += ISPC;
	i_printf("name = \"%s\"\n", param->name);
	i_printf("direction = %s%s\n",
		(param->direction & 0x1)? "IN" : "",
		(param->direction & 0x2)? "OUT" : "");
	i_printf("type = \n");
	print_node(param->type);
	indent -= ISPC;
}

static void print_operation(aoi_operation *op)
{
	u_int i;
	
	indent += ISPC;
	
	i_printf("operation name: \"%s\"\n", op->name);
	i_printf("flags:\n");
	print_op_flags(op->flags);
	
	i_printf("params:\n");
	for (i = 0; i < op->params.params_len; i++) {
		i_printf("#%d - ",i);
		print_parameter(&op->params.params_val[i]);
	}
	i_printf("return type:\n");
	print_node(op->return_type);
	i_printf("exceptions:\n");
	for (i = 0; i < op->exceps.exceps_len; i++)
		print_node(op->exceps.exceps_val[i]);
	i_printf("request code:\n");
	print_const(op->request_code, 1);
	i_printf("reply code:\n");
	print_const(op->reply_code, 1);
	indent -= ISPC;
}

static void print_field(aoi_field *field)
{
	indent += ISPC;
	i_printf("name = \"%s\", type:\n", field->name);
	print_node(field->type);
	indent -= ISPC;
}

static void print_const(aoi_const con, int doindent)
{
	u_int i;
	
	if (doindent)
		indent += ISPC;
	switch (con->kind) {
	case AOI_CONST_INT:
		i_printf("<%d>\n", con->aoi_const_u_u.const_int);
		break;
	case AOI_CONST_CHAR:
		i_printf(((con->aoi_const_u_u.const_char == '\'') ? "'\\%c'\n" :
			 (con->aoi_const_u_u.const_char == '\\') ? "'\\%c'\n" :
			 isprint(con->aoi_const_u_u.const_char) ?  "'%c'\n" :
			 /* Otherwise, print in octal notation. */
			 "'\\%03d'\n"),
			con->aoi_const_u_u.const_char);
		break;
	case AOI_CONST_FLOAT:
		i_printf("[%lf]\n", con->aoi_const_u_u.const_float);
		break;
	case AOI_CONST_STRUCT:
		i_printf("CONST_STRUCT:\n");
		for (i = 0;
		     i < con->aoi_const_u_u.const_struct.aoi_const_struct_len;
		     i++) {
			i_printf("#%d - ", i);
			print_const((con->aoi_const_u_u.const_struct.
				     aoi_const_struct_val[i]),
				    0);
		}
		break;
	case AOI_CONST_ARRAY:
		i_printf("CONST_ARRAY:\n");
		for (i = 0;
		     i < con->aoi_const_u_u.const_array.aoi_const_array_len;
		     i++) {
			i_printf("#%d - ", i);
			print_const((con->aoi_const_u_u.const_array.
				     aoi_const_array_val[i]),
				    0);
		}
		break;
	default:
		panic("unknown const kind");
	}
	if (doindent)
		indent -= ISPC;
}

static void print_item(aoi_def *n, int slot);  /* declaration, for reference in print_node() */

static void print_node(aoi_type node)
{
	u_int i;
	
	indent += ISPC;
	i_printf("kind = %s\n", aoi_kind_str(node->kind));
	switch (node->kind) {
	case AOI_INDIRECT: {
		aoi_ref ref = node->aoi_type_u_u.indirect_ref;
		
		i_printf("reference to slot #%d - \"%s\"\n", ref,
			the_aoi.defs.defs_val[ref].name);
		break;
	}
	
	case AOI_INTEGER:
		i_printf("integer %d-%u\n", 
			node->aoi_type_u_u.integer_def.min,
			(u_int) node->aoi_type_u_u.integer_def.min + 
			(u_int) node->aoi_type_u_u.integer_def.range);
		break;
		
	case AOI_SCALAR:
		i_printf("%s%d bit scalar\n",
			((node->aoi_type_u_u.scalar_def.flags ==
			  AOI_SCALAR_FLAG_NONE) ? "" :
			 (node->aoi_type_u_u.scalar_def.flags ==
			  AOI_SCALAR_FLAG_SIGNED) ? "signed " :
			 (node->aoi_type_u_u.scalar_def.flags ==
			  AOI_SCALAR_FLAG_UNSIGNED) ? "unsigned " :
			 "incorrectly-flagged "),
			node->aoi_type_u_u.scalar_def.bits);
		break;

	case AOI_FLOAT:
		i_printf("%d bit float\n",
			node->aoi_type_u_u.float_def.bits);
		break;
		
	case AOI_CHAR:
		i_printf("%s%d bit char\n",
			((node->aoi_type_u_u.char_def.flags ==
			  AOI_CHAR_FLAG_NONE) ? "" :
			 (node->aoi_type_u_u.char_def.flags ==
			  AOI_CHAR_FLAG_SIGNED) ? "signed " :
			 (node->aoi_type_u_u.char_def.flags ==
			  AOI_CHAR_FLAG_UNSIGNED) ? "unsigned " :
			 "incorrectly-flagged "),
			node->aoi_type_u_u.char_def.bits);
		break;
		
	case AOI_ARRAY:
		i_printf("array with length_type:\n");
		print_node(node->aoi_type_u_u.array_def.length_type);
		i_printf("of:\n");
		print_node(node->aoi_type_u_u.array_def.element_type);
		i_printf("flags:\n");
		
		if (node->aoi_type_u_u.array_def.flgs == AOI_ARRAY_FLAG_NONE)
			i_printf("AOI_ARRAY_FLAG_NONE\n");
		if (node->aoi_type_u_u.array_def.flgs &
		    AOI_ARRAY_FLAG_NULL_TERMINATED_STRING)
			i_printf("AOI_ARRAY_FLAG_NULL_TERMINATED_STRING\n");
		if (node->aoi_type_u_u.array_def.flgs & AOI_ARRAY_FLAG_OPAQUE)
			i_printf("AOI_ARRAY_FLAG_OPAQUE\n");
		break;
		
	case AOI_STRUCT:
		i_printf("slots:\n");
		for (i = 0;
		     i < node->aoi_type_u_u.struct_def.slots.slots_len;
		     i++) {
			i_printf("#%d name = \"%s\", type: \n",
				i,
				(node->aoi_type_u_u.struct_def.slots.
				 slots_val[i].name));
			print_node(node->aoi_type_u_u.struct_def.slots.
				   slots_val[i].type);
		}
		break;
		
	case AOI_UNION:
		i_printf("discriminator:\n");
		print_field(&node->aoi_type_u_u.union_def.discriminator);
		i_printf("union name: '%s'\n",
			node->aoi_type_u_u.union_def.union_label);
		for (i = 0; i < node->aoi_type_u_u.union_def.cases.cases_len;
		     i++) {
			i_printf("#%d value: ", i);
			print_const((node->aoi_type_u_u.union_def.cases.
				     cases_val[i].val),
				    0);
			i_printf("#%d var: ", i);
			print_field(&(node->aoi_type_u_u.union_def.cases.
				      cases_val[i].var));
		}
		if (node->aoi_type_u_u.union_def.dfault) {
			i_printf("default var = \n");
			print_field(node->aoi_type_u_u.union_def.dfault);
		}
		break;
		
	case AOI_INTERFACE:
		i_printf("code:\n");
		print_const(node->aoi_type_u_u.interface_def.code, 1);
		i_printf("parents:\n");
		for (i = 0;
		     i < node->aoi_type_u_u.interface_def.parents.parents_len;
		     i++)
			print_node(node->aoi_type_u_u.interface_def.parents.
				   parents_val[i]);
		
		i_printf("operations:\n");
		for (i = 0;
		     i < node->aoi_type_u_u.interface_def.ops.ops_len;
		     i++)
			print_operation(&(node->aoi_type_u_u.interface_def.ops.
					  ops_val[i]));
		
		i_printf("attributes:\n");
		for (i = 0;
		     i < node->aoi_type_u_u.interface_def.attribs.attribs_len;
		     i++)
			print_attrib(&(node->aoi_type_u_u.interface_def.
				       attribs.attribs_val[i]));
		
		i_printf("exceptions:\n");
		for (i = 0;
		     i < node->aoi_type_u_u.interface_def.excepts.excepts_len;
		     i++)
			print_node(node->aoi_type_u_u.interface_def.
				   excepts.excepts_val[i]);
		break;
		
	case AOI_EXCEPTION:
		i_printf("slots:\n");
		for (i = 0;
		     i < node->aoi_type_u_u.exception_def.slots.slots_len;
		     i++) {
			i_printf("name = \"%s\", type: \n", 
				node->aoi_type_u_u.exception_def.slots.
				slots_val[i].name);
			print_node(node->aoi_type_u_u.exception_def.slots.
				   slots_val[i].type);
		}
		break;
		
	case AOI_CONST:
		i_printf("type:\n");
		print_node(node->aoi_type_u_u.const_def.type);
		i_printf("value:\n");
		print_const(node->aoi_type_u_u.const_def.value, 1);
		break;
	
	case AOI_ENUM:
		i_printf("enum name: '%s'\n",
			node->aoi_type_u_u.enum_def.enum_label);
		i_printf("values:\n");
		indent += ISPC;
		for (i = 0; i < node->aoi_type_u_u.enum_def.defs.defs_len;
		     i++) {
			i_printf("Slot #%d\n", i);
			print_field(&(node->aoi_type_u_u.enum_def.defs.
				      defs_val[i]));
		}
		indent -= ISPC;
		break;
		
	case AOI_VOID:
		break;
		
	case AOI_NAMESPACE:
		break;
		
	case AOI_OPTIONAL:
		i_printf("type:\n");
		print_node(node->aoi_type_u_u.optional_def.type);
		break;
		
	case AOI_FWD_INTRFC:
		break;
		
	case AOI_ANY:
		break;
		
	case AOI_TYPE_TAG:
		break;
		
	case AOI_TYPED:
		i_printf("tag:\n");
		print_node(node->aoi_type_u_u.typed_def.tag);
		i_printf("type:\n");
		print_node(node->aoi_type_u_u.typed_def.type);
		break;
		
	case AOI_ERROR:
		i_printf("!!! AOI_ERROR nodes should not appear in `.aoi' "
			"files !!!\n");
		break;
		
	default:
		panic("Type %d is not yet implemented.", node->kind);
		break;
	}
	
	indent -= ISPC;
}

static void print_item(aoi_def *n, int slot)
{
	indent = 0;
	i_printf("Slot #%d\nScope #%d\nName \"%s\"\nIDL File #%d\n",
		 slot,
		 n->scope,
		 n->name,
		 n->idl_file);
	print_node(n->binding);
	i_printf("\n");
}

static void print_all()
{
	u_int i;
	
	for (i = 0; i < the_aoi.defs.defs_len; i++)
		print_item(&(the_aoi.defs.defs_val[i]), i);
}

void usage(void)
{
	fprintf(stderr,
		("Usage: %s [<options>] [<infile>]\n"
		 "<infile>: AOI input file; defaults to stdin.\n"
		 "<options>:\n"
		 "-?, -h, --help            "
		 "Print this usage message.\n"
		 
		 "-v, -V, --version         "
		 "Print the program version number.\n"
		 
		 "-o<file>, -o <file>,      "
		 "Write output to <file>.\n"
		 "  --output <file>         "
		 "Default is <infileroot>%s, or stdout.\n"),
		progname,
		AOID_SUFFIX);
	exit(1);
}

void version()
{
	fprintf(stderr,
		"AOI Dump Utility program, version "
		FLICK_VERSION"\n");
	exit(0);
}

int main(int argc, char **argv)
{
	FILE *input_file;
	char *outname = 0;
	
	progname = argv[0];
	argv++; argc--;
	
	/* Parse arguments. */
	while ((argc > 0) && (argv[0][0] == '-')) {
		switch (argv[0][1]) {
		case '?':
		case 'h':
			usage();
		case 'o':
			if (argv[0][2] == 0) {
				if (argc < 2)
					panic("filename required after -o");
				outname = argv[1];
				argv++;
				argc--;
			} else
				outname = argv[0]+2;
			break;
		case 'v':
		case 'V':
			version();
		default:
			if (!strcmp(argv[0], "--help"))
				usage();
			if (!strcmp(argv[0], "--version"))
				version();
			if (!strcmp(argv[0], "--output")) {
				if (argc < 2)
					panic("filename required after `%s'",
					      argv[0]);
				outname = argv[1];
				argv++;
				argc--;
				break;
			}
			fprintf(stderr,
				"unknown command-line option `%s'\n",
				argv[0]);
			usage();
		}
		argv++;
		argc--;
	}
	
	/* Handle the remaining input and output file setting. */
	if (argc > 1)
		usage();
	if (argc == 1) {
		char *inname=argv[0];
		
		input_file = fopen(inname, "rb");
		if (!input_file)
			panic("can't open input file `%s'", inname);
		if (outname == 0)
			outname = resuffix(inname, AOID_SUFFIX);
	} else
		input_file = stdin;
	
	if (outname != 0) {
		output_file = fopen(outname, "w");
		if (!output_file)
			panic("can't open output file `%s'", outname);
	} else		
		output_file = stdout;
	
	/* End of argument processing. */
	
	/* Read in AOI binary from standard input. */
	aoi_readfh(&the_aoi, input_file);
	
	print_meta(&the_aoi.meta_data, output_file, 0);
	/* Output an ASCII translation to standard output. */
	print_all();
	exit(0);
}

/* End of file. */

