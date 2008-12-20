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

#ifndef _mom_libcast_h_
#define _mom_libcast_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <mom/compiler.h>
#include <mom/cast.h>

struct cast_handler_entry {
	struct h_entry he;
	int (*c_func)(int indent, struct cast_handler *ch);
};

enum {
	CAST_C,
	CAST_CXX
};
extern int cast_language;
extern const char *cast_namespace_str;
extern struct h_table *cast_handler_table;
extern meta *cast_meta;

void init_cast();

/* Scopes */
cast_scope cast_new_scope(int defs);
cast_def_t cast_new_def_t(cast_scoped_name name,
			  cast_storage_class sc,
			  cast_def_kind kind,
			  data_channel_index channel,
			  cast_def_protection prot);
int cast_add_def(cast_scope *scope,
		 cast_scoped_name name,
		 cast_storage_class sc,
		 cast_def_kind kind,
		 data_channel_index channel,
		 cast_def_protection prot);

int cast_find_def(cast_scope **scope,
		  const cast_scoped_name name,
		  int kind);
int cast_find_def_pos(cast_scope **scope,
		      int scope_pos,
		      const cast_scoped_name name,
		      int kind);
cast_type cast_find_typedef_type(cast_scope *scope, cast_type name_ctype);
cast_enum_type *cast_find_enum_type(cast_scope *scope, cast_type ctype);
cast_aggregate_type *cast_find_struct_type(cast_scope *scope, cast_type ctype);
cast_aggregate_type *cast_find_union_type(cast_scope *scope, cast_type ctype);
cast_aggregate_type *cast_find_class_type(cast_scope *scope, cast_type ctype);

/* Node modifiers */
int cast_func_add_param(cast_func_type *func);

void cast_block_add_var(cast_block *b,
			const char *name, cast_type type,
			cast_init init, cast_storage_class sc);
void cast_block_add_stmt(cast_block *b, cast_stmt st);
void cast_block_add_initial_stmt(cast_block *b, cast_stmt st);
void cast_block_absorb_stmt(cast_block *b, cast_stmt st);

/* C++ Only */
int cast_template_add_param(struct cast_template_type *template_decl,
			    cast_template_param_kind kind,
			    const char *name,
			    cast_template_arg default_value);
void cast_try_add_handler(cast_stmt stmt,
			  cast_type type,
			  const char *name,
			  cast_stmt block);
void cast_class_add_parent(cast_type type,
			   cast_parent_flags flags,
			   cast_scoped_name name);

/* Routines to cons up various common cast nodes.  */

/* Names */
void cast_add_scope_name(cast_scoped_name *scname,
			 const char *name,
			 cast_template_arg_array args);
void cast_del_scope_name(cast_scoped_name *scname);
void cast_prepend_scope_name(cast_scoped_name *scname,
			     const char *name,
			     cast_template_arg_array args);
int cast_cmp_scoped_names(const cast_scoped_name *scn1,
			  const cast_scoped_name *scn2);
cast_scoped_name cast_new_scoped_name(const char *name, ...);
cast_scoped_name cast_set_scoped_name(cast_scoped_name *scname,
				      const char *name,
				      cast_template_arg_array args,
				      ...);
int cast_scoped_name_is_empty(cast_scoped_name *scname);
void cast_strip_name(cast_scoped_name *scope,
		     cast_scoped_name *name);
cast_scoped_name cast_copy_scoped_name(cast_scoped_name *name);

/* Types */
extern cast_type_array null_type_array;

cast_type_array cast_set_type_array(cast_type_array *array,
				    cast_type type, ...);
int cast_add_type_array_value(cast_type_array *array, cast_type type);
cast_type cast_new_type_name(const char *name);
cast_type cast_new_type_scoped_name(cast_scoped_name name);
cast_type cast_new_prim_type(cast_primitive_kind kind,
			     cast_primitive_modifier mod);
cast_type cast_new_prim_alias(cast_primitive_kind kind,
			      cast_primitive_modifier mod,
			      const cast_scoped_name name);
cast_type cast_new_aggregate_type(cast_aggregate_kind kind, int defs);
cast_type cast_new_struct_type(int slots);
cast_type cast_new_union_type(int cases);
cast_type cast_new_class_type(int defs); /* C++ Only */
cast_type cast_new_array_type(cast_expr length, cast_type element_type);
cast_type cast_new_pointer_type(cast_type target_type);
cast_type cast_new_reference_type(cast_type target_type); /* C++ Only */
cast_type cast_new_template_type(cast_type def_type);
cast_type cast_new_function_type(cast_type return_type, int params);
void cast_init_function_type(cast_func_type *func, int params);
cast_type cast_new_qualified_type(cast_type actual, cast_type_qualifier qual);
cast_type cast_new_type(cast_type_kind kind);

/* Expressions */
extern cast_expr_array null_expr_array;

cast_expr_array cast_set_expr_array(cast_expr_array *array,
				    cast_expr expr, ...);
int cast_add_expr_array_value(cast_expr_array *array, cast_expr expr);
cast_expr cast_new_expr_name(const char *name);
cast_expr cast_new_expr_type(cast_type type);
cast_expr cast_new_expr_scoped_name(cast_scoped_name name);
cast_expr cast_new_expr_const_name(const char *name);
cast_expr cast_new_expr_const_scoped_name(cast_scoped_name name);
cast_expr cast_new_expr_lit_char(char val, cast_primitive_modifier mod);
cast_expr cast_new_expr_lit_int(long val, cast_primitive_modifier mod);
cast_expr cast_new_expr_lit_float(float val);
cast_expr cast_new_expr_lit_double(double val, cast_primitive_modifier mod);
cast_expr cast_new_expr_lit_string(const char *str);
cast_expr cast_new_expr_lit_bool(char val); /* C++ Only */
cast_expr cast_new_expr_assign_to_zero(cast_expr expr, cast_type ctype,
				       cast_scope *scope);
cast_expr cast_new_unary_expr(cast_unary_op op, cast_expr sub);
cast_expr cast_new_binary_expr(cast_binary_op op, cast_expr a, cast_expr b);
cast_expr cast_new_expr_op_assign(cast_binary_op op, cast_expr a, cast_expr b);
cast_expr cast_new_expr_op_new(cast_expr placement,
			       cast_type type,
			       cast_init init); /* C++ Only */
cast_expr cast_new_expr_op_delete(int array, cast_expr expr); /* C++ Only */
cast_expr cast_new_expr_assign(cast_expr to, cast_expr from);
cast_expr cast_new_expr_call(cast_expr func, int params);
cast_expr cast_new_expr_call_0(cast_expr func);
cast_expr cast_new_expr_call_1(cast_expr func,
			       cast_expr p0);
cast_expr cast_new_expr_call_2(cast_expr func,
			       cast_expr p0, cast_expr p1);
cast_expr cast_new_expr_call_3(cast_expr func,
			       cast_expr p0, cast_expr p1, cast_expr p2);
cast_expr cast_new_expr_call_4(cast_expr func,
			       cast_expr p0, cast_expr p1, cast_expr p2,
			       cast_expr p3);
cast_expr cast_new_expr_call_5(cast_expr func,
			       cast_expr p0, cast_expr p1, cast_expr p2,
			       cast_expr p3, cast_expr p4);
cast_expr cast_new_expr_sel(cast_expr var_expr, cast_scoped_name member);
cast_expr cast_new_expr_sizeof_expr(cast_expr expr);
cast_expr cast_new_expr_sizeof_type(cast_type type);
cast_expr cast_new_expr_typeid_expr(cast_expr expr); /* C++ Only */
cast_expr cast_new_expr_typeid_type(cast_type type); /* C++ Only */
cast_expr cast_new_expr_cast(cast_expr expr, cast_type type);
cast_expr cast_new_expr_cxx_cast(cast_expr_kind kind,
				 cast_type type,
				 cast_expr expr); /* C++ Only */
cast_expr cast_new_expr_cond(cast_expr test,
			     cast_expr true_expr, cast_expr false_expr);
cast_expr cast_new_expr(cast_expr_kind kind);

/* Statements */
cast_stmt cast_new_stmt_expr(cast_expr expr);
cast_stmt cast_new_block(int defs, int stmts);
cast_stmt cast_new_break(void);
cast_stmt cast_new_continue(void);
cast_stmt cast_new_label(const char *name, cast_stmt sub_stmt);
cast_stmt cast_new_case(cast_expr expr, cast_stmt sub_stmt);
cast_stmt cast_new_default(cast_stmt sub_stmt);
cast_stmt cast_new_if(cast_expr test,
		      cast_stmt true_stmt,
		      cast_stmt false_stmt);
cast_stmt cast_new_switch(cast_expr test, cast_stmt body);
cast_stmt cast_new_while(cast_expr test, cast_stmt body);
cast_stmt cast_new_do(cast_expr test, cast_stmt body);
cast_stmt cast_new_for(cast_expr init,
		       cast_expr test,
		       cast_expr iter,
		       cast_stmt body);
cast_stmt cast_new_goto(const char *label);
cast_stmt cast_new_return(cast_expr return_val);
cast_stmt cast_new_try(cast_stmt block); /* C++ Only */
cast_stmt cast_new_throw(cast_expr expr); /* C++ Only */
cast_stmt cast_new_decl(int defs); /* C++ Only */
cast_stmt cast_new_stmt(cast_stmt_kind kind);
cast_stmt cast_new_text(const char *text);
cast_stmt cast_new_stmt_handler(const char *handler_name, ...);

/* This will find a label within the given block */
int cast_find_label(cast_block *block, const char *label);

/* Init */
extern cast_init_array null_init_array;

cast_init_array cast_set_init_array(cast_init_array *array,
				    cast_init init, ...);
int cast_add_init_array_value(cast_init_array *array, cast_init init);
cast_init cast_new_init_aggregate(cast_init_array array);
cast_init cast_new_init_construct(cast_expr_array array); /* C++ Only */
cast_init cast_new_init_expr(cast_expr expr);
cast_init cast_new_init(cast_init_kind kind);

/* Templates (C++ Only) */
extern cast_template_arg_array null_template_arg_array;

cast_template_arg_array cast_set_template_arg_array(
	cast_template_arg_array *array,
	cast_template_arg template_arg,
	...);
int cast_add_template_arg_array_value(cast_template_arg_array *array,
				      cast_template_arg template_arg);
cast_template_arg cast_new_template_arg_expr(cast_expr expr);
cast_template_arg cast_new_template_arg_type(cast_type type);
cast_template_arg cast_new_template_arg_name(cast_scoped_name name);
cast_template_arg cast_new_template_arg(cast_template_arg_kind kind);

/* Check various cast constructs for consistency and completeness, using assert().  */
void cast_check_scoped_name(cast_scoped_name *name);
void cast_check_opt_scoped_name(cast_scoped_name *name);
void cast_check_primitive_type(cast_primitive_kind kind,
			       cast_primitive_modifier mod);
void cast_check_aggregate_type(cast_aggregate_type *atype);
void cast_check_func_type(cast_func_type *ftype);
void cast_check_type(cast_type type);
void cast_check_expr(cast_expr expr);
void cast_check_block(cast_block *block);
void cast_check_stmt(cast_stmt stmt);
void cast_check_init(cast_init init);
void cast_check_def(cast_def *def);
void cast_check_enum(cast_enum_type *et);
void cast_check_template_arg(cast_template_arg arg);
void cast_check_template_param(cast_template_param *param);
void cast_check_template_arg_array(cast_template_arg_array *array);
void cast_check_init_array(cast_init_array *array);
void cast_check_expr_array(cast_expr_array *array);
void cast_check_type_array(cast_type_array *array);
void cast_check(cast_scope *scope);


int cast_expr_const(cast_expr expr);

/* Translate an aoi_const to a cast_expr */
cast_expr aoi_const_to_cast_expr(aoi_const c);
	
/* Routines to deep-compare cast trees to see if they are equivalent.
   The value returned is ordered, as with strcmp;
   thus it is possible to use this routine to sort cast trees.  */
int cast_cmp_type(cast_type a, cast_type b);
int cast_cmp_expr(cast_expr a, cast_expr b);
int cast_cmp_init(cast_init a, cast_init b);


/* Routine to dereference any CAST_TYPE_NAME ctype references. */
cast_type cast_dename(cast_1 *cast, cast_type c);

/* Routines to output cast trees as C code.  */
extern cast_scoped_name null_scope_name;
extern cast_scoped_name empty_scope_name;
void cast_w_type(cast_scoped_name name, cast_type d, int indent);
void cast_w_func_type(cast_scoped_name name, cast_func_type *fd, int indent);
void cast_w_def(cast_def_t def, int indent);
void cast_w_scope(cast_scope *scope, int indent);
void cast_w_scoped_name(cast_scoped_name *name);
void cast_w_template_arg(cast_template_arg arg, int indent);
void cast_w_template_param(cast_template_param *param);
void cast_w_expr_noncomma(cast_expr expr, int indent);
void cast_w_expr(cast_expr expr, int indent);

void cast_w_init(cast_init init, int indent);

void cast_w_block(cast_block *block, int indent);
void cast_w_stmt(cast_stmt st, int indent);


#ifdef __cplusplus
}
#endif

#endif /* _mom_libcast_h_ */

/* End of file. */

