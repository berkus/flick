/*
 * Copyright (c) 1995, 1996, 1997, 1998, 1999 The University of Utah and
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
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#include <mom/libmeta.h>
#include <mom/libaoi.h>
#include <mom/libmint.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include "private.hh"

/* global variables */
char *progname;

int main(int argc, char **argv) {
	pg_state *pg;
	
        /* Grab the appropriate generator (this needs to be linked in) */
	pg = getGenerator();
	
	progname = argv[0];
	return pg->main(argc, argv);
}

extern mint_1 out_mint;
extern mint_ref mom_msg;
extern mint_ref *aoi_to_mint_association;
extern aoi in_aoi;

int pg_state::main(int argc, char **argv)
{
	FILE *input_file;
	FILE *output_file;
	FILE **files = new FILE*[2];
	files = cmdline(argc,argv);
	input_file = files[0];
	output_file = files[1];
	pres_c_1 pres;
	unsigned int lpc;
	
	pres_c_1_init(&pres);
	root_scope = &pres.cast;
	push_ptr(scope_stack, &pres.cast);
	add_tag(pres.pres_attrs, "pres_type", TAG_TAG_LIST_ARRAY, 0);
	out_pres = &pres;
	
	/* Read AOI. */
	aoi_readfh(&::in_aoi, input_file);
	in_aoi = &::in_aoi;
	
	out_pres->a = *in_aoi;
	out_pres->pres_context = calc_name("%g", ""); /* Name of PG style. */
	out_pres->meta_data = in_aoi->meta_data;
	init_meta(&out_pres->a.meta_data);
	builtin_file = meta_find_file(&out_pres->meta_data, "(builtin)", 0, 1);
	assert(builtin_file != -1);
	for( lpc = 0; lpc < PG_CHANNEL_MAX; lpc++ ) {
		pg_channel_maps[lpc] = (data_channel_index *)
			mustmalloc(sizeof(data_channel_index) *
				   out_pres->meta_data.files.files_len);
	}
	for( lpc = 0; lpc < out_pres->meta_data.files.files_len; lpc++ ) {
		map_file_channels(lpc);
	}
	
	/* Allocate memory for aoi_to_mint_association array.
	   It will have aoi->defs.defs_len elements.  */
	if (in_aoi->defs.defs_len > 0)
	{
		::aoi_to_mint_association = (mint_ref *)mustcalloc(
		 sizeof(mint_ref)*in_aoi->defs.defs_len);
		aoi_to_mint_association = ::aoi_to_mint_association;
	}
	
	/*
	 * Build a MINT from the AOI.  `translate_aoi_to_mint' writes to the
	 * `out_mint' and `mom_msg' global variables.  We must initialize
	 * `out_mint' ourselves; it is difficult for the AOI library to call
	 * the MINT library.
	 */
	out_mint.defs.defs_len = 0;
	out_mint.defs.defs_val = 0;
	mint_add_standard_defs(&out_mint);
	
	make_prim_collections();
	
	preprocess();
	
	translate_aoi_to_mint();
	
	top_union = mom_msg;
	
	/* Put the MINT in the PRES_C.  */
	out_pres->mint = out_mint;
	
	/* Generate the error mappings for this presentation */
	out_pres->error_mappings.error_mappings_len = 0;
	out_pres->error_mappings.error_mappings_val = 0;
	gen_error_mappings();
	
	/* Add generic presentation for everything still needed.  */
	gen();

	out_pres->cast_language = cast_language;
	/* Write the interface+presentation definition.  */
	
	/* Write PRES_C. */
	pres_c_1_writefh(out_pres, output_file);
	
	return 0;
}

/* End of file. */

