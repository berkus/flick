/*
 * Copyright (c) 1998, 1999 The University of Utah and
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
#include <mom/c/pfe.hh>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include <mom/c/pg_corbaxx.hh>

void pg_corbaxx::p_indirect_type(aoi_ref ref, p_type_collection **out_ptc)
{
	p_type_collection *ptc;
	
	/*
	 * Generate a CAST type name (or an appropriate "special" type name)
	 * and a PRES_C_MAPPING_STUB.
	 */
	ptc = p_type_collection::find_collection(&type_collections,
						 ref);
	if( !ptc ) {
		p_type_node *ptn;
		
		/* Couldn't find the real collection make a fake one for now */
		ptc = p_new_type_collection(calc_name_from_ref(ref));
		ptn = new p_type_node;
		ptn->set_name("definition");
		ptn->set_format("%s");
		ptn->set_type( p_make_ctypename(ref) );
		ptn->set_mapping( pres_c_new_mapping(PRES_C_MAPPING_STUB) );
		ptc->add_type("default", ptn);
	}
	if( *out_ptc )
		(*out_ptc)->set_collection_ref(ptc);
	else
		*out_ptc = ptc;
}

/* End of file. */

