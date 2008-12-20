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
#include <ctype.h>

#include <rpc/types.h>
#include <rpc/xdr.h>

#include <mom/mint.h>
#include <mom/cast.h>
#include <mom/pres_c.h>
#include <mom/compiler.h>
#include <mom/c/libcast.h>

extern pres_c_1 pres;

/* The `mint_ref_marked' array allows us to manage circular MINT references.
   We do not handle circular MINT constants, however. */
static int *mint_ref_marked = 0;

void print_cast_ref(int indent, cast_ref cref)
{
	data_channel_index chn = pres.cast.cast_scope_val[cref].channel;
	w_printf("Channel #%d\n", chn);
	pres.cast.cast_scope_val[cref].channel = PASSTHRU_DATA_CHANNEL;
	cast_w_def(&(pres.cast.cast_scope_val[cref]), indent);
	pres.cast.cast_scope_val[cref].channel = chn;
}

void print_stub_cast_ref(int indent, cast_ref cref)
{
	data_channel_index chn = pres.stubs_cast.cast_scope_val[cref].channel;
	w_printf("Channel #%d\n", chn);
	pres.stubs_cast.cast_scope_val[cref].channel = PASSTHRU_DATA_CHANNEL;
	cast_w_def(&(pres.stubs_cast.cast_scope_val[cref]), indent);
	pres.stubs_cast.cast_scope_val[cref].channel = chn;
}

#define m(n) (pres.mint.defs.defs_val[n])

void print_mint_ref(int indent, mint_ref mref);

void print_mint_const(int indent, mint_const mc)
{
	switch (mc->kind) {
	case MINT_CONST_INT:
		w_i_printf(indent, "const_int: ");
		switch (mc->mint_const_u_u.const_int.kind) {
		case MINT_CONST_LITERAL:
			w_printf("literal %d\n",
				 mc->mint_const_u_u.const_int.
				 mint_const_int_u_u.value);
			break;
		case MINT_CONST_SYMBOLIC:
			w_printf("symbolic \"%s\"\n",
				 mc->mint_const_u_u.const_int.
				 mint_const_int_u_u.name);
			break;
		}
		break;
		
	case MINT_CONST_CHAR:
		w_i_printf(indent, "const_char: ");
		switch (mc->mint_const_u_u.const_char.kind) {
		case MINT_CONST_LITERAL: {
			char ch = mc->mint_const_u_u.const_char.
				  mint_const_char_u_u.value;
			
			if (isprint(((int) ch)))
				w_printf("literal <%c>\n", ch);
			else
				w_printf("literal <\\%03d>\n", ch);
			}
			break;
			
		case MINT_CONST_SYMBOLIC:
			w_printf("symbolic \"%s\"\n",
				 mc->mint_const_u_u.const_char.
				 mint_const_char_u_u.name);
			break;
		}
		break;
		
	case MINT_CONST_FLOAT:
		w_i_printf(indent, "const_float: ");
		switch (mc->mint_const_u_u.const_int.kind) {
		case MINT_CONST_LITERAL:
			w_printf("literal %f\n",
				 mc->mint_const_u_u.const_float.
				 mint_const_float_u_u.value);
			break;
		case MINT_CONST_SYMBOLIC:
			w_printf("symbolic \"%s\"\n",
				 mc->mint_const_u_u.const_float.
				 mint_const_float_u_u.name);
			break;
		}
		break;
		
	case MINT_CONST_STRUCT: {
		mint_const_struct *mcs = &mc->mint_const_u_u.const_struct;
		u_int i;
		
		w_i_printf(indent, "const_struct[%d]:\n",
			   mcs->mint_const_struct_len);
		for (i = 0; i < mcs->mint_const_struct_len; i++)
			print_mint_const(indent+1,
					 mcs->mint_const_struct_val[i]);
		break;
	}
	
	case MINT_CONST_ARRAY: {
		mint_const_array *mca = &mc->mint_const_u_u.const_array;
		u_int i;
		
		w_i_printf(indent, "const_array[%d]:\n",
			   mca->mint_const_array_len);
		for (i = 0; i < mca->mint_const_array_len; i++)
			print_mint_const(indent+1,
					 mca->mint_const_array_val[i]);
		break;
	}
	
	default:
		panic("print_mint_const: unknown mint_const_kind %d",
		      mc->kind);
	}
}

static void print_integer(int indent, mint_ref i)
{
	w_printf(": min %d, max %u\n",
		 m(i).mint_def_u.integer_def.min,
		 (m(i).mint_def_u.integer_def.min +
		  m(i).mint_def_u.integer_def.range));
}

static void print_scalar(int indent, mint_ref i)
{
	w_printf(": %s%d bits\n",
		 ((m(i).mint_def_u.scalar_def.flags ==
		   MINT_SCALAR_FLAG_NONE) ? "" :
		  (m(i).mint_def_u.scalar_def.flags ==
		   MINT_SCALAR_FLAG_SIGNED) ? "signed, " :
		  (m(i).mint_def_u.scalar_def.flags ==
		   MINT_SCALAR_FLAG_UNSIGNED) ? "unsigned, " :
		  "incorrectly-flagged, "),
		 m(i).mint_def_u.scalar_def.bits);
}

static void print_char(int indent, mint_ref i)
{
	w_printf(": %s%d bits\n",
		 ((m(i).mint_def_u.char_def.flags ==
		   MINT_CHAR_FLAG_NONE) ? "" :
		  (m(i).mint_def_u.char_def.flags ==
		   MINT_CHAR_FLAG_SIGNED) ? "signed, " :
		  (m(i).mint_def_u.char_def.flags ==
		   MINT_CHAR_FLAG_UNSIGNED) ? "unsigned, " :
		  "incorrectly-flagged, "),
		 m(i).mint_def_u.char_def.bits);
}

static void print_float(int indent, mint_ref i)
{
	w_printf(": %d bits\n", m(i).mint_def_u.float_def.bits);
}

static void print_array(int indent, mint_ref ai)
{
	w_printf(":\n");
	
	w_i_printf(indent, "element type:\n");
	print_mint_ref(indent+1, m(ai).mint_def_u.array_def.element_type);
	
	w_i_printf(indent, "length type:\n");
	print_mint_ref(indent+1, m(ai).mint_def_u.array_def.length_type);
}

static void print_struct(int indent, mint_ref si)
{
	u_int i;
	
	w_printf(":\n");
	
	w_i_printf(indent, "number of slots: %d\n",
		   m(si).mint_def_u.struct_def.slots.slots_len);
	
	for (i = 0; i < m(si).mint_def_u.struct_def.slots.slots_len; i++) {
		w_i_printf(indent, "slot %d:\n", i);
	        print_mint_ref(indent+1,
			       m(si).mint_def_u.struct_def.slots.slots_val[i]);
	}
}

static void print_union(int indent,mint_ref ui)
{
	u_int i;
	
	w_printf(":\n");
	
	w_i_printf(indent, "discriminator:\n");
	print_mint_ref(indent + 1, m(ui).mint_def_u.union_def.discrim);

	w_i_printf(indent, "number of cases: %d\n",
		   m(ui).mint_def_u.union_def.cases.cases_len);
	
	for (i = 0; i < m(ui).mint_def_u.union_def.cases.cases_len; i++) {
		w_i_printf(indent, "case %d:\n", i);
		w_i_printf(indent+1, "value:\n");
		print_mint_const(indent+2,
				 m(ui).mint_def_u.union_def.cases.cases_val[i].
				 val);
		
		w_i_printf(indent+1, "variant:\n");
		print_mint_ref(indent+2,
			       m(ui).mint_def_u.union_def.cases.cases_val[i].
			       var);
	}
	
	if (m(ui).mint_def_u.union_def.dfault >= 0) {
		w_i_printf(indent, "default variant:\n");
		print_mint_ref(indent+1, m(ui).mint_def_u.union_def.dfault);
	} else
		w_i_printf(indent, "default variant: null\n");
}
	
static void print_typed(int indent, mint_ref ti)
{
	w_printf(":\n");
	
	w_i_printf(indent, "tag:\n");
	print_mint_ref((indent + 1), m(ti).mint_def_u.typed_def.tag);
	
	w_i_printf(indent, "type:\n");
	print_mint_ref((indent + 1), m(ti).mint_def_u.typed_def.ref);
}

static void print_interface(int indent, mint_ref i)
{
	w_printf(":\n");
	
	w_i_printf(indent, "right: ");
	switch (m(i).mint_def_u.interface_def.right) {
	case MINT_INTERFACE_NAME:	 w_printf("name\n"); break;
	case MINT_INTERFACE_INVOKE:      w_printf("invoke\n"); break;
	case MINT_INTERFACE_INVOKE_ONCE: w_printf("invoke once\n"); break;
	case MINT_INTERFACE_SERVICE:     w_printf("service\n"); break;
	default:			 w_printf("[invalid right]\n"); break;
	}
}

void print_mint_ref(int indent, mint_ref mr)
{
	mint_def_kind kind;
	
	if (mr >= ((mint_ref) pres.mint.defs.defs_len))
		panic("mint_ref out of range");
	
	kind = m(mr).kind;
	w_i_printf(indent, "MINT %d %s",
		   mr,
		   ((kind == MINT_BOOLEAN) ? "boolean" :
		    (kind == MINT_INTEGER) ? "integer" :
		    (kind == MINT_SCALAR) ? "scalar" :
		    (kind == MINT_CHAR) ? "char" :
		    (kind == MINT_FLOAT) ? "float" :
		    (kind == MINT_ARRAY) ? "array" :
		    (kind == MINT_STRUCT) ? "struct" :
		    (kind == MINT_UNION) ? "union" :
		    (kind == MINT_VOID) ? "void" :
		    (kind == MINT_TYPE_TAG) ? "type tag" :
		    (kind == MINT_TYPED) ? "typed" :
		    (kind == MINT_INTERFACE) ? "interface" :
		    (kind == MINT_ANY) ? "any" :
		    (kind == MINT_SYSTEM_EXCEPTION) ? "system exception" :
		    "unknown"));
	
	if (mint_ref_marked[mr])
		w_printf(" (already described).\n");
	else {
		mint_ref_marked[mr] = 1;
		++indent;
		
		switch (kind) {
		case MINT_BOOLEAN:
			w_printf(".\n");
			break;
		case MINT_INTEGER:
			print_integer(indent, mr);
			break;
		case MINT_SCALAR:
			print_scalar(indent, mr);
			break;
		case MINT_FLOAT:
			print_float(indent, mr);
			break;
		case MINT_CHAR:
			print_char(indent, mr);
			break;
		case MINT_ARRAY:
			print_array(indent, mr);
			break;
		case MINT_STRUCT:
			print_struct(indent, mr);
			break;
		case MINT_UNION:
			print_union(indent, mr);
			break;
		case MINT_VOID:
			w_printf(".\n");
			break;
		case MINT_TYPE_TAG:
			w_printf(".\n");
			break;
		case MINT_TYPED:
			print_typed(indent, mr);
			break;
		case MINT_INTERFACE:
			print_interface(indent, mr);
			break;
		case MINT_ANY:
			w_printf(".\n");
			break;
		case MINT_SYSTEM_EXCEPTION:
			w_printf(".\n");
			break;
		default:
			panic("Unknown MINT type seen.");
		}
		--indent;
	}
}

void clear_mint_ref_marks()
{
	u_int i;
	
	if (!mint_ref_marked)
		mint_ref_marked = (int *) mustmalloc(sizeof(int) *
						     pres.mint.defs.
						     defs_len);
	for (i = 0; i < pres.mint.defs.defs_len; ++i)
		mint_ref_marked[i] = 0;
}

/* End of file. */

