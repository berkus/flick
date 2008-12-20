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

#include <mom/compiler.h>
#include <mom/libmint.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include "trapeze.h"

static void check_for_warning(unsigned len_max, cast_expr has_payload, cast_expr ptr)
{
	static int spitwarn1 = 0;
	static int spitwarn2 = 0;

	if (!spitwarn1
	    && len_max > TRAPEZE_MAX_PAYLOAD_SIZE) {
		warn("Maximum size of arrays cannot exceed %d bytes.",
		     TRAPEZE_MAX_PAYLOAD_SIZE);
		spitwarn1++;
	}
	
	if (!spitwarn2
	    && has_payload
	    && has_payload != ptr) {
		warn("Only one array can be sent in a message!");
		spitwarn2++;
	}
}

void trapeze_mu_state::mu_array(cast_expr array_expr,
				cast_type array_ctype,
				cast_type elem_ctype,
				mint_ref elem_itype,
				pres_c_mapping elem_map,
				char *cname)
{
	int elem_size, elem_align_bits;
	
	/* see if a bcopy is possible for this data */
	cast_expr bcopy_poss = mu_get_sizeof(elem_itype,
					     elem_ctype,
					     elem_map,
					     &elem_size,
					     &elem_align_bits);
	
	/* If the array is small, let's just put it in the control buffer. */
	unsigned len_min, len_max, control_msg_data_size;
	mu_array_get_encoded_bounds(&len_min, &len_max, cname);
	control_msg_data_size = get_control_msg_data_size();
	
	if ((len_max <= 1)
	    || (bcopy_poss
		&& ((len_max < control_msg_data_size) /* XXX check overflow */
		    && (len_max * elem_size + max_msg_size
			< control_msg_data_size)))) {
		/* Do the normal thing and bail. */
		mem_mu_state::mu_array(array_expr, array_ctype,
				       elem_ctype, elem_itype, elem_map,
				       cname);
		return;
	}
	
	/* allocate a DMA buffer, if needed
	   (but not for the special environment var _ev) */
	if ((op & MUST_ALLOCATE) &&
	    (array_ctype->kind == CAST_TYPE_POINTER) &&
	    (elem_itype == pres->mint.standard_refs.void_ref) &&
	    array_expr->kind == CAST_EXPR_NAME &&
	    strcmp(array_expr->cast_expr_u_u.name.
		   cast_scoped_name_val[0].name, "_ev")) {
		add_stmt(cast_new_stmt_expr(cast_new_expr_assign(
			array_expr, cast_new_expr_name(
			flick_asprintf("flick_%s_%s_array__alloc()",
			       get_be_name(),
			       which_stub,
			       get_buf_name())))));
		return;
	}
	
	/* Get the length expression from the arglist. */
	cast_expr len_expr;
	cast_type len_type;
	int gotarg = arglist->getargs(cname, "length", &len_expr, &len_type);
	assert(gotarg);
	assert(len_expr);
	assert(len_type);
	
	/* calculate the length in bytes of the array */
	if (!bcopy_poss)
		bcopy_poss = cast_new_expr_sizeof_type(elem_ctype);

	cast_expr length_expr =
		cast_new_binary_expr(CAST_BINARY_MUL,
				     bcopy_poss,
				     len_expr);
	
	/*
	 * Bounds check the array, but only for *real* arrays.
	 * Normally, this is taken care of by the standard mu_array(), but
	 * we aren't running that here.
	 */
	if (array_data.is_valid)
		mu_array_check_bounds(cname);
	
	/* Check for bit translation */
	cast_stmt bt_if = mu_bit_translation_necessary(0, elem_itype);
	assert(bt_if);
	cast_stmt bt_end = mu_bit_translation_necessary(2, elem_itype);
	assert(bt_end);
	if (strncmp("#if 0", bt_if->cast_stmt_u_u.text, 5) == 0) {
		bt_if = bt_end = 0;
	}

	/* swap on encode */
	if (bt_if && (op & MUST_ENCODE)
	    && (elem_size == 8 || elem_size == 4 || elem_size == 2)) {
		add_stmt(bt_if);
		add_stmt(cast_new_stmt_expr(cast_new_expr_call_2(
			cast_new_expr_name(flick_asprintf(
				"flick_%s_array_swap%d",
				get_be_name(),
				elem_size * 8)),
			array_expr,
			length_expr)));
		add_stmt(bt_end);
	}
	
	/* call the macro to do the [un]marshaling */
	cast_expr macro = cast_new_expr_name(
		flick_asprintf("flick_%s_%s_%s_array",
			       get_be_name(),
			       which_stub,
			       get_buf_name()));
	
	cast_expr call = cast_new_expr_call_2(macro,
					      array_expr,
					      length_expr);
	add_stmt(cast_new_stmt_expr(call));
	
	/* swap on decode */
	if (bt_if && (op & MUST_DECODE)
	    && (elem_size == 8 || elem_size == 4 || elem_size == 2)) {
		add_stmt(bt_if);
		add_stmt(cast_new_stmt_expr(cast_new_expr_call_2(
			cast_new_expr_name(flick_asprintf(
				"flick_%s_array_swap%d",
				get_be_name(),
				elem_size * 8)),
			array_expr,
			length_expr)));
		add_stmt(bt_end);
	}
	
	/*
	 * Terminate the array, but only for *real* arrays.
	 * Normally, this is taken care of by the standard mu_array(), but
	 * we aren't running that here.
	 */
	if (array_data.is_valid)
		mu_array_terminate(array_expr, elem_ctype, cname);
	
	/* check the size of the array */
	check_for_warning(array_data.mint_len_max, has_payload, array_expr);
	if (!has_payload)
		has_payload = array_expr;
	
	/* Can't dealloc until message is sent */
}

/*
 * For Trapeze strings, we encode the terminator as part of the string.
 */
int trapeze_mu_state::mu_array_encode_terminator(char *cname)
{
	return mu_array_is_string(cname);
}

/* End of file. */
