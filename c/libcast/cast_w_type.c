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

#include <assert.h>
#include <stdio.h>
#include <mom/compiler.h>
#include <mom/c/libcast.h>

typedef struct qual_node
{
	cast_qualified_type *this_qual;
	struct qual_node *next;
} qual_node;

typedef struct type_node
{
	struct type_node *next;
	void (*print_prefix)(struct type_node *n, int indent);
	void (*print_suffix)(struct type_node *n, int indent);
	int precedence; /* 0=*, 1=(),[] */
	qual_node *quals;
	void *print_func_data;
} type_node;

/*****/

static void type(cast_scoped_name name,
		 cast_type type, type_node *on, qual_node *quals,
		 int indent);


static void print_quals(qual_node *quals)
{
	if (quals) {
		print_quals(quals->next);
		if (quals->this_qual->qual & CAST_TQ_CONST)
			w_printf("const ");
		if (quals->this_qual->qual & CAST_TQ_VOLATILE)
			w_printf("volatile ");
	}
}

static void type_list(cast_scoped_name name,
		      type_node *n,
		      int last_prec,
		      int indent)
{
	if (n) {
		if (n->precedence < last_prec)
			w_putc('(');
		if (n->print_prefix)
			(*n->print_prefix)(n, indent);
		print_quals(n->quals);
		type_list(name, n->next, n->precedence, indent);
		if (n->print_suffix)
			(*n->print_suffix)(n, indent);
		if (n->precedence < last_prec)
			w_putc(')');
	}
	else
	{
		if (name.cast_scoped_name_len)
			cast_w_scoped_name(&name);
	}
}

static void func_type_print_suffix(type_node *n, int indent)
{
	cast_func_type *fd = n->print_func_data;
	
	w_putc('(');
	if (fd->params.params_len > 0)
	{
		int i;
		/*
		 * Count the number of ``real'' (non-implicit) parameters.
		 * We use this to figure out when/where to place commas
		 * when printing out the parameter list.
		 */
		int real_params = 0;
		for (i = 0; i < (signed int)fd->params.params_len; i++)
		{
			if (!(fd->params.params_val[i].spec
			      & CAST_PARAM_IMPLICIT))
				real_params++;
		}
		/* Write the parameter list. */
		for (i = 0; i < (signed int)fd->params.params_len; i++)
		{
			/* Place comments around ``implicit'' parameters. */
			if (fd->params.params_val[i].spec &
			    CAST_PARAM_IMPLICIT) {

				if (real_params == 0)
					w_printf("/* , ");
				else
					w_printf("/* ");
			} else
				real_params--;
			
			type((fd->params.params_val[i].name &&
			      !(fd->params.params_val[i].spec &
				CAST_PARAM_UNUSED)) ?
			     cast_new_scoped_name(fd->params.params_val[i].
						  name, NULL) :
			     null_scope_name,
			     fd->params.params_val[i].type,
			     0, 0,
			     indent);
			if (fd->params.params_val[i].default_value) {
				cast_w_init(fd->params.params_val[i].
					    default_value,
					    0);
			}
			if (real_params > 0) w_printf(", ");
			/* Place comments around ``implicit'' parameters. */
			if (fd->params.params_val[i].spec &
			    CAST_PARAM_IMPLICIT) {
				if (real_params == 0)
					w_printf(" */");
				else
					w_printf("*/ ");
			}
		}
	}
	else if( cast_language == CAST_C )
		w_printf("void");
	w_putc(')');
	if (fd->spec & CAST_FUNC_CONST)
		w_printf(" const");
	if (fd->exception_types.cast_type_array_len) {
		int i;
		
		w_printf(" throw(");
		for (i = 0;
		     i < (signed int)fd->exception_types.cast_type_array_len;
		     i++) {
			if (i > 0) w_printf(", ");
			type(null_scope_name,
			     fd->exception_types.cast_type_array_val[i],
			     0, 0,
			     indent);
		}
		w_putc(')');
	}
	if (fd->initializers.cast_expr_array_len) {
		int i;

		w_printf(" :\n");
		for (i = 0;
		     i < (signed int)fd->initializers.cast_expr_array_len;
		     i++) {
			if (i > 0)
				w_printf(",\n");
			w_indent(indent+1);
			cast_w_expr(fd->initializers.cast_expr_array_val[i],
				    indent+2);
		}
	}
	if (fd->spec & CAST_FUNC_PURE)
		w_printf(" = 0");
}

static void func_type(cast_scoped_name name,
		      cast_func_type *fd, type_node *on, qual_node *quals,
		      int indent)
{
	type_node nn = {0 /* Should be `on': initialized below */,
			0,
			func_type_print_suffix,
			1,
			0 /* Should be `quals': initialized below. */,
			0 /* Should be `fd': initialized below. */
			};
	
	nn.next = on;
	nn.quals = quals;
	nn.print_func_data = fd;
	
	/*****/
	
	assert(fd->return_type);
	if (fd->spec & CAST_FUNC_INLINE)
		w_printf("inline ");
	if (fd->spec & CAST_FUNC_INLINE)
		w_printf("explicit ");
	if (fd->spec & CAST_FUNC_VIRTUAL)
		w_printf("virtual ");
	if( (fd->spec & CAST_FUNC_OPERATOR) ) {
		cast_w_scoped_name(&name);
		w_printf(" ");
		type(null_scope_name, fd->return_type, &nn, 0, indent);
	}
	else
		type(name, fd->return_type, &nn, 0, indent);
}

static void array_type_print_suffix(type_node *n, int indent)
{
	w_putc('[');
	cast_w_expr(n->print_func_data, 0);
	w_putc(']');
}

static void pointer_type_print_prefix(type_node *n, int indent)
{
	w_putc('*');
}

static void reference_type_print_prefix(type_node *n, int indent)
{
	w_putc('&');
}


static void type(cast_scoped_name name,
		 cast_type d, type_node *on, qual_node *quals,
		 int indent)
{
	assert(d != 0);
	switch (d->kind)
	{
		case CAST_TYPE_NAME:
		case CAST_TYPE_STRUCT_NAME:
		case CAST_TYPE_UNION_NAME:
		case CAST_TYPE_ENUM_NAME:
		case CAST_TYPE_CLASS_NAME:
		{
			print_quals(quals);
			w_printf("%s",
				 ((d->kind == CAST_TYPE_NAME) ?
				  "" :
				  (d->kind == CAST_TYPE_CLASS_NAME) ?
				  "class " :
				  (d->kind == CAST_TYPE_TYPENAME) ?
				  "typename " :
				  (d->kind == CAST_TYPE_STRUCT_NAME) ?
				  "struct " :
				  (d->kind == CAST_TYPE_UNION_NAME) ?
				  "union " :
				  (d->kind == CAST_TYPE_ENUM_NAME) ?
				  "enum " :
				  (panic("Unrecognized type name in `type'"),
				   "")));
			cast_w_scoped_name(&d->cast_type_u_u.
					   name);
			w_printf(" ");
			type_list(name, on, 0, indent);
			break;
		}
		case CAST_TYPE_PRIMITIVE:
		{
			cast_primitive_type *pd = &d->cast_type_u_u.primitive_type;
			print_quals(quals);
			if (cast_scoped_name_is_empty(&pd->name))
				w_printf("%s%s%s ",
					 pd->mod & CAST_MOD_SIGNED ? "signed "
					 : pd->mod & CAST_MOD_UNSIGNED ? "unsigned "
					 : "",
					 pd->mod & CAST_MOD_LONG_LONG ? "long long "
					 : pd->mod & CAST_MOD_LONG ? "long "
					 : pd->mod & CAST_MOD_SHORT ? "short "
					 : "",
					 pd->kind == CAST_PRIM_CHAR ? "char"
					 : pd->kind == CAST_PRIM_INT ? "int"
					 : pd->kind == CAST_PRIM_FLOAT ? "float"
					 : pd->kind == CAST_PRIM_DOUBLE ? "double"
					 : pd->kind == CAST_PRIM_BOOL ? "bool"
					 : "???");
			else {
				cast_w_scoped_name(&pd->name);
				w_printf(" ");
			}
			type_list(name, on, 0, indent);
			break;
		}
		case CAST_TYPE_AGGREGATE:
		{
			cast_aggregate_type *ad = &d->cast_type_u_u.agg_type;
			cast_parent_spec *ps;
			char *type_kind;
			int i;

			type_kind = (ad->kind == CAST_AGGREGATE_STRUCT ?
				     "struct" :
				     ad->kind == CAST_AGGREGATE_UNION ?
				     "union" :
				     ad->kind == CAST_AGGREGATE_CLASS ?
				     "class" :
				     "???");
			print_quals(quals);
			w_printf("%s ", type_kind);
			if (!cast_scoped_name_is_empty(&(ad->name))) {
				cast_w_scoped_name(&ad->name);
				w_printf(" ");
			}
			if (ad->parents.parents_len) {
				w_printf(": ");
				for (i = 0;
				     i < (signed int) ad->parents.parents_len;
				     i++) {
					ps = &(ad->parents.parents_val[i]);
					w_printf("%s%s%s",
						 /* Arg 1. */
						 (i == 0 ? "" : ", "),
						 /* Arg 2. */
						 ((ps->flags
						   & CAST_PARENT_PUBLIC) ?
						  "public " :
						  (ps->flags &
						   CAST_PARENT_PROTECTED) ?
						  "protected " :
						  (ps->flags
						   & CAST_PARENT_PRIVATE) ?
						  "private " :
						  ""),
						 /* Arg 3. */
						 ((ps->flags
						  & CAST_PARENT_VIRTUAL) ?
						  "virtual " : ""));
					cast_w_scoped_name(&ps->name);
				}
				w_printf(" ");
			}
			w_printf("{\n");
			cast_w_scope(&ad->scope, indent+1);
			w_indent(indent);
			w_printf("} ");
			type_list(name, on, 0, indent);
			break;
		}
		case CAST_TYPE_ARRAY:
		{
			cast_array_type *ad = &d->cast_type_u_u.array_type;
			type_node nn = {0 /* `on', initialized below. */,
					0,
					array_type_print_suffix,
					1,
					0 /* `quals', initialized below. */,
					0 /* `ad->length', init'ed below. */
					};
			
			nn.next = on;
			nn.quals = quals;
			nn.print_func_data = ad->length;
			
			type(name, ad->element_type, &nn, 0, indent);
			break;
		}
		case CAST_TYPE_POINTER:
		{
			type_node nn = {0 /* `on', initialized below. */,
					pointer_type_print_prefix,
					0,
					0,
					0 /* `quals', initialized below. */,
					0};
			
			nn.next = on;
			nn.quals = quals;
			
			type(name, d->cast_type_u_u.pointer_type.target, &nn,
			     0, indent);
			break;
		}
		case CAST_TYPE_REFERENCE:
		{
			type_node nn = {0 /* `on', initialized below. */,
					reference_type_print_prefix,
					0,
					0,
					0 /* `quals', initialized below. */,
					0};
			
			nn.next = on;
			nn.quals = quals;
			
			type(name, d->cast_type_u_u.reference_type.target, &nn,
			     0, indent);
			break;
		}
		case CAST_TYPE_FUNCTION:
			func_type(name, &d->cast_type_u_u.func_type, on, quals,
				  indent);
			break;
		case CAST_TYPE_ENUM:
		{
			cast_enum_type *ed = &d->cast_type_u_u.enum_type;
			int i;
			
			print_quals(quals);
			w_printf("enum ");
			if (!cast_scoped_name_is_empty(&(ed->name))) {
				cast_w_scoped_name(&ed->name);
				w_printf(" ");
				}
			w_printf("{\n");
			for (i = 0; i < (signed int)ed->slots.slots_len; i++) {
				w_i_printf(indent+1,"%s = ",ed->slots.slots_val[i].name);
				cast_w_expr(ed->slots.slots_val[i].val, 0);
				if(i == (signed int)(ed->slots.slots_len-1)) {
					w_printf("\n");
					w_i_printf(indent,"} ");
				} else
					w_printf(",\n");
			}
			type_list(name, on, 0, indent);
			break;
		}
		case CAST_TYPE_VOID:
		{
			w_printf("void ");
			type_list(name, on, 0, indent);
			break;
		}
		case CAST_TYPE_QUALIFIED: {
			cast_qualified_type *qd = &(d->cast_type_u_u.qualified);
			qual_node qn;
			
			qn.this_qual = qd;
			qn.next = quals;
			
			type(name, qd->actual, on, &qn, indent);
			break;
		}
		case CAST_TYPE_NULL:
			type_list(name, on, 0, indent);
			break;
		case CAST_TYPE_TEMPLATE: {
			cast_template_param *tp;
			cast_scoped_name scname;
			unsigned int i;
			
			w_i_printf(0,
				   "%stemplate <",
				   d->cast_type_u_u.template_type.flags &
				   CAST_TEMP_EXPORT ? "export " : "");
			for (i = 0;
			     i < (d->cast_type_u_u.template_type.
				  params.params_len);
			     i++) {
				if (i > 0) w_printf(", ");
				tp = &d->cast_type_u_u.template_type.
				     params.params_val[i];
				cast_w_template_param(tp);
			}
			w_printf(">\n");
			w_indent(indent + 1);
			switch( d->cast_type_u_u.template_type.def->kind ) {
			case CAST_TYPE_AGGREGATE:
			case CAST_TYPE_ENUM:
				scname = null_scope_name;
				break;
			default:
				scname = name;
				break;
			}
			cast_w_type(scname,
				    d->cast_type_u_u.template_type.def,
				    indent + 1);
			break;
		}
		default:
			panic("unknown type kind %d\n", d->kind);
	}
}

void cast_w_type(cast_scoped_name name, cast_type d, int indent)
{
	type(name, d, 0, 0, indent);
}

void cast_w_func_type(cast_scoped_name name, cast_func_type *fd, int indent)
{
	func_type(name, fd, 0, 0, indent);
}

/* End of file. */

