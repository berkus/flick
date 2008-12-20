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

#include <assert.h>
#include <mom/compiler.h>
#include <mom/c/libcast.h>

#define CHECK_OPT_STRING(str) assert(str != 0);
#define CHECK_STRING(str) assert(str != 0); assert(strlen(str) > 0);

void cast_check_scoped_name(cast_scoped_name *name)
{
	int i;
	
	for (i = 0; i < (signed int)name->cast_scoped_name_len; i++) {
		assert(name->cast_scoped_name_val[i].name);
		cast_check_template_arg_array(&name->cast_scoped_name_val[i].
					      args);
	}
}

void cast_check_opt_scoped_name(cast_scoped_name *name)
{
	if (name->cast_scoped_name_len) {
		cast_check_scoped_name(name);
	}
}

void cast_check_primitive_type(cast_primitive_kind kind, cast_primitive_modifier mod)
{
	if (mod & CAST_MOD_SIGNED) assert(!(mod & CAST_MOD_UNSIGNED));
	if (mod & CAST_MOD_SHORT) assert(!(mod & CAST_MOD_LONG));
	if (mod & CAST_MOD_LONG_LONG) assert(!(mod & CAST_MOD_LONG));
	switch (kind)
	{
		case CAST_PRIM_BOOL:
			assert(!mod);
			break;
		case CAST_PRIM_CHAR:
			assert(!(mod & ~(CAST_MOD_SIGNED | CAST_MOD_UNSIGNED)));
			break;
		case CAST_PRIM_INT:
			break;
		case CAST_PRIM_FLOAT:
			assert(!mod);
			break;
		case CAST_PRIM_DOUBLE:
			assert(!(mod & ~(CAST_MOD_LONG | CAST_MOD_LONG_LONG)));
			break;
		default:
			panic("cast_check_lit_prim: unknown cast_primitive_kind %d", kind);
	}
}

void cast_check_enum(cast_enum_type *et)
{
	unsigned int i;
	
	cast_check_opt_scoped_name(&et->name);
	assert(et->slots.slots_val);
	
	for(i = 0; i < et->slots.slots_len; i++) {
		CHECK_STRING(et->slots.slots_val[i].name);
		cast_check_expr(et->slots.slots_val[i].val);
	}
}

void cast_check_aggregate_type(cast_aggregate_type *atype)
{
	unsigned int i;
	
	cast_check_scoped_name(&atype->name);
	
	cast_check(&atype->scope);
	
	for (i = 0; i < atype->parents.parents_len; i++ ) {
		cast_check_scoped_name(&atype->parents.parents_val[i].name);
	}
}

void cast_check_template_param(cast_template_param *param)
{
	int i;
	
	CHECK_STRING(param->name);
	if (param->default_value)
		cast_check_template_arg(param->default_value);
	switch(param->u.kind) {
	case CAST_TEMP_PARAM_TYPE:
		cast_check_type(param->u.cast_template_param_u_u.type_param);
		break;
	case CAST_TEMP_PARAM_CLASS:
		break;
	case CAST_TEMP_PARAM_TYPENAME:
		break;
	case CAST_TEMP_PARAM_TEMPLATE:
		for (i = 0;
		     i < (signed int)param->u.cast_template_param_u_u.
			 params.params_len;
		     i++) {
			cast_check_template_param(
				param->u.cast_template_param_u_u.
				params.params_val[i]);
		}
		break;
	default:
		panic("cast_check_template_param param kind %d is unknown",
		      param->u.kind);
	}
}

void cast_check_template_arg(cast_template_arg arg)
{
	assert(arg);
	switch(arg->kind) {
	case CAST_TEMP_ARG_NAME:
		cast_check_scoped_name(&arg->cast_template_arg_u_u.name);
		break;
	case CAST_TEMP_ARG_TYPE:
		cast_check_type(arg->cast_template_arg_u_u.type);
		break;
	case CAST_TEMP_ARG_EXPR:
		cast_check_expr(arg->cast_template_arg_u_u.expr);
		break;
	}
}

void cast_check_template_arg_array(cast_template_arg_array *array)
{
	int i;

	for (i = 0; i < (signed int)array->cast_template_arg_array_len; i++) {
		cast_check_template_arg(array->cast_template_arg_array_val[i]);
	}
}

void cast_check_type_array(cast_type_array *array)
{
	int i;

	for (i = 0; i < (signed int)array->cast_type_array_len; i++) {
		cast_check_type(array->cast_type_array_val[i]);
	}
}

void cast_check_expr_array(cast_expr_array *array)
{
	int i;

	for (i = 0; i < (signed int)array->cast_expr_array_len; i++) {
		cast_check_expr(array->cast_expr_array_val[i]);
	}
}

void cast_check_init_array(cast_init_array *array)
{
	int i;

	for (i = 0; i < (signed int)array->cast_init_array_len; i++) {
		cast_check_init(array->cast_init_array_val[i]);
	}
}

void cast_check_func_type(cast_func_type *ftype)
{
	int i;

	for (i = 0; i < (signed int)ftype->params.params_len; i++)
	{
		assert(ftype->params.params_val);
		CHECK_STRING(ftype->params.params_val[i].name);
		cast_check_type(ftype->params.params_val[i].type);
		if (ftype->params.params_val[i].default_value)
			cast_check_init(ftype->params.params_val[i].
					default_value);
	}

	cast_check_type(ftype->return_type);
	cast_check_type_array(&ftype->exception_types);
	cast_check_expr_array(&ftype->initializers);
}

void cast_check_type(cast_type type)
{
	assert(type);
	switch (type->kind)
	{
		case CAST_TYPE_NAME:
		case CAST_TYPE_STRUCT_NAME:
		case CAST_TYPE_UNION_NAME:
		case CAST_TYPE_ENUM_NAME:
		case CAST_TYPE_CLASS_NAME:
		case CAST_TYPE_TYPENAME:
			assert(type->cast_type_u_u.name.cast_scoped_name_val != 0);
			break;
		case CAST_TYPE_PRIMITIVE:
			cast_check_primitive_type(type->cast_type_u_u.primitive_type.kind,
						  type->cast_type_u_u.primitive_type.mod);
			break;
		case CAST_TYPE_AGGREGATE:
			cast_check_aggregate_type(&type->cast_type_u_u.agg_type);
			break;
		case CAST_TYPE_POINTER:
			cast_check_type(type->cast_type_u_u.pointer_type.target);
			break;
		case CAST_TYPE_REFERENCE:
			cast_check_type(type->cast_type_u_u.reference_type.target);
			break;
		case CAST_TYPE_ARRAY:
			cast_check_expr(type->cast_type_u_u.array_type.length);
			cast_check_type(type->cast_type_u_u.array_type.element_type);
			break;
		case CAST_TYPE_FUNCTION:
			cast_check_func_type(&type->cast_type_u_u.func_type);
			break;
		case CAST_TYPE_VOID:
			break;
		case CAST_TYPE_QUALIFIED:
			cast_check_type(type->cast_type_u_u.qualified.actual);
			break;
		case CAST_TYPE_ENUM:
			cast_check_enum(&type->cast_type_u_u.enum_type);
			break;
		case CAST_TYPE_NULL:
			break;
		case CAST_TYPE_TEMPLATE: {
			struct cast_template_type *td;
			struct cast_template_param *tp;
			int i;
			
			td = &type->cast_type_u_u.template_type;
			cast_check_type(td->def);
			for (i = 0;
			     i < (signed int)td->params.params_len;
			     i++) {
				tp = &td->params.params_val[i];
				cast_check_template_param(tp);
			}
			break;
		}
		default:
			panic("cast_check_type: unknown cast_type_kind %d", type->kind);
	}
}

void cast_check_expr(cast_expr expr)
{
	assert(expr);
	switch (expr->kind)
	{
		case CAST_EXPR_CONST_NAME:
		case CAST_EXPR_NAME:
			cast_check_scoped_name(&expr->cast_expr_u_u.name);
			break;
		case CAST_EXPR_LIT_PRIM:
			cast_check_primitive_type(expr->cast_expr_u_u.lit_prim.u.kind,
						  expr->cast_expr_u_u.lit_prim.mod);
			break;
		case CAST_EXPR_LIT_STRING:
			CHECK_OPT_STRING(expr->cast_expr_u_u.lit_string);
			break;
		case CAST_EXPR_CALL:
		{
			cast_expr_call *c = &expr->cast_expr_u_u.call;

			cast_check_expr(c->func);
			cast_check_expr_array(&c->params);
			break;
		}
		case CAST_EXPR_SEL:
			cast_check_expr(expr->cast_expr_u_u.sel.var);
			cast_check_scoped_name(&expr->cast_expr_u_u.
					       sel.member);
			break;
		case CAST_EXPR_UNARY:
			assert(expr->cast_expr_u_u.unary.op);
			cast_check_expr(expr->cast_expr_u_u.unary.expr);
			break;
		case CAST_EXPR_CONST_CAST:
		case CAST_EXPR_DYNAMIC_CAST:
		case CAST_EXPR_REINTERPRET_CAST:
		case CAST_EXPR_STATIC_CAST:
		case CAST_EXPR_CAST:
			cast_check_expr(expr->cast_expr_u_u.cast.expr);
			cast_check_type(expr->cast_expr_u_u.cast.type);
			break;
		case CAST_EXPR_SIZEOF_EXPR:
			cast_check_expr(expr->cast_expr_u_u.sizeof_expr);
			break;
		case CAST_EXPR_SIZEOF_TYPE:
			cast_check_type(expr->cast_expr_u_u.sizeof_type);
			break;
		case CAST_EXPR_BINARY:
		case CAST_EXPR_OP_ASSIGN:
			assert(expr->cast_expr_u_u.binary.op);
			cast_check_expr(expr->cast_expr_u_u.binary.expr[0]);
			cast_check_expr(expr->cast_expr_u_u.binary.expr[1]);
			break;
		case CAST_EXPR_COND:
			cast_check_expr(expr->cast_expr_u_u.cond.test);
			cast_check_expr(expr->cast_expr_u_u.cond.true_expr);
			cast_check_expr(expr->cast_expr_u_u.cond.false_expr);
			break;
		case CAST_EXPR_OP_NEW:
			if (expr->cast_expr_u_u.op_new.placement)
				cast_check_expr(expr->cast_expr_u_u.
						op_new.placement);
			cast_check_type(expr->cast_expr_u_u.op_new.type);
			if (expr->cast_expr_u_u.op_new.init)
				cast_check_init(expr->cast_expr_u_u.
						op_new.init);
			break;
		case CAST_EXPR_OP_DELETE:
			cast_check_expr(expr->cast_expr_u_u.op_delete.expr);
			break;
		case CAST_EXPR_TYPEID_EXPR:
			cast_check_expr(expr->cast_expr_u_u.typeid_expr);
			break;
		case CAST_EXPR_TYPEID_TYPE:
			cast_check_type(expr->cast_expr_u_u.typeid_type);
			break;
		case CAST_EXPR_TYPE:
			cast_check_type(expr->cast_expr_u_u.type_expr);
			break;
		default:
			panic("cast_check_expr: unknown cast_expr_kind %d", expr->kind);
	}
}

void cast_check_block(cast_block *block)
{
	int i;

	cast_check(&block->scope);

	for (i = 0; i < (signed int)block->stmts.stmts_len; i++)
	{
		assert(block->stmts.stmts_val);
		cast_check_stmt(block->stmts.stmts_val[i]);
	}
}

void cast_check_stmt(cast_stmt stmt)
{
	assert(stmt);
	switch (stmt->kind)
	{
		case CAST_STMT_EXPR:
			cast_check_expr(stmt->cast_stmt_u_u.expr);
			break;
		case CAST_STMT_BLOCK:
			cast_check_block(&stmt->cast_stmt_u_u.block);
			break;
		case CAST_STMT_IF:
			cast_check_expr(stmt->cast_stmt_u_u.s_if.test);
			cast_check_stmt(stmt->cast_stmt_u_u.s_if.true_stmt);
			cast_check_stmt(stmt->cast_stmt_u_u.s_if.false_stmt);
			break;
		case CAST_STMT_WHILE:
			cast_check_expr(stmt->cast_stmt_u_u.s_while.test);
			cast_check_stmt(stmt->cast_stmt_u_u.s_while.stmt);
			break;
		case CAST_STMT_DO_WHILE:
			cast_check_expr(stmt->cast_stmt_u_u.s_do_while.test);
			cast_check_stmt(stmt->cast_stmt_u_u.s_do_while.stmt);
			break;
		case CAST_STMT_FOR:
			if (stmt->cast_stmt_u_u.s_for.init)
				cast_check_expr(stmt->cast_stmt_u_u.s_for.init);
			if (stmt->cast_stmt_u_u.s_for.test)
				cast_check_expr(stmt->cast_stmt_u_u.s_for.test);
			if (stmt->cast_stmt_u_u.s_for.iter)
				cast_check_expr(stmt->cast_stmt_u_u.s_for.iter);
			cast_check_stmt(stmt->cast_stmt_u_u.s_for.stmt);
			break;
		case CAST_STMT_SWITCH:
			cast_check_expr(stmt->cast_stmt_u_u.s_switch.test);
			cast_check_stmt(stmt->cast_stmt_u_u.s_switch.stmt);
			break;
		case CAST_STMT_BREAK:
			break;
		case CAST_STMT_CONTINUE:
			break;
		case CAST_STMT_GOTO:
			CHECK_STRING(stmt->cast_stmt_u_u.goto_label);
			break;
		case CAST_STMT_LABEL:
			CHECK_STRING(stmt->cast_stmt_u_u.s_label.label);
			cast_check_stmt(stmt->cast_stmt_u_u.s_label.stmt);
			break;
		case CAST_STMT_CASE:
			cast_check_expr(stmt->cast_stmt_u_u.s_case.label);
			cast_check_stmt(stmt->cast_stmt_u_u.s_case.stmt);
			break;
		case CAST_STMT_DEFAULT:
			cast_check_stmt(stmt->cast_stmt_u_u.default_stmt);
			break;
		case CAST_STMT_RETURN:
			cast_check_expr(stmt->cast_stmt_u_u.return_expr);
			break;
		case CAST_STMT_NULL:
		case CAST_STMT_EMPTY:
			break;
		case CAST_STMT_TRY: {
			struct cast_try *ct;
			int i;

			ct = &stmt->cast_stmt_u_u.try_block;
			cast_check_stmt(ct->block);
			for (i = 0;
			     i < (signed int)ct->handlers.handlers_len;
			     i++) {
				CHECK_STRING(ct->handlers.handlers_val[i].
					     name);
				cast_check_type(ct->handlers.handlers_val[i].
						type);
				cast_check_stmt(ct->handlers.handlers_val[i].
						block);
			}
			break;
		}
		case CAST_STMT_THROW: {
			if (stmt->cast_stmt_u_u.throw_expr)
				cast_check_expr(stmt->cast_stmt_u_u.
						throw_expr);
			break;
		}
		case CAST_STMT_DECL: {
			cast_check(&stmt->cast_stmt_u_u.decl);
			break;
		}
		default:
			panic("cast_check_stmt: unknown cast_stmt_kind %d", stmt->kind);
	}
}

void cast_check_init(cast_init init)
{
	assert(init);
	switch (init->kind)
	{
		case CAST_INIT_EXPR:
		{
			cast_check_expr(init->cast_init_u_u.expr);
			break;
		}
		case CAST_INIT_AGGREGATE:
		{
			cast_check_init_array(&init->cast_init_u_u.subs);
			break;
		}
		case CAST_INIT_CONSTRUCT:
		{
			cast_check_expr_array(&init->cast_init_u_u.exprs);
			break;
		}
		default:
			panic("cast_check_init: unknown cast_init_kind %d", init->kind);
	}
}

void cast_check_def(cast_def *def)
{
	assert(def);
	cast_check_opt_scoped_name(&def->name);
	switch (def->u.kind)
	{
		case CAST_TYPEDEF:
			cast_check_type(def->u.cast_def_u_u.typedef_type);
			break;
		case CAST_TYPE:
			cast_check_type(def->u.cast_def_u_u.type);
			break;
		case CAST_FUNC_DECL:
			cast_check_func_type(&def->u.cast_def_u_u.func_type);
			break;
		case CAST_FUNC_DEF:
			cast_check_func_type(&def->u.cast_def_u_u.func_def.type);
			cast_check_block(&def->u.cast_def_u_u.func_def.block);
			break;
		case CAST_VAR_DECL:
			cast_check_type(def->u.cast_def_u_u.var_type);
			break;
		case CAST_VAR_DEF:
			cast_check_type(def->u.cast_def_u_u.var_def.type);
			if (def->u.cast_def_u_u.var_def.init)
				cast_check_init(def->u.cast_def_u_u.var_def.init);
			break;
		case CAST_DEFINE:
			cast_check_expr(def->u.cast_def_u_u.define_as);
			break;
		case CAST_INCLUDE:
			CHECK_STRING(def->u.cast_def_u_u.include.filename);
			break;
		case CAST_DIRECT_CODE:
			CHECK_STRING(def->u.cast_def_u_u.direct.code_string);
			break;
		case CAST_NAMESPACE:
			cast_check_scoped_name(&def->name);
			cast_check(def->u.cast_def_u_u.new_namespace);
			break;
		case CAST_LINKAGE:
			cast_check_scoped_name(&def->name);
			cast_check(def->u.cast_def_u_u.linkage);
			break;
		case CAST_USING:
			switch(def->u.cast_def_u_u.using_scope) {
			case CAST_USING_NAME:
			case CAST_USING_TYPENAME:
			case CAST_USING_NAMESPACE:
				break;
			default:
				panic("cast_check_def: unknown "
				      "cast_using_kind %d",
				      def->u.cast_def_u_u.using_scope);
			}
			break;
		case CAST_FRIEND:
			cast_check_type(def->u.cast_def_u_u.friend_decl);
			break;
		default:
			panic("cast_check_def: unknown cast_def_kind %d", def->u.kind);
	}
}

void cast_check(cast_scope *scope)
{
	int i;

	for (i = 0; i < (signed int)scope->cast_scope_len; i++)
	{
		assert(scope->cast_scope_val);
		cast_check_def(&scope->cast_scope_val[i]);
	}
}

