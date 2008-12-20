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

int cast_template_add_param(struct cast_template_type *template_decl,
			    cast_template_param_kind kind,
			    const char *name,
			    cast_template_arg default_value)
{
	unsigned int i;
	
	i = template_decl->params.params_len++;
	template_decl->params.params_val
		= mustrealloc(template_decl->params.params_val,
			      (template_decl->params.params_len *
			       sizeof(*template_decl->params.params_val)));
	
	template_decl->params.params_val[i].u.kind = kind;
	template_decl->params.params_val[i].name = ir_strlit(name);
	template_decl->params.params_val[i].default_value = default_value;
	
	return i;
}

/* End of file. */

