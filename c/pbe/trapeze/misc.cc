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
#include <mom/libmint.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>
#include "trapeze.h"

struct be_state *get_be_state()
{
	return( new trapeze_be_state );
}

trapeze_be_state::trapeze_be_state()
{
	this->name = "trapeze";
	cast_language = CAST_C;
}

trapeze_protocol_t what_protocol(pres_c_1 *pres)
{
	if (strcmp(pres->pres_context, "sun") == 0)
		return TRAPEZE_ONC;
	return TRAPEZE;
}

trapeze_mu_state::trapeze_mu_state(be_state *_state,
				   mu_state_op mu_op,
				   int mu_assumptions,
				   const char *mu_which,
				   int swap)
	: mem_mu_state(_state, mu_op, mu_assumptions, 2, 0, 120, mu_which)
{
	should_swap = swap;
	has_payload = 0;
	protocol = what_protocol(pres);
}

trapeze_mu_state::trapeze_mu_state(const trapeze_mu_state &must)
	: mem_mu_state(must)
{
	should_swap = must.should_swap;
	has_payload = must.has_payload;
	protocol = must.protocol;
}

mu_state *trapeze_mu_state::another(mu_state_op mu_op)
{
	return new trapeze_mu_state(state, mu_op, assumptions,
				    get_which_stub(),
				    should_swap);
}

mu_state *trapeze_mu_state::clone()
{
	return new trapeze_mu_state(*this);
}

trapeze_target_mu_state::trapeze_target_mu_state(be_state *_state,
						 mu_state_op mu_op,
						 int mu_assumptions,
						 const char *mu_which)
	: target_mu_state(_state, mu_op, mu_assumptions, mu_which)
{
	protocol = what_protocol(pres);
}

trapeze_target_mu_state::trapeze_target_mu_state(
	const trapeze_target_mu_state &must)
	: target_mu_state(must)
{
	protocol = must.protocol;
}

mu_state *trapeze_target_mu_state::clone()
{
	return new trapeze_target_mu_state(*this);
}

const char *trapeze_target_mu_state::get_encode_name() 
{
	const char *name;
	
	switch (protocol) {
	case TRAPEZE:
		name = "cdr";
		break;
	case TRAPEZE_ONC:
		name = "xdr";
		break;
	default:
		name = "";
		panic(("In `trapeze_target_mu_state::get_encode_name', "
		       "unrecognized protocol specifier."));
		break;
	}
	
	return name;
}

const char *trapeze_target_mu_state::get_be_name()
{
	return "trapeze";
}

trapeze_client_mu_state::trapeze_client_mu_state(be_state *_state,
						 mu_state_op mu_op,
						 int mu_assumptions,
						 const char *mu_which)
	: client_mu_state(_state, mu_op, mu_assumptions, mu_which)
{
	protocol = what_protocol(pres);
}

trapeze_client_mu_state::trapeze_client_mu_state(
	const trapeze_client_mu_state &must)
	: client_mu_state(must)
{
	protocol = must.protocol;
}

mu_state *trapeze_client_mu_state::clone()
{
	return new trapeze_client_mu_state(*this);
}

const char *trapeze_client_mu_state::get_encode_name() 
{
	const char *name;
	
	switch (protocol) {
	case TRAPEZE:
		name = "cdr";
		break;
	case TRAPEZE_ONC:
		name = "xdr";
		break;
	default:
		name = "";
		panic(("In `trapeze_client_mu_state::get_encode_name', "
		       "unrecognized protocol specifier."));
		break;
	}
	
	return name;
}

const char *trapeze_client_mu_state::get_be_name()
{
	return "trapeze";
}

const char *trapeze_mu_state::get_encode_name() 
{
	const char *name;
	
	switch (protocol) {
	case TRAPEZE:
		if ((should_swap == TRAPEZE_NO_SWAP) || !(op & MUST_DECODE))
			name = "cdr";
		else
			name = "cdr_swap";
		break;
	case TRAPEZE_ONC:
		if ((should_swap == TRAPEZE_NO_SWAP) || !(op & MUST_DECODE))
			name = "xdr";
		else
			name = "xdr_swap";
		break;
	default:
		name = "";
		panic(("In `trapeze_mu_state::get_encode_name', "
		       "unrecognized protocol specifier."));
		break;
	}
	
	return name;
}

/*
 * Return the index of the message word that contains the Trapeze reply token.
 * The index is relative to the start of the Trapeze control message data
 * buffer (as returned by `tpz_mtod') and is a word index, *not* a byte index.
 *
 * The position of the reply token depends on the protocol being used, because
 * we produce different message headers for each protocol.  See the header file
 * <flick/link/trapeze.h> for details.
 */
unsigned int trapeze_mu_state::get_replytoken_index()
{
	unsigned int index;
	
	switch (protocol) {
	case TRAPEZE:
		index = 3;
		break;
	case TRAPEZE_ONC:
		index = 7;
		break;
	default:
		index = 0;
		panic(("In `trapeze_mu_state::get_replytoken_index', "
		       "unrecognized protocol specifier."));
		break;
	}
	
	return index;
}

/*
 * Return the number of bytes in the control message buffer that are available
 * for message data marshaled by Flick --- i.e., the number of bytes in a
 * Trapeze control message data buffer, minus of whatever space is consumed by
 * ``magic'' header fields that are not m/u'ed through the back end's normal
 * mechanisms for m/u'ing message data.  In practice, the ``magic'' header data
 * is everything before the operation discriminator.
 *
 * The amount of data space depends on the protocol being used, because we
 * produce different message headers for each protocol.  See the header file
 * <flick/link/trapeze.h> for details.
 */
unsigned int trapeze_mu_state::get_control_msg_data_size()
{
	unsigned int header_words;
	
	switch (protocol) {
	case TRAPEZE:
		header_words = 2; // Two 4-byte words before Flick does m/u.
		break;
	case TRAPEZE_ONC:
		header_words = 6; // Six 4-byte word before Flick does m/u.
		break;
	default:
		header_words = 0;
		panic(("In `trapeze_mu_state::get_control_msg_data_size', "
		       "unrecognized protocol specifier."));
		break;
	}
	
	return (TRAPEZE_MAX_CONTROL_SIZE - (header_words * 4));
}

const char *trapeze_mu_state::get_be_name()
{
	return "trapeze";
}

const char *trapeze_mu_state::get_mu_stream_name()
{
	/*
	 * XXX - This is completely bogus!  The m/u stubs actually don't work
	 * at all, because we have taken away the stream model, and use local
	 * vars (which can be optimized into CPU registers).  However, m/u
	 * stubs need the stream abstraction if they are to get anything right.
	 */
	return "_buf_current";
};

void trapeze_mu_state::mu_server_func(pres_c_inline inl, mint_ref tn_r,
				      pres_c_server_func *sfunc,
				      pres_c_skel *sskel)
{
	has_payload = 0; /* reset the payload ptr */
	mem_mu_state::mu_server_func(inl, tn_r, sfunc, sskel);
}
	
/* This handles PRES_C_MAPPING_SYSTEM_EXCEPTION nodes,
   which are used to opaquely represent system exceptions.
   They're opaque, because their presentation varies greatly.
*/
void trapeze_mu_state::mu_mapping_system_exception(cast_expr cexpr,
						   cast_type /* ctype */,
						   mint_ref itype)
{
	mint_def *idef = &pres->mint.defs.defs_val[itype];
	
	assert(idef->kind == MINT_SYSTEM_EXCEPTION);
	
	/*
	 * The `mu_state' version of this method breaks the current glob
	 * because it doesn't know what the presentation-specific macro will
	 * do.  Well, neither do we!  But we don't call the presentation macro;
	 * we call a Trapeze-specific macro that implements our on-the-wire
	 * encoding of system exceptions, and which translates to/from the
	 * presented representation.
	 *
	 * XXX --- This is along the lines of the solution to PR flick/262.
	 */
	cast_expr macro
		= cast_new_expr_name(
			flick_asprintf("flick_%s_%s_system_exception",
				       get_be_name(),
				       get_buf_name()
				));
	
	cast_expr ofs_cexpr
		= cast_new_expr_lit_int(
			chunk_prim(TRAPEZE_SYSTEM_EXCEPTION_ALIGN_BITS,
				   TRAPEZE_SYSTEM_EXCEPTION_BYTES),
			0);
	
	cast_expr call_cexpr
		= cast_new_expr_call_3(macro,
				       ofs_cexpr,
				       cexpr,
				       cast_new_expr_name(pres->pres_context)
			);
	
	add_stmt(cast_new_stmt_expr(call_cexpr));
}

void
w_header_includes(pres_c_1 *pres)
{
	trapeze_protocol_t protocol;
	
	protocol = what_protocol(pres);
	
	w_printf("#define FLICK_PROTOCOL_TRAPEZE");
	switch (protocol) {
	case TRAPEZE:
		/* w_printf(""); */
		break;
	case TRAPEZE_ONC:
		w_printf("_ONC");
		break;
	default:
		panic(("In `w_header_includes', "
		       "unrecognized protocol specifier."));
		break;
	}
	w_printf("\n");
	
	w_printf("#include <flick/link/trapeze.h>\n");
	
	w_printf("#include <flick/encode/");
	switch (protocol) {
	case TRAPEZE:
		w_printf("cdr.h");
		break;
	case TRAPEZE_ONC:
		w_printf("xdr.h");
		break;
	default:
		panic(("In `w_header_includes', "
		       "unrecognized protocol specifier."));
		break;
	}
	w_printf(">\n");
}

/*
 * Replace the operation identifier with a sequentially numbered
 * constant value.  This reduces the overhead of [un]marshaling an
 * unbounded array for the operation name.
 */
void replace_operation_ids(pres_c_1 *pres,
			   mint_ref itype,
			   pres_c_inline inl)
{
	static int counter = 0;
	
	assert(pres->mint.defs.defs_val[itype].kind == MINT_UNION);
	mint_union_def *udef = &(pres->mint.defs.defs_val[itype].mint_def_u.
				 union_def);
	
	assert(inl->kind == PRES_C_INLINE_COLLAPSED_UNION);
	pres_c_inline_collapsed_union *cuinl
		= &(inl->pres_c_inline_u_u.collapsed_union);
	
	/* Find the appropriate case. */
	mint_ref case_var_r;
	for (int case_num = 0; ; case_num++) {
		if (case_num >= (signed int) udef->cases.cases_len) {
			if (udef->dfault == mint_ref_null)
				panic("union collapsed on nonexistent case");
			case_var_r = udef->dfault;
			break;
		}

		/* replace the mint_const here with a manufactured
		   constant integer, so we don't have to [un]marshal
		   an unbounded array -- just a 4-byte int! */
		if (mint_const_cmp(udef->cases.cases_val[case_num].val,
				   cuinl->discrim_val) == 0) {
			if (cuinl->discrim_val->kind != MINT_CONST_INT) {
				mint_const myconst =
					mint_new_const_int(counter++);
				cuinl->discrim_val = myconst;
				udef->cases.cases_val[case_num].val = myconst;
				udef->discrim =
					pres->mint.standard_refs.signed32_ref;
			}
			break;
		}
	}
}

/* End of file. */

