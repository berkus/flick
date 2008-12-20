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

#include <assert.h>

#include <mom/libaoi.h>

void aoi_get_int_size(aoi_integer *i, int *out_bits, int *out_is_signed)
{
	int bits, is_signed;

	if (i->min >= 0)
	{
		is_signed = 0;
		if (i->min + i->range <= 2)
			bits = 1;
		else if (i->min + i->range <= 255)
			bits = 8;
		else if (i->min + i->range <= 65536)
			bits = 16;
		else
			bits = 32;
	}
	else
	{
		is_signed = 1;
		if ((i->min >= -128) && (i->min + i->range <= 127))
			bits = 8;
		if ((i->min >= -32768) && (i->min + i->range <= 32767))
			bits = 16;
		else
			bits = 32;
	}

	*out_bits = bits;
	*out_is_signed = is_signed;
}

