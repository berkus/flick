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

#include <mom/compiler.h>
#include <mom/c/libcast.h>

void cast_w_scope(cast_scope *scope, int indent)
{
	cast_def_protection last_prot = CAST_PROT_PRIVATE;
	int i;
	
	for (i = 0; i < (signed int)scope->cast_scope_len; i++) {
		if ((last_prot != CAST_PROT_NONE) &&
		    (last_prot != scope->cast_scope_val[i].protection)) {
			last_prot = scope->cast_scope_val[i].protection;
			if( last_prot ) {
				w_printf("\n");
				w_i_printf(indent > 0 ? indent-1 : 0,
					   "%s\n",
					   (last_prot == CAST_PROT_PUBLIC ?
					    "public:" :
					    last_prot == CAST_PROT_PROTECTED ?
					    "protected:" :
					    last_prot == CAST_PROT_PRIVATE ?
					    "private:" :
					    ""));
			}
		}
		else
			last_prot = scope->cast_scope_val[i].protection;
		cast_w_def(&scope->cast_scope_val[i], indent);
	}
}

