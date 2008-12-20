/*
 * Copyright (c) 1996, 1997, 1998 The University of Utah and
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

#include <mom/libaoi.h>
#include <mom/libmint.h>
#include <mom/compiler.h>

mint_const build_op_const(aoi_interface *this_interface,
			  char *this_interface_name,
			  aoi_interface *derived_interface,
			  char *derived_interface_name,
			  aoi_operation *op,
			  int type_of_constant)
{
	/*
	 * XXX --- This implementation is a stopgap.  Really, we should change
	 * this method to call the new `calc_exception_code_name' function.
	 */
	int ilen = strlen(this_interface_name);
	/* WAS: int ilen = strlen(derived_interface_name); */
	int olen = strlen(op->name);
	char *tmp = (char *)mustmalloc(ilen + olen + 10);
	int reply = (type_of_constant == MAKE_OP_REPLY);
	int offset = 3 + reply;
	
	tmp[0] = 'r';
	tmp[1] = 'e';
	tmp[2] = reply ? 'p' : 'q';
	if (reply)
		tmp[3] = 'l';
	tmp[offset] = '_';
	
	strcpy(&tmp[++offset], this_interface_name);
	/* WAS: strcpy(&tmp[++offset], derived_interface_name); */
	offset += ilen;
	tmp[offset] = '_';
	strcpy(&tmp[++offset], op->name);
	
	return mint_new_symbolic_const(MINT_CONST_INT, tmp);
}

/* End of file. */

