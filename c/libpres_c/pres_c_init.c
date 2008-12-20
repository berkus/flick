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
#include <mom/c/libpres_c.h>
#include <mom/c/libcast.h>

void pres_c_1_init(pres_c_1 *pres)
{
	pres->mint.defs.defs_len = 0;
	pres->mint.defs.defs_val = 0;
	pres->cast = cast_new_scope(0);
	pres->stubs_cast = cast_new_scope(0);
	pres->pres_cast = cast_new_scope(0);
	pres->a.defs.defs_len = 0;
	pres->a.defs.defs_val = 0;
	init_meta(&pres->a.meta_data);
	pres->stubs.stubs_len = 0;
	pres->stubs.stubs_val = 0;
	pres->pres_context = 0;
	pres->error_mappings.error_mappings_len = 0;
	pres->error_mappings.error_mappings_val = 0;
	pres->unpresented_channels.unpresented_channels_len = 0;
	pres->unpresented_channels.unpresented_channels_val = 0;
	pres->pres_attrs = create_tag_list(0);
	init_meta(&pres->meta_data);
	pres->cast_language = CAST_C;
}
