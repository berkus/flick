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

#include <mom/compiler.h>
#include <mom/c/libpres_c.h>

/* Create a pres_c_inline node or set the attributes for one.
   If inl == 0 then kind needs to be set so that a node is created */
pres_c_inline pres_c_set_inline_tags( pres_c_inline_kind kind,
				      pres_c_inline inl,
				      int tag, ... )
{
	va_list arg_addr;
	pres_c_inline retval = inl;
	int index;
	
	/* object was passed, make sure to get the appropriate kind */
	if( inl )
		kind = inl->kind;
	va_start( arg_addr, tag );
	switch( kind ) {
	case PRES_C_INLINE_ATOM:
		if( !retval )
			retval = pres_c_new_inline( kind );
		/* Set default values */
		retval->pres_c_inline_u_u.atom.index = 0;
		retval->pres_c_inline_u_u.atom.mapping = 0;
		/* Walk the list of tags until the end (PRES_C_TAG_DONE) */
		while( tag != PRES_C_TAG_DONE ) {
			/* Pivot on the attribute ID */
			switch( tag ) {
			case PIA_Index:
				retval->pres_c_inline_u_u.atom.index =
					va_arg( arg_addr, int );
				break;
			case PIA_Mapping:
				retval->pres_c_inline_u_u.atom.mapping =
					va_arg( arg_addr, pres_c_mapping );
				break;
			default:
				/* The attribute ID is unknown for this kind,
				   choke and let the user know */
				panic( "Tag not understood" );
				break;
			}
			/* Get the next attribute ID */
			tag = va_arg( arg_addr, int );
		}
		break;
	case PRES_C_INLINE_STRUCT:
		if( !retval )
			retval = pres_c_new_inline_struct( 0 );
		while( tag != PRES_C_TAG_DONE ) {
			switch( tag ) {
			case PIA_Slot:
				index = pres_c_add_inline_struct_slot(retval);
				retval->pres_c_inline_u_u.struct_i.
					slots.slots_val[index].
					mint_struct_slot_index =
					va_arg( arg_addr, int );
				retval->pres_c_inline_u_u.struct_i.
					slots.slots_val[index].inl =
					va_arg( arg_addr, pres_c_inline );
				break;
			default:
				panic( "Tag not understood" );
				break;
			}
			tag = va_arg( arg_addr, int );
		}
		break;
	case PRES_C_INLINE_FUNC_PARAMS_STRUCT:
		if( !retval )
			retval = pres_c_new_inline_func_params_struct( 0 );
		while( tag != PRES_C_TAG_DONE ) {
			switch( tag ) {
			case PIA_Slot:
				index = pres_c_add_inline_struct_slot(retval);
				retval->pres_c_inline_u_u.func_params_i.
					slots.slots_val[index].
					mint_struct_slot_index =
					va_arg( arg_addr, int );
				retval->pres_c_inline_u_u.func_params_i.
					slots.slots_val[index].inl =
					va_arg( arg_addr, pres_c_inline );
				break;
			case PIA_Return:
				retval->pres_c_inline_u_u.func_params_i.
					return_slot =
					(pres_c_inline_struct_slot *)
					mustcalloc( sizeof(
						pres_c_inline_struct_slot ) );
				retval->pres_c_inline_u_u.func_params_i.
					return_slot->mint_struct_slot_index =
					va_arg( arg_addr, int );
				retval->pres_c_inline_u_u.func_params_i.
					return_slot->inl =
					va_arg( arg_addr, pres_c_inline );
				break;
			default:
				panic( "Tag not understood" );
				break;
			}
			tag = va_arg( arg_addr, int );
		}
		break;
	case PRES_C_INLINE_STRUCT_UNION:
		if( !retval )
			retval = pres_c_new_inline_struct_union( 0 );
		while( tag != PRES_C_TAG_DONE ) {
			switch( tag ) {
			case PIA_Case:
				index = pres_c_add_inline_union_case( retval );
				retval->pres_c_inline_u_u.struct_union.
					cases.cases_val[index].index =
					va_arg( arg_addr, int );
				retval->pres_c_inline_u_u.struct_union.
					cases.cases_val[index].mapping =
					va_arg( arg_addr, pres_c_mapping );
				break;
			case PIA_Discrim:
				retval->pres_c_inline_u_u.struct_union.
					discrim.index =
					va_arg( arg_addr, int );
				retval->pres_c_inline_u_u.struct_union.
					discrim.mapping =
					va_arg( arg_addr, pres_c_mapping );
				break;
			case PIA_Index:
				retval->pres_c_inline_u_u.struct_union.
					union_index =
					va_arg( arg_addr,
						pres_c_inline_index );
				break;
			case PIA_Default:
				retval->pres_c_inline_u_u.struct_union.dfault =
					va_arg( arg_addr,
						pres_c_inline_struct_union_case * );
				break;
			default:
				panic( "Tag not understood" );
				break;
			}
			tag = va_arg( arg_addr, int );
		}
		break;
	case PRES_C_INLINE_VOID_UNION:
		if( !retval )
			retval = pres_c_new_inline_void_union( 0 );
		while( tag != PRES_C_TAG_DONE ) {
			switch( tag ) {
			case PIA_Case:
				index = pres_c_add_inline_union_case( retval );
				retval->pres_c_inline_u_u.void_union.cases.
					cases_val[index].case_value =
					va_arg( arg_addr, cast_expr );
				retval->pres_c_inline_u_u.void_union.cases.
					cases_val[index].type =
					va_arg( arg_addr, cast_type );
				retval->pres_c_inline_u_u.void_union.cases.
					cases_val[index].mapping =
					va_arg( arg_addr, pres_c_mapping );
				break;
			case PIA_Discrim:
				retval->pres_c_inline_u_u.void_union.discrim.
					index = va_arg( arg_addr, int );
				retval->pres_c_inline_u_u.void_union.discrim.
					mapping = va_arg( arg_addr,
							  pres_c_mapping );
				break;
			case PIA_Index:
				retval->pres_c_inline_u_u.void_union.
					void_index =
					va_arg( arg_addr,
						pres_c_inline_index );
				break;
			case PIA_Default:
				retval->pres_c_inline_u_u.void_union.dfault =
					va_arg( arg_addr,
						pres_c_inline_void_union_case * );
				break;
			default:
				panic( "Tag not understood" );
				break;
			}
			tag = va_arg( arg_addr, int );
		}
		break;
	case PRES_C_INLINE_EXPANDED_UNION:
		if( !retval )
			retval = pres_c_new_inline_expanded_union( 0 );
		while( tag != PRES_C_TAG_DONE ) {
			switch( tag ) {
			case PIA_Case:
				index = pres_c_add_inline_union_case( retval );
				retval->pres_c_inline_u_u.expanded_union.
					cases.cases_val[index] =
					va_arg( arg_addr, pres_c_inline );
				break;
			case PIA_Discrim:
				retval->pres_c_inline_u_u.expanded_union.
					discrim.index =
					va_arg( arg_addr, int );
				retval->pres_c_inline_u_u.expanded_union.
					discrim.mapping =
					va_arg( arg_addr, pres_c_mapping );
				break;
			case PIA_Default:
				retval->pres_c_inline_u_u.expanded_union.
					dfault =
					va_arg( arg_addr, pres_c_inline );
				break;
			default:
				panic( "Tag not understood" );
				break;
			}
			tag = va_arg( arg_addr, int );
		}
		break;
	case PRES_C_INLINE_VIRTUAL_UNION:
		if( !retval )
			retval = pres_c_new_inline_virtual_union( 0 );
		while( tag != PRES_C_TAG_DONE ) {
			switch( tag ) {
			case PIA_ArgList:
				retval->pres_c_inline_u_u.virtual_union.
					arglist_name = va_arg(arg_addr,
							      char *);
				break;
			case PIA_Case:
				index = pres_c_add_inline_union_case( retval );
				retval->pres_c_inline_u_u.virtual_union.cases.
					cases_val[index] =
					va_arg( arg_addr, pres_c_inline );
				break;
			case PIA_Discrim:
				retval->pres_c_inline_u_u.virtual_union.discrim
					= va_arg( arg_addr, pres_c_inline );
				break;
			case PIA_Default:
				retval->pres_c_inline_u_u.virtual_union.dfault
					= va_arg( arg_addr, pres_c_inline );
				break;
			default:
				panic( "Tag not understood" );
				break;
			}
			tag = va_arg( arg_addr, int );
		}
		break;
	case PRES_C_INLINE_COLLAPSED_UNION:
		if( !retval )
			retval = pres_c_new_inline( kind );
		while( tag != PRES_C_TAG_DONE ) {
			switch( tag ) {
			case PIA_DiscrimVal:
				retval->pres_c_inline_u_u.collapsed_union.
					discrim_val =
					va_arg( arg_addr, mint_const );
				break;
			case PIA_SelectedCase:
				retval->pres_c_inline_u_u.collapsed_union.
					selected_case =
					va_arg( arg_addr, pres_c_inline );
				break;
			default:
				panic( "Tag not understood" );
				break;
			}
			tag = va_arg( arg_addr, int );
		}
		break;
	case PRES_C_INLINE_TYPED:
		if( !retval )
			retval = pres_c_new_inline( kind );
		while( tag != PRES_C_TAG_DONE ) {
			switch( tag ) {
			case PIA_Tag:
				retval->pres_c_inline_u_u.typed.tag =
					va_arg( arg_addr, pres_c_inline );
				break;
			case PIA_Inl:
				retval->pres_c_inline_u_u.typed.inl =
					va_arg( arg_addr, pres_c_inline );
				break;
			default:
				panic( "Tag not understood" );
				break;
			}
			tag = va_arg( arg_addr, int );
		}
		break;
	case PRES_C_INLINE_ALLOCATION_CONTEXT:
		if (!retval)
			retval = pres_c_new_inline(kind);
		while (tag != PRES_C_TAG_DONE) {
			switch (tag) {
			case PIA_ArgList:
				retval->pres_c_inline_u_u.acontext.
					arglist_name = va_arg(arg_addr,
							      char *);
				break;
				
			case PIA_Offset:
				retval->pres_c_inline_u_u.acontext.
					offset = va_arg(arg_addr,
							pres_c_inline);
				break;
			case PIA_Len:
				retval->pres_c_inline_u_u.acontext.
					length = va_arg(arg_addr,
							pres_c_inline);
				break;
			case PIA_MinLen:
				retval->pres_c_inline_u_u.acontext.
					min_len = va_arg(arg_addr,
							 pres_c_inline);
				break;
			case PIA_MaxLen:
				retval->pres_c_inline_u_u.acontext.
					max_len = va_arg(arg_addr,
							 pres_c_inline);
				break;
			case PIA_AllocLen:
				retval->pres_c_inline_u_u.acontext.
					alloc_len = va_arg(arg_addr,
							   pres_c_inline);
				break;
			case PIA_MinAllocLen:
				retval->pres_c_inline_u_u.acontext.
					min_alloc_len = va_arg(arg_addr,
							       pres_c_inline);
				break;
			case PIA_MaxAllocLen:
				retval->pres_c_inline_u_u.acontext.
					max_alloc_len = va_arg(arg_addr,
							       pres_c_inline);
				break;
			case PIA_Release:
				retval->pres_c_inline_u_u.acontext.
					release = va_arg(arg_addr,
							 pres_c_inline);
				break;
			case PIA_Terminator:
				retval->pres_c_inline_u_u.acontext.
					terminator = va_arg(arg_addr,
							    pres_c_inline);
				break;
			case PIA_MustCopy:
				retval->pres_c_inline_u_u.acontext.
					mustcopy = va_arg(arg_addr,
							  pres_c_inline);
				break;
			case PIA_Alloc:
				retval->pres_c_inline_u_u.acontext.
					alloc = va_arg(arg_addr,
						       pres_c_allocation);
				break;
			case PIA_Overwrite:
				retval->pres_c_inline_u_u.acontext.
					overwrite = va_arg(arg_addr,
							   int);
				break;
			case PIA_Owner:
				retval->pres_c_inline_u_u.acontext.
					owner = va_arg(arg_addr,
						       allocation_owner);
				break;
			case PIA_Ptr:
				retval->pres_c_inline_u_u.acontext.
					ptr = va_arg(arg_addr,
						     pres_c_inline);
				break;
			default:
				panic("Tag not understood");
			}
			tag = va_arg(arg_addr, int);
		}
		break;
	case PRES_C_INLINE_TEMPORARY:
		if (!retval) {
			retval = pres_c_new_inline(kind);
			/*
			 * The following *can* be unspecified, but they can't
			 * be NULL.  Thus we set them to empty strings here.
			 */
			retval->pres_c_inline_u_u.temp.prehandler = "";
			retval->pres_c_inline_u_u.temp.posthandler = "";
			retval->pres_c_inline_u_u.temp.type
				= TEMP_TYPE_PRESENTED; /* by default */
		}
		while (tag != PRES_C_TAG_DONE) {
			pres_c_temporary *temp
				= &retval->pres_c_inline_u_u.temp;
			switch (tag) {
			case PIA_Name:
				temp->name = va_arg(arg_addr, char *);
				break;
			case PIA_CType:
				temp->ctype = va_arg(arg_addr, cast_type);
				break;
			case PIA_Value:
				temp->init = va_arg(arg_addr, cast_expr);
				break;
			case PIA_IsConst:
				temp->is_const = va_arg(arg_addr, int);
				break;
			case PIA_PreHandler:
				temp->prehandler = va_arg(arg_addr, char *);
				break;
			case PIA_PostHandler:
				temp->posthandler = va_arg(arg_addr, char *);
				break;
			case PIA_TempType:
				temp->type = va_arg(arg_addr,
						    pres_c_temporary_type);
				break;
			case PIA_Mapping:
				temp->map = va_arg(arg_addr, pres_c_mapping);
				break;
			default:
				panic("Tag not understood.");
				break;
			}
			tag = va_arg(arg_addr, int);
		}
		break;
	case PRES_C_INLINE_THROWAWAY:
	case PRES_C_INLINE_ILLEGAL:
		if( !retval )
			retval = pres_c_new_inline( kind );
		break;
	default:
		break;
	}
	va_end( arg_addr );
	return( retval );
}

/* End of File */

