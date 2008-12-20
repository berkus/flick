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

#if 0

#include <mom/compiler.h>
#include <mom/c/pfe.hh>

#include "private.hh"

#ifdef ATTRIBUTE_STUBS
/* Generate server function presentation for an AOI interface attribute. */

// We need to increment the index i 1 for readonly, 
// or 2 for read/write attributes
void 
pg_state::p_server_attrib_func(pres_c_skel *sskel,
			       int *i,
			       aoi_interface *a,
			       aoi_attribute *aa)
{
	const char *attrib_name	= aa->name;
	const char *get_pref	= "_get_";
	const char *set_pref	= "_set_";
	
	aoi_operation *ao = (aoi_operation *)
			    mustcalloc(sizeof(aoi_operation));
	
	/* XXX --- Should `calc_name_*' this. */
	ao->name = flick_asprintf("%s%s", get_pref, attrib_name);
	
	ao->request_code = aa->read_request_code;
	ao->reply_code = aa->read_reply_code;
	
	ao->flags = AOI_OP_FLAG_NONE;
	
	ao->params.params_len = 0;
	ao->params.params_val = 0;
	
	ao->return_type = aa->type;
	
	ao->exceps.exceps_len = 0;
	ao->exceps.exceps_val = 0;
	
	p_server_func(&(sskel->funcs.funcs_val[*i]), a, ao);
	
	(*i)++;
	if (!aa->readonly) {
		/* XXX --- Should `calc_name_*' this. */
		ao->name = flick_asprintf("%s%s", set_pref, attrib_name);
		
		ao->request_code = aa->write_request_code;
		ao->reply_code = aa->write_reply_code;
		
		ao->flags = AOI_OP_FLAG_NONE;
		
		ao->params.params_len = 1;
		ao->params.params_val = (aoi_parameter *)
					mustcalloc(sizeof(aoi_parameter));
		
		ao->params.params_val[0].name = aa->name;
		ao->params.params_val[0].direction = AOI_DIR_IN;
		ao->params.params_val[0].type = aa->type;
		
		ao->return_type = (aoi_type) mustcalloc(sizeof(aoi_type_u));
		ao->return_type->kind = AOI_VOID;
		
		ao->exceps.exceps_len = 0;
		ao->exceps.exceps_val = 0;
		
		p_server_func(&(sskel->funcs.funcs_val[*i]), a, ao);
		
		(*i)++;
	}
}

#endif /* ATTRIBUTE_STUBS */
#endif /* 0 */

/* End of file. */

