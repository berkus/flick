/*
 * Copyright (c) 1995, 1996, 1998, 1999 The University of Utah and
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
#include <mom/c/libpres_c.h>

extern void print_pres_c_inline(int indent, pres_c_inline inl);


static void print_allocation_case(int indent, pres_c_allocation_case alloc)
{
	w_i_printf(indent, "allocator: ");
	switch(alloc.allocator.kind) {
	case PRES_C_ALLOCATOR_DONTCARE:
		w_printf("[Don't Care]");
		break;
	case PRES_C_ALLOCATOR_STATIC:
		w_printf("[Static]");
		break;
	case PRES_C_ALLOCATOR_OUTOFLINE:
		w_printf("[Out-of-line] \"%s\"",
			 alloc.allocator.pres_c_allocator_u.ool_name);
		break;
	case PRES_C_ALLOCATOR_NAME:
		w_printf("\"%s\"",
			 alloc.allocator.pres_c_allocator_u.name);
		break;
	default:
		panic("In print_allocation_case(), "
		      "unknown allocator kind!");
	}
	w_printf("\n");
	
	w_i_printf(indent, "allocation flags:");
	if (!alloc.flags)
		w_printf(" never");
	if (alloc.flags & PRES_C_ALLOC_IF_NULL)
		w_printf(" if-null");
	if (alloc.flags & PRES_C_ALLOC_IF_TOO_SMALL)
		w_printf(" if-too-small");
	if (alloc.flags & PRES_C_ALLOC_IF_TOO_LARGE)
		w_printf(" if-too-large");
	if (alloc.flags & PRES_C_ALLOC_ALWAYS)
		w_printf(" always");
	if (alloc.flags & PRES_C_ALLOC_CLEAR)
		w_printf(" clear");
	w_printf("\n");
	
	w_i_printf(indent, "reallocation flags:");
	if (alloc.flags & PRES_C_REALLOC_ALWAYS)
		w_printf(" always");
	if (alloc.flags & PRES_C_REALLOC_EVER)
		w_printf(" ever");
	if (alloc.flags & PRES_C_REALLOC_CLEAR)
		w_printf(" clear");
	if (alloc.flags & PRES_C_REALLOC_IF_TOO_SMALL)
		w_printf(" if-too-small");
	if (alloc.flags & PRES_C_REALLOC_IF_TOO_LARGE)
		w_printf(" if-too-large");
	w_printf("\n");
	
	w_i_printf(indent, "deallocation flags:");
	if (!alloc.flags)
		w_printf(" never");
	if (alloc.flags & PRES_C_DEALLOC_ALWAYS)
		w_printf(" always");
	if (alloc.flags & PRES_C_DEALLOC_EVER)
		w_printf(" ever");
	if (alloc.flags & PRES_C_DEALLOC_NULLIFY)
		w_printf(" nullify");
	if (alloc.flags & PRES_C_DEALLOC_ON_FAIL)
		w_printf(" on-fail");
	w_printf("\n");
	
	w_i_printf(indent, "misc flags:");
	if (alloc.flags & PRES_C_RUN_CTOR)
		w_printf(" run-ctor");
	if (alloc.flags & PRES_C_RUN_DTOR)
		w_printf(" run-dtor");
	w_printf("\n");
	
	w_i_printf(indent, "allocation initialization:");
	if (alloc.alloc_init) {
		cast_w_init(alloc.alloc_init, 0);
	} else
		w_printf("[none]");
	w_printf("\n");
}

void print_allocation(int indent, pres_c_allocation alloc)
{
	static char *dirs[PRES_C_DIRECTIONS] = {
		"UNKNOWN",
		"IN",
		"INOUT",
		"OUT",
		"RETURN" };
	int i;
	
	w_i_printf(indent, "Allocation Semantics:\n");

	for (i = 0; i < PRES_C_DIRECTIONS; i++) {
		
		w_i_printf(indent, "%s:", dirs[i]);
		if (alloc.cases[i].allow == PRES_C_ALLOCATION_INVALID)
			w_printf("[Invalid Case]\n");
		else {
			w_printf("\n");
			print_allocation_case(
				indent + 1,
				alloc.cases[i].pres_c_allocation_u_u.val);
		}
	}
}

static char *pres_c_sid_kind_name(pres_c_sid_kind kind)
{
	switch (kind) {
	case PRES_C_SID_CLIENT:
		return "client";
	case PRES_C_SID_SERVER:
		return "server";
	default:
		return "(invalid)";
	}
}

char *pres_c_message_attribute_kind_name(pres_c_message_attribute_kind
					 kind)
{
	switch (kind) {
	case PRES_C_MESSAGE_ATTRIBUTE_FLAGS:
		return "flags";
	case PRES_C_MESSAGE_ATTRIBUTE_TIMEOUT:
		return "time out";
	case PRES_C_MESSAGE_ATTRIBUTE_SEQUENCE_RECEIVED:
		return "sequence received";
	case PRES_C_MESSAGE_ATTRIBUTE_CLIENT_REFERENCE:
		return "client reference";
	case PRES_C_MESSAGE_ATTRIBUTE_SERVERCOPY:
		return "servercopy";
	default:
		return "(invalid)";
	}
}

void print_pres_c_mapping(int indent, pres_c_mapping map)
{
	u_int i;
	
	switch (map->kind) {
	case PRES_C_MAPPING_DIRECT:
		w_i_printf(indent, "direct mapping.\n");
		break;
		
	case PRES_C_MAPPING_IGNORE:
		w_i_printf(indent, "ignore mapping.\n");
		break;
		
	case PRES_C_MAPPING_ILLEGAL:
		w_i_printf(indent, "illegal mapping.\n");
		break;
		
	case PRES_C_MAPPING_ELSEWHERE:
		w_i_printf(indent, "elsewhere mapping.\n");
		break;
		
	case PRES_C_MAPPING_STUB:
		w_i_printf(indent, "stub mapping:\n");
		w_i_printf(indent+1, "mapping stub index: %d\n",
			   map->
			   pres_c_mapping_u_u.mapping_stub.mapping_stub_index);
		break;
		
	case PRES_C_MAPPING_POINTER:
		w_i_printf(indent, "pointer mapping:\n");
		w_i_printf(indent+1, "associated arglist name: %s\n",
			   map->pres_c_mapping_u_u.pointer.arglist_name);
		w_i_printf(indent+1, "pointer map:\n");
		print_pres_c_mapping(indent+2,
				     map->pres_c_mapping_u_u.pointer.target);
		break;
		
	case PRES_C_MAPPING_VAR_REFERENCE:
		w_i_printf(indent, "variable reference mapping:\n");
		w_i_printf(indent+1, "target map:\n");
		print_pres_c_mapping(indent+2,
				     map->pres_c_mapping_u_u.var_ref.target);
		break;
		
	case PRES_C_MAPPING_INTERNAL_ARRAY:
		w_i_printf(indent, "internal array mapping:\n");
		w_i_printf(indent+1, "associated arglist name: %s\n",
			   (map->pres_c_mapping_u_u.internal_array.
			    arglist_name));
		w_i_printf(indent+1, "element mapping:\n");
		print_pres_c_mapping(indent+2,
				     map->pres_c_mapping_u_u.internal_array.
				     element_mapping);
		break;
		
	case PRES_C_MAPPING_STRUCT:
		w_i_printf(indent, "struct mapping:\n");
		w_i_printf(indent+1, "inline:\n");
		print_pres_c_inline(indent+2,
				    map->pres_c_mapping_u_u.struct_i);
		break;
		
	case PRES_C_MAPPING_FLAT_UNION:
		w_i_printf(indent, "flat union mapping:\n");
		
		w_i_printf(indent+1, "discriminator mapping:\n");
		print_pres_c_mapping(indent+2,
				     map->
				     pres_c_mapping_u_u.flat_union.discrim);
		
		w_i_printf(indent+1, "number of cases: %d\n",
			   map->pres_c_mapping_u_u.flat_union.cases.cases_len);
		for (i = 0;
		     i < map->pres_c_mapping_u_u.flat_union.cases.cases_len;
		     i++) {
			w_i_printf(indent+2, "case %d:\n:", i);
			print_pres_c_mapping(indent+3,
					     map->
					     pres_c_mapping_u_u.flat_union.
					     cases.cases_val[i]);
		}
		
		w_i_printf(indent+1, "default mapping:\n");
		print_pres_c_mapping(indent+1,
				     map->
				     pres_c_mapping_u_u.flat_union.dfault);
		break;
		
	case PRES_C_MAPPING_SPECIAL:
		w_i_printf(indent, "special mapping:\n");
		w_i_printf(indent+1, "marshaler: %s\n",
			   map->pres_c_mapping_u_u.special.marshaler_name);
		break;
		
	case PRES_C_MAPPING_XLATE:
		w_i_printf(indent, "translation mapping:\n");
		
		w_i_printf(indent+1, "internal C type:\n");
		w_i_printf(indent+2, "");
		cast_w_type(empty_scope_name,
			    map->pres_c_mapping_u_u.xlate.internal_ctype,
			    indent+2);
		w_printf("\n");
		
		w_i_printf(indent+1, "internal mapping:\n");
		print_pres_c_mapping(indent+2,
				     map->
				     pres_c_mapping_u_u.xlate.
				     internal_mapping);
		
		w_i_printf(indent+1, "translator: %s\n",
			   map->pres_c_mapping_u_u.xlate.translator);
		w_i_printf(indent+1, "destructor: %s\n",
			   map->pres_c_mapping_u_u.xlate.destructor);
		break;
		
	case PRES_C_MAPPING_REFERENCE:
		w_i_printf(indent, "reference mapping:\n");
		
		w_i_printf(indent+1, "reference kind: ");
		switch (map->pres_c_mapping_u_u.ref.kind) {
		case PRES_C_REFERENCE_COPY: w_printf("copy\n"); break;
		case PRES_C_REFERENCE_MOVE: w_printf("move\n"); break;
		case PRES_C_REFERENCE_COPY_AND_CONVERT:
			w_printf("copy and convert\n");
			break;
		default: w_printf("unknown\n");
		}
		w_i_printf(indent+1, "reference count adjustment: %d\n",
			   map->pres_c_mapping_u_u.ref.ref_count);
		break;
		
	case PRES_C_MAPPING_TYPE_TAG:
		w_i_printf(indent, "type tag mapping.\n");
		break;
		
	case PRES_C_MAPPING_TYPED:
		w_i_printf(indent, "typed (type-tagged value) mapping.\n");
		break;
		
	case PRES_C_MAPPING_OPTIONAL_POINTER:
		w_i_printf(indent, "optional pointer mapping:\n");
		w_i_printf(indent+1, "associated arglist name: %s\n",
			   (map->pres_c_mapping_u_u.optional_pointer.
			    arglist_name));
		w_i_printf(indent+1, "pointer map:\n");
		print_pres_c_mapping(indent+2,
				     (map->pres_c_mapping_u_u.optional_pointer.
				      target));
		break;
		
	case PRES_C_MAPPING_SYSTEM_EXCEPTION:
		w_i_printf(indent, "system exception mapping.\n");
		break;
		
	case PRES_C_MAPPING_DIRECTION:
		w_i_printf(indent, "direction mapping:\n");
		
		w_i_printf(indent+1, "direction: %s\n",
			   pres_c_dir_name(
				   map->pres_c_mapping_u_u.direction.dir));
		w_i_printf(indent+1, "mapping:\n");
		print_pres_c_mapping(indent+2,
				     (map->pres_c_mapping_u_u.direction.
				      mapping));
		break;
		
	case PRES_C_MAPPING_SID:
		w_i_printf(indent, "SID mapping:\n");
		
		w_i_printf(indent+1, "kind: %s\n",
			   pres_c_sid_kind_name(map->pres_c_mapping_u_u.
						sid.kind));
		break;

	case PRES_C_MAPPING_ARGUMENT:
		w_i_printf(indent, "argument mapping:\n");
		w_i_printf(indent+1, "arglist name: %s\n",
			   map->pres_c_mapping_u_u.argument.arglist_name);
		w_i_printf(indent+1, "name: %s\n",
			   map->pres_c_mapping_u_u.argument.arg_name);
		if (map->pres_c_mapping_u_u.argument.map) {
			w_i_printf(indent+1, "map:\n");
			print_pres_c_mapping(indent+2,
					     (map->pres_c_mapping_u_u.
					      argument.map));
		} else {
			w_i_printf(indent+1, "map: null\n");
		}
		break;
		
	case PRES_C_MAPPING_MESSAGE_ATTRIBUTE:
		w_i_printf(indent, "message attribute mapping:\n");
		
		w_i_printf(indent+1, "kind: %s\n",
			   pres_c_message_attribute_kind_name(
				   map->pres_c_mapping_u_u.
				   message_attribute.kind));
		break;

	case PRES_C_MAPPING_INITIALIZE:
		w_i_printf(indent, "initializer mapping:\n");
		w_i_printf(indent+1, "value: " );
		cast_w_expr(map->pres_c_mapping_u_u.initialize.value, 0);
		w_i_printf(0,"\n");
		break;
		
	case PRES_C_MAPPING_PARAM_ROOT:
		w_i_printf(indent, "parameter root mapping:\n");
		if (map->pres_c_mapping_u_u.param_root.ctype) {
			w_i_printf(indent+1, "internal C type:\n");
			w_i_printf(indent+2, "");
			cast_w_type(empty_scope_name,
				    map->pres_c_mapping_u_u.param_root.ctype,
				    indent+2);
		} else {
			w_i_printf(indent+1, "internal C type: (none)");
		}
		w_printf("\n");
		
		w_i_printf(indent+1, "initialization: ");
		if (map->pres_c_mapping_u_u.param_root.init) {
			cast_w_init(map->pres_c_mapping_u_u.param_root.init,
				    0);
		} else {
			w_printf("[none]");
		};
		w_printf("\n");
		
		w_i_printf(indent+1, "mapping:\n");
		print_pres_c_mapping(indent+2,
				     map->pres_c_mapping_u_u.param_root.map);
		break;
		
	case PRES_C_MAPPING_SELECTOR:
		w_i_printf(indent, "selector mapping:\n");
		w_i_printf(indent+1, "index: %d\n",
			   map->pres_c_mapping_u_u.selector.index);
		w_i_printf(indent+1, "mapping:\n");
		print_pres_c_mapping(indent+2,
				     map->pres_c_mapping_u_u.selector.mapping);
		break;
		
	case PRES_C_MAPPING_TEMPORARY: {
		pres_c_temporary *temp = &map->pres_c_mapping_u_u.temp;
		
		w_i_printf(indent, "temporary mapping:\n");
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
	
	case PRES_C_MAPPING_SINGLETON:
		w_i_printf(indent, "singleton mapping:\n");
		print_pres_c_inline(indent + 2,
				    map->pres_c_mapping_u_u.singleton.inl);
		break;
		
	default:
		panic("Unknown mapping type %d seen.", map->kind);
		break;
	}
}

/* End of file. */

