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

#include <string.h>

#include <stdio.h>
#include <mom/c/libcast.h>

/*
 * ``Dereference'' a named type by locating and returning its definition.
 * If no definition can be found, return 0.
 *
 * XXX --- This function does not detect mutually-referential type names.
 */
cast_type cast_find_typedef_type(cast_scope *scope, cast_type name_ctype)
{
	cast_scope *deep_scope;
	cast_type current_ctype = name_ctype;
	
	while (current_ctype
	       && (current_ctype->kind == CAST_TYPE_NAME)) {
		/*
		 * Locate the definition of `current_ctype'.
		 */
		cast_scoped_name *current_ctype_name =
			&current_ctype->cast_type_u_u.name;
		int i;
		cast_type typedef_ctype = 0;
		
		/*****/
		
		deep_scope = scope;
		i = cast_find_def(&deep_scope,
				  *current_ctype_name,
				  CAST_TYPEDEF|CAST_TYPE);
		while (i >= 0) {
			if( deep_scope->cast_scope_val[i].u.cast_def_u_u.type->
			    kind == CAST_TYPE_CLASS_NAME ) {
				i = cast_find_def_pos(&deep_scope,
						      i + 1,
						      *current_ctype_name,
						      CAST_TYPEDEF|CAST_TYPE);
			} else {
				typedef_ctype = (deep_scope->cast_scope_val[i].
						 u.cast_def_u_u.typedef_type);
				i = -1;
			}
		}
		/* Guard against `typedef Foo Foo' insanity. */
		if (typedef_ctype != current_ctype)
			current_ctype = typedef_ctype;
		else
			current_ctype = 0;
	}
	
	return current_ctype;
}

/*
 * Return the `cast_enum_type' corresponding to the given C type.  If there is
 * no underlying enumeration definition, return 0.
 */
cast_enum_type *cast_find_enum_type(cast_scope *scope, cast_type ctype)
{
	cast_enum_type *enum_type = 0;
	
	/*****/
	
	/* If `ctype' is a named type, resolve it. */
	ctype = cast_find_typedef_type(scope, ctype);
	
	if (!ctype) {
		/* We failed to resolve the named type. */
		enum_type = 0;
		
	} else if (ctype->kind == CAST_TYPE_ENUM) {
		/* Easy case: we have a real C `enum' type. */
		enum_type = &(ctype->cast_type_u_u.enum_type);
		
	} else if (ctype->kind == CAST_TYPE_ENUM_NAME) {
		/*
		 * Harder case: we have a named enumeration type like `enum
		 * foo'.  The actual enumeration type may be found in a typedef
		 * or in a bare enumeration definition.
		 */
		cast_scoped_name *enum_name = &ctype->cast_type_u_u.enum_name;
		int i;
		
		i = cast_find_def(&scope, *enum_name, CAST_TYPEDEF|CAST_TYPE);
		if (i >= 0) {
			cast_def *this_def = &(scope->cast_scope_val[i]);
			cast_type typedef_ctype;
			
			switch (this_def->u.kind) {
			case CAST_TYPE:
			case CAST_TYPEDEF:
				typedef_ctype = this_def->u.cast_def_u_u.
						typedef_type;
				if (typedef_ctype->kind == CAST_TYPE_ENUM)
					enum_type
						= &(typedef_ctype->
						    cast_type_u_u.enum_type);
				break;
				
			default:
				break;
			}
		}
		
	} else {
		/* The `ctype' is not any kind of enumeration type! */
		enum_type = 0;
	}
	
	return enum_type;
}

/*****************************************************************************/

/* General function used by the specialized ones below */

cast_aggregate_type *cast_find_aggregate(cast_scope *scope,
					 cast_type ctype,
					 cast_aggregate_kind kind)
{
	cast_aggregate_type *a_type = 0;
	
	/*****/
	
	/* If `ctype' is a named type, resolve it. */
	ctype = cast_find_typedef_type(scope, ctype);
	
	if (!ctype) {
		/* We failed to resolve the named type. */
		return 0;
	}
	switch( ctype->kind ) {
	case CAST_TYPE_AGGREGATE:
		/* Easy case: we have a real C `struct' type. */
		a_type = &(ctype->cast_type_u_u.agg_type);
		break;
	case CAST_TYPE_STRUCT_NAME:
	case CAST_TYPE_UNION_NAME:
	case CAST_TYPE_CLASS_NAME: {
		/*
		 * Harder case: we have a named type like `struct foo'.
		 * The actual aggregate type may be found in a typedef or in a
		 * bare structure definition.
		 */
		cast_scoped_name *agg_name = &ctype->cast_type_u_u.struct_name;
		int i;
		
		i = cast_find_def(&scope, *agg_name,
				  CAST_TYPEDEF|CAST_TYPE);
		while (i >= 0) {
			if( scope->cast_scope_val[i].u.cast_def_u_u.type->
			    kind == CAST_TYPE_CLASS_NAME ) {
				i = cast_find_def_pos(&scope,
						      i + 1,
						      *agg_name,
						      CAST_TYPEDEF|CAST_TYPE);
			}
			else
				break;
		}
		if( i >= 0 ) {
			cast_def *this_def = &(scope->cast_scope_val[i]);
			cast_type typedef_ctype;
			
			switch (this_def->u.kind) {
			case CAST_TYPE:
			case CAST_TYPEDEF:
				typedef_ctype = this_def->u.cast_def_u_u.
					typedef_type;
				if (typedef_ctype->kind == CAST_TYPE_AGGREGATE)
					a_type = &(typedef_ctype->
						   cast_type_u_u.agg_type);
				if( a_type && a_type->kind != kind )
					a_type = 0;
				break;
			default:
				break;
			}
			i = -1;
		}
		break;
	}
	default:
		/* The `ctype' is not any kind of structure type! */
		a_type = 0;
		break;
	}
	
	return a_type;
}

/*
 * Return the `cast_aggregate_type' corresponding to the given C type.
 * If there is no underlying structure definition, return 0.
 */
cast_aggregate_type *cast_find_struct_type(cast_scope *scope, cast_type ctype)
{
	return cast_find_aggregate(scope, ctype, CAST_AGGREGATE_STRUCT);
}

/*****************************************************************************/

/*
 * Return the `cast_aggregate_type' corresponding to the given C type.
 * If there is no underlying union definition, return 0.
 */
cast_aggregate_type *cast_find_union_type(cast_scope *scope, cast_type ctype)
{
	return cast_find_aggregate(scope, ctype, CAST_AGGREGATE_UNION);
}

/*****************************************************************************/

/*
 * Return the `cast_aggregate_type' corresponding to the given C type.
 * If there is no underlying union definition, return 0.
 */
cast_aggregate_type *cast_find_class_type(cast_scope *scope, cast_type ctype)
{
	return cast_find_aggregate(scope, ctype, CAST_AGGREGATE_CLASS);
}

/* End of file. */

