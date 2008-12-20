/*
 * Copyright (c) 1999 The University of Utah and
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

#include <mom/c/libcast.h>
#include <mom/c/pbe.hh>

/*
 * This method handles `PRES_C_MAPPING_TYPED' presentations.
 *
 * We need to convert between a type-tagged value in our message --- i.e., some
 * data describing a type, and some data representing a value of the described
 * type --- and a single presented entity.  We have a `MINT_TYPED' node that
 * represents the message data and a CAST expression and CAST type describing
 * the presented data object.
 *
 * An example of this presentation style is the handling of CORBA `any' values.
 * In the standard CORBA C++ presentation, `any's are presented as instances of
 * (opaque) classes.  The C++ presentation generator doesn't (and shouldn't)
 * know how the class is implemented --- so all it can do is make a mapping
 * between the presented `CORBA::Any' and the `MINT_TYPED', leaving the back
 * end to figure out how to marshal or unmarshal the data.
 *
 * In this method, we handle the type tag and value as a single unit; compare
 * this with `mu_state::mu_inlined_typed', which handles the type tag and value
 * separately.
 *
 * XXX --- If the PG had a way to refer to the components of opaque types in an
 * abstract way, then we could potentially handle a CORBA `any' value with an
 * `INLINE_TYPED' node instead.  This would be a good thing, because PRES_C
 * inlines are generally more flexible than mappings.  (As described above, we
 * don't use an `INLINE_TYPED' now because we don't want the PG to hardwire the
 * names of the internal slots in the (unpresented) implementation of `any's.
 * Because these slots are unpresented, the BE should be free to choose the
 * names of the internal slots.  But clearly, PRES_C still needs to refer to
 * the slots in some abstract way --- i.e., the ``TypeCode'' slot and the
 * ``value'' slot --- in order to marshal/unmarshal data.  Unfortunately,
 * PRES_C can't describe ``abstract slots,'' at least not yet.)
 *
 * XXX --- Even if someday we can use `INLINE_TYPED' to handle CORBA `any's, it
 * might not be worth it.  Practically, it makes sense for there to be a single
 * runtime function that marshals/unmarshals `any's, rather than two separate
 * functions (one for the TypeCode and a second for the value).
 */
void
mu_state::mu_mapping_typed(
	cast_expr cexpr,
	cast_type /* ctype */,
	mint_ref itype
	)
{
	mint_def *idef;
	mint_typed_def *typed_def;
	
	mint_def *tag_def;
	mint_def *ref_def;
	
	cast_expr macro_call_cexpr;
	
	/*
	 * Check the MINT: make sure that it's a `MINT_TYPED' containing a
	 * `MINT_TYPE_TAG' and a `MINT_ANY'.  This is the only structure that
	 * our magic macro can handle.
	 */
	assert(itype >= 0);
	assert(itype < (signed int) pres->mint.defs.defs_len);
	
	idef = &(pres->mint.defs.defs_val[itype]);
	assert(idef->kind == MINT_TYPED);
	
	typed_def = &(pres->mint.defs.defs_val[itype].mint_def_u.typed_def);
	assert(typed_def->tag >= 0);
	assert(typed_def->tag < (signed int) pres->mint.defs.defs_len);
	assert(typed_def->ref >= 0);
	assert(typed_def->ref < (signed int) pres->mint.defs.defs_len);
	
	tag_def = &(pres->mint.defs.defs_val[typed_def->tag]);
	assert(tag_def->kind == MINT_TYPE_TAG);
	
	ref_def = &(pres->mint.defs.defs_val[typed_def->ref]);
	assert(ref_def->kind == MINT_ANY);
	
	/*
	 * We simply produce a special macro call to process the type-tagged
	 * value.  The arguments to the macro are:
	 *
	 *   1. the name of this back end, to be used in the macroexpansion
	 *      (to create calls to other macros);
	 *   2. the CAST expression for the value to be marshaled/unmarshaled;
	 *   3. a fresh label, which may be used in the macroexpansion (see
	 *      <flick/link/iiopxx.h>); and
	 *   4. the current abort label.
	 */
	macro_call_cexpr
		= cast_new_expr_call_4(
			cast_new_expr_name(
				flick_asprintf(
					"flick_%s_%s_type_tag_and_value",
					get_encode_name(),
					get_buf_name()
					)),
			cast_new_expr_name(get_be_name()),
			cexpr,
			cast_new_expr_name(add_label()),
			cast_new_expr_name(abort_block->use_current_label())
			);
	
	add_stmt(cast_new_stmt_expr(macro_call_cexpr));
}

/* End of file. */

