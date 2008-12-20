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

#include "helpers.hh"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <mom/compiler.h>
#include <mom/libmeta.h>
#include <mom/libaoi.h>

/*
 * `ALLOW_ROOT_TYPECODE' allows IDL specifications to use `TypeCode' as a
 * built-in equivalent to `CORBA::TypeCode'.  (This is not standard CORBA IDL.)
 *
 * If `ALLOW_ROOT_TYPECODE' is defined, IDL files need not `#include <orb.idl>'
 * in order to use `TypeCode'.  In short, we handle `TypeCode' in much the same
 * way as we handle `Object' as a built-in.  References to `CORBA::TypeCode'
 * are always interpreted correctly, whether or not this symbol is defined.
 *
 * XXX --- Really, `ALLOW_ROOT_TYPECODE' is wrong: correct IDL must refer to
 * `CORBA::TypeCode', not simply `TypeCode'.  We support this behavior only
 * because TAO 0.3.23 allows it.  In the near future, we will undefine this
 * symbol by default.
 */
#define ALLOW_ROOT_TYPECODE (1)

/*****************************************************************************/

extern int warningcount;
extern int errorcount;

extern io_file_index builtin_file;
extern io_file_index root_file;
extern io_file_index current_i_file;

/* #define PACKED  - use this for 6 chars/word packing */

#define UPCASE(ch) ((ch >= 'a' && ch <= 'z') ? (ch & 0x5F) : ch)

/*
 * Case-sensitive string comparison.  We wrap `strcmp' within a C++ function in
 * order to avoid a warning from the Sun WorkShop C++ 5.0 compiler about C/C++
 * linkage:
 *
 * ``Warning (Anachronism): Using extern "C" int(*)(const char*,const char*) to
 * initialize int(*)(const char*,const char*).''
 */
static int
string_cmp(const char *a, const char *b)
{
	return strcmp(a, b);
}

/*
 * Case-insentive string comparison.  XXX --- This should be rewritten simply
 * as a C++ wrapper around `flick_strcasecmp'.
 */
static int
string_icmp(const char *a, const char *b)
{
	while (*a && *b) {
		if (UPCASE(*a) != UPCASE(*b))
			return UPCASE(*a) - UPCASE(*b);
		a++, b++;
	}
	if (*a != *b)
		return (*a ? 1 : -1);
	return 0;
}

typedef int (*cmp)(const char *, const char *);

static cmp compare = &string_cmp;

static ref_list *the_scope = 0;

/* `undefined' stores names that are reported as undefined. */
static VoidArray undefined;
#define UNDEFINED_ARRAY_ELEM_TYPE	const char *


/*****************************************************************************/

void
AddScope(char *a)
{
	scope++;
	assert(a);
	ref_list *end;
	if (!the_scope)
		the_scope = new ref_list(a, 0);
	else {
		end = the_scope;
		while (end->next)
			end = end->next;
		end->next = new ref_list(a, 0);
	}
}

void
DelScope()
{
	assert(the_scope);
	ref_list *tmp, tmp2(0, the_scope);
	tmp = &tmp2;
	while (tmp->next->next)
		tmp = tmp->next;
	if (tmp->next == the_scope)
		the_scope = 0;
	delete tmp->next;
	tmp->next = 0;
	scope--;
}

static const char *
GetFullScopePrefix(ref_list *sc = the_scope)
{
	if (!sc)
		return "";
	if (!sc->next)
		return flick_asprintf("%s::", sc->name);
	return flick_asprintf("%s::%s",
			      sc->name,
			      GetFullScopePrefix(sc->next));
}


/*****************************************************************************/

void
DupeError(aoi_def *d, aoi_kind kind)
{
	/* Override the `kind' passed in, if this has already been defined. */
	if (d->binding)
		kind = d->binding->kind;
	
	if (*d->name && /* if not "" */
	    (kind != AOI_ERROR)) {
		int pos;
		
		pos = cur_aoi.defs.defs_len - 1;
		while( (pos >= 0) &&
		       (cur_aoi.defs.defs_val[pos].scope >= scope) )
			pos--;
		/* Disallow using the name of the
		   immediately enclosing scope */
		if ((pos < 0) ||
		    ((!cur_aoi.defs.defs_val[pos].name ||
		      ((cur_aoi.defs.defs_val[pos].scope + 1) != d->scope) ||
		      strcmp(cur_aoi.defs.defs_val[pos].name, d->name)) &&
		     !aoi_def_has_member(&cur_aoi, &cur_aoi.defs.defs_val[pos],
					 d->name))) {
			cmp func = compare;
			compare = &string_icmp;
			/* Find this name, but don't report errors. */
			pos = FindLocalName(d->name, 0);
			compare = func;
			if (pos < 0) /* It wasn't found; no problem... */
				return;
			if (cur_aoi.defs.defs_val[pos].scope != d->scope)
				return;
			
			/* Forward interface cases. */
			if ((cur_aoi.defs.defs_val[pos].binding->kind ==
			     AOI_INTERFACE)
			    && (kind == AOI_FWD_INTRFC))
				return;
			if ((cur_aoi.defs.defs_val[pos].binding->kind ==
			     AOI_FWD_INTRFC)
			    && (kind == AOI_INTERFACE))
				return;
			if ((cur_aoi.defs.defs_val[pos].binding->kind ==
			     AOI_FWD_INTRFC)
			    && (kind == AOI_FWD_INTRFC))
				return;
			
			/* Errors and namespaces. */
			if (cur_aoi.defs.defs_val[pos].binding->kind ==
			    AOI_ERROR)
				return;
			if ((cur_aoi.defs.defs_val[pos].binding->kind ==
			     AOI_NAMESPACE)
			    && (kind == AOI_NAMESPACE))
				return;
		}
		
		SemanticError("`%s' already defined", d->name);
	}
#if 0 /* KBF - Hopefully, this is now fixed */
	int pos = GetScopedNameList(the_scope);
	if (pos >= 0) {
		if (!i)
			i = 1;
		else
			i = (cur_aoi.defs.defs_val[pos].binding->kind
			     != AOI_FWD_INTRFC);
		if (i && (cur_aoi.defs.defs_val[pos].binding->kind !=
			  AOI_ERROR)) {
			SemanticError("`%s' already defined", a);
		}
	}
#endif /* 0 */
}

void
UndefError(char *name)
{
	for (int i = 0; i < undefined.cur_num; ++i)
		if (!compare(name,
			     ARRAY_REF(undefined,
				       UNDEFINED_ARRAY_ELEM_TYPE,
				       i)
			     ))
			return;
	SemanticError("`%s' not defined", name);
	ADD_TO_ARRAY(undefined, UNDEFINED_ARRAY_ELEM_TYPE, name);
}

/* Simply allocates and initializes a new `aoi_def'. */
aoi_def *
new_aoi_def(const char *name, int scope)
{
	aoi_def *def = (aoi_def *) mustcalloc(sizeof(aoi_def));
	
	def->name = ir_strlit(name);
	def->scope = scope;
	
	return def;
}

int
new_error_ref(const char *name)
{
	aoi_def *error_def;
	
	error_def = new_aoi_def(name, scope);
	error_def->idl_file = 0;
	error_def->binding = (aoi_type) mustcalloc(sizeof(aoi_type_u));
	error_def->binding->kind = AOI_ERROR;
	AddDef(error_def, AOI_ERROR);
	
	return cur_aoi.defs.defs_len - 1;
}


/*****************************************************************************/

static aoi_type
xl_predef_int(int min, unsigned range)
{
	aoi_type node = (aoi_type) mustcalloc(sizeof(aoi_type_u));
	
	node->kind = AOI_INTEGER;
	node->aoi_type_u_u.integer_def.min = min;
	node->aoi_type_u_u.integer_def.range = range;
	
	return node;
}

#ifdef PACKED
static aoi_type
xl_predef_array_of_int(int array_min, unsigned array_range)
{
	aoi_type t = (aoi_type) mustcalloc(sizeof(aoi_type_u));
	
	t->kind = AOI_ARRAY;
	t->aoi_type_u_u.array_def.element_type = xl_predef_int(0,4294967295U);
	t->aoi_type_u_u.array_def.length_type = xl_predef_int(array_min,
							      array_range);
	t->aoi_type_u_u.array_def.flgs = AOI_ARRAY_FLAG_NONE;
	return t;
}
#else
static aoi_type
xl_predef_char(int bits)
{
	aoi_type node = (aoi_type) mustcalloc(sizeof(aoi_type_u));
	
	node->kind = AOI_CHAR;
	node->aoi_type_u_u.char_def.bits = bits;
	node->aoi_type_u_u.char_def.flags = AOI_CHAR_FLAG_NONE;
	
	return node;
}

static aoi_type
xl_predef_string(int array_min, unsigned array_range)
{
	aoi_type t = (aoi_type) mustcalloc(sizeof(aoi_type_u));
	
	t->kind = AOI_ARRAY;
	t->aoi_type_u_u.array_def.element_type = xl_predef_char(8);
	t->aoi_type_u_u.array_def.length_type = xl_predef_int(array_min,
							      array_range);
	t->aoi_type_u_u.array_def.flgs = AOI_ARRAY_FLAG_NULL_TERMINATED_STRING;
	return t;
}
#endif /* PACKED */


/*****************************************************************************/

void
Start()
{
	init_meta(&cur_aoi.meta_data);
	meta_add_channel(&cur_aoi.meta_data,
			 meta_add_file(&cur_aoi.meta_data, "(generated)",
				       IO_FILE_INPUT),
			 "");
	builtin_file = meta_add_file(&cur_aoi.meta_data,
				     "(builtin)",
				     IO_FILE_BUILTIN);
	/*
	 * We strip off any leading path on `root_filename'.  We don't want AOI
	 * files to differ based on the path to the input file.
	 */
	root_file = meta_add_file(&cur_aoi.meta_data,
				  file_part(root_filename),
				  IO_FILE_INPUT|IO_FILE_ROOT);
	current_i_file = root_file;
	cur_aoi.defs.defs_len = 0;
	cur_aoi.defs.defs_val = (aoi_def *) mustcalloc(8*sizeof(aoi_def));
	saved_aoi_len = 0; /* Initial value not important. */
	aoi_length = 8;
	
	cur_interface = GetNewInterface();
	
	aoi_def *tmp = new_aoi_def("CORBA", 0);
	tmp->binding = (aoi_type) mustcalloc(sizeof(aoi_type_u));
	tmp->binding->kind = AOI_NAMESPACE;
	AddDef(tmp, AOI_NAMESPACE);
	cur_aoi.defs.defs_val[cur_aoi.defs.defs_len - 1].idl_file =
		builtin_file;
	
	tmp = new_aoi_def("Object", 1);
	tmp->binding = (aoi_type) mustcalloc(sizeof(aoi_type_u));
	tmp->binding->kind = AOI_FWD_INTRFC;
	AddDef(tmp, AOI_FWD_INTRFC);
	cur_aoi.defs.defs_val[cur_aoi.defs.defs_len - 1].idl_file =
		builtin_file;
	
#ifdef ALLOW_ROOT_TYPECODE
	/*
	 * A hack: TAO 0.3.23 allows IDL to use `TypeCode' instead of requiring
	 * one to `#include <orb.idl>' and then refer to `CORBA::TypeCode'.  To
	 * support that behavior, we can make an implied `TypeCode' interface.
	 */
	tmp = new_aoi_def("TypeCode", 0);
	tmp->binding = (aoi_type) mustcalloc(sizeof(aoi_type_u));
	tmp->binding->kind = AOI_FWD_INTRFC;
	AddDef(tmp, AOI_FWD_INTRFC);
	cur_aoi.defs.defs_val[cur_aoi.defs.defs_len - 1].idl_file =
		builtin_file;
#endif /* ALLOW_ROOT_TYPECODE */
	
	NEW_ARRAY(undefined, UNDEFINED_ARRAY_ELEM_TYPE, "");
}

void
Finish()
{
	/* Right now, there's nothing needed for cleanup. */
}

aoi_interface *
GetNewInterface(void)
{
	aoi_interface *res = (aoi_interface *)
		mustcalloc(sizeof(aoi_interface));
	
	res->idl = AOI_IDL_CORBA;
	/* XXX ---- This uses PACKED STRINGS, not char arrays. */
#ifdef PACKED
	res->code_type = xl_predef_array_of_int(0, ~0);
#else
	res->code_type = xl_predef_string(0, 66000);
#endif
	res->code = 0;
	res->parents.parents_len = 0;
	res->parents.parents_val = 0;
	res->op_code_type = res->code_type;
	res->ops.ops_len = 0;
	res->ops.ops_val = 0;
	res->attribs.attribs_len = 0;
	res->attribs.attribs_val = 0;
	res->excepts.excepts_len = 0;
	res->excepts.excepts_val = 0;
	return res;
}

aoi_def *
AoiConst(types t, char *name, aoi_const val)
{
	aoi_def *res = new_aoi_def(name, scope);
	/* XXX --- Need to check type against kind. */
	if (t != kERROR) {
		res->binding = (aoi_type) mustcalloc(sizeof(aoi_type_u));
		res->binding->kind = AOI_CONST;
		res->binding->aoi_type_u_u.const_def.type = MakeAoiType(t);
		res->binding->aoi_type_u_u.const_def.value = val;
	}
	return res;
}

aoi_def *
AddDef(aoi_def *def, aoi_kind kind)
{
	DupeError(def, kind);
	
	if (aoi_length == (signed int) cur_aoi.defs.defs_len) {
		aoi_length += 8;
		aoi_def *array = (aoi_def *) mustcalloc(sizeof(aoi_def)
							* aoi_length);
		
		for (int tmp = 0;
		     tmp <(signed int) cur_aoi.defs.defs_len;
		     ++tmp)
			array[tmp] = cur_aoi.defs.defs_val[tmp];
		/* free(cur_aoi.defs.defs_val); */
		cur_aoi.defs.defs_val = array;
	}
	
	def->idl_file = current_i_file;
	cur_aoi.defs.defs_val[cur_aoi.defs.defs_len++] = *def;
	return &cur_aoi.defs.defs_val[cur_aoi.defs.defs_len - 1];
}

void
AddAttrs(VoidArray *attrs)
{
	if (!cur_interface->attribs.attribs_len) {
		cur_interface->attribs.attribs_len
			= attrs->cur_num;
		cur_interface->attribs.attribs_val
			= &(ARRAY_REF(*attrs, aoi_attribute, 0));
	} else {
		aoi_attribute *old = cur_interface->attribs.attribs_val;
		int total = (attrs->cur_num
			     + cur_interface->attribs.attribs_len);
		int tmp;
		
		cur_interface->attribs.attribs_val
			= (aoi_attribute *) mustcalloc(sizeof(aoi_attribute)
						       * total);
		for (tmp = 0;
		     tmp < (signed int) cur_interface->attribs.attribs_len;
		     ++tmp)
			cur_interface->attribs.attribs_val[tmp] = old[tmp];
		
		for (int z = 0; z < attrs->cur_num; ++z)
			cur_interface->attribs.attribs_val[tmp + z]
				= ARRAY_REF(*attrs, aoi_attribute, z);
		
		cur_interface->attribs.attribs_len = total;
	}
}

void
AddOp(aoi_operation *op)
{
	aoi_def fake_def;
	
	fake_def.binding = 0;
	fake_def.name = op->name;
	fake_def.scope = scope;
	DupeError(&fake_def, AOI_STRUCT);
	
	aoi_operation *ops = (aoi_operation *)
		mustcalloc(sizeof(aoi_operation)
			   * (cur_interface->ops.ops_len + 1));
	
	for (int tmp = 0; tmp < (signed int) cur_interface->ops.ops_len; ++tmp)
		ops[tmp] = cur_interface->ops.ops_val[tmp];
	ops[cur_interface->ops.ops_len++] = *op;
	/* free(cur_interface->ops.ops_val); */
	cur_interface->ops.ops_val = ops;
}

void
AddParents(VoidArray *parents)
{
	int ref;
	
	cur_interface->parents.parents_len
		= parents->cur_num;
	cur_interface->parents.parents_val
		= (aoi_type *) mustcalloc(sizeof(aoi_type) * parents->cur_num);
	
	for (int tmp = 0; tmp < parents->cur_num; ++tmp) {
		aoi_type par = (aoi_type) mustcalloc(sizeof(aoi_type_u));
		
		par->kind = AOI_INDIRECT;
		ref = par->aoi_type_u_u.indirect_ref = ARRAY_REF(*parents,
								 int, tmp);
		if (cur_aoi.defs.defs_val[ref].binding
		    && (cur_aoi.defs.defs_val[ref].binding->kind !=
			AOI_INTERFACE)) {
			SemanticError("`%s' is not defined as an interface",
				      cur_aoi.defs.defs_val[ref].name);
		}
		cur_interface->parents.parents_val[tmp] = par;
	}
}

int
FindLocalName(char *name, int err)
{
	ref_list *scopes = new ref_list(0, 0);
	int cur_pos = cur_aoi.defs.defs_len - 1;
	int last_scope = scope;
	int res;
	
	/* Build the maximum scoped name to search for. */
	while (cur_pos >= 0) {
		while ((cur_pos >= 0)
		       && (cur_aoi.defs.defs_val[cur_pos].scope >= last_scope))
			cur_pos--;
		if (cur_pos >= 0) {
			scopes = new ref_list(cur_aoi.defs.defs_val[cur_pos].
					      name, scopes);
			last_scope = cur_aoi.defs.defs_val[cur_pos].scope;
		}
	}
	
	/* Step through each possibility from `a::b::c' to `a::c' to `::c'. */
	while (1) {
		ref_list *tmp = scopes, *parent = scopes;
		
		while (tmp->name) {
			parent = tmp;
			tmp = tmp->next;
		}
		tmp->name = name;
		res = GetScopedNameList(scopes);
		if (res >= 0)
			return res;
		if (parent == tmp) {
			if (err) {
				UndefError(name);
				return new_error_ref(name);
			}
			return -1;
		}
		parent->name = 0;
		parent->next = 0;
		delete tmp;
	}
}

int
FindGlobalName(char *name)
{
	int res;
	ref_list tmp(name, 0);
	
	res = GetScopedNameList(&tmp);
	if (res < 0) {
		UndefError(name);
		return new_error_ref(name);
	}
	return res;
}

int
FindScopedName(char *name, int ref)
{
	ref_list *scopes = new ref_list(cur_aoi.defs.defs_val[ref].name,
					new ref_list(name, 0));
	int last_scope = cur_aoi.defs.defs_val[ref].scope;
	
	/* Build the maximum scoped name to search for. */
	while (ref >= 0) {
		while ((cur_aoi.defs.defs_val[ref].scope >= last_scope) &&
		       (ref >= 0))
			ref--;
		if (ref >= 0) {
			scopes = new ref_list(cur_aoi.defs.defs_val[ref].name,
					      scopes);
			last_scope = cur_aoi.defs.defs_val[ref].scope;
		}
	}
	
	int res = GetScopedNameList(scopes);
	if (res < 0) {
		UndefError(name);
		return new_error_ref(name);
	}
	return res;
}

int
GetScopedNameList(ref_list *scopes)
{
	int res, pos, last_if = -1;
	
	for (pos = 0; pos < (signed int) cur_aoi.defs.defs_len; ++pos) {
		if (!compare(scopes->name, cur_aoi.defs.defs_val[pos].name)
		    && (cur_aoi.defs.defs_val[pos].scope == 0)) {
			if (!scopes->next) {
				/* The value is a top scope and we found it. */
				if (cur_aoi.defs.defs_val[pos].binding
				    && (cur_aoi.defs.defs_val[pos].binding->
					kind == AOI_FWD_INTRFC))
					last_if = pos;
				else
					return pos;
			}
			res = GetInsideScope(scopes->next, pos);
			if (res >= 0) {
				if (cur_aoi.defs.defs_val[res].binding
				    && (cur_aoi.defs.defs_val[res].binding->
					kind == AOI_FWD_INTRFC))
					last_if = res;
				else
					return res;
			}
		}
	}
	return last_if;
}

int
GetInsideScope(ref_list *scopes, int pos)
{
	/* Search for the scoped name ONLY within the specified scope. */
	unsigned int top = pos + 1;
	unsigned int bottom = pos + 1;
	int par_scope = cur_aoi.defs.defs_val[pos].scope;
	int res;
	int last_if = -1;
	
	while ((bottom < cur_aoi.defs.defs_len)
	       && (cur_aoi.defs.defs_val[bottom].scope > par_scope))
		bottom++;
	
	while (top < bottom) {
		if (!compare(scopes->name, cur_aoi.defs.defs_val[top].name)
		    && (cur_aoi.defs.defs_val[top].scope == par_scope + 1)) {
			if (!scopes->next) {
				if (cur_aoi.defs.defs_val[top].binding
				    && (cur_aoi.defs.defs_val[top].binding->
					kind == AOI_FWD_INTRFC))
					last_if = top;
				else
					return top;
			}
			res = GetInsideScope(scopes->next, top);
			if (res >= 0) {
				if (cur_aoi.defs.defs_val[res].binding
				    && (cur_aoi.defs.defs_val[res].binding->
					kind == AOI_FWD_INTRFC))
					last_if = res;
				else
					return res;
			}
		}
		top++;
	}
	return last_if;
}

types
GetConstType(aoi_ref type)
{
	aoi_kind k = cur_aoi.defs.defs_val[type].binding->kind;
	switch (k) {
	case AOI_INTEGER: {
		aoi_integer t = cur_aoi.defs.defs_val[type].binding->
			aoi_type_u_u.integer_def;
		
		if (t.min < -32768)
			return kSLONG;
		else if (t.min < 0)
			return kSSHORT;
		else if (t.range > 65536)
			return kULONG;
		else if (t.range > 256)
			return kUSHORT;
		else if (t.range > 1)
			return kOCTET;
		else
			return kBOOL;
	}
	
	case AOI_FLOAT: {
		aoi_float f = cur_aoi.defs.defs_val[type].binding->
			aoi_type_u_u.float_def;
		return ((f.bits > 32) ? kDOUBLE : kFLOAT);
	}
	
	case AOI_CHAR:
		return kCHAR;
		
	case AOI_INDIRECT:
		return GetConstType(cur_aoi.defs.defs_val[type].binding->
				    aoi_type_u_u.indirect_ref);
		
	case AOI_ERROR:
		return kERROR;
		
	default:
		SemanticError("`%s' is not a valid type for a constant",
			      cur_aoi.defs.defs_val[type].name);
		return kERROR;
	}
}

aoi_const
GetConstVal(aoi_ref r)
{
	if (cur_aoi.defs.defs_val[r].binding->kind == AOI_INDIRECT)
		return GetConstVal(cur_aoi.defs.defs_val[r].binding->
				   aoi_type_u_u.indirect_ref);
	
	if (cur_aoi.defs.defs_val[r].binding->kind == AOI_ERROR) {
		aoi_const error_const = (aoi_const)
			mustcalloc(sizeof(aoi_const_u));
		
		error_const->kind = AOI_CONST_INT;
		error_const->aoi_const_u_u.const_int = 0;
		return error_const;
		
	} else {
		if (cur_aoi.defs.defs_val[r].binding->kind != AOI_CONST) {
			SemanticError("incorrect type (not a constant)");
			ConfusedExit();
		}
		return (cur_aoi.defs.defs_val[r].binding->aoi_type_u_u.
			const_def.value);
	}
}

/* A helper function for `GetAoiType', below. */
static int
GetAoiTypeSpecial(aoi_ref r, aoi_type at)
{
	int is_special = 0;
	aoi_ref scope_ref;
	
	/*****/
	
	if ((r < 0) || (r >= ((signed int) cur_aoi.defs.defs_len)))
		InternalError("invalid `aoi_ref' in `GetAoiTypeSpecial'.");
	
	/*
	 * If `r' refers to `CORBA::TypeCode', set `is_special' to true and
	 * set `at' to be an `AOI_TYPE_TAG'.
	 */
	if (!strcmp(cur_aoi.defs.defs_val[r].name, "TypeCode")) {
		/* It's a `TypeCode', but is it a `CORBA::TypeCode'? */
		
		scope_ref = aoi_get_parent_scope(&cur_aoi, r);
		if ((scope_ref == aoi_ref_null)
		    || strcmp(cur_aoi.defs.defs_val[scope_ref].name, "CORBA")
		    || (cur_aoi.defs.defs_val[scope_ref].scope != 0)) {
			/* It's not `CORBA::TypeCode'. */
			is_special = 0;
			
		} else {
			/* Yes, it's `CORBA::TypeCode'. */
			is_special = 1;
			at->kind = AOI_TYPE_TAG;
		}
		
#ifdef ALLOW_ROOT_TYPECODE
		/*
		 * If we allow IDL to use `TypeCode', then references to that
		 * interface are special, too.
		 */
		if (scope_ref == aoi_ref_null) {
			/* Yes, it's `TypeCode' in the root scope. */
			is_special = 1;
			at->kind = AOI_TYPE_TAG;
		}
#endif /* ALLOW_ROOT_TYPECODE */
	}
	
	return is_special;
}

aoi_type
GetAoiType(aoi_ref r)
{
	aoi_type res = (aoi_type) mustcalloc(sizeof(aoi_type_u));
	
	/*
	 * The simple and overwhelmingly common case is simply to make an
	 * `AOI_INDIRECT' reference to the AOI definition at `r'; this
	 * represents a use of a named type.
	 *
	 * But there are special cases: e.g., a reference to `CORBA::TypeCode'
	 * must be turned into a literal `AOI_TYPE_TAG'.  This special meaning
	 * for `CORBA::TypeCode' is part of the semantics of the CORBA IDL ---
	 * not particular to any single presentation --- and so we must make
	 * the translation here rather than in the PG.  Special translations
	 * are handled by a separate `GetAoiTypeSpecial' function.
	 */
	if (GetAoiTypeSpecial(r, res) == 0) {
		res->kind = AOI_INDIRECT;
		res->aoi_type_u_u.indirect_ref = r;
	}
	
	return res;
}

aoi_type
MakeAoiType(types t)
{
	aoi_type res = (aoi_type) mustcalloc(sizeof(aoi_type_u));
	
	switch (t) {
	case kBOOL:
		res->kind = AOI_INTEGER;
		res->aoi_type_u_u.integer_def.min = 0;
		res->aoi_type_u_u.integer_def.range = 1U;
		break;
		
	case kCHAR:
		res->kind = AOI_CHAR;
		res->aoi_type_u_u.char_def.bits = 8;
		res->aoi_type_u_u.char_def.flags = AOI_CHAR_FLAG_NONE;
		break;
		
	case kDOUBLE:
		res->kind = AOI_FLOAT;
		res->aoi_type_u_u.float_def.bits = 64;
		break;
		
	case kFLOAT:
		res->kind = AOI_FLOAT;
		res->aoi_type_u_u.float_def.bits = 32;
		break;
		
	case kOCTET:
		res->kind = AOI_INTEGER;
		res->aoi_type_u_u.integer_def.min = 0;
		res->aoi_type_u_u.integer_def.range = 255U;
		break;
		
	case kSSHORT:
		res->kind = AOI_INTEGER;
		res->aoi_type_u_u.integer_def.min = -32768;
		res->aoi_type_u_u.integer_def.range = 65535U;
		break;
		
	case kSLONG:
		res->kind = AOI_INTEGER;
		res->aoi_type_u_u.integer_def.min = (-2147483647 - 1);
		res->aoi_type_u_u.integer_def.range = 4294967295U;
		break;
		
	case kUSHORT:
		res->kind = AOI_INTEGER;
		res->aoi_type_u_u.integer_def.min = 0;
		res->aoi_type_u_u.integer_def.range = 65535U;
		break;
		
	case kULONG:
		res->kind = AOI_INTEGER;
		res->aoi_type_u_u.integer_def.min = 0;
		res->aoi_type_u_u.integer_def.range = 4294967295U;
		break;
		
	case kOBJECT:
		res->kind = AOI_INDIRECT;
		res->aoi_type_u_u.indirect_ref = 1;
		break;
		
	case kANY:
		/*
		 * A CORBA IDL `any' represents a type-tagged value, i.e., an
		 * `AOI_TYPED', not just an `AOI_ANY'.
		 */
		res->kind = AOI_TYPED;
		
		res->aoi_type_u_u.typed_def.tag
			= (aoi_type) mustcalloc(sizeof(aoi_type_u));
		res->aoi_type_u_u.typed_def.tag->kind
			= AOI_TYPE_TAG;
		
		res->aoi_type_u_u.typed_def.type
			= (aoi_type) mustcalloc(sizeof(aoi_type_u));
		res->aoi_type_u_u.typed_def.type->kind
			= AOI_ANY;
		break;
		
	case kERROR:
		res->kind = AOI_ERROR;
		break;
		
	case kSLLONG:
	case kULLONG:
		res->kind = AOI_SCALAR;
		res->aoi_type_u_u.scalar_def.bits = 64;
		res->aoi_type_u_u.scalar_def.flags
			= ((t == kULLONG) ?
			   AOI_SCALAR_FLAG_UNSIGNED :
			   AOI_SCALAR_FLAG_NONE);
		break;
		
	case kSTRING:
	default:
		InternalError("unknown type from which to create an AOI_TYPE");
		break;
	}
	return res;
}

aoi_type
GetAoiTypeFromDecl(aoi_type type, Declaration d)
{
	if (d.sizes.cur_num == 0)
		return type;
	else {
		aoi_type res = (aoi_type) mustcalloc(sizeof(aoi_type_u));
		
		res->kind = AOI_ARRAY;
		res->aoi_type_u_u.array_def.length_type
			= xl_predef_int(ARRAY_REF(d.sizes, unsigned int, 0),
					0);
		res->aoi_type_u_u.array_def.flgs = AOI_ARRAY_FLAG_NONE;
		d.sizes.cur_num--;
		d.sizes.data = &(ARRAY_REF(d.sizes, unsigned int, 1));
		res->aoi_type_u_u.array_def.element_type
			= GetAoiTypeFromDecl(type, d);
		return res;
	}
}

aoi_def *
GetAoiDefFromDecl(aoi_type type, Declaration d)
{
	aoi_def *res = (aoi_def *) mustcalloc(sizeof(aoi_def));
	
	res->scope = scope;
	res->name = d.name;
	res->binding = GetAoiTypeFromDecl(type, d);
	return res;
}

aoi_const
GetReadRequest(char *attr_name)
{
	return GetRequest(flick_asprintf("_get_%s", attr_name));
}

aoi_const
GetReadReply(char *attr_name)
{
	return GetReply(flick_asprintf("<_get_%s", attr_name));
}

aoi_const
GetWriteRequest(char *attr_name, int val)
{
	return (val ? 0 : GetRequest(flick_asprintf("_set_%s", attr_name)));
}

aoi_const
GetWriteReply(char *attr_name, int val)
{
	return (val ? 0 : GetRequest(flick_asprintf("<_set_%s", attr_name)));
}  

aoi_const
GetRequest(char *name)
{
	return MakeConstPackedString(name);
}

aoi_const
GetReply(char *name)
{
	return MakeConstPackedString(flick_asprintf("$%s", name));
}

aoi_const
GetInterfaceCode(char *name)
{
	return MakeConstPackedString(flick_asprintf("%s%s",
						    GetFullScopePrefix(),
						    name));
}

unsigned int
GetPosInt(aoi_const c)
{
	unsigned int res = 1;
	
	if (!c)
		InternalError(("null constant provided as positive constant "
			       "integer"));
	else if (c->kind != AOI_CONST_INT)
		SemanticError(("invalid type provided as positive constant "
			       "integer"));
	else
		res = (unsigned int) (c->aoi_const_u_u.const_int);
	
	return res;
}


/*****************************************************************************/

int
isInt(aoi_const_u c)
{
	return (c.kind == AOI_CONST_INT);
}

int
isFloat(aoi_const_u c)
{
	return (c.kind == AOI_CONST_FLOAT);
}

#define INT_OPER(op1, op2, oper) {					\
	if (!op1 || !op2)						\
		InternalError("null constant passed to operator");	\
	if (!isInt(*op1) || !isInt(*op2)) {				\
		SemanticError("invalid type passed to operator");	\
		return op1;						\
	}								\
	return MakeConstInt(op1->aoi_const_u_u.const_int oper		\
			    op2->aoi_const_u_u.const_int);		\
}

aoi_const
const_or(aoi_const a, aoi_const b)
{
	INT_OPER(a, b, |);
}

aoi_const
const_xor(aoi_const a, aoi_const b)
{
	INT_OPER(a, b, ^);
}

aoi_const
const_and(aoi_const a, aoi_const b)
{
	INT_OPER(a, b, &);
}

aoi_const
const_lshft(aoi_const a, aoi_const b)
{
	INT_OPER(a, b, <<);
}

aoi_const
const_rshft(aoi_const a, aoi_const b)
{
	INT_OPER(a, b, >>);
}

#define FLOAT_INT_OP(op1, op2, oper) {					\
	if (!op1 || !op2)						\
		InternalError("null constant passed to operator");	\
									\
	int isfloat = isFloat(*op1) || isFloat(*op2);			\
									\
	if (   !(isInt(*op1) || isFloat(*op1))				\
	    || !(isInt(*op2) || isFloat(*op2))) {			\
		SemanticError("invalid type passed to operator");	\
		return op1;						\
	}								\
	if (isfloat) {							\
		double val1 = isInt(*op1) ?				\
			      (double) op1->aoi_const_u_u.const_int :	\
			      op1->aoi_const_u_u.const_float;		\
		double val2 = isInt(*op2) ?				\
			      (double) op2->aoi_const_u_u.const_int :	\
			      op2->aoi_const_u_u.const_float;		\
		return MakeConstReal(val1 oper val2);			\
	} else								\
		return MakeConstInt(op1->aoi_const_u_u.const_int oper	\
				    op2->aoi_const_u_u.const_int);	\
}

aoi_const
const_mul(aoi_const a, aoi_const b)
{
	FLOAT_INT_OP(a, b, *);
}

aoi_const
const_div(aoi_const a, aoi_const b)
{
	FLOAT_INT_OP(a, b, /);
}

aoi_const
const_add(aoi_const a, aoi_const b)
{
	FLOAT_INT_OP(a, b, +);
}

aoi_const
const_sub(aoi_const a, aoi_const b)
{
	FLOAT_INT_OP(a, b, +);
}

aoi_const
const_mod(aoi_const a, aoi_const b)
{
	INT_OPER(a, b, %);
}

aoi_const
const_bit(aoi_const a)
{
	if (!a)
		InternalError("null constant passed to `~' operator");
	if (!isInt(*a)) {
		SemanticError("invalid type passed to `~' operator");
		return a;
	}
	return MakeConstInt(~(a->aoi_const_u_u.const_int));
}

aoi_const
const_neg(aoi_const a)
{
	if (!a)
		InternalError("null constant passed to `-' operator");
	if (isInt(*a))
		return MakeConstInt(-(a->aoi_const_u_u.const_int));
	if (isFloat(*a))
		return MakeConstReal(-(a->aoi_const_u_u.const_float));
	
	SemanticError("invalid type passed to `-' operator");
	return a;
}

aoi_const
const_pos(aoi_const a)
{
	if (!a)
		InternalError("null constant passed to `+' operator");
	if (!isInt(*a) || !isFloat(*a))
		SemanticError("invalid type passed to `+' operator");
	return a;
}

aoi_const
MakeConstInt(int a)
{
	aoi_const res = (aoi_const) mustcalloc(sizeof(aoi_const_u));
	
	res->kind = AOI_CONST_INT;
	res->aoi_const_u_u.const_int = a;
	return res;
}

aoi_const
MakeConstReal(double a)
{
	aoi_const res = (aoi_const) mustcalloc(sizeof(aoi_const_u));
	
	res->kind = AOI_CONST_FLOAT;
	res->aoi_const_u_u.const_float = a;
	return res;
}

aoi_const
MakeConstChar(char a)
{
	aoi_const res = (aoi_const) mustcalloc(sizeof(aoi_const_u));
	
	res->kind = AOI_CONST_CHAR;
	res->aoi_const_u_u.const_char = a;
	return res;
}

aoi_const
MakeConstString(char *a)
{
	aoi_const res = (aoi_const) mustcalloc(sizeof(aoi_const_u));
	int len = strlen(a) + 1;
	
	res->kind = AOI_CONST_ARRAY;
	res->aoi_const_u_u.const_array.aoi_const_array_len
		= len;
	res->aoi_const_u_u.const_array.aoi_const_array_val
		= (aoi_const *) mustcalloc(sizeof(aoi_const) * len);
	
	for (int tmp = 0; tmp < len; ++tmp) {
		aoi_const elem = (aoi_const) mustcalloc(sizeof(aoi_const_u));
		
		elem->kind = AOI_CONST_CHAR;
		elem->aoi_const_u_u.const_char = a[tmp];
		res->aoi_const_u_u.const_array.aoi_const_array_val[tmp] = elem;
	}
	
	return res;
}

#ifdef PACKED
static int
GetCharVal(char c)
{
	switch (c) {
	case '_':
		return 27;
	case '<':
		return 28;
	case '>':
		return 29;
	case '$':
		return 30;
	default:
		return (c & 31);
	}
}
#endif

aoi_const
MakeConstPackedString(char *a)
{
	/*
	 * If you want to remove this optimization,
	 * `return MakeConstString(a)' and fix the code you'll find with an
	 * XXX about packed strings.
	 */
#ifdef PACKED
	aoi_const res = (aoi_const) mustcalloc(sizeof(aoi_const_u));
	int slen = strlen(a);
	int len = (slen + 5) / 6;
	
	res->kind = AOI_CONST_ARRAY;
	res->aoi_const_u_u.const_array.aoi_const_array_len
		= len;
	res->aoi_const_u_u.const_array.aoi_const_array_val
		= (aoi_const *) mustcalloc(sizeof(aoi_const) * len);
	
	for (int tmp = 0; tmp < len; ++tmp) {
		aoi_const elem = (aoi_const) mustcalloc(sizeof(aoi_const_u));
		
		elem->kind = AOI_CONST_INT;
		int val = 0;
		for (int z = 0; (tmp * 6 + z < slen) && (z < 6); ++z)
			val = val * 32 + GetCharVal(a[tmp * 6 + z]);
		elem->aoi_const_u_u.const_int = val;
		res->aoi_const_u_u.const_array.aoi_const_array_val[tmp] = elem;
	}
	return res;
#else
	return MakeConstString(a);
#endif /* PACKED */
}


/*****************************************************************************/

/*
 * Check that the given `aoi_union_case' array does not already contain a case
 * with the given discriminator value.  If the value is found in the array,
 * report an error.
 */
void
CheckUnionCaseDuplicate(VoidArray *void_array, aoi_union_case *new_case)
{
	int			i, j;
	aoi_union_case *	cases;
	aoi_def			fake_def;
	
	/*****/
	
	cases = (aoi_union_case *) (void_array->data);
	
	for (i = 0; i < void_array->cur_num; ++i) {
		if (aoi_const_eq(cases[i].val, new_case->val)) {
			SemanticError("duplicate case value in union");
			break;
		}
		for (j = 0; j < void_array->cur_num; j++) {
			if (!strcmp(cases[i].var.name, new_case->var.name)) {
				SemanticError("Case name '%s' already used",
					      new_case->var.name);
				break;
			}
		}
	}
	fake_def.binding = 0;
	fake_def.scope = scope;
	fake_def.name = new_case->var.name;
	DupeError(&fake_def, AOI_STRUCT);
}


/*****************************************************************************/

/*
 * This will make a connection from the forward interfaces to their real
 * interfaces.
 */
void
ResolveForwardInterfaces()
{
	unsigned int i;
	aoi_ref par_ref;
	
	for (i = 0; i < cur_aoi.defs.defs_len; i++) {
		if (cur_aoi.defs.defs_val[i].binding->kind == AOI_FWD_INTRFC) {
			/*
			 * Set to -1 so we will know if it's been connected to
			 * a real interface or not.  Note: This is needed for
			 * implied interfaces so that we will still make the
			 * stub.
			 */
			cur_aoi.defs.defs_val[i].binding->aoi_type_u_u.
				fwd_intrfc_def = -1;
			if (cur_aoi.defs.defs_val[i].idl_file !=
			    builtin_file) {
				par_ref = aoi_deref_fwd(&cur_aoi, i);
				/*
				 * If there isn't a real interface then signal
				 * a warning, but a marshal and unmarshal stub
				 * will still be made.
				 */
				if (par_ref == -1) {
					SemanticWarning(
						("interface `%s' has not been "
						 "defined"),
						cur_aoi.defs.defs_val[i].name
						);
				}
				cur_aoi.defs.defs_val[i].binding->aoi_type_u_u.
					fwd_intrfc_def = par_ref;
			}
		}
	}
}

/*****************************************************************************/

/* End of file. */

