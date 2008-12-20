/*
 * Copyright (c) 1996, 1997, 1999 The University of Utah and
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

#include <mom/c/libpres_c.h>

#include "private.hh"

pres_c_inline pg_state::p_inline_exception(aoi_exception *ae,
					   p_type_collection *inl_ptc,
					   cast_type inl_ctype)
{
	pres_c_inline		inl   = pres_c_new_inline_struct(ae->slots.
								 slots_len);
	pres_c_inline_struct	*sinl = &(inl->pres_c_inline_u_u.struct_i);
	
	aoi_exception_slot	*slot;
	
	/*****/
	
	/*
	 * XXX --- Need to save/restore `name' for now, for the benefit of
	 * `p_variable_array_type' at least.  Grrr.
	 */
	char *old_name = name;
	
	for (unsigned int i = 0; i < ae->slots.slots_len; i++) {
		slot = &(ae->slots.slots_val[i]);
		name = calc_struct_slot_name(slot->name);
		
		sinl->slots.slots_val[i].mint_struct_slot_index
			= i;
		sinl->slots.slots_val[i].inl
			= p_inline_type(slot->type,
					// calc_struct_slot_name(slot->name)
					name,
					inl_ptc,
					inl_ctype);
	}
	
	name = old_name;
	
	return inl;
}

/* End of file. */

