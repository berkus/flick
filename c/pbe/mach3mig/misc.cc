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

#include <mom/idl_id.h>
#include <mom/compiler.h>
#include <mom/libmint.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include "mach3.h"

#define m(n) (&pres->mint.defs.defs_val[n])
#define MAX_GLOB_SIZE 1050000

struct be_state *get_be_state()
{
	return( new mach3_be_state );
}

mach3_be_state::mach3_be_state()
{
	this->name = "mach3mig";
	cast_language = CAST_C;
}

mach3_mu_state::mach3_mu_state(be_state *_state,
			       mu_state_op mu_op,
			       int mu_assumptions,
			       const char *mu_which)
	: mem_mu_state(_state, mu_op, mu_assumptions, 2, 0, MAX_GLOB_SIZE,
		       mu_which)
{
	mach3_array = 0;
	mach3_outofline = 0;
	mach3_error = 0;
	is_complex = 0;
	id_expected = 1;
	is_mig_message = 0;
	inhibit_marshal = 0;
	
	/* Set our PRES_C_INLINE_TYPED flag and CAST expression to null. */
	marshaling_inline_typed = 0;
	tag_cexpr = 0;
	msg_option_expr = 0;
	timeout_expr = 0;
}

mach3_mu_state::mach3_mu_state(const mach3_mu_state &must)
	: mem_mu_state(must)
{
	mach3_array = must.mach3_array;
	mach3_outofline = must.mach3_outofline;
	mach3_error = must.mach3_error;
	is_complex = must.is_complex;
	id_expected = must.id_expected;
	is_mig_message = must.is_mig_message;
	inhibit_marshal = must.inhibit_marshal;
	marshaling_inline_typed = must.marshaling_inline_typed;
	tag_cexpr = must.tag_cexpr;
	msg_option_expr = must.msg_option_expr;
	timeout_expr = must.timeout_expr;
}

mu_state *mach3_mu_state::another(mu_state_op mu_op)
{
	return new mach3_mu_state(state, mu_op, assumptions, which_stub);
}

mu_state *mach3_mu_state::clone()
{
	return new mach3_mu_state(*this);
}

#if 0
/*
 * This code is no longer used.  id_expected is never 1 since we don't
 * marshal anything to identify the message as a Mig message.
 * Since mach3's mu_union_case was the only one that needed the 
 * discrim_val parameter, I removed it.  This helped hash_const::add_case()
 * which now won't have to convert its data_type into a mint_const.
 */
void mach3_mu_state::mu_union_case(functor *f, mint_const discrim_val)
{
	int old_id_expected = id_expected;
	
	if (id_expected == 1) {
		assert(discrim_val->kind == MINT_CONST_INT);
		/*
		 * XXX --- Now that we have symbolic constants we should use
		 * them!  But as a stopgap, assert that we have a literal.
		 */
		assert(discrim_val->mint_const_u_u.const_int.kind
		       == MINT_CONST_LITERAL);
		is_mig_message = ((discrim_val->
				   mint_const_u_u.const_int.mint_const_int_u_u.
				   value)
				  == IDL_MIG);
		id_expected = is_mig_message ? 2 : 0;
	}
	
	mem_mu_state::mu_union_case(f, discrim_val);
	
	id_expected = old_id_expected;
}
#endif

cast_stmt mach3_mu_state::make_error(int err_val) 
{
	/* suppress "array" appendages to back-end and encode names */
	mach3_error = 1;
	cast_stmt retval = mem_mu_state::make_error(err_val);
	mach3_error = 0;
	return retval;
}

mach3_target_mu_state::mach3_target_mu_state(be_state *_state,
					     mu_state_op mu_op,
					     int mu_assumptions,
					     const char *mu_which)
	: target_mu_state(_state, mu_op, mu_assumptions, mu_which)
{
	target_remote = cast_new_expr_name("MACH_MSG_TYPE_COPY_SEND");
	target_local = cast_new_expr_name("MACH_MSG_TYPE_MAKE_SEND_ONCE");
}

mach3_target_mu_state::mach3_target_mu_state(const mach3_target_mu_state &must)
	: target_mu_state(must)
{
	target_remote = must.target_remote;
	target_local = must.target_local;
}

mu_state *mach3_target_mu_state::clone()
{
	return new mach3_target_mu_state(*this);
}

const char *mach3_target_mu_state::get_be_name()
{
	return "mach3mig";
}

const char *mach3_target_mu_state::get_encode_name()
{
	return "mach3mig";
}

mach3_client_mu_state::mach3_client_mu_state(be_state *_state,
					     mu_state_op mu_op,
					     int mu_assumptions,
					     const char *mu_which)
	: client_mu_state(_state, mu_op, mu_assumptions, mu_which)
{
	client_remote = cast_new_expr_lit_int(0, 0);
	client_local = cast_new_expr_lit_int(0, 0);
}

mach3_client_mu_state::mach3_client_mu_state(const mach3_client_mu_state &must)
	: client_mu_state(must)
{
	client_remote = must.client_remote;
	client_local = must.client_local;
}

mu_state *mach3_client_mu_state::clone()
{
	return new mach3_client_mu_state(*this);
}

const char *mach3_client_mu_state::get_be_name()
{
	return "mach3mig";
}

const char *mach3_client_mu_state::get_encode_name()
{
	return "mach3mig";
}

const char *mach3_mu_state::get_be_name()
{
	if (mach3_array && !mach3_error)
		return "mach3mig_array";
	else
		return "mach3mig";
}

const char *mach3_mu_state::get_encode_name()
{
	if (mach3_array && !mach3_error)
		return "mach3mig_array";
	else
		return "mach3mig";
}

const char *mach3_mu_state::get_mu_stream_name()
{
	/*
	 * XXX - This is completely bogus!  The m/u stubs actually don't work
	 * at all, because we have taken away the stream model, and use local
	 * vars (which can be optimized into CPU registers).  However, m/u
	 * stubs need the stream abstraction if they are to get anything right.
	 */
	return "_buf_current";
};

void w_header_includes(pres_c_1 * /*p*/)
{
	w_printf("#include <flick/link/mach3mig.h>\n\n");
	w_printf("#include <flick/encode/mach3mig.h>\n\n");
}

/*
 * `remove_idl_and_interface_ids' strips away the ``collapsed union'' stuff
 * that encodes IDL and interface information.  Mach3MIG client and server
 * stubs don't need to encode this information because it is manifest in the
 * object references (ports).
 *
 * Similarly, `remove_operation_id' strips away the ``collapsed union'' stuff
 * that encodes an operation identifier.  This information isn't required in
 * reply messages.
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

void mach3_mu_state::set_id_expected(
	mint_ref operation_union_iref)
{
	mint_def *operation_union_idef = m(operation_union_iref);
	mint_ref discrim_iref;
	
	if (operation_union_idef->kind == MINT_UNION) {
		/* Dig the discriminator out. */
		discrim_iref = operation_union_idef->
			       mint_def_u.union_def.discrim;
		
		if ((m(discrim_iref))->kind == MINT_INTEGER)
			/*
			 * The operation ID is an integer, so we encode it in
			 * the `msgh_id' field of the message headers.
			 */
			id_expected = 2;
		else
			/*
			 * The operation identifier is too complicated to
			 * encode in the Mach message header.
			 */
			id_expected = 0;
	} else
		/*
		 * We don't even have a union?!
		 */
		id_expected = 0;
}

void mach3_mu_state::mu_end()
{
	/* we need to align ourselves to a 4-byte boundary */
	if ((align_bits < 2) || (align_ofs & 3))
		chunk_prim(2, 0);

	/* now do the default thing */
	mem_mu_state::mu_end();
}

/* End of file. */

