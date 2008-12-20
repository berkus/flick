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

#include <mom/c/pbe.hh>

void preprocess_pres(pres_c_1 *pres)
{
	data_channel_mask all_channels_mask;
	io_file_mask system_mask;
	io_file_index orb_idl;
	unsigned int lpc;
	
	orb_idl = meta_find_file(&pres->meta_data, "orb.idl", 0, 0);
	if( orb_idl != -1 )
		pres->meta_data.files.files_val[orb_idl].flags |=
			IO_FILE_SYSTEM;
	all_channels_mask = meta_make_channel_mask(CMA_TAG_DONE);
	system_mask = meta_make_file_mask(FMA_SetFlags, IO_FILE_SYSTEM,
					  FMA_TAG_DONE);
	meta_squelch_files(&pres->meta_data,
			   &system_mask,
			   &all_channels_mask);
	for( lpc = 0;
	     lpc < pres->unpresented_channels.unpresented_channels_len;
	     lpc++ ) {
		meta_squelch_channels(&pres->meta_data,
				      pres->unpresented_channels.
				      unpresented_channels_val[lpc]);
	}
}
