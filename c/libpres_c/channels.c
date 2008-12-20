/*
 * Copyright (c) 1999 The University of Utah and
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

void pres_c_add_unpresented_channel(pres_c_1 *pres, data_channel_mask dcm)
{
	int i;
	
	i = pres->unpresented_channels.unpresented_channels_len++;
	pres->unpresented_channels.unpresented_channels_val =
		mustrealloc(pres->unpresented_channels.
			    unpresented_channels_val,
			    sizeof(data_channel_mask) *
			    pres->unpresented_channels.
			    unpresented_channels_len);
	pres->unpresented_channels.unpresented_channels_val[i] = dcm;
}
