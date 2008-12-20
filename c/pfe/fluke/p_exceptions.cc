/*
 * Copyright (c) 1997, 1998, 1999 The University of Utah and
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

#include <mom/compiler.h>
#include <mom/libaoi.h>
#include <mom/libmint.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include "pg_fluke.hh"

/* The Fluke presentation lets the user define the exception constants. */
void pg_fluke::p_except_type(aoi_exception *as,
			     p_type_collection **out_ptc)
{
	/* skip the pg_corba code */
	pg_state::p_except_type(as, out_ptc);
}

cast_type pg_fluke::p_get_env_struct_type() 
{
	static cast_type env = 0;
	if (env)
		return env;
	env = cast_new_struct_type(0);

	cast_scope *scope = &env->cast_type_u_u.agg_type.scope;
	int cdef;
	
	cdef = cast_add_def(scope,
			    cast_new_scoped_name("_type", NULL),
			    CAST_SC_NONE,
			    CAST_VAR_DEF,
			    ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
			    CAST_PROT_NONE);
	scope->cast_scope_val[cdef].u.cast_def_u_u.var_def.type =
		cast_new_prim_type(CAST_PRIM_INT, CAST_MOD_SIGNED);
	cdef = cast_add_def(scope,
			    cast_new_scoped_name("_except", NULL),
			    CAST_SC_NONE,
			    CAST_VAR_DEF,
			    ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
			    CAST_PROT_NONE);
	scope->cast_scope_val[cdef].u.cast_def_u_u.var_def.type =
		cast_new_pointer_type(cast_new_type(CAST_TYPE_VOID));
	
	if (gen_client)
		env->cast_type_u_u.agg_type.name
			= cast_new_scoped_name(
				calc_client_stub_environment_type_name(""),
				NULL);
	else if (gen_server)
		env->cast_type_u_u.agg_type.name
			= cast_new_scoped_name(
				calc_server_func_environment_type_name(""),
				NULL);
	else
		panic("In `pg_corba::p_get_env_struct_type', "
		      "generating neither client nor server.");
	
	return env;
}

int pg_fluke::p_get_exception_discrim() 
{
	return 0;
}

int pg_fluke::p_get_exception_void()
{
	return 1;
}

pres_c_inline_atom pg_fluke::p_get_user_discrim() 
{
	pres_c_mapping direct = pres_c_new_mapping(PRES_C_MAPPING_ELSEWHERE);
	return  pres_c_new_inline_atom(0, direct)->pres_c_inline_u_u.atom;
}

/* Given a MINT constant that refers to an exception id, this function
   creates and returns a corresponding CAST expression.
   In Fluke, the exception id is already in the _type field of the
   environment, and thus needs no cast expression assigned to it at all. */
cast_expr pg_fluke::p_mint_exception_id_const_to_cast(
	mint_const /* mint_literal */)
{
	return 0;
}

void pg_fluke::p_do_return_union(aoi_operation *ao,
				 pres_c_inline* reply_l4_inl,
				 mint_ref reply_ref,
				 cast_ref cfunc,
				 pres_c_inline_index discrim_idx)
{
	// We need to map all the constants to the appropriate symbolic
	// constants
	mint_union_def *mu = &(m(reply_ref).mint_def_u.union_def);
	
	assert(mu->cases.cases_len == 2);
	
	mu->cases.cases_val[0].val
		= mint_new_symbolic_const(MINT_CONST_INT,
					  "exc_mom_no_exception");
	mu->cases.cases_val[1].val
		= mint_new_symbolic_const(MINT_CONST_INT,
					  "exc_mom_system_exception");
	
	if (mu->dfault != -1) {
		mu = &(m(mu->dfault).mint_def_u.union_def);
		for (unsigned int i = 0; i < mu->cases.cases_len; i++) {
			unsigned int j = 0;
			mint_union_case *ucase = &(mu->cases.cases_val[i]);
			
			for (; j < in_aoi->defs.defs_len; j++) {
				if (aoi_to_mint_association[j] == ucase->var)
					break;
			}
			assert(j < in_aoi->defs.defs_len);
			
			ucase->val
				= mint_new_symbolic_const(
					MINT_CONST_INT,
					flick_asprintf("exc_%s",
						       getscopedname(j))
					);
		}
	}
	pg_state::p_do_return_union(ao, reply_l4_inl, reply_ref, cfunc,
				    discrim_idx);
}

/* End of file. */

