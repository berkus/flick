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

#include <assert.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include <mom/c/pg_corbaxx.hh>

void pg_corbaxx::p_namespace_def()
{
	cast_scope *poa_scope, *scope, *new_scope;
	cast_def_protection last_protection = current_protection;
	cast_scoped_name scn;
	char *poa_name;
	int cdef;
	
	current_protection = CAST_PROT_NONE;
	scope = (cast_scope *)top_ptr(scope_stack);
	/* Default scope */
	scn = cast_new_scoped_name(name, NULL);
	new_scope = (cast_scope *)mustmalloc(sizeof(cast_scope));
	*new_scope = cast_new_scope(0);
	cdef = cast_add_def(scope,
			    scn,
			    CAST_SC_NONE,
			    CAST_NAMESPACE,
			    ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
			    CAST_PROT_NONE);
	scope->cast_scope_val[cdef].u.cast_def_u_u.
		new_namespace = new_scope;
	cast_add_scope_name(&current_scope_name,
			    name,
			    null_template_arg_array);
	push_ptr(scope_stack, new_scope);
	
	/* POA scope */
	poa_scope = (cast_scope *)top_ptr(poa_scope_stack);
	poa_name = add_poa_scope(name);
	scn = cast_new_scoped_name(poa_name, NULL);
	new_scope = (cast_scope *)mustmalloc(sizeof(cast_scope));
	*new_scope = cast_new_scope(0);
	cdef = cast_add_def(poa_scope,
			    scn,
			    CAST_SC_NONE,
			    CAST_NAMESPACE,
			    ch(cur_aoi_idx, PG_CHANNEL_SERVER_DECL),
			    CAST_PROT_NONE);
	poa_scope->cast_scope_val[cdef].u.
		cast_def_u_u.new_namespace = new_scope;
	cast_add_scope_name(&current_poa_scope_name,
			    poa_name,
			    null_template_arg_array);
	push_ptr(poa_scope_stack, new_scope);
	
	/* Recursively generate the defs */
	gen_scope(a(cur_aoi_idx).scope + 1);
	
	pop_ptr(scope_stack);
	cast_del_scope_name(&current_scope_name);
	pop_ptr(poa_scope_stack);
	cast_del_scope_name(&current_poa_scope_name);
	
	current_protection = last_protection;
}

/* End of file. */

