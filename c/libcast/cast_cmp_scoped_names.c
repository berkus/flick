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

/* XXX - This doesn't work totally correctly yet.  We need
   to also compare cast_expr's in tempalte arguments
   to see if they evaluate to equal values. */
int cast_cmp_scoped_names(const cast_scoped_name *scn1,
			  const cast_scoped_name *scn2)
{
	int retval = 1;
	cast_template_arg_array *t1_args, *t2_args;
	unsigned int i, j;
	
	if (scn1->cast_scoped_name_len !=
	    scn2->cast_scoped_name_len)
		return 0;
	for (i = 0; i < scn1->cast_scoped_name_len; i++ ) {
		retval = retval &&
			 !strcmp( scn1->cast_scoped_name_val[i].name,
				  scn2->cast_scoped_name_val[i].name );
		t1_args = &scn1->cast_scoped_name_val[i].args;
		t2_args = &scn2->cast_scoped_name_val[i].args;
		if (t1_args->cast_template_arg_array_len !=
		    t2_args->cast_template_arg_array_len)
			return 0;
		for (j = 0;
		     j < t1_args->cast_template_arg_array_len;
		     j++) {
			if (t1_args->cast_template_arg_array_val[j]->
			    kind !=
			    t2_args->cast_template_arg_array_val[j]->
			    kind)
				return 0;
			switch(t1_args->
			       cast_template_arg_array_val[j]->kind) {
			case CAST_TEMP_ARG_NAME:
				retval = retval && cast_cmp_scoped_names(
					&t1_args->
					cast_template_arg_array_val[j]->
					cast_template_arg_u_u.name,
					&t2_args->
					cast_template_arg_array_val[j]->
					cast_template_arg_u_u.name);
				break;
			case CAST_TEMP_ARG_TYPE:
				retval = retval &&
					 cast_cmp_type(t1_args->
					cast_template_arg_array_val[j]->
					cast_template_arg_u_u.type,
					t2_args->
					cast_template_arg_array_val[j]->
					cast_template_arg_u_u.type);
				break;
			case CAST_TEMP_ARG_EXPR:
				/* XXX - Need to test here */
				break;
			}
		}
	}
	return retval;
}

/* End of file. */

