/*
 * Copyright (c) 1998 The University of Utah and
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

#include <mom/compiler.h>
#include <mom/c/libcast.h>

void cast_w_template_arg(cast_template_arg arg, int indent)
{
	switch (arg->kind)
	{
	case CAST_TEMP_ARG_NAME:
		cast_w_scoped_name(&arg->cast_template_arg_u_u.name);
		break;
	case CAST_TEMP_ARG_TYPE:
		cast_w_type(empty_scope_name,
			    arg->cast_template_arg_u_u.type,
			    indent);
		break;
	case CAST_TEMP_ARG_EXPR:
		cast_w_expr(arg->cast_template_arg_u_u.expr, indent);
		break;
	default:
		panic("Unrecognized cast_template_arg kind %d", arg->kind);
		break;
	}
}

