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

#ifndef _mom_c_be_flags_hh_
#define _mom_c_be_flags_hh_

#include <mom/compiler.h>

struct be_flags {
	const char *input;
	const char *output;
	const char *header;
	const char *inline_file;
	const char *prefix;
	int system_header;
	int no_timestamp;
	int no_included_implementations;
	int no_included_declarations;
	int all_mu_stubs;
	const char **pres_impl_dirs;
	const char *pres_impl;
	flag_value_seq src_includes;
	flag_value_seq hdr_includes;
	flag_value_seq scml_defs;
};

#endif
