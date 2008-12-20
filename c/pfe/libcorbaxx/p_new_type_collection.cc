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

#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include <mom/c/pg_corbaxx.hh>

/* Build a new type collection, this just adds the poa
   scope to the collection made by pg_state */
p_type_collection *pg_corbaxx::p_new_type_collection(const char *base_name)
{
	p_type_collection *retval;
	p_scope_node *psn;
	
	retval = pg_state::p_new_type_collection(base_name);
	
	psn = new p_scope_node;
	psn->set_name("poa");
	psn->set_prefix("POA_");
	psn->set_scope((cast_scope *)top_ptr(poa_scope_stack));
	psn->set_scope_name(cast_copy_scoped_name(&current_poa_scope_name));
	retval->add_scope(psn);
	psn->set_channel(ch(cur_aoi_idx, PG_CHANNEL_SERVER_DECL));
	
	return( retval );
}

/* End of file. */

