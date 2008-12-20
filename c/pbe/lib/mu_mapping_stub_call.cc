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

#include <assert.h>

#include <mom/compiler.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>
#include <mom/c/pbe.hh>

cast_scoped_name mu_state::mu_mapping_stub_call_name(int stub_idx)
{
	/* Get the original scoped name and make a copy of it. */
	int cidx = pres->stubs.stubs_val[stub_idx].pres_c_stub_u.mstub.c_func;
	cast_scoped_name base = pres->stubs_cast.cast_scope_val[cidx].name;
	cast_scoped_name stub_name = cast_copy_scoped_name(&base);

	/*
	 * Convert the filename into a valid C identifier, so we can include
	 * it in the m/u stub name to make it more unique.
	 */
	io_file_index root_file = meta_find_file(&pres->meta_data, 0,
						 IO_FILE_ROOT, 1);
	assert(root_file != -1);
	const char *rname
		= file_part(pres->meta_data.files.files_val[root_file].id);
	
	/* Get the channel id (name) the stub is associated with. */
	data_channel_index channel
		= pres->stubs_cast.cast_scope_val[cidx].channel;
	char *chname = pres->meta_data.channels.channels_val[channel].id;
	
	/* Generate the specialized m/u stub name. */
	unsigned int last = base.cast_scoped_name_len - 1;
	stub_name.cast_scoped_name_val[last].name
		= flick_asprintf("%s_%s_%s_%s",
				 base.cast_scoped_name_val[last].name,
				 rname,
				 chname,
				 pres_c_dir_name(current_param_dir));
	filename_to_c_id(stub_name.cast_scoped_name_val[last].name);
	
	return stub_name;
}

/*
 * This routine is used to generate a runtime function call to a separate
 * marshal/unmarshal stub, if it is discovered that a PRES_C_MAPPING_STUB node
 * cannot or should not be inlined.  (See `mu_state::mu_mapping_stub'.)
 *
 * XXX --- This mechanism needs a lot more working out.
 */

void mu_state::mu_mapping_stub_call(cast_expr expr, cast_type ctype,
				    mint_ref itype, pres_c_mapping map)
{
	cast_scoped_name stub_name;
	int idx;
	pres_c_stub_kind kind = ((op & MUST_ENCODE) ?
				 PRES_C_MARSHAL_STUB : PRES_C_UNMARSHAL_STUB);
	/*****/
	
	idx = pres_c_find_mu_stub(pres, itype, ctype, map, kind);
	
	if (idx < 0)
		panic("In `mu_state::mu_mapping_stub_call', "
		      "can't find marshal stub.");
	
	stub_name = mu_mapping_stub_call_name(idx);
	
	cast_expr cex = cast_new_expr_call_2(
		cast_new_expr_scoped_name(stub_name),
		cast_new_expr_name(get_mu_stream_name()),
		cast_new_unary_expr(CAST_UNARY_ADDR, expr));
	
	/* The m/u stubs now return whether there was an error
	   or not so we need to do a check. */
	add_stmt(cast_new_if(cex,
			     cast_new_goto(abort_block->use_current_label()),
			     0));
	
	/* Remember that we used this stub, and mark it to be output. */
	mu_stub_info_node *msi;
	msi = (struct mu_stub_info_node *)state->get_mu_stub_list()->head;
	while( msi->link.succ ) {
		if ((msi->stub_idx == idx)
		    && (msi->stub_dir == current_param_dir)
		    && (msi->stub_kind == kind))
			/* Already marked. */
			return;
		msi = (struct mu_stub_info_node *)msi->link.succ;
	}
	/* Didn't find one already marked, so make one. */
	msi = (mu_stub_info_node *) mustcalloc(sizeof(mu_stub_info_node));
	msi->stub_dir = current_param_dir;
	msi->stub_idx = idx;
	msi->stub_kind = kind;
	add_tail(state->get_mu_stub_list(), &msi->link);
}

/* End of file. */

