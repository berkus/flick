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

#include <assert.h>
#include <string.h>

#include <mom/compiler.h>
#include <mom/libmint.h>
#include <mom/c/pfe.hh>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include "pg_sun.hh"

/* Generate a client stub presentation for an AOI interface operation. */
void pg_sun::p_client_stub(aoi_interface *a, aoi_operation *ao)
{
	/*
	 * For every operation, `rpcgen' outputs a #define:
	 * #define <operation_name> ((unsigned long) <operation_code>).
	 * We emulate that behavior here.
	 */
	cast_expr value = cast_new_expr(CAST_EXPR_LIT_PRIM);
	
	/* Fill out the value. */
	assert(ao->request_code->kind == AOI_CONST_INT);
	value->cast_expr_u_u.lit_prim.u.kind = CAST_PRIM_INT;
	value->cast_expr_u_u.lit_prim.mod    = (CAST_MOD_UNSIGNED |
						CAST_MOD_LONG);
	value->cast_expr_u_u.lit_prim.u.cast_lit_prim_u_u.i =
		ao->request_code->aoi_const_u_u.const_int;
	
	/* Fill out the CAST defintion. */
	int cdef = cast_add_def((cast_scope *)top_ptr(scope_stack),
				cast_new_scoped_name(ao->name, NULL),
				CAST_SC_NONE,
				CAST_DEFINE,
				ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
				CAST_PROT_NONE);
	c(cdef).u.cast_def_u_u.define_as = value;
	
	/*
	 * Finally, invoke the `real' version of `p_client_stub()' to actually
	 * create the client stub.
	 */
	pg_state::p_client_stub(a, ao);
}

void pg_sun::p_client_stub_special_params(aoi_operation *ao,
					  stub_special_params *specials)
{
	/* Do the library thing... */
	pg_state::p_client_stub_special_params(ao, specials);
	
	/*
	 * ...but change the type and position of the object reference.  In
	 * Sun RPC presentations, the object reference is a `CLIENT *' and
	 * comes after the ordinary parameters.
	 */
	specials->params[stub_special_params::object_ref].ctype =
		cast_new_pointer_type(
			cast_new_type_name(
				calc_client_stub_object_type_name(ao->name)
				));
	
	specials->params[stub_special_params::object_ref].index =
		(ao->params.params_len);
	
	specials->params[stub_special_params::environment_ref].spec
		= CAST_PARAM_IMPLICIT;
	specials->params[stub_special_params::environment_ref].index
		= ao->params.params_len + 1;
}

/*
 * This method determines how `p_client_stub' processes the return type of a
 * client stub.
 */
void pg_sun::p_client_stub_return_type(aoi_operation *ao, mint_ref /*mr*/,
				       cast_type *out_ctype,
				       pres_c_mapping *out_mapping)
{
	p_type_collection *ptc = 0;
	p_type_node *ptn;
	pres_c_allocation alloc;
	
	/* The default behavior is to simply call `p_type'. */
	p_type(ao->return_type, &ptc);
	ptn = ptc->find_type("definition");
	*out_ctype = ptn->get_type();
	*out_mapping = ptn->get_mapping();
	
	/*
	 * However, a Sun RPC client stub returns a pointer to the type
	 * specified in the IDL file.  The data is statically allocated.
	 */
	alloc.cases[PRES_C_DIRECTION_IN].allow = PRES_C_ALLOCATION_INVALID;
	alloc.cases[PRES_C_DIRECTION_INOUT].allow = PRES_C_ALLOCATION_INVALID;
	alloc.cases[PRES_C_DIRECTION_OUT].allow = PRES_C_ALLOCATION_INVALID;
	alloc.cases[PRES_C_DIRECTION_UNKNOWN].allow
		= PRES_C_ALLOCATION_INVALID;
	alloc.cases[PRES_C_DIRECTION_RETURN].allow
		= PRES_C_ALLOCATION_ALLOW;
	alloc.cases[PRES_C_DIRECTION_RETURN].pres_c_allocation_u_u.val.flags
		= PRES_C_ALLOC_ALWAYS | PRES_C_DEALLOC_ALWAYS;
	alloc.cases[PRES_C_DIRECTION_RETURN].pres_c_allocation_u_u.val.
		allocator.kind = PRES_C_ALLOCATOR_STATIC;
	alloc.cases[PRES_C_DIRECTION_RETURN].pres_c_allocation_u_u.val.
		alloc_init = 0;
	pres_c_interpose_indirection_pointer(out_ctype, out_mapping, alloc);
	
	/*
	 * Tell the back end that this is the ``root'' of the parameter.
	 */
	pres_c_interpose_param_root(out_mapping, 0, 0);
	
	/*
	 * Finally, wrap the mapping in a `hint' that tells the back end what
	 * kind of parameter this is: `in', `out', etc.
	 */
	pres_c_interpose_direction(out_mapping, AOI_DIR_RET);
}

/* End of file. */

