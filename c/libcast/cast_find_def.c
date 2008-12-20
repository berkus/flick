/*
 * Copyright (c) 1997, 1998, 1999 The University of Utah and
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

int cast_find_def_in_scope(cast_scope *scope,
			   int scope_pos,
			   const cast_scoped_name name,
			   int name_pos,
			   int kind,
			   cast_scope **result_scope)
{
	unsigned int i;
	int retval = -1;
	
	for (i = scope_pos;
	     (i < scope->cast_scope_len) && (retval == -1);
	     i++) {
		if (!scope->cast_scope_val[i].name.cast_scoped_name_len ||
		    strcmp(scope->cast_scope_val[i].name.
			   cast_scoped_name_val[0].name,
			   name.cast_scoped_name_val[name_pos].name))
			continue;
		if((name_pos + 1) == (int)name.cast_scoped_name_len) {
			if((!kind ||
			    (scope->cast_scope_val[i].u.kind) &
			    kind)) {
				*result_scope = scope;
				retval = i;
			}
		} else {
			switch( scope->cast_scope_val[i].u.kind ) {
			case CAST_TYPEDEF:
			case CAST_TYPE:
				switch( scope->cast_scope_val[i].u.
					cast_def_u_u.type->kind ) {
				case CAST_TYPE_AGGREGATE:
					retval = cast_find_def_in_scope(
						&scope->cast_scope_val[i].u.
						cast_def_u_u.type->
						cast_type_u_u.agg_type.scope,
						0,
						name,
						name_pos + 1,
						kind,
						result_scope);
					break;
				case CAST_TYPE_TEMPLATE:
					if( scope->cast_scope_val[i].u.
					    cast_def_u_u.type->
					    cast_type_u_u.template_type.
					    def->kind != CAST_TYPE_AGGREGATE )
						break;
					retval = cast_find_def_in_scope(
						&scope->cast_scope_val[i].u.
						cast_def_u_u.type->
						cast_type_u_u.template_type.
						def->cast_type_u_u.agg_type.
						scope,
						0,
						name,
						name_pos + 1,
						kind,
						result_scope);
					break;
				default:
					break;
				}
				break;
			case CAST_NAMESPACE:
				retval = cast_find_def_in_scope(
					scope->cast_scope_val[i].u.
					cast_def_u_u.new_namespace,
					0,
					name,
					name_pos + 1,
					kind,
					result_scope);
				break;
			default:
				break;
			}
		}
	}
	return( retval );
}

int cast_find_def(cast_scope **scope,
		  const cast_scoped_name name,
		  int kind)
{
	if (!name.cast_scoped_name_len)
		return -1;
	return( cast_find_def_in_scope(*scope,
				       0,
				       name,
				       !strlen(name.cast_scoped_name_val[0].
					       name) ? 1 : 0,
				       kind,
				       scope) );
}

int cast_find_def_pos(cast_scope **scope,
		      int scope_pos,
		      const cast_scoped_name name,
		      int kind)
{
	if (!name.cast_scoped_name_len)
		return -1;
	
	return( cast_find_def_in_scope(*scope, scope_pos,
				       name, name.cast_scoped_name_len - 1,
				       kind, scope) );
}

/* End of file. */

