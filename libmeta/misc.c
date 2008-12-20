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

#include <mom/libmeta.h>

void init_meta(meta *m)
{
	m->files.files_len = 0;
	m->files.files_val = 0;
	m->channels.channels_len = 0;
	m->channels.channels_val = 0;
}

void check_meta(meta *m)
{
	unsigned int lpc;
	
	for( lpc = 0; lpc < m->files.files_len; lpc++ ) {
		meta_check_file(m, lpc);
	}
	for( lpc = 0; lpc < m->channels.channels_len; lpc++ ) {
		meta_check_channel(m, lpc);
	}
}

void print_meta(meta *m, FILE *file, int indent)
{
	unsigned int lpc;
	
	for( lpc = 0; lpc < m->files.files_len; lpc++ ) {
		meta_print_file(m, file, indent, lpc);
	}
	for( lpc = 0; lpc < m->channels.channels_len; lpc++ ) {
		meta_print_channel(m, file, indent, lpc);
	}
}
