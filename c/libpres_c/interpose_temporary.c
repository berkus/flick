/*
 * Copyright (c) 1999 The University of Utah and
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

#include <assert.h>

#include <mom/libaoi.h>
#include <mom/c/libpres_c.h>
#include <mom/compiler.h>

void pres_c_interpose_temporary(cast_type ctype,
				const char *name,
				cast_expr init,
				const char *prehandler,
				const char *posthandler,
				int is_const,
				pres_c_temporary_type temp_type,
				pres_c_mapping *inout_mapping)
{
	pres_c_mapping new_mapping;
	pres_c_temporary *temp;
	
	assert(inout_mapping);
	assert(*inout_mapping);
	
	/*
	 * Create a PRES_C_MAPPING_TEMPORARY node.
	 */
	
	new_mapping = pres_c_new_mapping(PRES_C_MAPPING_TEMPORARY);
	temp = &new_mapping->pres_c_mapping_u_u.temp;
	
	assert(name);
	temp->name = flick_asprintf("%s", name);
	assert(ctype);
	temp->ctype = ctype;
	temp->init = init;
	/* `prehandler' and `posthandler' cannot be NULL. */
	temp->prehandler = ir_strlit(prehandler ? prehandler : "");
	temp->posthandler = ir_strlit(posthandler ? posthandler : "");
	temp->is_const = is_const;
	temp->type = temp_type;
	temp->map = *inout_mapping;
	
	*inout_mapping = new_mapping;
}

/* End of file. */

