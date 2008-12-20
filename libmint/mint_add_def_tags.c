/*
 * Copyright (c) 1998 The University of Utah and
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

#include "private.h"

/* Add a mint node with the specified attributes/values */
mint_ref mint_add_def_tags( mint_def_kind kind, mint_1 *mint, int tag, ... )
{
        va_list arg_addr;
	mint_ref retval = -1;
	
	va_start( arg_addr, tag );
	switch( kind ) {
	case MINT_VOID:
	case MINT_BOOLEAN:
	case MINT_SYSTEM_EXCEPTION:
	case MINT_ANY:
	case MINT_TYPE_TAG:
		retval = mint_add_def( mint );
		m(retval).kind = kind;
		break;
	case MINT_INTEGER: {
		int int_min = 0, int_range = 0;
		int changed = 0;
		
		/* Go through the list of tags until we hit the
		   end (MINT_TAG_DONE).  Use the attribute ID to
		   figure out what the user is setting and then
		   grab the value from the stack, repeat */
		while( tag != MINT_TAG_DONE ) {
			/* Pivot on the attribute ID */
			switch( tag ) {
			case MDA_Min:
				int_min = va_arg( arg_addr, int );
				break;
			case MDA_Range:
				int_range = va_arg( arg_addr, int );
				break;
			default:
				/* User passed an unsupported attribute
				   ID, alert them */
				panic( "Tag not understood" );
				break;
			}
			changed = 1;
			/* Proceed to the next attribute ID */
			tag = va_arg( arg_addr, int );
		}
		/* If there were no tags we use the default of the
		   signed23_ref, otherwise we add it with the proper
		   bounds */
		if( changed )
			retval = mint_add_integer_def( mint,
						       int_min,
						       int_range );
		else
			retval = mint->standard_refs.signed32_ref;
		break;
	}
	case MINT_SCALAR:
		retval = mint_add_def( mint );
		m(retval).kind = MINT_SCALAR;
		m(retval).mint_def_u.scalar_def.bits = 32;
		m(retval).mint_def_u.scalar_def.flags =
			MINT_SCALAR_FLAG_SIGNED;
		while( tag != MINT_TAG_DONE ) {
			switch( tag ) {
			case MDA_Bits:
				m(retval).mint_def_u.scalar_def.bits =
					va_arg( arg_addr, int );
				break;
			case MDA_Flags:
				m(retval).mint_def_u.scalar_def.flags =
					va_arg( arg_addr, int );
				break;
			default:
				panic( "Tag not understood" );
				break;
			}
			tag = va_arg( arg_addr, int );
		}
		break;
	case MINT_FLOAT:
		retval = mint_add_def( mint );
		m(retval).kind = MINT_FLOAT;
		m(retval).mint_def_u.float_def.bits = 32;
		while( tag != MINT_TAG_DONE ) {
			switch( tag ) {
			case MDA_Bits:
				m(retval).mint_def_u.float_def.bits =
					va_arg( arg_addr, int );
				break;
			default:
				panic( "Tag not understood" );
				break;
			}
			tag = va_arg( arg_addr, int );
		}
		break;
	case MINT_CHAR:
		retval = mint_add_def( mint );
		m(retval).kind = MINT_CHAR;
		m(retval).mint_def_u.char_def.bits = 8;
		m(retval).mint_def_u.char_def.flags = MINT_CHAR_FLAG_NONE;
		while( tag != MINT_TAG_DONE ) {
			switch( tag ) {
			case MDA_Bits:
				m(retval).mint_def_u.char_def.bits =
					va_arg( arg_addr, int );
				break;
			case MDA_Flags:
				m(retval).mint_def_u.char_def.flags =
					va_arg( arg_addr, int );
				break;
			default:
				panic( "Tag not understood" );
				break;
			}
			tag = va_arg( arg_addr, int );
		}
		break;
	case MINT_ARRAY:
		retval = mint_add_def( mint );
		m(retval).kind = MINT_ARRAY;
		m(retval).mint_def_u.array_def.element_type =
			mint->standard_refs.signed32_ref;
		m(retval).mint_def_u.array_def.length_type =
			mint->standard_refs.unsigned32_ref;
		while( tag != MINT_TAG_DONE ) {
			switch( tag ) {
			case MDA_ElementType:
				m(retval).mint_def_u.array_def.element_type =
					va_arg( arg_addr, mint_ref );
				break;
			case MDA_LengthType:
				m(retval).mint_def_u.array_def.length_type =
					va_arg( arg_addr, mint_ref );
				break;
			default:
				panic( "Tag not understood" );
				break;
			}
			tag = va_arg( arg_addr, int );
		}
		break;
	case MINT_STRUCT:
		retval = mint_add_def( mint );
		m(retval).kind = MINT_STRUCT;
		m(retval).mint_def_u.struct_def.slots.slots_len = 0;
		m(retval).mint_def_u.struct_def.slots.slots_val = 0;
		while( tag != MINT_TAG_DONE ) {
			switch( tag ) {
			case MDA_Slot:
			{
				mint_ref my_slot;
				
				my_slot = mint_add_struct_slot( mint, retval );
				m(retval).mint_def_u.struct_def.slots.
					slots_val[my_slot] =
					va_arg( arg_addr, mint_ref );
			}
			break;
			default:
				panic( "Tag not understood" );
				break;
			}
			tag = va_arg( arg_addr, int );
		}
		break;
	case MINT_UNION:
		retval = mint_add_def( mint );
		m(retval).kind = MINT_UNION;
		m(retval).mint_def_u.union_def.discrim = mint->standard_refs.
							 signed32_ref;
		m(retval).mint_def_u.union_def.dfault = -1;
		m(retval).mint_def_u.union_def.cases.cases_len = 0;
		m(retval).mint_def_u.union_def.cases.cases_val = 0;
		while( tag != MINT_TAG_DONE ) {
			switch( tag ) {
			case MDA_Discrim:
				m(retval).mint_def_u.union_def.discrim =
					va_arg( arg_addr, mint_ref );
				break;
			case MDA_Default:
				m(retval).mint_def_u.union_def.dfault =
					va_arg( arg_addr, mint_ref );
				break;
			case MDA_Case:
			{
				mint_ref my_case;
				
				my_case = mint_add_union_case( mint, retval );
				m(retval).mint_def_u.union_def.cases.
					cases_val[my_case].val =
					va_arg( arg_addr, mint_const );
				m(retval).mint_def_u.union_def.cases.
					cases_val[my_case].var =
					va_arg( arg_addr, mint_ref );
			}
			break;
			default:
				panic( "Tag not understood" );
				break;
			}
			tag = va_arg( arg_addr, int );
		}
		break;
	case MINT_INTERFACE:
		retval = mint_add_def( mint );
		m(retval).kind = kind;
		m(retval).mint_def_u.interface_def.right = MINT_INTERFACE_NAME;
		while( tag != MINT_TAG_DONE ) {
			switch( tag ) {
			case MDA_Right:
				m(retval).mint_def_u.interface_def.right =
					va_arg( arg_addr,
						mint_interface_right );
				break;
			break;
			default:
				panic( "Tag not understood" );
				break;
			}
			tag = va_arg( arg_addr, int );
		}
		break;
	case MINT_TYPED:
		retval = mint_add_def( mint );
		m(retval).kind = kind;
		m(retval).mint_def_u.typed_def.tag = -1;
		m(retval).mint_def_u.typed_def.ref = -1;
		while( tag != MINT_TAG_DONE ) {
			switch( tag ) {
			case MDA_Tag:
				m(retval).mint_def_u.typed_def.tag =
					va_arg( arg_addr, mint_ref );
				break;
			case MDA_Ref:
				m(retval).mint_def_u.typed_def.ref =
					va_arg( arg_addr, mint_ref );
				break;
			break;
			default:
				panic( "Tag not understood" );
				break;
			}
			tag = va_arg( arg_addr, int );
		}
		break;
	default:
		break;
	}
	va_end( arg_addr );
	return( retval );
}

/* End of File */

