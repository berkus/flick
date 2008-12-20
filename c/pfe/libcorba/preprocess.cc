/*
 * Copyright (c) 1995, 1996, 1997, 1998, 1999 The University of Utah and
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

#include <mom/compiler.h>
#include <mom/libaoi.h>
#include <mom/libmint.h>

#include <mom/c/pg_corba.hh>

extern mint_1 out_mint;

#define m(n) (out_mint.defs.defs_val[n])

void pg_corba::preprocess()
{
	pg_state::preprocess();
	
	/*
	 * Initialize the customizations needed for converting AOI exceptions
	 * in `translate_aoi_to_mint'.
	 */
	
	mint_custom_exception_const = build_exception_const_string;
	
	mint_custom_exception_discrim_ref = mint_add_def(&out_mint);
	assert(mint_custom_exception_discrim_ref);
	m(mint_custom_exception_discrim_ref).kind = MINT_ARRAY;
	m(mint_custom_exception_discrim_ref).mint_def_u.array_def.length_type
		= out_mint.standard_refs.unsigned32_ref;
	m(mint_custom_exception_discrim_ref).mint_def_u.array_def.element_type
		= out_mint.standard_refs.char8_ref;
	
	/*
	 * XXX --- All of this code should be unnecessary.
	 *
	 * XXX --- Let the manual translation of attributes --> operations
	 * continue until the PG library supports attributes in a more general
	 * way.
	 */
	unsigned int aoi_index;
	
	for (aoi_index = 0; aoi_index < in_aoi->defs.defs_len; ++aoi_index) {
		/*
		 * If this AOI definition is for an interface, translate all of
		 * the interface's attributes into corresponding operations.
		 */
		aoi_interface *interface;

		aoi_operation *old_ops_val;
		unsigned int old_ops_len;
		
		aoi_attribute *old_attribs_val;
		unsigned int old_attribs_len;
		
		aoi_operation *new_ops_val;
		unsigned int new_ops_len;
		
		unsigned int ops_index;
		unsigned int attribs_index;
		
		/*****/
		
		if (in_aoi->defs.defs_val[aoi_index].binding->kind !=
		    AOI_INTERFACE)
			continue;
		
		interface = &(in_aoi->defs.defs_val[aoi_index].binding->
			      aoi_type_u_u.interface_def);
		
		if (interface->attribs.attribs_len == 0)
			/* No attributes?  No munging necessary. */
			continue;
		
		old_ops_val = interface->ops.ops_val;
		old_ops_len = interface->ops.ops_len;
		
		old_attribs_val = interface->attribs.attribs_val;
		old_attribs_len = interface->attribs.attribs_len;
		
		/* Count up the new number of operations that we will have. */
		new_ops_len = old_ops_len;
		for (attribs_index = 0;
		     attribs_index < old_attribs_len;
		     ++attribs_index) {
			if (old_attribs_val[attribs_index].readonly)
				new_ops_len += 1;
			else
				new_ops_len += 2;
		}
		
		new_ops_val = (aoi_operation *)
			      mustmalloc(new_ops_len * sizeof(aoi_operation));
		
		/* Copy the old operations into the new vector. */
		for (ops_index = 0;
		     ops_index < old_ops_len;
		     ++ops_index)
			new_ops_val[ops_index] = old_ops_val[ops_index];
		
		/* Produce new operations for each old attribute. */
		ops_index = old_ops_len;
		for (attribs_index = 0;
		     attribs_index < old_attribs_len;
		     ++attribs_index) {
			
			aoi_attribute *this_attrib
				= &(old_attribs_val[attribs_index]);
			
			aoi_operation *this_op
				= &(new_ops_val[ops_index]);
			
			/*
			 * Construct the `get' operation.
			 */
			this_op->name
				= flick_asprintf("_get_%s", this_attrib->name);
			
			this_op->request_code = this_attrib->read_request_code;
			this_op->reply_code = this_attrib->read_reply_code;
			
			this_op->flags = AOI_OP_FLAG_NONE;
			
			this_op->params.params_len = 0;
			this_op->params.params_val = 0;
			
			this_op->return_type = this_attrib->type;
			
			this_op->exceps.exceps_len = 0;
			this_op->exceps.exceps_val = 0;
			
			++ops_index;
			
			/*
			 * Construct the `set' operation.
			 */
			if (this_attrib->readonly)
				continue;
			this_op = &(new_ops_val[ops_index]);
			
			this_op->name
				= flick_asprintf("_set_%s", this_attrib->name);
			
			this_op->request_code = this_attrib->
						write_request_code;
			this_op->reply_code = this_attrib->write_reply_code;
			
			this_op->flags = AOI_OP_FLAG_NONE;
			
			this_op->params.params_len = 1;
			this_op->params.params_val
				= (aoi_parameter *)
				  mustmalloc(sizeof(aoi_parameter));
			
			this_op->params.params_val[0].name
				= this_attrib->name;
			this_op->params.params_val[0].direction
				= AOI_DIR_IN;
			this_op->params.params_val[0].type
				= this_attrib->type;
			
			this_op->return_type
				= (aoi_type) mustmalloc(sizeof(aoi_type_u));
			this_op->return_type->kind
				= AOI_VOID;
			
			this_op->exceps.exceps_len = 0;
			this_op->exceps.exceps_val = 0;
			
			++ops_index;
		}
		
		/*
		 * Finally, update the operations and attributes of this
		 * interface.
		 */
		interface->ops.ops_len = new_ops_len;
		interface->ops.ops_val = new_ops_val;
		
		interface->attribs.attribs_len = 0;
		interface->attribs.attribs_val = 0;
	}
}

/* End of file. */

