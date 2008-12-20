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
#include <mom/compiler.h>
#include <mom/c/libpres_c.h>
#include <mom/c/libcast.h>

void pres_descend_mapping_stub(pres_c_1 *pres, mint_ref itype,
			       cast_type *ctypep, pres_c_mapping *mapp)
{
	assert(ctypep); assert(*ctypep);
	assert(mapp); assert(*mapp);

	int stub_num = pres_c_find_mu_stub(pres, itype, *ctypep, *mapp,
					   PRES_C_MARSHAL_STUB);
	if (stub_num < 0)
		panic("can't find marshal/unmarshal stub for itype %d", itype);
	
	pres_c_marshal_stub *mstub = &(pres->stubs.stubs_val[stub_num].
				       pres_c_stub_u.mstub);
	
	/*
	 * The ctype must be a simple named type of some kind, so that we can
	 * easily determine how to call the marshal/unmarshal stub.
	 */
	cast_type ctype_name = *ctypep;
	
	while (ctype_name->kind == CAST_TYPE_QUALIFIED)
		ctype_name = ctype_name->cast_type_u_u.qualified.actual;
	
	assert((ctype_name->kind == CAST_TYPE_NAME)
		|| (ctype_name->kind == CAST_TYPE_STRUCT_NAME)
		|| (ctype_name->kind == CAST_TYPE_UNION_NAME)
		|| (ctype_name->kind == CAST_TYPE_ENUM_NAME));
	
	/* Find the actual ctype definition. */
	cast_scope *deep_scope = &pres->cast;
	cast_type ctype;
	int i;
	
	i = cast_find_def(&deep_scope,
			  ctype_name->cast_type_u_u.name,
			  CAST_TYPEDEF|CAST_TYPE);
	while (i >= 0) {
		if( deep_scope->cast_scope_val[i].u.cast_def_u_u.type->
		    kind == CAST_TYPE_CLASS_NAME ) {
			i = cast_find_def_pos(&deep_scope,
					      i + 1,
					      ctype_name->cast_type_u_u.name,
					      CAST_TYPEDEF|CAST_TYPE);
		}
		else
			break;
	}
	if (i < 0) {
		unsigned int j;

		printf( "Couldn't find the definition for " );
		for( j = 0;
		     j < ctype_name->cast_type_u_u.name.cast_scoped_name_len;
		     j++ ) {
			printf( "%s%s",
				j > 0 ? "::" : "",
				ctype_name->cast_type_u_u.name.
				cast_scoped_name_val[j].name );
		}
		printf("\n");
		panic("in pres_descend_mapping_stub");
	}
	/* Enum's are basically atomic so there's no point getting
	   the typedef for them here, unlike structs/unions/etc... */
	switch( ctype_name->kind ) {
	case CAST_TYPE_ENUM_NAME:
		ctype = *ctypep;
		break;
	default:
		ctype = deep_scope->cast_scope_val[i].u.cast_def_u_u.
			typedef_type;
		break;
	}
	/* Find the mapping.
	   Some of this code may be obsolete, now that the mapping we want is
	   stored in the `seethru_map' slot of the `mstub' structure.  (see the
	   comment below.)  Still, it serves as useful sanity checking code for
	   now.
	   */
	assert(pres->stubs_cast.cast_scope_val[mstub->c_func].u.kind
	       == CAST_FUNC_DECL);
	
	cast_func_type *cfunc = &(pres->stubs_cast.
				  cast_scope_val[mstub->c_func].u.
				  cast_def_u_u.func_type);
	
	assert(mstub->i->kind == PRES_C_INLINE_ATOM);
	pres_c_mapping map = mstub->i->pres_c_inline_u_u.atom.mapping;
	assert(map);
	cast_type func_ctype = cfunc->params.params_val[
		mstub->i->pres_c_inline_u_u.atom.index].type;
	
	assert(func_ctype->kind == CAST_TYPE_POINTER);
	
	/*
	 * The following code can no longer be part of our sanity check, since
	 * the pointer we'll eventually get to requires an allocation context,
	 * which requires an inline, etc.  Lucky for us, we are not in need of
	 * the mapping we eventually get to, since it's just the MAPPING_STUB
	 * we already know about (see "New code" comment below).
	 *
	 * assert(map->kind == PRES_C_MAPPING_POINTER);
	 * func_ctype = func_ctype->cast_type_u_u.pointer_type.target;
	 * map = map->pres_c_mapping_u_u.pointer.target;
	 * assert(map);
	 */
	
	/* KBF - XXX - XXX - XXX Removed this assertion, because
	   it was causing problems with Sequence parameters in Corba
	   This should probabaly be restored later, but it doesn't
	   hurt anything (func_ctype is a TYPE_NAME, ctype is a struct name */
	/* assert(func_ctype->kind == ctype_name->kind); */
	
	/* New code, 07/15/1996.
	   The problem with the code above is that `map' ends up referring to a
	   PRES_C_MAPPING_STUB.  Consider: The code above (1) digs through the
	   defintion of the marshaling stub to find the argument that indicates
	   the data to be marshaled, (2) checks that the type of that argument
	   is a pointer --- because a marshaling function always takes a
	   pointer to the data to be marshaled --- and (3) follows the pointer
	   type to locate the mapping for the type that we're going to marshal.
	   Well, the mapping of that type will always be a PRES_C_MAPPING_STUB
	   --- that's why we came here in the first place!  We previously
	   discovered that we had a PRES_C_MAPPING_STUB for that type, and we
	   want to inline the code of the marshaling function!
	   
	   This was a real problem until I added the `seethru_map' field.  It
	   contains the map that tells us how to inline the type at hand.  In
	   short, this field allows us to "see through" or "see into" the body
	   of the marshal function.  No more going around in circles!
	   */
	map = mstub->seethru_map;
	/* End new code, 07/15/1996. */
	
	/* assert(!strcmp(func_ctype->cast_type_u_u.name,
	 *                ctype_name->cast_type_u_u.name));
	 */
	assert(map);
	/* Whew! What a kludge! */
	
	*ctypep = ctype;
	*mapp = map;
}

/* End of file. */

