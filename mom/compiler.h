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

#ifndef _mom_compiler_h_
#define _mom_compiler_h_

#include <stdio.h>
#include <stdarg.h>
#include <limits.h>

#ifdef __cplusplus
extern "C" {
#endif


#if defined(__GNUC__) && (__GNUC__ >= 2)
#  if __GNUC_MINOR__ >= 7
#    define DECL_NORETURN(decl) decl __attribute__((__noreturn__))
#    define DEFN_NORETURN(decl) decl
#  else
#    if __GNUC_MINOR__ >= 6
#      define DECL_NORETURN(decl) decl __attribute__((noreturn))
#      define DEFN_NORETURN(decl) decl
#    else
#      define DECL_NORETURN(decl) volatile decl
#      define DEFN_NORETURN(decl) volatile decl
#    endif
#  endif
#else
#  define DECL_NORETURN(decl) decl
#  define DEFN_NORETURN(decl) decl
#endif


/* suffixes for default output filenames */
#define AOID_SUFFIX ".aod"
#define PRES_C_SUFFIX ".prc"
#define PRESD_SUFFIX ".prd"


/* main() must provide this variable
   and set it to the value of argv[0].  */
extern char *progname;


/*
 * Print an error message and exit the program.  Takes a `printf'-style format
 * string and arguments.
 */
DECL_NORETURN(void panic(const char *format, ...));

/*
 * Print a warning.  Takes a `printf'-style format string and arguments.
 */
void warn(const char *format, ...);

/*
 * Like the normal allocation routines, but these never return null.  They call
 * `panic' if we run out of memory.
 */
void *real_mustmalloc(long size, const char *file, int line);
void *real_mustcalloc(long size, const char *file, int line);
void *real_mustrealloc(void *orig_buf,
		       long new_size, const char *file, int line);
char *real_muststrdup(const char *str, const char *file, int line);

#define mustmalloc(size)	real_mustmalloc((size), __FILE__, __LINE__)
#define mustcalloc(size)	real_mustcalloc((size), __FILE__, __LINE__)
#define mustrealloc(org, size)	real_mustrealloc((org), \
						 (size), __FILE__, __LINE__)
#define muststrdup(str)		real_muststrdup((str), __FILE__, __LINE__)

/*
 * Invoke the C or C++ preprocessor on `filename', with command arguments
 * `flags'.  If `filename' is null, arrange for the preprocessor to read from
 * stdin.
 *
 * Return a `FILE *' so that we receive the preprocessor's output.  As a side
 * effect, set the global variable `infilename' to be a printable version of
 * `filename'.
 */
FILE *call_c_preprocessor(const char *filename, const char *flags);
FILE *call_cxx_preprocessor(const char *filename, const char *flags);

/* Various file name processors */

/*
 * Returns the file part of the file name, its just a pointer into the given
 * string, so dup it if you want a new one
 */
const char *file_part(const char *filename);
/* Returns the directory part of the file name, its a new string */
const char *dir_part(const char *filename);
/* Appends filename onto pathname, being careful to insert a '/' if needed */
const char *add_file_part(const char *pathname, const char *filename);
/* Returns true if filename is an absolute path */
int absolute_path(const char *filename);
/* Returns true if filename refers to the current directory (i.e. '.') */
int current_dir(const char *filename);
/*
 * Copy the specified `orig' filename into a new buffer, and replace its suffix
 * (the last `.' and everything beyond it) with the specified `newsuffix'.
 */
char *resuffix(const char *orig, const char *newsuffix);
/* Converts, in place, a filename to a valid C identifier */
void filename_to_c_id(char *filename);

/*
 * Open the file named by `path', in the mode specified by `mode', searching
 * the directories named in `dir_list'.
 */
FILE *fopen_search(const char *path,
		   const char * const *dir_list,
		   const char *mode,
		   /* OUT */ char **out_path);

/*
 * "Allocating sprintf" --- allocates exactly the right amount of memory to
 * contain the formatted string.  Panics if the allocation fails; never returns
 * null.
 */
char *flick_asprintf(const char *fmt, ...);

/*
 * Case-insensitive string comparison.  Since `strcasecmp' is not an ANSI C
 * standard function, we provide this function for ourselves.
 */
int flick_strcasecmp(const char *s1, const char *s2);

/* w_indent.c */
void w_indent(int indent);

/* w_print.c */
void w_putc(char c);
void w_vprintf(const char *format, va_list vl);
void w_printf(const char *format, ...);
void w_set_fh(FILE *out_fh);

/* w_i_print.c */
void w_i_vprintf(int indent, const char *format, va_list vl);
void w_i_printf(int indent, const char *format, ...);

/*
 * We often want to use string literals when building parts of our intermediate
 * representations --- e.g., `cast_new_expr_name("printf")'.  This should be
 * allowed, because we never try to modify or free a string through an IR node
 * reference; in other words, we treat strings in our IRs as `const char *'s.
 *
 * Unfortunately, in ANSI C++, literal strings have type `const char[]'.  The
 * type definitions of our IR nodes always use `char *' because the code for
 * IRs is created for us by `rpcgen', which always maps strings to `char *'s.
 *
 * Since we can't change our node typedefs, we compensate.  We use the special
 * `ir_strlit' macro to import a string into our IRs.  `ir_strlit' accepts a
 * string and produces a `char *', which we may then insert into an IR node
 * without warning from an ANSI C++ compiler.
 *
 * Currently, `ir_strlit' simply casts away the constness of the string, since
 * we promise not to modify the string through the IR reference.  Copying the
 * string is just too expensive, spacewise.
 *
 * Use this macro ONLY to insert a string literal into an `rpcgen'-created IR
 * node.  Don't use this macro to cast away `const'ness in other situations!
 */
#define ir_strlit(str)		((char *) (str))


/*****************************************************************************/

/*
 * This stuff is for standard command line handling.
 */

/* `flag_kind' identifies the types of our flag data. */
enum flag_kind {
	fk_FLAG,
	fk_STRING,
	fk_NUMBER
};
typedef enum flag_kind flag_kind;

/*
 * A `flag_value' contains the data value associated with one occurrence of a
 * command-line flag.
 */
union flag_value {
	int		flag;
	const char *	string;
	int		number;
};
typedef union flag_value flag_value;

/*
 * A `flag_value_seq' contains the data associted with all occurrences of a
 * particular flag on the command line.
 */
struct flag_value_seq {
	unsigned int	len;		/* # of set slots in `values'. */
	unsigned int	max;		/* Allocated length of `values'. */
	flag_value *	values;		/* The data specified for this flag. */
};
typedef struct flag_value_seq flag_value_seq;
void grow_flag_value_seq(flag_value_seq *fvs, int needed);

/*
 * A `flags_in' describes the properties of a command-line flag: its name,
 * type, the number of times that it may appear on the command line, and so on.
 */
struct flags_in {
	char		sng;		/* The short form (e.g., `-o'). */
	const char *	dbl;		/* The long form (e.g., `--opt'). */
	flag_kind	kind;		/* The argument type. */
	int		max_occur;	/* Max # of times flag may occur. */
	flag_value	dfault;		/* Default value. */
	const char *	explain;	/* A string explaining the flag. */
};
typedef struct flags_in flags_in;

/*
 * Special values for `max_occur' above, indicating that the flag may appear
 * any number of times, and how those multiple occurrences should be
 * aggregated.
 */
#define FLAG_UNLIMITED_LIST (-1)	/* Collect the values in a list. */
#define FLAG_UNLIMITED_USE_LAST (-2)	/* Last value wins. */
#define FLAG_UNLIMITED_OR (-3)		/* Logical `or', for flags. */

#define FLAG_UNLIMITED_P(max_occur) ((max_occur) < 0)

/*
 * A `flags_out' describes the parsed command line.
 */
struct flags_out {
	char *			progname;
	flag_value_seq *	flag_seqs;
	int			other_count;
	char **			other;
	int			error;
};
typedef struct flags_out flags_out;

/*
 * This takes a bunch of flags (in the flgs array) and a command line, and
 * builds the flags_out.
 */
flags_out parse_args(int argc, char *argv[], int flag_count, flags_in *flgs);
/*
 * This will print a usage statement automatically.
 */
void print_args_usage(const char *progname,
		      int flag_count, flags_in *flgs,
		      const char *cl_extra, const char *extra);
/*
 * This will pretty-print all the flags --- useful for debugging, etc.
 */
void print_args_flags(flags_out res, int flag_count, flags_in *flgs);

/*
 * This will add the standard flick flags `-v/--version', `-u/--usage', and
 * `-o/--output <filename>' to the array.
 */
void set_def_flags(flags_in *flgs);
/*
 * This will handle version, usage, and errors.
 */
void std_handler(flags_out out, int flag_count, flags_in *flgs,
		 const char *cl_info, const char *info);

/* These are standard locations for the flags to go */
#define STD_FLAGS 4
#define OUTPUT_FILE_FLAG 2

struct fe_flags {
	const char *output;
	const char *cpp_flags;
	int nocpp;
	const char *input;
};
typedef struct fe_flags fe_flags;
/* Since we don't have an FE library, this is here... */
fe_flags front_end_args(int argc, char **argv, const char *info);


/*****************************************************************************/

struct ptr_stack {
	void **ptrs;
	int top;
	int size;
};

struct ptr_stack *create_ptr_stack();
void delete_ptr_stack(struct ptr_stack *stack);
void push_ptr(struct ptr_stack *stack, void *ptr);
void pop_ptr(struct ptr_stack *stack);
void *top_ptr(struct ptr_stack *stack);
int empty_ptr_stack(struct ptr_stack *stack);
int ptr_stack_length(struct ptr_stack *stack);

/* Node struct that is used for
   doubly linked list */
struct list_node {
	struct list_node *succ;
	struct list_node *pred;
};

/* List struct itself, just think of it
   as a combined header and tail node */
struct dl_list {
	struct list_node *head; /* points to the head node */
	struct list_node *tail; /* always == 0 */
	struct list_node *tail_pred; /* points to the tail node */
};

/* list_node/dl_list manipulation functions */
void remove_node(struct list_node *list_node);
void new_list(struct dl_list *list);
void add_head(struct dl_list *list, struct list_node *list_node);
void add_tail(struct dl_list *list, struct list_node *list_node);
struct list_node *rem_head(struct dl_list *list);
struct list_node *rem_tail(struct dl_list *list);
int empty_list(struct dl_list *list);
void insert_node(struct list_node *pred, struct list_node *list_node);

struct h_entry {
	struct h_entry *next;
	const char *name;
};

struct h_table {
	int table_size;
	struct h_entry **entries;
};

struct h_table *create_hash_table(int size);
void delete_hash_table(struct h_table *ht);
void add_entry(struct h_table *ht, struct h_entry *he);
void rem_entry(struct h_table *ht, const char *name);
struct h_entry *find_entry(struct h_table *ht, const char *name);

#ifdef __cplusplus
}
#endif

#endif /* _mom_compiler_h_ */

/* End of file. */

