/*
 * Copyright (c) 1995, 1996, 1997, 1998 The University of Utah and
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

#include <memory.h>
#include <mom/compiler.h>
#include <mom/c/libpres_c.h>

int pres_c_add_inline_struct_slot(pres_c_inline inl)
{
	int i;
	
	/*
	 * Because we use anonymous sequence types in the PRES_C structure
	 * definitions, we must handle each kind of inline_struct separately.
	 *
	 * The alternative would be for `pres_c.x' to include a typedef for the
	 * slot sequence type.  We could then remove redundant code here, but
	 * the price would be less readable code everywhere else in Flick,
	 * because `rpcgen' would use the typedef name to create the names of
	 * the `_len' and `_val' sequence slots!
	 */
	switch (inl->kind) {
	default:
		panic("In `pres_c_add_inline_struct_slot', unexpected "
		      "`pres_c_inline' kind.");
		break;
		
	case PRES_C_INLINE_STRUCT: {
		pres_c_inline_struct *struct_inl
			= &(inl->pres_c_inline_u_u.struct_i);
		
		i = struct_inl->slots.slots_len++;
		
		struct_inl->slots.slots_val
			= mustrealloc(struct_inl->slots.slots_val,
				      (struct_inl->slots.slots_len
				       * sizeof(*(struct_inl->slots.slots_val))
				       ));
		
		memset(&(struct_inl->slots.slots_val[i]),
		       0,
		       sizeof(struct_inl->slots.slots_val[i]));
		
		break;
	}
	
	case PRES_C_INLINE_FUNC_PARAMS_STRUCT: {
		pres_c_inline_func_params_struct *struct_inl
			= &(inl->pres_c_inline_u_u.func_params_i);
		
		i = struct_inl->slots.slots_len++;
		
		struct_inl->slots.slots_val
			= mustrealloc(struct_inl->slots.slots_val,
				      (struct_inl->slots.slots_len
				       * sizeof(*(struct_inl->slots.slots_val))
				       ));
		
		memset(&(struct_inl->slots.slots_val[i]),
		       0,
		       sizeof(struct_inl->slots.slots_val[i]));
		
		break;
	}

	case PRES_C_INLINE_HANDLER_FUNC: {
		pres_c_inline_handler_func *func_inl
			= &(inl->pres_c_inline_u_u.handler_i);
		
		i = func_inl->slots.slots_len++;
		
		func_inl->slots.slots_val
			= mustrealloc(func_inl->slots.slots_val,
				      (func_inl->slots.slots_len
				       * sizeof(*(func_inl->slots.slots_val))
				       ));
		
		memset(&(func_inl->slots.slots_val[i]),
		       0,
		       sizeof(func_inl->slots.slots_val[i]));
		
		break;
	}
	}
	
	return i;
}

/* End of file. */

