/*
 * Copyright (c) 1995, 1996 The University of Utah and
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

#include <stdlib.h>
#include <string.h>
#include <mom/compiler.h>
#include <mom/c/libcast.h>

int cast_func_add_param(cast_func_type *func)
{
	int i;

	i = func->params.params_len++;
	func->params.params_val = mustrealloc(func->params.params_val,
		func->params.params_len*sizeof(*func->params.params_val));

	memset(&func->params.params_val[i], 0, sizeof(func->params.params_val[i]));

	return i;
}

