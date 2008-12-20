/*
 * Copyright (c) 1995, 1996, 1997, 1999 The University of Utah and
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

#include "private.hh"

/*
 * By default, this routine doesn't do any further inlining; it just creates an
 * atom and switches back into 1-1 mapping mode.  Subclasses can override this
 * to do more elaborate inlining.
 */
pres_c_inline pg_state::p_inline_type(aoi_type at,
				      char *slot_name,
				      p_type_collection */*inl_ptc*/,
				      cast_type inl_ctype)
{
	p_type_collection *ptc = 0;
	p_type_node *ptn;
	cast_type ctype;
	pres_c_mapping map;
	
	p_type(at, &ptc);
	ptn = ptc->find_type("definition");
	ctype = ptn->get_type();
	map = ptn->get_mapping();
	
	return p_inline_add_atom(inl_ctype, slot_name, ctype, map);
}

/* End of file. */

