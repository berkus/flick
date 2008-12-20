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

#include <string.h>
#include <mom/compiler.h>
#include <mom/c/libcast.h>

int cast_add_def(cast_scope *scope,
		 cast_scoped_name name,
		 cast_storage_class sc,
		 cast_def_kind kind,
		 data_channel_index channel,
		 cast_def_protection prot)
{
	int i;

	/* XXX horribly inefficient,
	   but needed until we get a max field... */
	i = scope->cast_scope_len++;
	scope->cast_scope_val = mustrealloc(scope->cast_scope_val,
		scope->cast_scope_len*sizeof(*scope->cast_scope_val));

	memset(&scope->cast_scope_val[i],
	       0,
	       sizeof(*scope->cast_scope_val));
	scope->cast_scope_val[i].name = name;
	scope->cast_scope_val[i].sc = sc;
	scope->cast_scope_val[i].u.kind = kind;
	scope->cast_scope_val[i].channel = channel;
	scope->cast_scope_val[i].protection = prot;

	return i;
}

