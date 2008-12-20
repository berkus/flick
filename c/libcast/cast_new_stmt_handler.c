/*
 * Copyright (c) 1999 The University of Utah and
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

#include <mom/c/libcast.h>

cast_stmt cast_new_stmt_handler(const char *handler_name, ...)
{
	cast_stmt stmt = cast_new_stmt(CAST_STMT_HANDLER);
	va_list args;
	const char *arg_str;
	
	stmt->cast_stmt_u_u.handler.name = ir_strlit(handler_name);
	stmt->cast_stmt_u_u.handler.args.args_val = 0;
	stmt->cast_stmt_u_u.handler.args.args_len = 0;
	
	va_start(args, handler_name);
	while( (arg_str = va_arg(args, const char *)) != 0 ) {
		stmt->cast_stmt_u_u.handler.args.args_len++;
		stmt->cast_stmt_u_u.handler.args.args_val = (char **)
			mustrealloc(stmt->cast_stmt_u_u.handler.args.args_val,
				    stmt->cast_stmt_u_u.handler.args.args_len *
				    sizeof(char *));
		stmt->cast_stmt_u_u.handler.args.args_val
			[stmt->cast_stmt_u_u.handler.args.args_len - 1] =
			ir_strlit(arg_str);
	}
	va_end(args);
	
	return stmt;
}
