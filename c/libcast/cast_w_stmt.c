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

void cast_w_stmt(cast_stmt st, int indent)
{
	assert(st);
	switch (st->kind)
	{
		case CAST_STMT_EXPR:
			w_indent(indent);
			cast_w_expr(st->cast_stmt_u_u.expr, 0);
			w_printf(";\n");
			break;
		case CAST_STMT_BLOCK:
			cast_w_block(&st->cast_stmt_u_u.block, indent);
			break;
		case CAST_STMT_IF:
			w_i_printf(indent, "if (");
			cast_w_expr(st->cast_stmt_u_u.s_if.test, 0);
			w_printf(")\n");
			cast_w_stmt(st->cast_stmt_u_u.s_if.true_stmt, indent+1);
			if (st->cast_stmt_u_u.s_if.false_stmt)
			{
				w_i_printf(indent, "else\n");
				cast_w_stmt(st->cast_stmt_u_u.s_if.false_stmt, indent+1);
			}
			break;
		case CAST_STMT_WHILE:
			w_i_printf(indent, "while (");
			cast_w_expr(st->cast_stmt_u_u.s_while.test, 0);
			w_printf(")\n");
			cast_w_stmt(st->cast_stmt_u_u.s_while.stmt, indent+1);
			break;
		case CAST_STMT_DO_WHILE:
			w_i_printf(indent, "do\n");
			cast_w_stmt(st->cast_stmt_u_u.s_do_while.stmt, indent+1);
			w_i_printf(indent, "while (");
			cast_w_expr(st->cast_stmt_u_u.s_do_while.test, 0);
			w_printf(")\n");
			break;
		case CAST_STMT_FOR:
			w_i_printf(indent, "for (");
			if (st->cast_stmt_u_u.s_for.init)
				cast_w_expr(st->cast_stmt_u_u.s_for.init, 0);
			w_printf("; ");
			if (st->cast_stmt_u_u.s_for.test)
				cast_w_expr(st->cast_stmt_u_u.s_for.test, 0);
			w_printf("; ");
			if (st->cast_stmt_u_u.s_for.iter)
				cast_w_expr(st->cast_stmt_u_u.s_for.iter, 0);
			w_printf(")\n");
			cast_w_stmt(st->cast_stmt_u_u.s_for.stmt, indent+1);
			break;
		case CAST_STMT_SWITCH:
			w_i_printf(indent, "switch (");
			cast_w_expr(st->cast_stmt_u_u.s_switch.test, 0);
			w_printf(")\n");
			cast_w_stmt(st->cast_stmt_u_u.s_switch.stmt, indent+1);
			break;
		case CAST_STMT_BREAK:
			w_i_printf(indent, "break;\n");
			break;
		case CAST_STMT_CONTINUE:
			w_i_printf(indent, "continue;\n");
			break;
		case CAST_STMT_GOTO:
			w_i_printf(indent, "goto %s;\n", st->cast_stmt_u_u.goto_label);
			break;
		case CAST_STMT_LABEL:
			/* If there are no users for this label then
			   we don't print the label, just the statement. */
			if( st->cast_stmt_u_u.s_label.users != 0 )
				w_i_printf((indent > 0) ? indent - 1 : 0,
					   "%s:\n",
					   st->cast_stmt_u_u.s_label.label);
			cast_w_stmt(st->cast_stmt_u_u.s_label.stmt,
				    indent);
			break;
		case CAST_STMT_CASE:
			w_i_printf(indent > 0 ? indent-1 : 0, "case ");
			cast_w_expr(st->cast_stmt_u_u.s_case.label, 0);
			w_printf(":\n");
			cast_w_stmt(st->cast_stmt_u_u.s_case.stmt, indent);
			break;
		case CAST_STMT_DEFAULT:
			w_i_printf(indent > 0 ? indent-1 : 0, "default:\n");
			cast_w_stmt(st->cast_stmt_u_u.default_stmt, indent);
			break;
		case CAST_STMT_RETURN:
			w_i_printf(indent, "return ");
			cast_w_expr(st->cast_stmt_u_u.return_expr, 0);
			w_printf(";\n");
			break;
		case CAST_STMT_TEXT:
			if (st->cast_stmt_u_u.text) {
				/* Preprocessor directives are never indented!
				 */
				if (st->cast_stmt_u_u.text[0] == '#')
					w_i_printf(0, "%s",
						   st->cast_stmt_u_u.text);
				else
					w_i_printf(indent, "%s",
						   st->cast_stmt_u_u.text);
			}
			w_printf("\n");
			break;
		case CAST_STMT_NULL:
			w_i_printf(indent, ";\n");
			break;
	        case CAST_STMT_EMPTY:
		        break;
		case CAST_STMT_TRY: {
			unsigned int i;
			cast_try *ct;
			
			w_i_printf(indent, "try\n");
			ct = &st->cast_stmt_u_u.try_block;
			cast_w_block(&ct->block->cast_stmt_u_u.block,
				     indent+1);
			for (i = 0; i < ct->handlers.handlers_len; i++ ) {
				w_i_printf(indent, "catch(");
				cast_w_type(
					cast_new_scoped_name(ct->handlers.
							     handlers_val[i].
							     name, NULL),
					ct->handlers.handlers_val[i].type,
					0);
				w_printf(")\n");
				cast_w_block(&ct->handlers.handlers_val[i].
					     block->cast_stmt_u_u.block,
					     indent+1);
			}
			break;
		}
		case CAST_STMT_THROW:
			w_i_printf(indent, "throw ");
			if (st->cast_stmt_u_u.throw_expr)
				cast_w_expr(st->cast_stmt_u_u.throw_expr, 0);
			w_printf(";\n");
			break;
		case CAST_STMT_DECL:
			cast_w_scope(&st->cast_stmt_u_u.decl, indent);
			break;
		case CAST_STMT_HANDLER: {
			struct cast_handler_entry *che;
			cast_handler *ch = &st->cast_stmt_u_u.handler;
			
			che = (struct cast_handler_entry *)
				find_entry(cast_handler_table, ch->name);
			if( che && che->c_func(indent, ch) ) {
			} else {
				panic("Couldn't find CAST "
				      "statement handler %s", ch->name);
			}
			w_printf("\n");
			break;
		}
		default:
			panic("cast_w_stmt: unknown stmt type %d", st->kind);
			break;
	}
}

