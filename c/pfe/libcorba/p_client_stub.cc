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

#include <mom/libaoi.h>
#include <mom/c/libcast.h>

#include <mom/c/pg_corba.hh>

void pg_corba::p_client_stub_special_params(aoi_operation *ao,
					    stub_special_params *specials)
{
	stub_special_params::stub_param_info *this_param;
	
	/* Do the library thing... */
	pg_state::p_client_stub_special_params(ao, specials);
	
	/*
	 * Set the object reference type and index.  In CORBA, the object
	 * reference is the first parameter to the client stub.  (This is what
	 * the PG library does anyway; we redo it just to be explicit!)
	 */
	this_param = &(specials->params[stub_special_params::object_ref]);
	
	this_param->ctype =
		cast_new_type_name(
			calc_client_stub_object_type_name(ao->name));
	this_param->index = 0;
	
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
	this_param->index = (ao->params.params_len + 1
			     + (gen_sids ?
				3 /* after three SID arguments */ :
				0 /* no SID arguments */ ));
	
	/*
	 * Set the (effective) client SID index.  We do not set the type; CORBA
	 * has no standard for SID types so we accept whatever type the PG
	 * library has provided.
	 */
	this_param = &(specials->params[stub_special_params::client_sid]);
	
	if (gen_sids)
		this_param->index = (ao->params.params_len + 1);
	else
		this_param->index = -1;
	
	/*
	 * Set the required server SID index.  Again, we do not set the type.
	 */
	this_param = &(specials->params[stub_special_params::
				       required_server_sid]);
	
	if (gen_sids)
		this_param->index = (ao->params.params_len + 2);
	else
		this_param->index = -1;
	
	/*
	 * Set the actual server SID index.  Again, we do not set the type.
	 */
	this_param = &(specials->params[stub_special_params::
				       actual_server_sid]);
	
	if (gen_sids)
		this_param->index = (ao->params.params_len + 3);
	else
		this_param->index = -1;
	
	/* Finally, we're done! */
}

/*
 * This method determines the return type of a client stub.
 */
void pg_corba::p_client_stub_return_type(aoi_operation *ao, int mr,
					 cast_type *out_ctype,
					 pres_c_mapping *out_mapping)
{
	/* CORBA-ize the return value --- se `pg_corba::p_param_type()'. */
	p_param_type(ao->return_type, mr, AOI_DIR_RET, out_ctype, out_mapping);
}

/* End of file. */

