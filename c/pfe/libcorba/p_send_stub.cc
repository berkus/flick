/*
 * Copyright (c) 1998 The University of Utah and
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

#include <mom/c/pg_corba.hh>

int pg_corba::p_msg_send_stub_special_params(aoi_interface *ai,
					     stub_special_params *specials,
					     int request)
{
	/* Do the library thing */
	int newparms
		= pg_state::p_msg_send_stub_special_params(ai, specials,
							   request);
	stub_special_params::stub_param_info *this_param;
	
	/*
	 * Set the environment reference type and index.  A CORBA environment
	 * reference has type `CORBA_Environment *' and appears after all of
	 * the normal parameters.  We place it after any SIDs as well.
	 */
	this_param = &(specials->params[stub_special_params::environment_ref]);
	
	this_param->ctype =
		cast_new_pointer_type(
			cast_new_type_name(
				/*
				 * XXX --- Don't use `ao->name' until
				 * `pg_corba::p_get_env_struct_type' has access
				 * to the operation name, too.
				 */
				calc_client_stub_environment_type_name("")
				));
	this_param->index = newparms;
	newparms++;
	
	/* Finally, we're done! */
	return newparms;
}

/* End of file. */
