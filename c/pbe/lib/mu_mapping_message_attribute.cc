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

#include <mom/compiler.h>
#include <mom/c/pbe.hh>

void mu_state::mu_mapping_message_attribute(
	cast_expr /*expr*/,
	cast_type /*ctype*/,
	mint_ref /*itype*/,
	pres_c_mapping_message_attribute *attr_map)
{
	static int warned_flags = 0;
	static int warned_timeout = 0;
	static int warned_sequence = 0;
	static int warned_client = 0;
	static int warned_scopy = 0;
	const char *msg = 0;
	
	switch (attr_map->kind) {
	case PRES_C_MESSAGE_ATTRIBUTE_FLAGS:
		if (!warned_flags++)
			msg = "message flags";
		break;
		
	case PRES_C_MESSAGE_ATTRIBUTE_TIMEOUT:
		if (!warned_timeout++)
			msg = "message timeouts";
		break;
		
	case PRES_C_MESSAGE_ATTRIBUTE_SEQUENCE_RECEIVED:
		if (!warned_sequence++)
			msg = "message sequencing";
		break;
		
	case PRES_C_MESSAGE_ATTRIBUTE_CLIENT_REFERENCE:
		if (!warned_client++)
			msg = "client references";
		break;
		
	case PRES_C_MESSAGE_ATTRIBUTE_SERVERCOPY:
		if (!warned_scopy++)
			msg = "server copy information";
		break;
		
	default:
		panic("In `mu_state::mu_mapping_message_attribute', "
		      "unknown attribute %d.",
		      attr_map->kind);
		break;
	}
	
	if (msg)
		warn("This back end doesn't support %s.", msg);
}

/* End of file. */

