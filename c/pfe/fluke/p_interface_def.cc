/*
 * Copyright (c) 1996, 1997 The University of Utah and
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

#include "pg_fluke.hh"

/*
 * In Fluke, server-side target objects are represented by application-defined
 * structures.  (A target object is an object upon which an operation is being
 * invoked.)  Server work functions receive pointers to these structures.
 * Object references passed as parameters are presented as `mom_ref_t's, as
 * determined by `pg_fluke::p_indirect_type'.
 *
 * On the client side, all object references are presented as `mom_ref_t's.
 * When we are generating a client presentation, we never get to the method
 * below because `pg_fluke::p_typedef_def' screens out client-side interface
 * typedefs.
 */

/*
 * NOTE that at the time of this writing, the following method is identical to
 * the CORBA PG library version.  I decided to keep this explicit override,
 * however, to make it more clear what is going on with interface types in the
 * Fluke presentation.
 */

void pg_fluke::p_interface_type(aoi_interface * /*ai*/,
				p_type_collection **out_ptc)
{
	/* Call the method that we use for forward interface declarations. */
	p_forward_type(out_ptc);
}

/* End of file. */

