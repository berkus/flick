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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <rpc/types.h>

#include <mom/compiler.h>
#include <mom/libaoi.h>
#include <mom/libmeta.h>

#include "rpc_parse.h"

extern definition *defs;
aoi outaoi;

/*
 * This is the scope of the corresponding AOI nodes.  This is necessary to
 * prevent references to structures that are defined within other scopes
 * (structures, etc.).
 */
int curr_scope = 0;

int aoi_max = 0;

#define CAN_CREATE (0)
#define MUST_CREATE (1)
#define NO_CREATE (2)

static aoi_type xl_enum(definition *def_ptr, enum_def def, char *name);
static int xl_eval(definition *def_ptr, char *val);

static int IsDigit(char c)
{
	return (c == '-') || (c == '~') || isdigit(((int) c));
}

static void itoa(unsigned int i, char *a)
{
	char temp[20];
	int x, y;
	
	for (x = 0; x < 20; x++)
		temp[x] = 0;
	x = 0;
	do {
		temp[x++] = i % 10;
		i /= 10;
	} while (i);

	for (y = 0; x; y++, x--)
		a[y] = temp[x - 1] + '0';
	a[y] = 0;
}

static char *trim_string(char *in_string)
{
	/*
	 * Return a copy of `in_string' but with the first and last characters
	 * removed.  This function is used to trim the double-quotes from
	 * string constants.
	 */
	int in_length;
	char *out_string;
	
	in_length = strlen(in_string);
	out_string = (char *) mustmalloc(sizeof(char) * (in_length - 1));
	strncpy(out_string, (in_string + 1), (in_length - 2));
	out_string[in_length - 2] = 0;
	
	return out_string;
}

static int get_def()
{
	/*
	 * This will dynamically size the outaoi array to fit as many slots as
	 * are needed.
	 */
	int i = outaoi.defs.defs_len++;
	
	if ((signed int) outaoi.defs.defs_len > aoi_max) {
		aoi_max += 10;
		outaoi.defs.defs_val = (aoi_def *)
				 mustrealloc(outaoi.defs.defs_val,
					     sizeof(aoi_def) * aoi_max);
	}
	
	return i;
}

static int aoi_name(definition *def, char *name, int errs)
{
	/*
	 * This will find the node named 'name' and will return a reference
	 * to it.  This is not necessarily the best code, but it works fine.
	 */
	int i;
	int min_scope = curr_scope;
	
	for (i = outaoi.defs.defs_len - 1; i >= 0; i--) {
		/* Look in all scopes from this point, and above... */
		if (outaoi.defs.defs_val[i].scope <= min_scope) {
			min_scope = outaoi.defs.defs_val[i].scope;
			if (!strcmp(outaoi.defs.defs_val[i].name, name)) {
				if (errs == MUST_CREATE)
					return -1;
				else
					return i;
			}
		}
	}
	/* Not already defined. */
	if (errs == NO_CREATE)
		return -1;
	
	i = get_def();
	outaoi.defs.defs_val[i].name     = name;
	outaoi.defs.defs_val[i].binding  = 0;
	outaoi.defs.defs_val[i].scope    = curr_scope;
	outaoi.defs.defs_val[i].idl_file = def->idl_file;
	
	return i;
}

static aoi_type new_int(int min, unsigned int range)
{
	/* This allocates a new integer type and sets the min/range values. */
	aoi_type res = (aoi_type) mustmalloc(sizeof(aoi_type_u));
	
	res->kind = AOI_INTEGER;
	res->aoi_type_u_u.integer_def.min = min;
	res->aoi_type_u_u.integer_def.range = range;
	
	return res;
}

static aoi_type make_optional(aoi_type type)
{
	/*
	 * Create and return an AOI_OPTIONAL that refers to the given
	 * `aoi_type'.
	 */
	aoi_type res = (aoi_type) mustmalloc(sizeof(aoi_type_u));
	
	res->kind = AOI_OPTIONAL;
	res->aoi_type_u_u.optional_def.type = type;
	
	return res;
}

static aoi_const xl_deref_const(definition *def_ptr, const_def def,
				int must_be_int)
{
	/*
	 * This function returns the value associated with the definition of a
	 * constant --- in other words, this function "dereferences" the given
	 * constant.  Literals are translated into AOI and returned; symbolic
	 * names are looked up.  NOTE that this function will panic if it is
	 * given a name that does not refer to an already-defined constant.
	 * This means that we don't handle forward references to constants.
	 * `rpcgen' can handle certain forward references, such as this:
	 *
	 *   const a = b; const b = 1; (OK for `rpcgen', but not OK for us.)
	 *
	 * `rpcgen' can also handle constants that don't refer to anything, so
	 * long as the constant is never needed:
	 *
	 *   const bar = foo; (`bar' is never used, `foo' is never defined.)
	 *
	 * `rpcgen' can handle these cases because it doesn't need to know
	 * anything about constants in order to generate code.  We, however,
	 * have different requirements, and therefore enforce the rule that the
	 * definition of a constant must precede its use in all cases.
	 */
	aoi_const val;
	
	if (!def)
		panic("Nil constant definition!");
	
	if (IsDigit(def[0]))
		/* If it is a number, evaluate it. */
		val = aoi_new_const_int(xl_eval(def_ptr, def));
	else if (def[0] == '"') {
		/* If it is a string, evaluate it. */
		if (must_be_int)
			panic("`%s' is not an integer constant.", def);
		else
			val = aoi_new_const_string(trim_string(def));
	} else {
		/* Otherwise, it is a name.  Find its binding. */
		int ref = aoi_name(def_ptr, def, NO_CREATE);
		
		if ((ref < 0) || (outaoi.defs.defs_val[ref].binding == 0)) {
			/* If it's not defined, it's an error. */
			panic("`%s' is not a defined constant.", def);
		} else {
			/*
			 * The name has a binding.  It must be a CONST_INT or a
			 * CONST_ARRAY, otherwise stuff won't work as expected.
			 * So we need to verify that it is.
			 */
			aoi_type bind = outaoi.defs.defs_val[ref].binding;
			aoi_const refd;
			
			if (bind->kind != AOI_CONST)
				panic("`%s' does not refer to a constant.",
				      def);
			
			refd = bind->aoi_type_u_u.const_def.value;
			if (refd->kind == AOI_CONST_INT)
				val = refd;
			else if (must_be_int)
				panic(("`%s' does not refer to an integer "
				       "constant."),
				      def);
			else if (refd->kind == AOI_CONST_ARRAY)
				val = refd;
			else
				panic(("`%s' does not refer to a constant "
				       "integer or array."),
				      def);
		}
	}
	return val;
}

static aoi_type xl_const(definition *def_ptr, const_def def)
{
	/*
	 * Constants are UNTYPED in XDR.  This causes some pretty hokey
	 * problems in the optimizer, so they are constrained to be
	 * dereferenceable if used.  This means that you can't just say
	 * `const a = b', unless `b' is declared as `const b = 1'.
	 */
	aoi_type res = (aoi_type) mustmalloc(sizeof(aoi_type_u));
	
	res->kind = AOI_CONST;
	res->aoi_type_u_u.const_def.type = new_int(-2147483647-1, ~0U);
	res->aoi_type_u_u.const_def.value = xl_deref_const(def_ptr, def, 0);
	
	return res;
}

static aoi_type xl_enum(definition *def_ptr, enum_def def, char *name)
{
	/*
	 * Iterate through the list of enum's, creating a new const in the
	 * list of data types.  Note that there is a disclaimer regarding
	 * enums: it states that they must be ints, but then we lose data
	 * about the enumeration.  (CORBA doesn't specify.)  BE CAREFUL!
	 */
	enumval_list *curr;
	aoi_type res = (aoi_type) mustmalloc(sizeof(aoi_type_u));
	aoi_enum *val = &(res->aoi_type_u_u.enum_def);
	aoi_field *vals;
	int currval = 0;
	int pos = 0;
	
	/*****/
	
	res->kind = AOI_ENUM;
	
	val->enum_label = name;
	
	/* Count the # of fields. */
	val->defs.defs_len = 0;
	for (curr = def.vals; curr; curr = curr->next)
		val->defs.defs_len++;
	/* Allocate space for the values. */
	vals = (aoi_field *)
	       mustmalloc(sizeof(aoi_field) * val->defs.defs_len);
	
	/* Create new const's for each value. */
	for (curr = def.vals; curr; curr = curr->next, pos++) {
		vals[pos].name = curr->name;
		/*
		 * If they haven't specified the value, choose the next value.
		 */
		if (!curr->assignment) {
			char name[15];
			itoa(currval, name);
			vals[pos].type = xl_const(def_ptr, name);
		} else
			vals[pos].type = xl_const(def_ptr, curr->assignment);
		
		if (vals[pos].type->aoi_type_u_u.const_def.value->kind
		    != AOI_CONST_INT)
			panic("Invalid value for enumeration.");
		
		/* Increment the next default value. */
		currval = (vals[pos].type->aoi_type_u_u.const_def.value->
			   aoi_const_u_u.const_int)
			  + 1;
	}
	
	val->defs.defs_val = vals;
	return res;
}

static aoi_type xl_td_buildtype(definition *def_ptr, char *type)
{
	aoi_type res;
	
	if (!strcmp(type, "float")) {
		res = (aoi_type) mustmalloc(sizeof(aoi_type_u));
		res->kind = AOI_FLOAT;
		res->aoi_type_u_u.float_def.bits = 32;
		
	} else if (!strcmp(type, "double")) {
		res = (aoi_type) mustmalloc(sizeof(aoi_type_u));
		res->kind = AOI_FLOAT;
		res->aoi_type_u_u.float_def.bits = 64;
		
	} else if (!strcmp(type, "char")) {
		res = (aoi_type) mustmalloc(sizeof(aoi_type_u));
		res->kind = AOI_CHAR;
		res->aoi_type_u_u.char_def.bits = 8;
		res->aoi_type_u_u.char_def.flags = AOI_CHAR_FLAG_NONE;
		
	} else if (!strcmp(type, "u_char")) {
		res = (aoi_type) mustmalloc(sizeof(aoi_type_u));
		res->kind = AOI_CHAR;
		res->aoi_type_u_u.char_def.bits = 8;
		res->aoi_type_u_u.char_def.flags = AOI_CHAR_FLAG_UNSIGNED;
		
	} else if (!strcmp(type, "void")) {
		res = (aoi_type) mustmalloc(sizeof(aoi_type_u));
		res->kind = AOI_VOID;
		
	} else if (!strcmp(type, "string")) {
		aoi_type chr, len;
		
		res = (aoi_type)mustmalloc(sizeof(aoi_type_u));
		res->kind = AOI_ARRAY;
		res->aoi_type_u_u.array_def.flgs
			= AOI_ARRAY_FLAG_NULL_TERMINATED_STRING;
		chr = (aoi_type)mustmalloc(sizeof(aoi_type_u));
		chr->kind = AOI_CHAR;
		chr->aoi_type_u_u.char_def.bits = 8;
		chr->aoi_type_u_u.char_def.flags = AOI_CHAR_FLAG_NONE;
		len = new_int(0, 4294967295U);
		res->aoi_type_u_u.array_def.element_type = chr;
		res->aoi_type_u_u.array_def.length_type = len;
		
	} else if (!strcmp(type, "opaque")) {
		aoi_type chr, len;
		
		res = (aoi_type) mustmalloc(sizeof(aoi_type_u));
		res->kind = AOI_ARRAY;
		res->aoi_type_u_u.array_def.flgs = AOI_ARRAY_FLAG_OPAQUE;
		chr = (aoi_type) mustmalloc(sizeof(aoi_type_u));
		chr->kind = AOI_INTEGER;
		chr->aoi_type_u_u.integer_def.min = 0;
		chr->aoi_type_u_u.integer_def.range = 255U;
		len = new_int(0, 4294967295U);
		res->aoi_type_u_u.array_def.element_type = chr;
		res->aoi_type_u_u.array_def.length_type = len;
		
	} else if (!strcmp(type, "int") || !strcmp(type, "long"))
		res = new_int(-2147483647-1, ~0U);
	
	else if (!strcmp(type, "u_int") || !strcmp(type, "u_long"))
		res = new_int(0, ~0U);
	
	else if (!strcmp(type, "short"))
		res = new_int(-32768, 65535);
	
	else if (!strcmp(type, "u_short"))
		res = new_int(0, 65535);
	
	else if (!strcmp(type, "bool"))
		res = new_int(0, 1);
	
	else {
		/*
		 * At this point, it had better be a reference, and it should
		 * already exist in the file.  If not, we're dead...
		 */
		int ref = aoi_name(def_ptr, type, NO_CREATE);
		
		if (ref < 0)
			panic("Undefined type `%s'.", type);
		res = (aoi_type) mustmalloc(sizeof(aoi_type_u));
		res->kind = AOI_INDIRECT;
		res->aoi_type_u_u.indirect_ref = ref;
	}
	
	return res;
}

static int xl_eval(definition *def_ptr, char *val)
{
	if (!val)
		panic("Null value found in evaluation!");
	if (IsDigit(val[0])) {
		if (val[0] == '~')
			return ~xl_eval(def_ptr, &val[1]);
		if (val[1] == '-')
			return -xl_eval(def_ptr, &val[1]);
		if ((val[0] == '0') && (val[1] == 'x')) {
			/* Hexadecimal. */
			int num = 0;
			int pos = 2;
			
			while (val[pos]) {
				num = num << 4;
				if (isdigit(((int) val[pos])))
					num += (val[pos] - '0');
				else {
					num += (((val[pos] - 'A') & 0x1f)
						+ 10);
					if ((val[pos] < 'A')
					    || (val[pos] > 'f'))
						panic(("Invalid hex value "
						       "`%s'."),
						      val);
					if ((val[pos] & 0x1f) > 5)
						panic(("Invalid hex value "
						       "`%s'."),
						      val);
				}
				pos++;
			}
			return num;
		} else
			return atoi(val);
		
	} else {
		int ref = aoi_name(def_ptr, val, NO_CREATE);
		aoi_type bind = outaoi.defs.defs_val[ref].binding;
		
		if (ref < 0)
			panic("Undefined const `%s'.", val);
		if (bind->kind != AOI_CONST)
			panic("Invalid type found in evaluation.");
		if (bind->aoi_type_u_u.const_def.value->kind != AOI_CONST_INT)
			panic("Invalid const type found in evaluation.");
		
		return (bind->aoi_type_u_u.const_def.value->aoi_const_u_u.
			const_int);
	}
}

static aoi_type xl_typedef(definition *def_ptr, typedef_def def)
{
	/*
	 * This should be an indirect to the appropriate type, be it structure,
	 * union, enum, etc.  It will also possible be an array/pointer to that
	 * structure (requires deeper indirection old_prefix is necessary for
	 * forward references).
	 */
	aoi_type component_type, result_type;
	int array_maximum;
	
	/* First, build the component type. */
	component_type = xl_td_buildtype(def_ptr, def.old_type);
	
	if (def.rel == REL_ALIAS)
		return component_type;
	else if (def.rel == REL_POINTER)
		return make_optional(component_type);
	
	if (!strcmp(def.old_type, "string") || !strcmp(def.old_type, "opaque"))
		/*
		 * `xl_td_buildtype' builds strings and opaques as variable
		 * length arrays.  We need to reset the array bounds below.
		 */
		result_type = component_type;
	
	else if ((def.rel == REL_VECTOR) || (def.rel == REL_ARRAY)) {
		/* It's an array. */
		result_type = (aoi_type) mustmalloc(sizeof(aoi_type_u));
		result_type->kind = AOI_ARRAY;
		result_type->aoi_type_u_u.array_def.element_type
			= component_type;
		result_type->aoi_type_u_u.array_def.flgs = AOI_ARRAY_FLAG_NONE;
		
	} else
		panic("Unknown typedef type %d in `xl_typedef'.", def.rel);
	
	/*
	 * At this point we are dealing with a REL_VECTOR or a REL_ARRAY.
	 * We need to determine the array's length.
	 */
	array_maximum = xl_eval(def_ptr, def.array_max);
	switch (def.rel) {
	case REL_VECTOR:
		result_type->aoi_type_u_u.array_def.length_type =
			new_int(array_maximum, 0);
		break;
	case REL_ARRAY:
		result_type->aoi_type_u_u.array_def.length_type =
			new_int(0, array_maximum);
		break;
	default:
		panic("Unknown array type %d in `xl_typedef'.", def.rel);
		break;
	}
	
	return result_type;
}

static aoi_type xl_decl(definition *def_ptr, declaration decl)
{
	typedef_def td;
	
	td.old_prefix = decl.prefix;
	td.old_type = decl.type;
	td.rel = decl.rel;
	td.array_max = decl.array_max;
	
	return xl_typedef(def_ptr, td);
}

static aoi_type xl_struct(definition *def_ptr, struct_def def)
{
	/*
	 * Since XDR doesn't allow nested _definitions_, this code is REALLY
	 * simple.  We just count the number of slots, and then define them
	 * using the typedef code.
	 */
	aoi_type res = (aoi_type) mustmalloc(sizeof(aoi_type_u));
	int len = 0;
	decl_list *curr_slot;
	aoi_struct_slot *slots;
	
	/* Count the number of slots to allocate. */
	for (curr_slot = def.decls; curr_slot; curr_slot = curr_slot->next)
		len++;
	
	res->kind = AOI_STRUCT;
	slots = (aoi_struct_slot *) mustmalloc(len * sizeof(aoi_struct_slot));
	res->aoi_type_u_u.struct_def.slots.slots_len = len;
	res->aoi_type_u_u.struct_def.slots.slots_val = slots;
	
	/* Step through the list, dealing with each item in the struct. */
	len = 0;
	for (curr_slot = def.decls;
	     curr_slot;
	     curr_slot = curr_slot->next, len++) {
		slots[len].name = curr_slot->decl.name;
		slots[len].type = xl_decl(def_ptr, curr_slot->decl);
	}
	
	return res;
}

static aoi_type xl_union(definition *def_ptr, union_def def, char *name)
{
	/*
	 * Since XDR doesn't allow nested _definitions_, this code is REALLY
	 * simple.  We just count the number of slots and then define them
	 * using the typedef code.
	 */
	aoi_type res = (aoi_type) mustmalloc(sizeof(aoi_type_u));
	int len = 0;
	case_list *curr_slot;
	aoi_union_case *slots;
	char *newname;
	
	/* Set up the discriminator. */
	res->aoi_type_u_u.union_def.discriminator.type
		= xl_decl(def_ptr, def.enum_decl);
	res->aoi_type_u_u.union_def.discriminator.name
		= def.enum_decl.name;
	
	len = strlen(name);
	newname = (char *) mustmalloc(len + 2);
	strcpy(newname, name);
	newname[len]     = '_';
	newname[len + 1] = 'u';
	newname[len + 2] = 0;
	res->aoi_type_u_u.union_def.union_label = newname;
	
	/* Allocate space for the variants. */
	len = 0;
	for (curr_slot = def.cases; curr_slot; curr_slot = curr_slot->next)
		len++;
	
	res->kind = AOI_UNION;
	slots = (aoi_union_case *) mustmalloc(len * sizeof(aoi_union_case));
	res->aoi_type_u_u.union_def.cases.cases_len = len;
	res->aoi_type_u_u.union_def.cases.cases_val = slots;
	
	/* Set up the default case (if it exists). */
	if (def.default_decl) {
		res->aoi_type_u_u.union_def.dfault
			= (aoi_field *) mustmalloc(sizeof(aoi_field));
		res->aoi_type_u_u.union_def.dfault->name
			= def.default_decl->name;
		res->aoi_type_u_u.union_def.dfault->type
			= xl_decl(def_ptr, *def.default_decl);
	} else
		res->aoi_type_u_u.union_def.dfault = (aoi_field *) 0;
	
	/* Set the individual cases. */
	len = 0;
	for (curr_slot = def.cases;
	     curr_slot;
	     curr_slot = curr_slot->next, len++) {
		slots[len].var.name = curr_slot->case_decl.name;
		slots[len].var.type = xl_decl(def_ptr, curr_slot->case_decl);
		
		if (res->aoi_type_u_u.union_def.discriminator.type->kind
		    == AOI_INDIRECT) {
			int ref = (res->aoi_type_u_u.union_def.discriminator.
				   type->aoi_type_u_u.indirect_ref);
			unsigned int pos;
			aoi_enum enum_def;
			
			if (outaoi.defs.defs_val[ref].binding->kind != AOI_ENUM)
				panic(("Type `%s' used in a union is not an "
				       "enum."),
				      def.enum_decl);
			/*
			 * Now we look up the appropriate name and insert the
			 * correct value.
			 */
			enum_def = outaoi.defs.defs_val[ref].binding->aoi_type_u_u.
				   enum_def;
			for (pos = 0; pos < enum_def.defs.defs_len; pos++)
				if (!strcmp(curr_slot->case_name,
					    enum_def.defs.defs_val[pos].name))
					break;
			if (pos == enum_def.defs.defs_len)
				slots[len].val
					= xl_deref_const(def_ptr,
							 curr_slot->case_name,
							 1);
			else {
				aoi_type bind = enum_def.defs.defs_val[pos].
						type;
				int val = bind->aoi_type_u_u.const_def.value->
					  aoi_const_u_u.const_int;
				
				slots[len].val = aoi_new_const_int(val);
			}
			
	        } else
			slots[len].val = xl_deref_const(def_ptr,
							curr_slot->case_name,
							1);
	}
	
	return res;
}

static aoi_interface xl_new_prog_interface(definition *def_ptr, char *code)
{
	/* This builds an empty interface to represent a program. */
	aoi_interface res;
	
	res.idl = AOI_IDL_SUN;
	res.code_type = new_int(0, 4294967295U);
	res.code = aoi_new_const_int(xl_eval(def_ptr, code));
	res.parents.parents_len = 0;
	res.ops.ops_len = 0;
	res.attribs.attribs_len = 0;
	res.excepts.excepts_len = 0;
	res.op_code_type = new_int(0, 4294967295U);
	
	return res;
}

static aoi_type xl_new_prog(definition *def_ptr, char *prog_num)
{
	/*
	 * This just creates an empty interface to represent the program
	 * containing the individual versions.
	 */
	aoi_type res = (aoi_type) mustmalloc(sizeof(aoi_type_u));
	
	res->kind = AOI_INTERFACE;
	res->aoi_type_u_u.interface_def = xl_new_prog_interface(def_ptr,
								prog_num);
	
	return res;
}

static aoi_type xl_get_vers_discriminator_type()
{
	static aoi_type type = 0;
	
	if (type == 0) {
		aoi_struct *type_struct;
		
		type = (aoi_type) mustmalloc(sizeof(aoi_type_u));
		type->kind = AOI_STRUCT;
		type_struct = &(type->aoi_type_u_u.struct_def);
		
		/* Allocate and fill in the structure type slots. */
		type_struct->slots.slots_len
			= 2;
		type_struct->slots.slots_val
			= ((aoi_struct_slot *)
			   mustmalloc(sizeof(aoi_struct_slot) *
				      type_struct->slots.slots_len));
		
		type_struct->slots.slots_val[0].name = "prog_code";
		type_struct->slots.slots_val[0].type = new_int(0, 4294967295U);
		type_struct->slots.slots_val[1].name = "vers_code";
		type_struct->slots.slots_val[1].type = new_int(0, 4294967295U);
	}
	
	return type;
}

static aoi_interface xl_new_vers_interface(definition *def_ptr,
					   int prog_ref, char *vers_code)
{
	/* This builds an interface to represent a version. */
	aoi_interface res;
	aoi_const_struct *discriminator;
	aoi_type prog_binding;
	aoi_const prog_code;
	
	/*
	 * First, make sure we're inheriting from a program, and extract the
	 * program's discriminator code.
	 */
	prog_binding = outaoi.defs.defs_val[prog_ref].binding;
	if (prog_binding->kind != AOI_INTERFACE)
		panic("`xl_new_vers_interface' received a bogus `prog_ref'.");
	prog_code = prog_binding->aoi_type_u_u.interface_def.code;
	
	res.idl = AOI_IDL_SUN;
	
	/*
	 * Construct the union discriminator value.
	 * XXX --- Say something about why we use a struct here.
	 */
	res.code = aoi_new_const(AOI_CONST_STRUCT);
	discriminator = &(res.code->aoi_const_u_u.const_struct);
	discriminator->aoi_const_struct_len
		= 2;
	discriminator->aoi_const_struct_val
		= ((aoi_const *)
		   mustmalloc(sizeof(aoi_const) *
			      discriminator->aoi_const_struct_len));
	
	discriminator->aoi_const_struct_val[0]
		= prog_code;
	discriminator->aoi_const_struct_val[1]
		= aoi_new_const_int(xl_eval(def_ptr, vers_code));
	
	res.code_type = xl_get_vers_discriminator_type();
	
	/* Inherit the parent program. */
	res.parents.parents_len = 1;
	res.parents.parents_val = (aoi_type *) mustmalloc(sizeof(aoi_type));
	res.parents.parents_val[0] = (aoi_type) mustmalloc(sizeof(aoi_type_u));
	res.parents.parents_val[0]->kind = AOI_INDIRECT;
	res.parents.parents_val[0]->aoi_type_u_u.indirect_ref = prog_ref;
	
	/* Initialize the remaining interface fields. */
	res.ops.ops_len = 0;
	res.attribs.attribs_len = 0;
	res.excepts.excepts_len = 0;
	res.op_code_type = new_int(0, 4294967295U);
	
	return res;
}

static void xl_add_vers(definition *def_ptr, version_list *v, int prog_ref)
{
	/*
	 * This builds an interface for the version given, which inherits the
	 * program that contains this version.
	 */
	int ref = get_def();
	aoi_type bind;
	aoi_interface *ver;
	proc_list *curr_proc;
	int proc_ref;
	
	/* Create the interface. */
	outaoi.defs.defs_val[ref].name = v->vers_name;
	outaoi.defs.defs_val[ref].binding = (aoi_type)
				      mustmalloc(sizeof(aoi_type_u));
	outaoi.defs.defs_val[ref].scope = curr_scope;
	outaoi.defs.defs_val[ref].idl_file = def_ptr->idl_file;
	bind = outaoi.defs.defs_val[ref].binding;
        bind->kind = AOI_INTERFACE;
	bind->aoi_type_u_u.interface_def = xl_new_vers_interface(def_ptr,
								 prog_ref,
								 v->vers_num);
	ver = &(bind->aoi_type_u_u.interface_def);
	
	/* Add the operations. */
	proc_ref = 0;
	for (curr_proc = v->procs; curr_proc; curr_proc = curr_proc->next)
		proc_ref++;
	ver->ops.ops_len = proc_ref;
	ver->ops.ops_val = (aoi_operation *)
			   mustmalloc(sizeof(aoi_operation) * proc_ref);
	
	for (curr_proc = v->procs, proc_ref = 0;
	     curr_proc;
	     curr_proc = curr_proc->next, proc_ref++) {
		aoi_operation *curr_op = &(ver->ops.ops_val[proc_ref]);
		
		curr_op->name = curr_proc->proc_name;
		curr_op->request_code
			= aoi_new_const_int(xl_eval(def_ptr,
						    curr_proc->proc_num));
		/*
		 * XXX --- Adding 1024 to the procedure number is a hack.
		 * Flick currently requires that the request and reply codes be
		 * different.
		 */
		curr_op->reply_code
			= aoi_new_const_int(xl_eval(def_ptr,
						    curr_proc->proc_num)
					    + 1024);
		curr_op->flags = AOI_OP_FLAG_NONE;
		curr_op->return_type = xl_td_buildtype(def_ptr,
						       curr_proc->res_type);
		curr_op->params.params_len = 1;
		curr_op->params.params_val
			= (aoi_parameter *) mustmalloc(sizeof(aoi_parameter));
		/*
		 * curr_op->params.params_val[0].name = (char *) 0;
		 * ENE: A parameter with no name is bad news when we check the
		 * CAST.  We might as well give the parameter a name now.
		 */
		curr_op->params.params_val[0].name = "arg";
		curr_op->params.params_val[0].direction = AOI_DIR_IN;
		curr_op->params.params_val[0].type
			= xl_td_buildtype(def_ptr, curr_proc->arg_type);
	}
}

static aoi_type xl_program(definition *def_ptr, program_def def, int ref)
{
	/*
	 * Create an empty interface for return, but also create interfaces for
	 * each version, each of which inherit the program interface, and
	 * contain the individual procedures.
	 */
	aoi_type res = xl_new_prog(def_ptr, def.prog_num);
	version_list *curr_vers;
	
	/*
	 * We must establish the `outaoi' binding for this program now, so that
	 * `xl_new_vers_intreface' can dereference `ref' to find the current
	 * program.  (Normally, the binding is established by `translate'.)
	 * Alternately, we could pass the current program code down, but it
	 * seems like a better idea just to establish the binding.
	 */
	outaoi.defs.defs_val[ref].binding = res;
	for (curr_vers = def.versions; curr_vers; curr_vers = curr_vers->next)
		xl_add_vers(def_ptr, curr_vers, ref);
	
	return res;
}

static aoi_type xl_scope(definition *def, int refnum)
{
	switch (def->def_kind){
	case DEF_TYPEDEF:
		return xl_typedef(def, def->def.ty);
		break;
		
	case DEF_CONST:
		return xl_const(def, def->def.co);
		break;
		
	case DEF_STRUCT:
		return xl_struct(def, def->def.st);
		break;
		
	case DEF_UNION:
		return xl_union(def, def->def.un, def->def_name);
		break;
		
	case DEF_ENUM:
		return xl_enum(def, def->def.en, def->def_name);
		break;
		
	case DEF_PROGRAM:
		return xl_program(def, def->def.pr, refnum);
		break;
		
	default:
		panic("Don't know def_kind %d.", def->def_kind);
	}
	
	return ((aoi_type) 0);
}

void translate(void)
{
	definition *def;
	
	outaoi.defs.defs_len = 0;
	outaoi.defs.defs_val = 0;
	aoi_max = 0;
	
	for (def = defs; def; def = def->next)
		if (aoi_name(def, def->def_name, MUST_CREATE) < 0)
			panic("Duplicate symbol `%s'.", def->def_name);
	
	for (def = defs; def; def = def->next) {
		int refnum = aoi_name(def, def->def_name, NO_CREATE);
		
		/* Find its binding. */
		outaoi.defs.defs_val[refnum].binding = xl_scope(def, refnum);
	}
}

/* End of file. */

