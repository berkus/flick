/*
 * Copyright (c) 1995, 1996 The University of Utah and
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

#include <stdio.h>
#include <mom/compiler.h>
#include <mom/c/libpres_c.h>

void pres_c_1_writefh(pres_c_1 *pres, FILE *fh)
{
	XDR xdrs;

	pres_c_1_check(pres);

	xdrstdio_create(&xdrs, fh, XDR_ENCODE);
	if (!xdr_pres_c_1(&xdrs, pres))
		panic("error encoding PRES_C output file");
	xdr_destroy(&xdrs);
}

