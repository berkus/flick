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

#include <string.h>

#include <mom/compiler.h>
#include <mom/c/libpres_c.h>

/* Create a pres_c_mapping node, or Set the attributes
   for one.  if map == 0 then kind needs to be set so
   that the proper node can be created.  */
pres_c_mapping pres_c_set_mapping_tags( pres_c_mapping_kind kind,
					pres_c_mapping map,
					int tag, ... )
{
	va_list arg_addr;
	pres_c_mapping retval = map;
	
	if( map )
		kind = map->kind;
	va_start( arg_addr, tag );
	switch( kind ) {
	case PRES_C_MAPPING_TYPE_TAG:
	case PRES_C_MAPPING_TYPED:
	case PRES_C_MAPPING_IGNORE:
	case PRES_C_MAPPING_SYSTEM_EXCEPTION:
	case PRES_C_MAPPING_DIRECT:
	case PRES_C_MAPPING_ILLEGAL:
	case PRES_C_MAPPING_ELSEWHERE:
		if( !retval )
			retval = pres_c_new_mapping( kind );
		break;
	case PRES_C_MAPPING_STUB:
		/* create a new pres_c_mapping if one wasn't passed in */
		if( !retval )
			retval = pres_c_new_mapping( kind );
		/* Walk the list of tags until the end (PRES_C_TAG_DONE) */
		while( tag != PRES_C_TAG_DONE ) {
			/* Pivot on the attribute ID */
			switch( tag ) {
			case PMA_Index:
				retval->pres_c_mapping_u_u.mapping_stub.
					mapping_stub_index =
					va_arg( arg_addr, int );
				break;
			default:
				/* Attribute ID wasn't recognized for
				   this kind, alert the user */
				panic( "Tag not understood" );
				break;
			}
			/* Get the next Attribute ID */
			tag = va_arg( arg_addr, int );
		}
		break;
	case PRES_C_MAPPING_DIRECTION:
		if( !retval )
			retval = pres_c_new_mapping( kind );
		while( tag != PRES_C_TAG_DONE ) {
			switch( tag ) {
			case PMA_Dir:
				retval->pres_c_mapping_u_u.direction.dir =
					va_arg( arg_addr, pres_c_direction );
				break;
			case PMA_Mapping:
				retval->pres_c_mapping_u_u.direction.mapping =
					va_arg( arg_addr, pres_c_mapping );
				break;
			default:
				panic( "Tag not understood" );
				break;
			}
			tag = va_arg( arg_addr, int );
		}
		break;
	case PRES_C_MAPPING_POINTER:
		if( !retval )
			retval = pres_c_new_mapping( kind );
		while( tag != PRES_C_TAG_DONE ) {
			switch( tag ) {
			case PMA_ArgList:
				retval->pres_c_mapping_u_u.pointer.arglist_name
					= va_arg(arg_addr, char *);
				break;
			case PMA_Target:
				retval->pres_c_mapping_u_u.pointer.target =
					va_arg( arg_addr, pres_c_mapping );
				break;
			default:
				panic( "Tag not understood" );
				break;
			}
			tag = va_arg( arg_addr, int );
		}
		break;
	case PRES_C_MAPPING_OPTIONAL_POINTER:
		if( !retval )
			retval = pres_c_new_mapping( kind );
		while( tag != PRES_C_TAG_DONE ) {
			switch( tag ) {
			case PMA_ArgList:
				retval->pres_c_mapping_u_u.optional_pointer.
					arglist_name
					= va_arg(arg_addr, char *);
				break;
			case PMA_Target:
				retval->pres_c_mapping_u_u.optional_pointer.
					target = va_arg( arg_addr,
							 pres_c_mapping );
				break;
			default:
				panic( "Tag not understood" );
				break;
			}
			tag = va_arg( arg_addr, int );
		}
		break;
	case PRES_C_MAPPING_INTERNAL_ARRAY:
		if( !retval )
			retval = pres_c_new_mapping( kind );
		while( tag != PRES_C_TAG_DONE ) {
			switch( tag ) {
			case PMA_ArgList:
				retval->pres_c_mapping_u_u.internal_array.
					arglist_name
					= va_arg(arg_addr, char *);
				break;
			case PMA_ElementMapping:
				retval->pres_c_mapping_u_u.internal_array.
					element_mapping
					= va_arg(arg_addr, pres_c_mapping);
				break;
			default:
				panic( "Tag not understood" );
				break;
			}
			tag = va_arg( arg_addr, int );
		}
		break;
	case PRES_C_MAPPING_REFERENCE:
		if( !retval )
			retval = pres_c_new_mapping( kind );
		retval->pres_c_mapping_u_u.ref.arglist_name = ir_strlit("");
		while( tag != PRES_C_TAG_DONE ) {
			switch( tag ) {
			case PMA_Kind:
				retval->pres_c_mapping_u_u.ref.kind =
					va_arg( arg_addr,
						pres_c_reference_kind );
				break;
			case PMA_RefCount:
				retval->pres_c_mapping_u_u.ref.ref_count =
					va_arg( arg_addr, int );
				break;
			case PMA_ArgList:
				retval->pres_c_mapping_u_u.ref.arglist_name =
					va_arg(arg_addr, char *);
				break;
			default:
				panic( "Tag not understood" );
				break;
			}
			tag = va_arg( arg_addr, int );
		}
		break;
	case PRES_C_MAPPING_STRUCT:
		if( !retval )
			retval = pres_c_new_mapping( kind );
		while( tag != PRES_C_TAG_DONE ) {
			switch( tag ) {
			case PMA_Target:
				retval->pres_c_mapping_u_u.struct_i =
					va_arg( arg_addr,
						pres_c_mapping_struct );
				break;
			default:
				panic( "Tag not understood" );
				break;
			}
			tag = va_arg( arg_addr, int );
		}
		break;
	case PRES_C_MAPPING_ARGUMENT:
		if( !retval )
			retval = pres_c_new_mapping( kind );
		while( tag != PRES_C_TAG_DONE ) {
			switch( tag ) {
			case PMA_ArgList:
				retval->pres_c_mapping_u_u.argument.
					arglist_name
					= va_arg( arg_addr, char * );
				break;
			case PMA_Name:
				retval->pres_c_mapping_u_u.argument.arg_name =
					va_arg( arg_addr, char * );
				break;
			case PMA_Mapping:
				retval->pres_c_mapping_u_u.argument.map =
					va_arg( arg_addr, pres_c_mapping );
				break;
			default:
				panic( "Tag not understood" );
				break;
			}
			tag = va_arg( arg_addr, int );
		}
		break;
	case PRES_C_MAPPING_INITIALIZE:
		if( !retval )
			retval = pres_c_new_mapping( kind );
		while( tag != PRES_C_TAG_DONE ) {
			switch( tag ) {
			case PMA_Value:
				retval->pres_c_mapping_u_u.initialize.value =
					va_arg( arg_addr,
						cast_expr );
				break;
			default:
				panic( "Tag not understood" );
				break;
			}
			tag = va_arg( arg_addr, int );
		}
		break;
	case PRES_C_MAPPING_VAR_REFERENCE:
		if (!retval)
			retval = pres_c_new_mapping(kind);
		while (tag != PRES_C_TAG_DONE) {
			switch (tag) {
			case PMA_Target:
				retval->pres_c_mapping_u_u.var_ref.target
					= va_arg(arg_addr, pres_c_mapping);
				break;
			default:
				panic("Tag not understood");
				break;
			}
			tag = va_arg(arg_addr, int);
		}
		break;
	case PRES_C_MAPPING_PARAM_ROOT:
		if (!retval)
			retval = pres_c_new_mapping(kind);
		while (tag != PRES_C_TAG_DONE) {
			switch (tag) {
			case PMA_CType:
				retval->pres_c_mapping_u_u.param_root.ctype
					= va_arg(arg_addr, cast_type);
				break;
			case PMA_Mapping:
				retval->pres_c_mapping_u_u.param_root.map
					= va_arg(arg_addr, pres_c_mapping);
				break;
			default:
				panic("Tag not understood.");
				break;
			}
			tag = va_arg(arg_addr, int);
		}
		break;
	case PRES_C_MAPPING_SELECTOR:
		if (!retval)
			retval = pres_c_new_mapping(kind);
		while (tag != PRES_C_TAG_DONE) {
			switch (tag) {
			case PMA_Index:
				retval->pres_c_mapping_u_u.selector.index
					= va_arg(arg_addr, int);
				break;
			case PMA_Mapping:
				retval->pres_c_mapping_u_u.selector.mapping
					= va_arg(arg_addr, pres_c_mapping);
				break;
			default:
				panic("Tag not understood.");
				break;
			}
			tag = va_arg(arg_addr, int);
		}
		break;
	case PRES_C_MAPPING_TEMPORARY:
		if (!retval) {
			retval = pres_c_new_mapping(kind);
			/*
			 * The following *can* be unspecified, but they can't
			 * be NULL.  Thus we set them to empty strings here.
			 */
			retval->pres_c_mapping_u_u.temp.prehandler = "";
			retval->pres_c_mapping_u_u.temp.posthandler = "";
			retval->pres_c_mapping_u_u.temp.type
				= TEMP_TYPE_PRESENTED; /* by default */
		}
		while (tag != PRES_C_TAG_DONE) {
			pres_c_temporary *temp
				= &retval->pres_c_mapping_u_u.temp;
			switch (tag) {
			case PMA_Name:
				temp->name = va_arg(arg_addr, char *);
				break;
			case PMA_CType:
				temp->ctype = va_arg(arg_addr, cast_type);
				break;
			case PMA_Value:
				temp->init = va_arg(arg_addr, cast_expr);
				break;
			case PMA_IsConst:
				temp->is_const = va_arg(arg_addr, int);
				break;
			case PMA_PreHandler:
				temp->prehandler = va_arg(arg_addr, char *);
				break;
			case PMA_PostHandler:
				temp->posthandler = va_arg(arg_addr, char *);
				break;
			case PMA_TempType:
				temp->type = va_arg(arg_addr,
						    pres_c_temporary_type);
				break;
			case PMA_Target:
				temp->map = va_arg(arg_addr, pres_c_mapping);
				break;
			default:
				panic("Tag not understood.");
				break;
			}
			tag = va_arg(arg_addr, int);
		}
		break;
	case PRES_C_MAPPING_SINGLETON:
		if (!retval)
			retval = pres_c_new_mapping(kind);
		while (tag != PRES_C_TAG_DONE) {
			switch (tag) {
			case PMA_Target:
				retval->pres_c_mapping_u_u.singleton.inl
					= va_arg(arg_addr, pres_c_inline);
				break;
			default:
				panic("Tag not understood.");
				break;
			}
			tag = va_arg(arg_addr, int);
		}
		break;
	case PRES_C_MAPPING_XLATE: {
		pres_c_mapping_xlate *xmap;
		
		if( !retval )
			retval = pres_c_new_mapping(kind);
		xmap = &retval->pres_c_mapping_u_u.xlate;
		xmap->translator = "";
		xmap->destructor = "";
		while( tag != PRES_C_TAG_DONE ) {
			switch( tag ) {
			case PMA_InternalCType:
				xmap->internal_ctype = va_arg(arg_addr,
							      cast_type);
				break;
			case PMA_InternalMapping:
				xmap->internal_mapping =
					va_arg(arg_addr, pres_c_mapping);
				break;
			case PMA_Translator:
				xmap->translator = va_arg(arg_addr, char *);
				break;
			case PMA_Destructor:
				xmap->destructor = va_arg(arg_addr, char *);
				break;
			default:
				panic("Tag not understood.");
				break;
			}
			tag = va_arg(arg_addr, int);
		}
		break;
	}
	default:
		break;
	}
	va_end( arg_addr );
	return( retval );
}

/* End of file. */

