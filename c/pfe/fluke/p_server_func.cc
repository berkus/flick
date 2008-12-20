/*
 * Copyright (c) 1996, 1997, 1998, 1999 The University of Utah and
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

#include "pg_fluke.hh"

void pg_fluke::p_server_func_special_params(aoi_operation *ao,
					    stub_special_params *specials)
{
	/* Do the library thing... */
	pg_corba::p_server_func_special_params(ao, specials);
	
	/*
	 * ...and then change the types of the object reference, the client
	 * SID, and the required server SID.
	 *
	 * In Fluke, the server-side object reference type is a pointer to an
	 * application-defined structure.
	 */
	specials->params[stub_special_params::object_ref].ctype =
		cast_new_pointer_type(
			cast_new_type_name(
				calc_server_func_object_type_name(ao->name)
				));
	
	/*
	 * The presented SIDs are constant.
	 */
	specials->params[stub_special_params::client_sid].ctype =
		cast_new_qualified_type(
			cast_new_type_name(
				calc_server_func_client_sid_type_name(
					ao->name
					)),
			CAST_TQ_CONST);
	
	/* XXX --- Note that this is ultimately ineffectual; see code below. */
	specials->params[stub_special_params::required_server_sid].ctype =
		cast_new_qualified_type(
			cast_new_type_name(
				calc_server_func_server_sid_type_name(
					ao->name
					)),
			CAST_TQ_CONST);
	
	/*
	 * Remove the required server SID from the argument list.  As of
	 * December 1997, Flask no longer requires the server SIDs in the
	 * server work function parameter list.
	 *
	 * Reset all the SID and environment reference indices, to be safe.
	 */
	specials->params[stub_special_params::client_sid].index
		= (gen_sids ?
		   ((signed int) (ao->params.params_len + 1)) :
		   -1);
	
	specials->params[stub_special_params::required_server_sid].index = -1;
	specials->params[stub_special_params::actual_server_sid].index   = -1;
	
	specials->params[stub_special_params::environment_ref].index
		= (ao->params.params_len
		   + 1 /* the target object reference */
		   + (gen_sids ?
		      1 /* after the client sid */ :
		      0 /* no SID arguments */ ));
}

/* End of file. */

