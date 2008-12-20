/*
 * Copyright (c) 1995, 1996, 1998, 1999 The University of Utah and
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

#ifdef RPC_HDR
%#ifndef _flick_cast_h
%#define _flick_cast_h
%
%#include <rpc/types.h>
%#include <rpc/xdr.h>
%#include <mom/aoi.h>
#endif

/*
 * Note that a cast represents an abstract syntax tree representation, not a
 * parse tree: it's somewhat higher-level than a parse tree in various
 * respects:
 *   
 *   + Type constructors are represented here forwards,
 *     rather than backwards as they would be in a parse tree.
 *   + There is no representation of grouping parentheses;
 *     all grouping is represented implicitly by the AST nodes.
 *   + There is no concept of declaring multiple identifiers
 *     in a single declaration.
 *   + The -> operator is not present; (*foo).bar is used instead.
 *   + The array index operator is not present; *((array)+(index))
 *     is used instead.
 *
 * However, it's still a tree representation, not a graph:
 * cross-references are by name, not by pointer.
 *
 * Features of C that currently aren't supported at all:
 *   + Bit fields in structures.
 *
 * Cleanups due at some point:
 *   + Get rid of bogus pointer indirections when MOM can fix it.
 *   + Get rid of cast_pointer_type structure
 *   + Rename cast_block to cast_compound?
 *   + Rename cast_def* to cast_decl*
 */

/* These are kludges to get around the limitations of current C language
   mappings.  */
typedef struct cast_type_u *cast_type;
typedef struct cast_expr_u *cast_expr;
typedef struct cast_stmt_u *cast_stmt;
typedef struct cast_init_u *cast_init;
typedef struct cast_template_arg_u *cast_template_arg;
typedef struct cast_template_param *cast_template_param_t;
typedef cast_template_arg cast_template_arg_array<>;
/* Scoped names are C++ only, so for C the len must be 1. */
typedef struct cast_name_s cast_scoped_name<>;
typedef struct cast_def cast_scope<>;
typedef struct cast_def *cast_def_t;
typedef cast_expr cast_expr_array<>;
typedef cast_type cast_type_array<>;
typedef cast_init cast_init_array<>;

enum cast_template_arg_kind
{
	CAST_TEMP_ARG_NAME		= 1,
	CAST_TEMP_ARG_TYPE		= 2,
	CAST_TEMP_ARG_EXPR		= 3
};
union cast_template_arg_u
switch(cast_template_arg_kind kind)
{
	case CAST_TEMP_ARG_NAME:	cast_scoped_name	name;
	case CAST_TEMP_ARG_TYPE:	cast_type		type;
	case CAST_TEMP_ARG_EXPR:	cast_expr		expr;
};
struct cast_name_s
{
	string			name<>;
	cast_template_arg_array	args;		/* C++ Only */
};

/***** TYPE CONSTRUCTORS *****/

enum cast_primitive_kind
{
	CAST_PRIM_CHAR		= 1,
	CAST_PRIM_INT		= 2,
	CAST_PRIM_FLOAT		= 3,
	CAST_PRIM_DOUBLE	= 4,
	CAST_PRIM_BOOL		= 5	/* C++ Only */
};
typedef unsigned cast_primitive_modifier;
const CAST_MOD_SIGNED		= 0x0001;
const CAST_MOD_UNSIGNED		= 0x0002;
const CAST_MOD_SHORT		= 0x0010;
const CAST_MOD_LONG		= 0x0020;
const CAST_MOD_LONG_LONG	= 0x0040;
struct cast_primitive_type
{
	cast_primitive_kind	kind;
	cast_primitive_modifier	mod;
	cast_scoped_name	name;
};

struct cast_enum_field
{
	/* The name of the slot */
	string name<>;

	/* The value (should probably be a literal primitive */
	cast_expr val;
};

struct cast_enum_type
{
	/* Enum tag. */
	cast_scoped_name name;

	/* value and name of each slot */
	cast_enum_field slots<>;
};

struct cast_array_type
{
	/* Constant expression that determines the array's length.
	   Can be omitted if outside of anything or at the end of a structure;
	   means array is variable-length.  */
	cast_expr		length;

	/* Type of each element.  */
	cast_type		element_type;
};

struct cast_pointer_type
{
	/* What the pointer points to.  */
	cast_type		target;
};

/* C++ Only */
struct cast_reference_type
{
	/* What the reference refers to. */
	cast_type		target;
};

typedef unsigned cast_param_spec;
const CAST_PARAM_IMPLICIT	= 0x0001;
const CAST_PARAM_UNUSED		= 0x0002;
struct cast_param
{
	cast_param_spec	spec;
	string		name<>;
	cast_type	type;
	cast_init	default_value;	/* C++ Only */
};

typedef cast_param cast_func_param;
typedef unsigned cast_func_spec;
const CAST_FUNC_VIRTUAL		= 0x0001;
const CAST_FUNC_PURE		= 0x0002;
const CAST_FUNC_INLINE		= 0x0004;
const CAST_FUNC_EXPLICIT	= 0x0008;
const CAST_FUNC_CONST		= 0x0010;
const CAST_FUNC_OPERATOR	= 0x0020;
struct cast_func_type
{
	cast_func_param 	params<>;
	cast_type		return_type;
	cast_func_spec		spec;			/* C++ Only */
	cast_type_array		exception_types;	/* C++ Only */
	cast_expr_array		initializers;		/* C++ Only */
};

/* C++ Only */
typedef unsigned cast_parent_flags;
const CAST_PARENT_PUBLIC	= 0x0001;
const CAST_PARENT_PROTECTED	= 0x0002;
const CAST_PARENT_PRIVATE	= 0x0004;
const CAST_PARENT_VIRTUAL	= 0x0008;
struct cast_parent_spec
{
	cast_parent_flags	flags;
	cast_scoped_name	name;
};

enum cast_aggregate_kind
{
	CAST_AGGREGATE_STRUCT = 1,
	CAST_AGGREGATE_UNION = 2,
	CAST_AGGREGATE_CLASS = 3
};
struct cast_aggregate_type
{
	cast_aggregate_kind	kind;
	cast_scoped_name	name;
	cast_parent_spec	parents<>;	/* C++ Only */
	cast_scope		scope;		/* Adding things besides
						   vars is C++ Only */
};

typedef int cast_type_qualifier;
const CAST_TQ_CONST		= 0x01;
const CAST_TQ_VOLATILE		= 0x02;
struct cast_qualified_type
{
	cast_type_qualifier qual;
	cast_type actual;
};

enum cast_template_param_kind
{
	CAST_TEMP_PARAM_TYPE		= 1,
	CAST_TEMP_PARAM_CLASS		= 2,
	CAST_TEMP_PARAM_TYPENAME	= 3,
	CAST_TEMP_PARAM_TEMPLATE	= 4
};
union cast_template_param_u
switch(cast_template_param_kind kind)
{
	case CAST_TEMP_PARAM_TYPE:	cast_type		type_param;
	case CAST_TEMP_PARAM_CLASS:	void;
	case CAST_TEMP_PARAM_TYPENAME:	void;
	case CAST_TEMP_PARAM_TEMPLATE:	cast_template_param_t	params<>;
};
struct cast_template_param
{
	string			name<>;
	cast_template_param_u	u;
	cast_template_arg	default_value;
};

typedef unsigned cast_template_flags;
const CAST_TEMP_EXPORT		= 0x0001;
struct cast_template_type
{
	cast_template_flags	flags;
	cast_template_param	params<>;
	cast_type		def;
};

enum cast_type_kind
{
	CAST_TYPE_NAME		= 1,
	CAST_TYPE_PRIMITIVE	= 2,
	CAST_TYPE_ARRAY		= 3,
	CAST_TYPE_POINTER	= 4,
	CAST_TYPE_FUNCTION	= 5,
	CAST_TYPE_ENUM		= 6,
	CAST_TYPE_STRUCT_NAME	= 7,
	CAST_TYPE_UNION_NAME	= 8,
	CAST_TYPE_ENUM_NAME	= 9,
	CAST_TYPE_VOID		= 10,
	CAST_TYPE_QUALIFIED	= 11,
	CAST_TYPE_AGGREGATE	= 12,
	CAST_TYPE_NULL		= 13,	/* A Null type, used for
					   constructor return types */
	CAST_TYPE_REFERENCE	= 14,	/* C++ Only */
	CAST_TYPE_CLASS_NAME	= 15,	/* C++ Only */
	CAST_TYPE_TYPENAME	= 16,	/* C++ Only */
	CAST_TYPE_TEMPLATE	= 17	/* C++ Only */
};
union cast_type_u
switch (cast_type_kind kind)
{
	case CAST_TYPE_NAME:		cast_scoped_name	name;
	case CAST_TYPE_PRIMITIVE:	cast_primitive_type	primitive_type;
	case CAST_TYPE_POINTER:		cast_pointer_type	pointer_type;
	case CAST_TYPE_ARRAY:		cast_array_type		array_type;
	case CAST_TYPE_FUNCTION:	cast_func_type		func_type;
	case CAST_TYPE_ENUM:		cast_enum_type		enum_type;
	case CAST_TYPE_STRUCT_NAME:	cast_scoped_name	struct_name;
	case CAST_TYPE_UNION_NAME:	cast_scoped_name	union_name;
	case CAST_TYPE_ENUM_NAME:	cast_scoped_name	enum_name;
	case CAST_TYPE_VOID:		void;
	case CAST_TYPE_NULL:		void;
	case CAST_TYPE_QUALIFIED:	cast_qualified_type	qualified;
	case CAST_TYPE_AGGREGATE:	cast_aggregate_type	agg_type;
	case CAST_TYPE_CLASS_NAME:	cast_scoped_name	class_name;
	case CAST_TYPE_REFERENCE:	cast_reference_type	reference_type;
	case CAST_TYPE_TYPENAME:	cast_scoped_name	typename_name;
	case CAST_TYPE_TEMPLATE:	cast_template_type	template_type;
};


/***** EXPRESSIONS *****/

union cast_lit_prim_u
switch (cast_primitive_kind kind)
{
	case CAST_PRIM_CHAR:	char		c;
	case CAST_PRIM_INT:	long		i; /* XXX long long */
	case CAST_PRIM_FLOAT:	float		f;
	case CAST_PRIM_DOUBLE:	double		d; /* XXX long double */
	case CAST_PRIM_BOOL:	char		b; /* C++ Only */
};
struct cast_lit_prim
{
	cast_primitive_modifier	mod;
	cast_lit_prim_u		u;
};

struct cast_expr_call
{
	cast_expr	func;
	cast_expr_array	params;
};

struct cast_expr_sel
{
	cast_expr		var;
	cast_scoped_name	member;
};

enum cast_unary_op
{
	CAST_UNARY_DEREF	= 1,	/* * (unary) */
	CAST_UNARY_ADDR		= 2,	/* & (unary) */
	CAST_UNARY_NEG		= 3,	/* - (unary) */
	CAST_UNARY_LNOT		= 4,	/* ! (unary) */
	CAST_UNARY_BNOT		= 5,	/* ~ (unary) */
	CAST_UNARY_PRE_INC	= 6,	/* ++foo */
	CAST_UNARY_PRE_DEC	= 7,	/* --foo */
	CAST_UNARY_POST_INC	= 8,	/* foo++ */
	CAST_UNARY_POST_DEC	= 9	/* foo-- */
};
struct cast_unary_expr
{
	cast_unary_op	op;
	cast_expr	expr;
};

struct cast_expr_cast
{
	/* Expression to cast.  */
	cast_expr	expr;

	/* Type to cast it to.  */
	cast_type	type;
};

enum cast_binary_op
{
	CAST_BINARY_MUL		= 1,	/* * (binary) */
	CAST_BINARY_DIV		= 2,	/* / */
	CAST_BINARY_MOD		= 3,	/* % */
	CAST_BINARY_ADD		= 4,	/* + (binary) */
	CAST_BINARY_SUB		= 5,	/* - (binary) */
	CAST_BINARY_SHL		= 6,	/* << */
	CAST_BINARY_SHR		= 7,	/* >> */

	CAST_BINARY_LT		= 8,	/* < */
	CAST_BINARY_GT		= 9,	/* > */
	CAST_BINARY_LE		= 10,	/* <= */
	CAST_BINARY_GE		= 11,	/* >= */
	CAST_BINARY_EQ		= 12,	/* == */
	CAST_BINARY_NE		= 13,	/* != */

	CAST_BINARY_BAND	= 14,	/* & */
	CAST_BINARY_BXOR	= 15,	/* ^ */
	CAST_BINARY_BOR		= 16,	/* | */

	CAST_BINARY_LAND	= 17,	/* && */
	CAST_BINARY_LOR		= 18,	/* || */

	CAST_BINARY_ASSIGN	= 19,	/* = */

	CAST_BINARY_COMMA	= 20	/* , */
};
struct cast_binary_expr
{
	cast_binary_op	op;
	cast_expr	expr[2];
};

struct cast_cond_expr
{
	cast_expr	test;
	cast_expr	true_expr;
	cast_expr	false_expr;
};

/* C++ Only */
struct cast_op_new_expr
{
	cast_expr	placement;
	cast_type	type;
	cast_init	init;
};

/* C++ Only */
struct cast_op_delete_expr
{
	long		array;
	cast_expr	expr;
};

enum cast_expr_kind
{
	CAST_EXPR_NAME		= 1,
	CAST_EXPR_LIT_PRIM	= 2,
	CAST_EXPR_LIT_STRING	= 3,
	CAST_EXPR_CALL		= 4,	/* <expr>(<params>) */
	CAST_EXPR_SEL		= 5,	/* <struct/union>.<member> */
	CAST_EXPR_UNARY		= 6,
	CAST_EXPR_CAST		= 7,	/* (<type>) */
	CAST_EXPR_SIZEOF_EXPR	= 8,	/* sizeof(<expr>) */
	CAST_EXPR_SIZEOF_TYPE	= 9,	/* sizeof(<type>) */
	CAST_EXPR_BINARY	= 10,
	CAST_EXPR_OP_ASSIGN	= 11,	/* <op>= */
	CAST_EXPR_COND		= 12,	/* ? : */
	CAST_EXPR_CONST_NAME	= 13,
	CAST_EXPR_CONST_CAST	= 14,	/* C++ Only */
	CAST_EXPR_DYNAMIC_CAST	= 15,	/* C++ Only */
	CAST_EXPR_REINTERPRET_CAST = 16,	/* C++ Only */
	CAST_EXPR_STATIC_CAST	= 17,	/* C++ Only */
	CAST_EXPR_OP_NEW	= 18,	/* C++ Only */
	CAST_EXPR_OP_DELETE	= 19,	/* C++ Only */
	CAST_EXPR_TYPEID_EXPR	= 20,	/* C++ Only */
	CAST_EXPR_TYPEID_TYPE	= 21,	/* C++ Only */
	CAST_EXPR_TYPE		= 22
};
union cast_expr_u
switch (cast_expr_kind kind)
{
	case CAST_EXPR_NAME:		cast_scoped_name	name;
	case CAST_EXPR_LIT_PRIM:	cast_lit_prim		lit_prim;
	case CAST_EXPR_LIT_STRING:	string			lit_string<>;
	case CAST_EXPR_CALL:		cast_expr_call		call;
	case CAST_EXPR_SEL:		cast_expr_sel		sel;

	case CAST_EXPR_UNARY:		cast_unary_expr		unary;
	case CAST_EXPR_CAST:		cast_expr_cast		cast;
	case CAST_EXPR_SIZEOF_EXPR:	cast_expr		sizeof_expr;
	case CAST_EXPR_SIZEOF_TYPE:	cast_type		sizeof_type;

	case CAST_EXPR_BINARY:		cast_binary_expr	binary;
	case CAST_EXPR_OP_ASSIGN:	cast_binary_expr	op_assign;
	case CAST_EXPR_COND:		cast_cond_expr		cond;
	case CAST_EXPR_CONST_NAME:	cast_scoped_name	const_name;
	case CAST_EXPR_CONST_CAST:	cast_expr_cast		c_cast;
	case CAST_EXPR_DYNAMIC_CAST:	cast_expr_cast		d_cast;
	case CAST_EXPR_REINTERPRET_CAST: cast_expr_cast		r_cast;
	case CAST_EXPR_STATIC_CAST:	cast_expr_cast		s_cast;
	case CAST_EXPR_OP_NEW:		cast_op_new_expr	op_new;
	case CAST_EXPR_OP_DELETE:	cast_op_delete_expr	op_delete;
	case CAST_EXPR_TYPEID_EXPR:	cast_expr		typeid_expr;
	case CAST_EXPR_TYPEID_TYPE:	cast_type		typeid_type;
	case CAST_EXPR_TYPE:		cast_type		type_expr;
};

/***** STATEMENTS *****/

typedef unsigned cast_block_flags;
/* The statements in the block are in reverse order */
const CAST_BLOCK_REVERSE	= 0x00000001;
/* The statements in the block should be treated as if they were
   inlined with the rest of the statements in the parent block.
   (e.g. there is no '{' when printed. */
const CAST_BLOCK_INLINE		= 0x00000002;

struct cast_block
{
	/* Variable declarations and such at beginning of block.  */
	cast_scope		scope;

	/* Statements following decls, but preceeding ``regular'' code.
	   (Mainly useful for initializing temporary vars, etc.) */
	cast_stmt		initials<>;
	
	/* Statements following them (``regular'' code).  */
	cast_stmt		stmts<>;
	cast_block_flags	flags;
};

struct cast_if
{
	cast_expr		test;
	cast_stmt		true_stmt;
	cast_stmt		false_stmt;	/* optional */
};

struct cast_while
{
	cast_expr		test;
	cast_stmt		stmt;
};

struct cast_for
{
	cast_expr		init;	/* optional */
	cast_expr		test;	/* optional */
	cast_expr		iter;	/* optional */
	cast_stmt		stmt;
};

struct cast_switch
{
	cast_expr		test;
	cast_stmt		stmt;
};

struct cast_label
{
	string			label<>;
	cast_stmt		stmt;
	int			users;
};

struct cast_case
{
	cast_expr		label;
	cast_stmt		stmt;
};

/* C++ Only */
struct cast_catch
{
	cast_type		type;
	string			name<>;
	cast_stmt		block;
};

/* C++ Only */
struct cast_try
{
	cast_stmt		block;
	cast_catch		handlers<>;
};

typedef string			cast_handler_arg<>;
struct cast_handler
{
	string			name<>;
	cast_handler_arg	args<>;
};

enum cast_stmt_kind
{
	CAST_STMT_EXPR		= 1,
	CAST_STMT_BLOCK		= 2,
	CAST_STMT_IF		= 3,
	CAST_STMT_WHILE		= 4,
	CAST_STMT_DO_WHILE	= 5,
	CAST_STMT_FOR		= 6,
	CAST_STMT_SWITCH	= 7,
	CAST_STMT_BREAK		= 8,
	CAST_STMT_CONTINUE	= 9,
	CAST_STMT_GOTO		= 10,
	CAST_STMT_LABEL		= 11,
	CAST_STMT_CASE		= 12,
	CAST_STMT_DEFAULT	= 13,
	CAST_STMT_RETURN	= 14,
        CAST_STMT_TEXT		= 15,
	CAST_STMT_NULL		= 16,
	/* Empty means nothing is printed compared
	   to the ';' that goes with a CAST_STMT_NULL */
	CAST_STMT_EMPTY         = 17,
	CAST_STMT_TRY		= 18,	/* C++ Only */
	CAST_STMT_THROW		= 19,	/* C++ Only */
	CAST_STMT_DECL		= 20,	/* C++ Only */
	CAST_STMT_HANDLER	= 21
};
union cast_stmt_u
switch (cast_stmt_kind kind)
{
	case CAST_STMT_EXPR:		cast_expr		expr;
	case CAST_STMT_BLOCK:		cast_block		block;
	case CAST_STMT_IF:		cast_if			s_if;
	case CAST_STMT_WHILE:		cast_while		s_while;
	case CAST_STMT_DO_WHILE:	cast_while		s_do_while;
	case CAST_STMT_FOR:		cast_for		s_for;
	case CAST_STMT_SWITCH:		cast_switch		s_switch;
	case CAST_STMT_BREAK:		void;
	case CAST_STMT_CONTINUE:	void;
	case CAST_STMT_GOTO:		string			goto_label<>;
	case CAST_STMT_LABEL:		cast_label		s_label;
	case CAST_STMT_CASE:		cast_case		s_case;
	case CAST_STMT_DEFAULT:		cast_stmt		default_stmt;
								/* optional: */
	case CAST_STMT_RETURN:		cast_expr		return_expr;
	case CAST_STMT_TEXT:		string			text<>;
	case CAST_STMT_NULL:		void;
	case CAST_STMT_EMPTY:		void;
	case CAST_STMT_TRY:		cast_try		try_block;
	case CAST_STMT_THROW:		cast_expr		throw_expr;
	case CAST_STMT_DECL:		cast_scope		decl;
	case CAST_STMT_HANDLER:		cast_handler		handler;
};



/***** DECLARATIONS *****/

struct cast_func_def
{
	cast_func_type	type;
	cast_block	block;
};

enum cast_init_kind
{
	CAST_INIT_EXPR		= 1,
	CAST_INIT_AGGREGATE	= 2,
	CAST_INIT_CONSTRUCT	= 3	/* C++ Only */
};
union cast_init_u
switch (cast_init_kind kind)
{
	case CAST_INIT_EXPR:		cast_expr	expr;
	case CAST_INIT_AGGREGATE:	cast_init_array	subs;
	case CAST_INIT_CONSTRUCT:	cast_expr_array	exprs;
};
struct cast_var_def
{
	cast_type	type;

	/* Optional - if present, specifies the variable's initializer.  */
	cast_init	init;
};

struct cast_include
{
	string		filename<>;
	bool		system_only;
};

struct cast_direct
{
	string		code_string<>;
};

enum cast_storage_class
{
	CAST_SC_NONE		= 0,
	CAST_SC_AUTO		= 1,
	CAST_SC_STATIC		= 2,
	CAST_SC_EXTERN		= 3,
	CAST_SC_REGISTER	= 4,
	CAST_SC_MUTABLE		= 5	/* C++ Only */
};

/* C++ Only */
enum cast_using_kind
{
	CAST_USING_NAME		= 1,
	CAST_USING_TYPENAME	= 2,
	CAST_USING_NAMESPACE	= 3
};

/* A pres_c_def directly produces a C definition.
   One doesn't necessarily correspond directly to an itype.  */
enum cast_def_kind
{
	CAST_TYPEDEF	= 0x00000001,
	CAST_TYPE	= 0x00000002,
	CAST_FUNC_DECL	= 0x00000004,
	CAST_FUNC_DEF	= 0x00000008,
	CAST_VAR_DECL	= 0x00000010,
	CAST_VAR_DEF	= 0x00000020,
	CAST_DEFINE	= 0x00000040,	/* #define XXX */
	CAST_INCLUDE	= 0x00000080,	/* #include XXX */
	CAST_DIRECT_CODE = 0x00000100,
	CAST_NAMESPACE	= 0x00000200,	/* C++ Only */
	CAST_USING	= 0x00000400,	/* C++ Only */
	CAST_LINKAGE	= 0x00000800,	/* C++ Only */
	CAST_FRIEND	= 0x00001000	/* C++ Only */
};

union cast_def_u
switch (cast_def_kind kind)
{
	case CAST_TYPEDEF:		cast_type		typedef_type;
	case CAST_TYPE:			cast_type		type;
	case CAST_FUNC_DECL:		cast_func_type		func_type;
	case CAST_FUNC_DEF:		cast_func_def		func_def;
	case CAST_VAR_DECL:		cast_type		var_type;
	case CAST_VAR_DEF:		cast_var_def		var_def;
	case CAST_DEFINE:		cast_expr		define_as;
	case CAST_INCLUDE:		cast_include		include;
	case CAST_DIRECT_CODE:		cast_direct		direct;
	case CAST_NAMESPACE:		cast_scope		*new_namespace;
	case CAST_USING:		cast_using_kind		using_scope;
	case CAST_LINKAGE:		cast_scope		*linkage;
	case CAST_FRIEND:		cast_type		friend_decl;
};

/* C++ Only */
enum cast_def_protection {
     CAST_PROT_NONE		= 0,
     CAST_PROT_PUBLIC		= 1,
     CAST_PROT_PROTECTED	= 2,
     CAST_PROT_PRIVATE		= 3
};

struct cast_def
{
	/* C identifier of type (or whatever) to be declared/defined.
	   Length 0 if none.  */
	cast_scoped_name	name;

	/* Storage class for the declaration/definition.  */
	cast_storage_class	sc;
	
	/* Description of definition.  */
	cast_def_u		u;
	
	/* The data_channel this definition is a part of */
	data_channel_index	channel;
	
	cast_def_protection	protection;	/* C++ Only */
};

typedef cast_scope cast_1;

#ifdef RPC_HDR
%#endif /* _flick_cast_h */
#endif

/* End of file. */
