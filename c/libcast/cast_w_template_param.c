/*
 * Copyright (c) 1998, 1999 The University of Utah and
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

void cast_w_template_param(cast_template_param *tp)
{
	unsigned int i;
	
	switch(tp->u.kind) {
	case CAST_TEMP_PARAM_TYPE:
		cast_w_type(cast_new_scoped_name(tp->name, NULL),
			    tp->u.cast_template_param_u_u.type_param,
			    0);
		break;
	case CAST_TEMP_PARAM_CLASS:
		w_printf("class %s", tp->name);
		break;
	case CAST_TEMP_PARAM_TYPENAME:
		w_printf("typename %s", tp->name);
		break;
	case CAST_TEMP_PARAM_TEMPLATE:
		w_printf("template <");
		for (i = 0;
		     i < tp->u.cast_template_param_u_u.params.params_len;
		     i++) {
			if (i > 0) w_printf(", ");
			cast_w_template_param(tp->u.cast_template_param_u_u.
					      params.params_val[i]);
		}
		w_printf("> class %s", tp->name);
		break;
	}
	if (tp->default_value) {
		w_printf(" = ");
		cast_w_template_arg(tp->default_value, 0);
	}
}

/* End of file. */

