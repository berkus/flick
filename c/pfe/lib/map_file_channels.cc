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

#include <string.h>

#include <mom/compiler.h>
#include <mom/libmeta.h>
#include <mom/c/pfe.hh>
#include <mom/c/libcast.h>

const char *pg_state::pg_channel_names[] = {
	"client",
	"client",
	"server",
	"server"
};

void pg_state::map_file_channels(io_file_index file)
{
	data_channel_index dc;
	int lpc;
	
	for( lpc = 0; lpc < PG_CHANNEL_MAX; lpc++ ) {
		dc = meta_add_channel(&out_pres->meta_data, file,
				      pg_channel_names[lpc]);
		if( !(lpc % 2) )
			out_pres->meta_data.channels.channels_val[dc].flags |=
				DATA_CHANNEL_DECL;
		else
			out_pres->meta_data.channels.channels_val[dc].flags |=
				DATA_CHANNEL_IMPL;
		pg_channel_maps[lpc][file] = dc;
	}
}
