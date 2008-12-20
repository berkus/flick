/*
 * Copyright (c) 1997, 1998, 1999 The University of Utah and
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

#include <mom/libmint.h>

#include "private.h"

void mint_add_standard_defs(mint_1 *mint)
{
	mint_standard_refs *refs = &(mint->standard_refs);
	
	/* Create the void type */
	refs->void_ref = mint_add_def(mint);
	m(refs->void_ref).kind = MINT_VOID;
	
	/* Create the basic integer types. */
	refs->bool_ref       = mint_add_integer_def(mint, 0, 1);
	refs->signed8_ref    = mint_add_integer_def(mint, -128, 255U);
	refs->signed16_ref   = mint_add_integer_def(mint, -32768, 65535U);
	refs->signed32_ref   = mint_add_integer_def(mint,
						    -2147483647-1,
						    4294967295U);
	refs->signed64_ref   = mint_add_def(mint);
	m(refs->signed64_ref).kind = MINT_SCALAR;
	m(refs->signed64_ref).mint_def_u.scalar_def.bits = 64;
	m(refs->signed64_ref).mint_def_u.scalar_def.flags
		= MINT_SCALAR_FLAG_SIGNED;
	
        refs->unsigned8_ref  = mint_add_integer_def(mint, 0, 255);
        refs->unsigned16_ref = mint_add_integer_def(mint, 0, 65535U);
	refs->unsigned32_ref = mint_add_integer_def(mint, 0, 4294967295U);
	refs->unsigned64_ref   = mint_add_def(mint);
	m(refs->unsigned64_ref).kind = MINT_SCALAR;
	m(refs->unsigned64_ref).mint_def_u.scalar_def.bits = 64;
	m(refs->unsigned64_ref).mint_def_u.scalar_def.flags
		= MINT_SCALAR_FLAG_UNSIGNED;
	
	/* Create the basic character types. */
        refs->char8_ref = mint_add_def(mint);
        m(refs->char8_ref).kind = MINT_CHAR;
        m(refs->char8_ref).mint_def_u.char_def.bits = 8;
        m(refs->char8_ref).mint_def_u.char_def.flags = MINT_CHAR_FLAG_NONE;
	
	/* Create the basic float types. */
	refs->float32_ref = mint_add_def(mint);
	m(refs->float32_ref).kind = MINT_FLOAT;
	m(refs->float32_ref).mint_def_u.float_def.bits = 32;
	refs->float64_ref = mint_add_def(mint);
	m(refs->float64_ref).kind = MINT_FLOAT;
	m(refs->float64_ref).mint_def_u.float_def.bits = 64;
	
	/* Create the interface types. */
	refs->interface_name_ref = mint_add_def(mint);
	m(refs->interface_name_ref).kind = MINT_INTERFACE;
	m(refs->interface_name_ref).mint_def_u.interface_def.right
		= MINT_INTERFACE_NAME;
	
	refs->interface_invoke_ref = mint_add_def(mint);
	m(refs->interface_invoke_ref).kind = MINT_INTERFACE;
	m(refs->interface_invoke_ref).mint_def_u.interface_def.right
		= MINT_INTERFACE_INVOKE;
	
	refs->interface_invoke_once_ref = mint_add_def(mint);
	m(refs->interface_invoke_once_ref).kind = MINT_INTERFACE;
	m(refs->interface_invoke_once_ref).mint_def_u.interface_def.right
		= MINT_INTERFACE_INVOKE_ONCE;
	
	refs->interface_service_ref = mint_add_def(mint);
	m(refs->interface_service_ref).kind = MINT_INTERFACE;
	m(refs->interface_service_ref).mint_def_u.interface_def.right
		= MINT_INTERFACE_SERVICE;
	
	/* Create the system exception types. */
	refs->system_exception_ref = mint_add_def(mint);
	m(refs->system_exception_ref).kind = MINT_SYSTEM_EXCEPTION;
}

/* End of file. */

