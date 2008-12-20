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

#include <stdio.h>
#include <stdarg.h>
#include <rpc/types.h>
#include <rpc/xdr.h>

#include <mom/mint.h>
#include <mom/cast.h>
#include <mom/pres_c.h>
#include <mom/compiler.h>
#include <mom/c/libcast.h>

extern pres_c_1 pres;

extern void print_allocation(int indent, pres_c_allocation alloc);
extern void print_pres_c_mapping(int indent, pres_c_mapping map);

extern void print_cast_ref(int indent, cast_ref cref);
extern void print_mint_const(int indent, mint_const mc);
extern void print_mint_ref(int indent, mint_ref mref);
extern char *pres_c_message_attribute_kind_name(pres_c_message_attribute_kind
						kind);

void print_pres_c_inline(int indent, pres_c_inline inl);

#define W_OPTIONAL_INLINE(inl, name, indent)		\
if ((inl)) {						\
	w_i_printf((indent), "%s:\n", (name));		\
	print_pres_c_inline((indent)+1, (inl));		\
} else {						\
	w_i_printf((indent), "%s: (none)\n", (name));	\
}
/*****************************************************************************/

void print_pres_c_inline_atom(int indent, pres_c_inline_atom atom)
{
	w_i_printf(indent, "inline atom:\n");
	w_i_printf(indent+1, "mapping index: %d\n", atom.index);
	w_i_printf(indent+1, "mapping:\n");
	print_pres_c_mapping(indent+2, atom.mapping);
}

static void print_pres_c_inline_struct_slot(int indent,
					    pres_c_inline_struct_slot slot)
{
	w_i_printf(indent,
		   "MINT struct slot index: %d%s\n",
		   slot.mint_struct_slot_index,
		   ((slot.mint_struct_slot_index == mint_slot_index_null) ?
		    " (meaning: no slot)" :
		    ""));
	w_i_printf(indent,
		   "inline:\n");
	print_pres_c_inline(indent+1, slot.inl);
}

void print_pres_c_inline(int indent, pres_c_inline inl)
{
	u_int i;
	u_int cases_len;
	
	switch(inl->kind) {
	case PRES_C_INLINE_ATOM:
		w_i_printf(indent, "inline atom:\n");
		w_i_printf(indent+1, "mapping index: %d\n",
			   inl->pres_c_inline_u_u.atom.index);
		
		w_i_printf(indent+1, "mapping:\n");
		print_pres_c_mapping(indent+2,
				     inl->pres_c_inline_u_u.atom.mapping);
		break;
		
	case PRES_C_INLINE_STRUCT:
		w_i_printf(indent, "inline struct:\n");
		w_i_printf(indent+1,
			   "number of slots: %d\n",
			   inl->pres_c_inline_u_u.struct_i.slots.slots_len);
		w_i_printf(indent+1, "structure slots:\n");
		
		for (i = 0;
		     i < inl->pres_c_inline_u_u.struct_i.slots.slots_len;
		     i++) {
			w_i_printf(indent+2, "slot %d:\n", i);
			print_pres_c_inline_struct_slot(
				indent+3,
				(inl->pres_c_inline_u_u.struct_i.
				 slots.slots_val[i])
				);
		}
		
		break;
		
	case PRES_C_INLINE_FUNC_PARAMS_STRUCT:
		w_i_printf(indent, "inline function parameters struct:\n");
		w_i_printf(indent+1,
			   "number of parameter slots: %d\n",
			   (inl->
			    pres_c_inline_u_u.func_params_i.slots.slots_len));
		w_i_printf(indent+1, "parameter slots:\n");
		
		for (i = 0;
		     i < inl->pres_c_inline_u_u.func_params_i.slots.slots_len;
		     i++) {
			w_i_printf(indent+2, "slot %d:\n", i);
			print_pres_c_inline_struct_slot(
				indent+3,
				(inl->pres_c_inline_u_u.func_params_i.
				 slots.slots_val[i])
				);
		}
		
		if (inl->pres_c_inline_u_u.func_params_i.return_slot) {
			w_i_printf(indent+1, "return value slot:\n");
			print_pres_c_inline_struct_slot(
				indent+2,
				*(inl->pres_c_inline_u_u.func_params_i.
				  return_slot)
				);
		} else
			w_i_printf(indent+1, "return value slot: (none)\n");
		
		break;
		
	case PRES_C_INLINE_HANDLER_FUNC:
		w_i_printf(indent, "inline handler func:\n");
		w_i_printf(indent+1,
			   "number of slots: %d\n",
			   inl->pres_c_inline_u_u.handler_i.slots.slots_len);
		w_i_printf(indent+1, "param slots:\n");
		
		for (i = 0;
		     i < inl->pres_c_inline_u_u.handler_i.slots.slots_len;
		     i++) {
			w_i_printf(indent+2, "slot %d:\n", i);
			print_pres_c_inline_struct_slot(
				indent+3,
				(inl->pres_c_inline_u_u.handler_i.
				 slots.slots_val[i])
				);
		}
		
		break;
		
	case PRES_C_INLINE_VIRTUAL_UNION:
		w_i_printf(indent, "inline virtual union:\n");
		
		w_i_printf(indent + 1, "arglist name: %s\n",
			   inl->pres_c_inline_u_u.virtual_union.arglist_name);
		
		w_i_printf(indent+1, "discrim inline:\n");
		print_pres_c_inline(indent+2,
				     (inl->pres_c_inline_u_u.virtual_union.
				      discrim));
		cases_len = inl->
			    pres_c_inline_u_u.virtual_union.cases.cases_len;
		w_i_printf(indent+1, "number of union cases: %d\n", cases_len);
		w_i_printf(indent+1, "union cases:\n");
		
		for (i = 0; i < cases_len; i++) {
			w_i_printf(indent+2, "case %d:\n", i);
			
			w_i_printf(indent+3, "into inline:\n");
			print_pres_c_inline(indent+4,
					    (inl->pres_c_inline_u_u.
					     virtual_union.cases.
					     cases_val[i]));
		}
		
		if (inl->pres_c_inline_u_u.virtual_union.dfault) {
			w_i_printf(indent+2, "default case:\n");
			
			w_i_printf(indent+3, "into inline:\n");
			print_pres_c_inline(indent+4,
					    (inl->pres_c_inline_u_u.
					     virtual_union.dfault));
		}
		break;
		
	case PRES_C_INLINE_STRUCT_UNION:
		w_i_printf(indent, "inline struct union:\n");
		
		w_i_printf(indent+1, "discriminator atom:\n");
		print_pres_c_inline_atom(indent+2,
					 inl->pres_c_inline_u_u.
					 struct_union.discrim);
		
		w_i_printf(indent+1, "union index: %d\n",
			   inl->pres_c_inline_u_u.struct_union.union_index);
		
		cases_len = inl->pres_c_inline_u_u.struct_union.cases.
			    cases_len;
		w_i_printf(indent+1, "number of union cases: %d\n",
			   cases_len);
		
		w_i_printf(indent+1, "union cases:\n");
		for (i = 0; i < cases_len; i++) {
			w_i_printf(indent+2, "case %d:\n",
				   i);
			w_i_printf(indent+3, "index: %d\n",
				   (inl->pres_c_inline_u_u.struct_union.cases.
				    cases_val[i].index));
			w_i_printf(indent+3, "mapping:\n");
			print_pres_c_mapping(indent+4,
					     (inl->pres_c_inline_u_u.
					      struct_union.cases.cases_val[i].
					      mapping));
		}
		/* Do the default case, too. */
		if (inl->pres_c_inline_u_u.struct_union.dfault) {
			w_i_printf(indent+1, "default case:\n");
			w_i_printf(indent+2, "index: %d\n",
				   (inl->pres_c_inline_u_u.struct_union.
				    dfault->index));
			w_i_printf(indent+2, "mapping:\n");
			print_pres_c_mapping(indent+3,
					     (inl->pres_c_inline_u_u.
					      struct_union.dfault->mapping));
		} else
			w_i_printf(indent+1, "default case: null\n");
		break;
		
	case PRES_C_INLINE_VOID_UNION:
		w_i_printf(indent, "inline void union:\n");
		
		w_i_printf(indent+1, "discriminator atom:\n");
		print_pres_c_inline_atom(indent+2,
					 inl->pres_c_inline_u_u.
					 void_union.discrim);
		
		w_i_printf(indent+1, "void index: %d\n",
			   inl->pres_c_inline_u_u.void_union.void_index);
		
		cases_len = inl->pres_c_inline_u_u.void_union.cases.cases_len;
		w_i_printf(indent+1, "number of union cases: %d\n",
			   cases_len);
		
		w_i_printf(indent+1, "union cases:\n");
		for (i = 0; i < cases_len; i++) {
			w_i_printf(indent+2, "case %d:\n",
				   i);
			w_i_printf(indent+3, "value: ");
			if (inl->pres_c_inline_u_u.void_union.cases.
			    cases_val[i].case_value)
				cast_w_expr(inl->pres_c_inline_u_u.void_union.
					    cases.cases_val[i].case_value,
					    0);
			else
				w_printf("(none)");
			w_printf("\n");
			w_i_printf(indent+3, "type: ");
			cast_w_type(empty_scope_name,
				    (inl->pres_c_inline_u_u.void_union.cases.
				     cases_val[i].type),
				    0);
			w_printf("\n");
			w_i_printf(indent+3, "mapping:\n");
			print_pres_c_mapping(indent+4,
					     (inl->pres_c_inline_u_u.
					      void_union.cases.cases_val[i].
					      mapping));
		}
		/* Do the default case, too. */
		if (inl->pres_c_inline_u_u.void_union.dfault) {
			w_i_printf(indent+1, "default case:\n");
			w_i_printf(indent+2, "type: ");
			cast_w_type(empty_scope_name,
				    (inl->pres_c_inline_u_u.void_union.cases.
				     cases_val[i].type),
				    0);
			w_printf("\n");
			w_i_printf(indent+2, "mapping:\n");
			print_pres_c_mapping(indent+3,
					     (inl->pres_c_inline_u_u.
					      void_union.dfault->mapping));
		} else
			w_i_printf(indent+1, "default case: null\n");
		break;
		
	case PRES_C_INLINE_EXPANDED_UNION:
		w_i_printf(indent, "inline expanded union:\n");
		
		w_i_printf(indent+1, "discriminator atom:\n");
		print_pres_c_inline_atom(indent+2,
					 inl->pres_c_inline_u_u.expanded_union.
					 discrim);
		
		w_i_printf(indent+1, "number of union cases: %d\n",
			   inl->pres_c_inline_u_u.expanded_union.cases.
			   cases_len);
		
		w_i_printf(indent+1, "union cases:\n");
		for (i = 0;
		     i < inl->pres_c_inline_u_u.expanded_union.cases.cases_len;
		     i++) {
			w_i_printf(indent+2, "case %d:\n", i);
			print_pres_c_inline(indent+3,
					    inl->pres_c_inline_u_u.
					    expanded_union.cases.cases_val[i]);
		}
		
		if (inl->pres_c_inline_u_u.expanded_union.dfault) {
			w_i_printf(indent+1, "default case:\n");
			print_pres_c_inline(indent+2,
					    inl->pres_c_inline_u_u.
					    expanded_union.dfault);
		} else
			w_i_printf(indent+1, "default case: null\n");
		break;

	case PRES_C_INLINE_COLLAPSED_UNION:
		w_i_printf(indent, "inline collapsed union:\n");
		
		w_i_printf(indent+1, "MINT discriminator:\n");
		print_mint_const(indent+2,
				 inl->pres_c_inline_u_u.collapsed_union.
				 discrim_val);
		
		if (inl->pres_c_inline_u_u.collapsed_union.selected_case) {
			w_i_printf(indent+1, "selected case inline:\n");
			print_pres_c_inline(indent+2,
					    inl->pres_c_inline_u_u.
					    collapsed_union.selected_case);
		} else
			w_i_printf(indent+1, "selected case inline: null\n");
		break;
		
	case PRES_C_INLINE_THROWAWAY:
		w_i_printf(indent, "inline throwaway.\n");
		break;
		
	case PRES_C_INLINE_TYPED:
		w_i_printf(indent, "inline typed:\n");
		
		w_i_printf(indent+1, "type tag inline:\n");
		print_pres_c_inline(indent+2,
				    inl->pres_c_inline_u_u.typed.tag);
		w_i_printf(indent+1, "typed data inline:\n");
		print_pres_c_inline(indent+2,
				    inl->pres_c_inline_u_u.typed.inl);
		break;
		
	case PRES_C_INLINE_XLATE:
		w_i_printf(indent, "inline translation:\n");
		
		w_i_printf(indent+1, "mapping index: %d\n",
			   inl->pres_c_inline_u_u.xlate.index);
		
		w_i_printf(indent+1, "internal C type:\n");
		w_i_printf(indent+2, "");
		cast_w_type(empty_scope_name, inl->pres_c_inline_u_u.xlate.
			    internal_ctype, indent+2);
		w_printf("\n");
		
		w_i_printf(indent+1, "translator: %s\n",
			   inl->pres_c_inline_u_u.xlate.translator);
		w_i_printf(indent+1, "destructor: %s\n",
			   inl->pres_c_inline_u_u.xlate.destructor);
		
		w_i_printf(indent+1, "sub inline:\n");
		print_pres_c_inline(indent+2,
				    inl->pres_c_inline_u_u.xlate.sub);
		break;
		
	case PRES_C_INLINE_ASSIGN:
		w_i_printf(indent, "inline assign:\n");
		
		w_i_printf(indent+1, "mapping index: %d\n",
			   inl->pres_c_inline_u_u.assign.index);
		
		w_i_printf(indent+1, "sub inline:\n");
		print_pres_c_inline(indent+2,
				    inl->pres_c_inline_u_u.assign.sub);
		
		w_i_printf(indent+1, "CAST expr:\n");
		cast_w_expr(inl->pres_c_inline_u_u.assign.value, indent+2);
		w_printf("\n");
		break;
		
	case PRES_C_INLINE_COND:
		w_i_printf(indent, "inline cond:\n");
		
		w_i_printf(indent+1, "mapping index: %d\n",
			   inl->pres_c_inline_u_u.cond.index);
		
		w_i_printf(indent+1, "true inline:\n");
		print_pres_c_inline(indent+2,
				    inl->pres_c_inline_u_u.cond.true_inl);
		
		w_i_printf(indent+1, "false inline:\n");
		print_pres_c_inline(indent+2,
				    inl->pres_c_inline_u_u.cond.false_inl);
		break;
		
	case PRES_C_INLINE_MESSAGE_ATTRIBUTE:
		w_i_printf(indent, "inline message attribute:\n");
		
		w_i_printf(indent+1, "kind: %s\n",
			   pres_c_message_attribute_kind_name(
				   inl->pres_c_inline_u_u.msg_attr.kind));
		
		break;
		
	case PRES_C_INLINE_ALLOCATION_CONTEXT:
		w_i_printf(indent, "inline allocation context:\n");
		
		w_i_printf(indent + 1, "arglist name: %s\n",
			   inl->pres_c_inline_u_u.acontext.arglist_name);
		
		W_OPTIONAL_INLINE(inl->pres_c_inline_u_u.acontext.offset,
				  "offset inline", indent + 1);
		W_OPTIONAL_INLINE(inl->pres_c_inline_u_u.acontext.length,
				  "length inline", indent + 1);
		W_OPTIONAL_INLINE(inl->pres_c_inline_u_u.acontext.min_len,
				  "minimum inline", indent + 1);
		W_OPTIONAL_INLINE(inl->pres_c_inline_u_u.acontext.max_len,
				  "maximum inline", indent + 1);
		W_OPTIONAL_INLINE(inl->pres_c_inline_u_u.acontext.alloc_len,
				  "allocated length inline", indent + 1);
		W_OPTIONAL_INLINE(
			inl->pres_c_inline_u_u.acontext.min_alloc_len,
			"min allocated length inline", indent + 1);
		W_OPTIONAL_INLINE(
			inl->pres_c_inline_u_u.acontext.max_alloc_len,
			"max allocated length inline", indent + 1);
		W_OPTIONAL_INLINE(inl->pres_c_inline_u_u.acontext.release,
				  "release inline", indent + 1);
		W_OPTIONAL_INLINE(inl->pres_c_inline_u_u.acontext.terminator,
				  "terminator inline", indent + 1);
		W_OPTIONAL_INLINE(inl->pres_c_inline_u_u.acontext.mustcopy,
				  "mustcopy inline", indent + 1);
		
		w_i_printf(indent + 1, "allocation:\n");
		print_allocation(indent + 2,
				 inl->pres_c_inline_u_u.acontext.alloc);
		
		w_i_printf(indent+1, "inline:\n");
		print_pres_c_inline(indent+2,
				    inl->pres_c_inline_u_u.acontext.ptr);
		
		break;
		
	case PRES_C_INLINE_TEMPORARY: {
		pres_c_temporary *temp = &inl->pres_c_inline_u_u.temp;
		
		w_i_printf(indent, "temporary inline:\n");
		w_i_printf(indent + 1, "name/type: ");
		cast_w_type(cast_new_scoped_name(temp->name, NULL),
			    temp->ctype, 0);
		w_printf("\n");
		w_i_printf(indent + 1, "init: ");
		if (temp->init)
			cast_w_expr(temp->init, 0);
		else
			w_printf("0");
		w_printf("\n");
		w_i_printf(indent + 1, "prehandler: ");
		if (temp->prehandler)
			w_printf("\"%s\"", temp->prehandler);
		else
			w_printf("0");
		w_printf("\n");
		w_i_printf(indent + 1, "posthandler: ");
		if (temp->posthandler)
			w_printf("\"%s\"", temp->prehandler);
		else
			w_printf("0");
		w_printf("\n");
		w_i_printf(indent + 1, "is_const: %d\n", temp->is_const);
		w_i_printf(indent + 1, "mapping:\n");
		print_pres_c_mapping(indent+2, temp->map);
		break;
	}
	
	case PRES_C_INLINE_ILLEGAL:
		w_i_printf(indent, "illegal inline.\n");
		break;
		
	default:
		panic("Unknown inline type %d seen.", inl->kind);
		break;
	}
}
