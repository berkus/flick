/*
 * Copyright (c) 1998, 1999 The University of Utah and
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
#include <mom/c/libcast.h>

cast_def_t cast_new_def_t(cast_scoped_name name,
			  cast_storage_class sc,
			  cast_def_kind kind,
			  data_channel_index channel,
			  cast_def_protection prot)
{
	cast_def_t def = mustcalloc(sizeof(*def));
	memset(def, 0, sizeof(*def));
	def->name = name;
	def->sc = sc;
	def->u.kind = kind;
	def->channel = channel;
	def->protection = prot;
	return def;
}

