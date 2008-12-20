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
#include <ctype.h>

#include <mom/compiler.h>
#include <mom/c/libcast.h>

#define doprec(new_prec, stuff)							\
	if (new_prec < last_prec) w_putc('(');					\
	stuff;									\
	if (new_prec < last_prec) w_putc(')');

#define dolunary(prec, str)							\
	doprec(prec,								\
		w_printf(str);							\
		w_expr(expr->cast_expr_u_u.unary.expr, prec, 0);		\
	);

#define dorunary(prec, str)							\
	doprec(prec,								\
		w_expr(expr->cast_expr_u_u.unary.expr, prec+1, 0);		\
		w_printf(str);							\
	);

#define dobinary(prec, str)							\
	doprec(prec,								\
		w_expr(expr->cast_expr_u_u.binary.expr[0], prec, 0);	\
		w_printf(str);							\
		w_expr(expr->cast_expr_u_u.binary.expr[1], prec+1, 0);	\
	);

#define dobinaryop(prec, str)							\
	doprec(prec,								\
		w_expr(expr->cast_expr_u_u.op_assign.expr[0], prec, 0);	\
		w_printf(str);							\
		w_expr(expr->cast_expr_u_u.op_assign.expr[1], prec+1, 0);	\
	);

static void w_expr_prim_type(cast_primitive_modifier mod,
			     cast_primitive_kind kind)
{
	if (mod != 0)
		w_printf("(%s%s%s) ",
			 ((mod & CAST_MOD_SIGNED) ? "signed " :
			  (mod & CAST_MOD_UNSIGNED) ? "unsigned " :
			  ""),
			 ((mod & CAST_MOD_LONG_LONG) ? "long long " :
			  (mod & CAST_MOD_LONG) ? "long " :
			  (mod & CAST_MOD_SHORT) ? "short " :
			  ""),
			 ((kind == CAST_PRIM_CHAR) ? "char" :
			  (kind == CAST_PRIM_INT) ? "int" :
			  (kind == CAST_PRIM_FLOAT) ? "float" :
			  (kind == CAST_PRIM_DOUBLE) ? "double" :
			  (panic("unknown kind %d in w_expr_prim_type", kind),
			   ""))
			);
}

static void w_expr(cast_expr expr, int last_prec, int indent)
{
	int i;
	
	w_indent(indent);
	assert(expr);
	switch (expr->kind) {
	case CAST_EXPR_NAME:
		cast_w_scoped_name(&expr->cast_expr_u_u.name);
		break;
		
	case CAST_EXPR_CONST_NAME:
		cast_w_scoped_name(&expr->cast_expr_u_u.
				   const_name);
		break;
		
	case CAST_EXPR_LIT_PRIM:
		w_expr_prim_type(expr->cast_expr_u_u.lit_prim.mod,
				 expr->cast_expr_u_u.lit_prim.u.kind);
		switch (expr->cast_expr_u_u.lit_prim.u.kind) {
		case CAST_PRIM_CHAR: {
			int ch = expr->
				 cast_expr_u_u.lit_prim.u.cast_lit_prim_u_u.c;
			w_printf(((ch == '\'') ? "'\\''" :
				  ((ch >= ' ') && (ch < 127)) ? "'%c'" :
				  "%d"),
				 ch);
			break;
		}
		case CAST_PRIM_INT: {
			w_printf("%d",
				 expr->
				 cast_expr_u_u.lit_prim.u.cast_lit_prim_u_u.i);
			break;
		}
		case CAST_PRIM_FLOAT: {
			w_printf("%f",
				 expr->
				 cast_expr_u_u.lit_prim.u.cast_lit_prim_u_u.f);
			break;
		}
		case CAST_PRIM_DOUBLE: {
			w_printf("%f",
				 expr->
				 cast_expr_u_u.lit_prim.u.cast_lit_prim_u_u.d);
			break;
		}
		case CAST_PRIM_BOOL: {
			w_printf("%s",
				 expr->
				 cast_expr_u_u.lit_prim.u.cast_lit_prim_u_u.b ?
				"true" :
				"false");
			break;
		}
		default:
			panic("cast_w_expr: unknown lit_prim kind %d",
			      expr->cast_expr_u_u.lit_prim.u.kind);
		}
		break;
		
	case CAST_EXPR_LIT_STRING: {
		char *s;
		
		w_putc('\"');
		for (s = expr->cast_expr_u_u.lit_string; *s; ++s)
			switch (*s) {
			case '\a':
				/* Bell. */
				w_printf("\\a");
				break;
			case '\b':
				/* Backspace. */
				w_printf("\\b");
				break;
			case '\f':
				/* Form feed. */
				w_printf("\\f");
				break;
			case '\n':
				/* Newline. */
				w_printf("\\n");
				break;
			case '\r':
				/* Carriage return. */
				w_printf("\\r");
				break;
			case '\t':
				/* Tab. */
				w_printf("\\t");
				break;
			case '\v':
				/* Vertical tab. */
				w_printf("\\v");
				break;
			case '\'':
				/* Single quote. */
				w_printf("\\'");
				break;
			case '\"':
				/* Double quote. */
				w_printf("\\\"");
				break;
			case '\\':
				/* Backslash. */
				w_printf("\\\\");
				break;
			default:
				/*
				 * Cast `*s' to `int' to avoid warning from
				 * gcc 2.8.0.
				 */
				if (isprint(((int) *s)))
					w_putc(*s);
				else
					w_printf("\\%03o", *s);
				break;
			}
		w_putc('\"');
		break;
	}
	
	case CAST_EXPR_CALL:
		doprec(14,
		       w_expr(expr->cast_expr_u_u.call.func, 14, 0);
		       w_putc('(');
		       for (i = 0;
			    i < (signed int)expr->cast_expr_u_u.call.params.cast_expr_array_len;
			    i++) {
			       if (i > 0) {
				       w_putc(',');
				       w_putc(' ');
			       }
			       w_expr(expr->
				      cast_expr_u_u.call.params.cast_expr_array_val[i],
				      1, 0);
		       }
		       w_putc(')');
			);
		break;
		
	case CAST_EXPR_SEL: {
		cast_expr_sel *sel = &expr->cast_expr_u_u.sel;
		cast_unary_expr *ue = &sel->var->cast_expr_u_u.unary;
		
		if ((sel->var->kind == CAST_EXPR_UNARY)
		    && (ue->op == CAST_UNARY_DEREF)
		    && ((ue->expr->kind != CAST_EXPR_BINARY)
			|| (ue->expr->cast_expr_u_u.binary.op !=
			    CAST_BINARY_ADD))) {
			/* Combine the (*). operators into a single ->
			   operator.  */
			doprec(14,
			       w_expr(ue->expr, 14, 0);
			       w_printf("->");
			       cast_w_scoped_name(&sel->member);
				);
		} else {
			doprec(14,
			       w_expr(sel->var, 14, 0);
			       w_printf(".");
			       cast_w_scoped_name(&sel->member);
				);
		}
		break;
	}
	
	case CAST_EXPR_UNARY:
		switch (expr->cast_expr_u_u.unary.op) {
		case CAST_UNARY_DEREF: {
			cast_unary_expr *ue = &expr->cast_expr_u_u.unary;
			cast_binary_expr *be = &ue->expr->cast_expr_u_u.binary;
			cast_unary_expr *ue2 = &ue->expr->cast_expr_u_u.unary;
			
			if ((ue->expr->kind == CAST_EXPR_BINARY)
			    && (be->op == CAST_BINARY_ADD)) {
				/* Use a single [] operator.
				   Note that this is legal even if the pointer
				   is the _second_ element of the + expression,
				   because C defines x[y] to be _exactly_
				   equivalent to *(x+y).
				   In other words,
				   idx[ptr] will work just as well as ptr[idx].
				   */
				doprec(14,
				       w_expr(be->expr[0], 14, 0);
				       w_putc('[');
				       w_expr(be->expr[1], 1, 0);
				       w_putc(']');
					);
			} else if ((ue->expr->kind == CAST_EXPR_UNARY)
				   && (ue2->op == CAST_UNARY_ADDR)) {
				/* The operators cancel each other.  */
				w_expr(ue2->expr, last_prec, 0);
			} else {
				dolunary(13, "*");
			}
			break;
		}
		case CAST_UNARY_ADDR: {
			cast_unary_expr *ue = &expr->cast_expr_u_u.unary;
			cast_unary_expr *ue2 = &ue->expr->cast_expr_u_u.unary;
			
			if ((ue->expr->kind == CAST_EXPR_UNARY)
			    && (ue2->op == CAST_UNARY_DEREF)) {
				/* The operators cancel each other.  */
				w_expr(ue2->expr, last_prec, 0);
			} else {
				dolunary(13, "&");
			}
			break;
		}
		case CAST_UNARY_NEG:
			dolunary(13, "-");
			break;
		case CAST_UNARY_LNOT:
			dolunary(13, "!");
			break;
		case CAST_UNARY_BNOT:
			dolunary(13, "~");
			break;
		case CAST_UNARY_PRE_INC:
			dolunary(13, "++");
			break;
		case CAST_UNARY_PRE_DEC:
			dolunary(13, "--");
			break;
		case CAST_UNARY_POST_INC:
			dorunary(13, "++");
			break;
		case CAST_UNARY_POST_DEC:
			dorunary(13, "--");
			break;
		default:
			panic("cast_w_expr: unknown unary op %d",
			      expr->cast_expr_u_u.unary.op);
		}
		break;
		
	case CAST_EXPR_CAST:
		doprec(13,
		       w_putc('(');
		       cast_w_type(empty_scope_name,
				   expr->cast_expr_u_u.cast.type, 0);
		       w_putc(')');
		       w_expr(expr->cast_expr_u_u.cast.expr, 14+1, 0);
			);
		break;
		
	case CAST_EXPR_SIZEOF_EXPR:
		doprec(14,
		       w_printf("sizeof(");
		       w_expr(expr->cast_expr_u_u.sizeof_expr, 0, 0);
		       w_putc(')');
			);
		break;
		
	case CAST_EXPR_SIZEOF_TYPE:
		doprec(14,
		       w_printf("sizeof(");
		       cast_w_type(empty_scope_name,
				   expr->cast_expr_u_u.sizeof_type, 0);
		       w_putc(')');
			);
		break;
		
	case CAST_EXPR_BINARY:
		switch (expr->cast_expr_u_u.binary.op) {
		case CAST_BINARY_MUL:
			dobinary(12, " * ");
			break;
		case CAST_BINARY_DIV:
			dobinary(12, " / ");
			break;
		case CAST_BINARY_MOD:
			dobinary(12, " % ");
			break;
		case CAST_BINARY_ADD:
			/* XXX - This is a silly hack to get around
			   a gcc warning that complains when an '&'
			   is surrounded by a '+' or a '-'. */
			if( last_prec == 7 ) {
				dobinary(6, " + ");
			}
			else {
				dobinary(11, " + ");
			}
			break;
		case CAST_BINARY_SUB:
			if( last_prec == 7 ) {
				dobinary(6, " - ");
			}
			else {
				dobinary(11, " - ");
			}
			break;
		case CAST_BINARY_SHL:
			dobinary(10, " << ");
			break;
		case CAST_BINARY_SHR:
			dobinary(10, " >> ");
			break;
		case CAST_BINARY_LT:
			dobinary(9, " < ");
			break;
		case CAST_BINARY_GT:
			dobinary(9, " > ");
			break;
		case CAST_BINARY_LE:
			dobinary(9, " <= ");
			break;
		case CAST_BINARY_GE:
			dobinary(9, " >= ");
			break;
		case CAST_BINARY_EQ:
			dobinary(8, " == ");
			break;
		case CAST_BINARY_NE:
			dobinary(8, " != ");
			break;
		case CAST_BINARY_BAND:
			dobinary(7, " & ");
			break;
		case CAST_BINARY_BXOR:
			dobinary(6, " ^ ");
			break;
		case CAST_BINARY_BOR:
			dobinary(5, " | ");
			break;
		case CAST_BINARY_LAND:
			dobinary(4, " && ");
			break;
		case CAST_BINARY_LOR:
			dobinary(3, " || ");
			break;
		case CAST_BINARY_ASSIGN:
			dobinary(1, " = ");
			break;
		case CAST_BINARY_COMMA:
			dobinary(0, ", ");
			break;
		default:
			panic("cast_w_expr: unknown binary op %d",
			      expr->cast_expr_u_u.binary.op);
		}
		break;
		
	case CAST_EXPR_OP_ASSIGN:
		switch (expr->cast_expr_u_u.op_assign.op) {
		case CAST_BINARY_MUL:
			dobinaryop(1, " *= ");
			break;
		case CAST_BINARY_DIV:
			dobinaryop(1, " /= ");
			break;
		case CAST_BINARY_MOD:
			dobinaryop(1, " %= ");
			break;
		case CAST_BINARY_ADD:
			dobinaryop(1, " += ");
			break;
		case CAST_BINARY_SUB:
			dobinaryop(1, " -= ");
			break;
		case CAST_BINARY_SHL:
			dobinaryop(1, " <<= ");
			break;
		case CAST_BINARY_SHR:
			dobinaryop(1, " >>= ");
			break;
		case CAST_BINARY_BAND:
			dobinaryop(1, " &= ");
			break;
		case CAST_BINARY_BXOR:
			dobinaryop(1, " ^= ");
			break;
		case CAST_BINARY_BOR:
			dobinaryop(1, " |= ");
			break;
		case CAST_BINARY_ASSIGN:
			/* Should this really be here? */
			dobinaryop(1, " = ");
			break;
		default:
			panic("cast_w_expr: unknown op_assign op %d",
			      expr->cast_expr_u_u.op_assign.op);
			break;
		}
		break;
		
	case CAST_EXPR_COND:
		doprec(2,
		       w_expr(expr->cast_expr_u_u.cond.test, 2+1, 0);
		       w_printf(" ? ");
		       w_expr(expr->cast_expr_u_u.cond.true_expr, 2+1, 0);
		       w_printf(" : ");
		       w_expr(expr->cast_expr_u_u.cond.false_expr, 2, 0);
			);
		break;

	case CAST_EXPR_CONST_CAST:
		doprec(13,
		       w_printf("const_cast");
		       w_putc('<');
		       cast_w_type(empty_scope_name,
				   expr->cast_expr_u_u.c_cast.type,
				   0);
		       w_putc('>');
		       w_putc('(');
		       w_expr(expr->cast_expr_u_u.cast.expr, 14+1, 0);
		       w_putc(')');
			);
		break;
		
	case CAST_EXPR_DYNAMIC_CAST:
		doprec(13,
		       w_printf("dynamic_cast");
		       w_putc('<');
		       cast_w_type(empty_scope_name,
				   expr->cast_expr_u_u.d_cast.type,
				   0);
		       w_putc('>');
		       w_putc('(');
		       w_expr(expr->cast_expr_u_u.cast.expr, 14+1, 0);
		       w_putc(')');
			);
		break;
		
	case CAST_EXPR_REINTERPRET_CAST:
		doprec(13,
		       w_printf("reinterpret_cast");
		       w_putc('<');
		       cast_w_type(empty_scope_name,
				   expr->cast_expr_u_u.r_cast.type,
				   0);
		       w_putc('>');
		       w_putc('(');
		       w_expr(expr->cast_expr_u_u.cast.expr, 14+1, 0);
		       w_putc(')');
			);
		break;
		
	case CAST_EXPR_STATIC_CAST:
		doprec(13,
		       w_printf("static_cast");
		       w_putc('<');
		       cast_w_type(empty_scope_name,
				   expr->cast_expr_u_u.cast.type,
				   0);
		       w_putc('>');
		       w_putc('(');
		       w_expr(expr->cast_expr_u_u.cast.expr, 14+1, 0);
		       w_putc(')');
			);
		break;
		
	case CAST_EXPR_OP_NEW: {
		cast_op_new_expr	*ne;
		int			is_simple;
		
		w_printf("new ");
		ne = &expr->cast_expr_u_u.op_new;
		/*
		 * If the type is not a ``simple'' type, put parens around it.
		 * This avoids problems with some versions of `gcc' (2.7.2.1)
		 * that sometimes get confused by things like `new struct foo'.
		 */
		switch (ne->type->kind) {
		case CAST_TYPE_NAME:
		case CAST_TYPE_PRIMITIVE:
		case CAST_TYPE_VOID:
		case CAST_TYPE_TYPENAME:
			is_simple = 1;
			break;
		default:
			is_simple = 0;
			break;
		}
		
		if (!is_simple)
			w_printf("(");
		cast_w_type(null_scope_name,
			    ne->type,
			    0);
		if (!is_simple)
			w_printf(")");
		if (ne->init)
			cast_w_init(ne->init, 0);
		break;
	}
	
	case CAST_EXPR_OP_DELETE:
		w_printf("delete %s",
			 expr->cast_expr_u_u.op_delete.array ? "[] " : "");
		cast_w_expr(expr->cast_expr_u_u.op_delete.expr, 0);
		break;
		
	case CAST_EXPR_TYPEID_EXPR:
		doprec(14,
		       w_printf("typeid(");
		       w_expr(expr->cast_expr_u_u.typeid_expr, 0, 0);
		       w_putc(')');
			);
		break;
		
	case CAST_EXPR_TYPEID_TYPE:
		doprec(14,
		       w_printf("typeid(");
		       cast_w_type(empty_scope_name,
				   expr->cast_expr_u_u.typeid_type, 0);
		       w_putc(')');
			);
		break;
		
	case CAST_EXPR_TYPE:
		doprec(14,
		       cast_w_type(empty_scope_name,
				   expr->cast_expr_u_u.type_expr, 0);
			);
		break;
		
	default:
		panic("cast_w_expr: unknown expr kind %d", expr->kind);
	}
}

void cast_w_expr(cast_expr expr, int indent)
{
	w_expr(expr, 0, indent);
}

void cast_w_expr_noncomma(cast_expr expr, int indent)
{
	w_expr(expr, 1, indent);
}

/* End of file. */

