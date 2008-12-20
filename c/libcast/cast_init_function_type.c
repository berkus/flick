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

#include <string.h>
#include <mom/compiler.h>
#include <mom/c/libcast.h>

void cast_init_function_type(cast_func_type *cfunc, int params)
{
	if (params) {
		cfunc->params.params_len = params;
		cfunc->params.params_val 
			= (cast_func_param *)
			  mustcalloc(params * sizeof(cast_func_param));
		memset(cfunc->params.params_val,
		       0,
		       params * sizeof(cast_func_param));
	}
	else {
		cfunc->params.params_len = 0;
		cfunc->params.params_val = 0;
	}
	cfunc->spec = 0;
	cfunc->exception_types = null_type_array;
	cfunc->initializers = null_expr_array;
}

