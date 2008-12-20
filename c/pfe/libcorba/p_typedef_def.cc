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

#include <mom/libaoi.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include <mom/c/pg_corba.hh>

void pg_corba::p_typedef_def(aoi_type at)
{
	/* Do the normal `pg_state' thing... */
	pg_state::p_typedef_def(at);
	
	/* ...and then do something special for fixed-length array types. */
	if (at->kind == AOI_ARRAY) {
		aoi_array *aa = &(at->aoi_type_u_u.array_def);
		unsigned int min, max;
		
		aoi_get_array_len(in_aoi, aa, &min, &max);
		if (min == max)
			/*
			 * Define an ``array slice'' type corresponding to the
			 * array type that we just defined.
			 */
			p_typedef_array_slice_def(aa);
	}
}

void pg_corba::p_typedef_array_slice_def(aoi_array *aa)
{
	p_type_collection *array_ptc = 0, *elem_ptc = 0;
	p_type_node *ptn, *new_ptn;
	
	/* Create the ctype and mapping for the array's target. */
	p_type(aa->element_type, &elem_ptc);
	p_indirect_type(cur_aoi_idx, &array_ptc);
	ptn = elem_ptc->find_type("definition");
	
	/*
	 * Emit the `typedef' for the ``array slice'' type corresponding to
	 * the presented array type.  See Section 14.13 of the CORBA 2.0 spec
	 * for inforamtion about array slices.
	 */
	new_ptn = new p_type_node;
	new_ptn->set_name("array slice");
	new_ptn->set_format("%s_slice");
	new_ptn->set_type(ptn->get_type());
	array_ptc->add_type("default", new_ptn);
	new_ptn->add_def(array_ptc->find_scope("default"));
}

/* End of file. */

