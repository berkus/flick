/*
 * Copyright (c) 1995, 1996 The University of Utah and
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
#include <mom/libmint.h>
#include <mom/c/libpres_c.h>
#include <mom/c/pbe.hh>

/* `remove_idl_and_interface_ids' strips away the ``collapsed union'' stuff
   that encodes IDL and interface information.  Fluke client and server stubs
   don't need to encode this information because it is manifest in Fluke
   object references.
   
   Similarly, `remove_operation_id' strips away the ``collapsed union'' stuff
   that encodes an operation identifier.  This information isn't required in
   reply messages.
   */

void remove_idl_and_interface_ids(
	pres_c_1 *pres,
	mint_ref in_itype, pres_c_inline in_inline,
	mint_ref *out_itype, pres_c_inline *out_inline)
{
	*out_itype  = in_itype;
	*out_inline = in_inline;
	
	descend_collapsed_union(pres, out_itype, out_inline);
	descend_collapsed_union(pres, out_itype, out_inline);
}

void remove_operation_id(
	pres_c_1 *pres,
	mint_ref in_itype, pres_c_inline in_inline,
	mint_ref *out_itype, pres_c_inline *out_inline)
{
	*out_itype  = in_itype;
	*out_inline = in_inline;
	
	descend_collapsed_union(pres, out_itype, out_inline);
}

