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

#include "private.hh"

void pg_state::p_type(aoi_type at,
		      p_type_collection **out_ptc)
{
	switch (at->kind) {
	case AOI_FWD_INTRFC:
		p_forward_type(out_ptc);
		break;
	case AOI_INTERFACE:
		p_interface_type(&(at->aoi_type_u_u.interface_def),
				 out_ptc);
		break;
	case AOI_INDIRECT:
		p_indirect_type(at->aoi_type_u_u.indirect_ref,
				out_ptc);
		break;
	case AOI_INTEGER:
		p_integer_type(&(at->aoi_type_u_u.integer_def),
			       out_ptc);
		break;
	case AOI_SCALAR:
		p_scalar_type(&(at->aoi_type_u_u.scalar_def),
			      out_ptc);
		break;
	case AOI_FLOAT:
		p_float_type(&(at->aoi_type_u_u.float_def),
			     out_ptc);
		break;
	case AOI_CHAR:
		p_char_type(&(at->aoi_type_u_u.char_def),
			    out_ptc);
		break;
	case AOI_STRUCT:
		p_struct_type(&(at->aoi_type_u_u.struct_def),
			      out_ptc);
		break;
	case AOI_UNION:
		p_union_type(&(at->aoi_type_u_u.union_def),
			     out_ptc);
		break;
	case AOI_ARRAY:
		p_array_type(&(at->aoi_type_u_u.array_def),
			     out_ptc);
		break;
	case AOI_ENUM:
		p_enum_type(&(at->aoi_type_u_u.enum_def),
			    out_ptc);
		break;
	case AOI_EXCEPTION:
		p_except_type(&(at->aoi_type_u_u.exception_def),
			      out_ptc);
		break;
	case AOI_CONST:
	case AOI_VOID:
		/* XXX AOI_CONST and AOI_ENUM, to be verified */
		p_void_type(out_ptc);
		break;
	case AOI_OPTIONAL:
		p_optional_type(&(at->aoi_type_u_u.optional_def),
				out_ptc);
		break;
	case AOI_ANY:
		p_any_type(out_ptc);
		break;
	case AOI_TYPE_TAG:
		p_type_tag_type(out_ptc);
		break;
	case AOI_TYPED:
		p_typed_type(&(at->aoi_type_u_u.typed_def),
			     out_ptc);
		break;
	default:
		panic("unknown AOI type kind %d in p_type", at->kind);
		break;
	}
}

/* End of file. */

