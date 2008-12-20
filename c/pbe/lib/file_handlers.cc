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

#include <assert.h>

#include <mom/c/libcast.h>
#include <mom/c/pbe.hh>
#include <mom/c/be/be_state.hh>
#include <mom/c/be/be_file.hh>

struct be_event *be_file_prologue_handler(struct be_handler *bh,
					  struct be_event *be)
{
	struct be_file *file = (struct be_file *)bh->get_parent();
	struct be_state *state = file->get_state();
	
	switch( be->id ) {
	case BEFE_INIT: {
		struct scml_scope *scope;
		
		if( (scope = state->get_scml_root()->
		     find_child(flick_asprintf("%s_file_defs",
					       file->get_name()))) ) {
			struct scml_context *sc;
			
			sc = new scml_context;
			sc->set_scope(scope);
			sc->exec_cmd("prologue", 0, NULL);
			delete sc;
		}
		break;
	}
	default:
		break;
	}
	return( be );
}

struct be_handler be_file_prologue("prologue", 128, be_file_prologue_handler);

struct be_event *be_file_header_defs_handler(struct be_handler *bh,
					     struct be_event *be)
{
	struct be_file *file = (struct be_file *)bh->get_parent();
	struct be_state *state = file->get_state();
	pres_c_1 *pres = state->get_pres();
	
	switch( be->id ) {
	case BEFE_BODY: {
		unsigned int i;
		
		w_header_includes(pres);
		for (i = 0; i < pres->cast.cast_scope_len; i++) {
			if( !(pres->meta_data.channels.channels_val
			      [pres->cast.cast_scope_val[i].channel].
			      flags & DATA_CHANNEL_SQUELCHED) ) {
				cast_w_def(&pres->cast.cast_scope_val[i], 0);
				w_printf("\n");
			}
		}
		break;
	}
	default:
		break;
	}
	return( be );
}

struct be_handler be_file_header_defs("header defs",
				      0,
				      be_file_header_defs_handler);

struct be_event *be_file_pre_stubs_handler(struct be_handler *bh,
				    struct be_event *be)
{
	struct be_file *file = (struct be_file *)bh->get_parent();
	struct be_state *state = file->get_state();
	
	switch( be->id ) {
	case BEFE_BODY:
		make_interface_graph(state->get_pres());
		cast_w_scope(&state->get_pres()->pres_cast, 0);
		do_main_output(state->get_pres());
		if( state->get_pres_collection() )
			state->get_pres_collection()->
				implement(state->get_pres());
		break;
	default:
		break;
	}
	return( be );
}

struct be_handler be_file_pre_stubs("pre stubs",
				    1,
				    be_file_pre_stubs_handler);

struct be_event *be_file_gen_stubs_handler(struct be_handler *bh,
					   struct be_event *be)
{
	struct be_file *file = (struct be_file *)bh->get_parent();
	struct be_state *state = file->get_state();
	pres_c_1 *pres = state->get_pres();
	struct mu_stub_info_node *msi;
	unsigned int i;
	
	if( be->id != BEFE_BODY )
		return be;
	
	/* Print the regular presented stubs out.  This is the first pass. */
	for (i = 0; i < pres->stubs.stubs_len; i++) {
		data_channel_index channel;
		
		switch (pres->stubs.stubs_val[i].kind) {
		case PRES_C_MARSHAL_STUB:
		case PRES_C_UNMARSHAL_STUB:
			/* We never output m/u stubs on the first pass. */
			channel = pres->stubs_cast.cast_scope_val[
				pres->stubs.stubs_val[i].
				pres_c_stub_u.mstub.c_func].channel;
			assert(pres->meta_data.channels.channels_val[channel].
			       flags & DATA_CHANNEL_SQUELCHED);
			continue;
			
		case PRES_C_CLIENT_STUB:
			channel = pres->stubs_cast.cast_scope_val[
				pres->stubs.stubs_val[i].
				pres_c_stub_u.cstub.c_func].channel;
			break;
			
		case PRES_C_CLIENT_SKEL:
			channel = pres->stubs_cast.cast_scope_val[
				pres->stubs.stubs_val[i].
				pres_c_stub_u.cskel.c_def].channel;
			break;
			
		case PRES_C_SERVER_SKEL:
			channel = pres->stubs_cast.cast_scope_val[
				pres->stubs.stubs_val[i].
				pres_c_stub_u.sskel.c_def].channel;
			break;
			
		case PRES_C_SEND_STUB:
			channel = pres->stubs_cast.cast_scope_val[
				pres->stubs.stubs_val[i].
				pres_c_stub_u.send_stub.c_func].channel;
			break;
			
		case PRES_C_RECV_STUB:
			channel = pres->stubs_cast.cast_scope_val[
				pres->stubs.stubs_val[i].
				pres_c_stub_u.recv_stub.c_func].channel;
			break;
			
		case PRES_C_MESSAGE_MARSHAL_STUB:
			channel = pres->stubs_cast.cast_scope_val[
				pres->stubs.stubs_val[i].
				pres_c_stub_u.mmstub.c_func].channel;
			break;
			
		case PRES_C_MESSAGE_UNMARSHAL_STUB:
			channel = pres->stubs_cast.cast_scope_val[
				pres->stubs.stubs_val[i].
				pres_c_stub_u.mustub.c_func].channel;
			break;
			
		case PRES_C_CONTINUE_STUB:
			channel = pres->stubs_cast.cast_scope_val[
				pres->stubs.stubs_val[i].
				pres_c_stub_u.continue_stub.c_func].channel;
			break;
			
		default:
			panic("Unknown PRES_C_STUB_KIND!");
			break;
		}
		
		if (!(pres->meta_data.channels.channels_val[channel].
		      flags & DATA_CHANNEL_SQUELCHED))
			w_stub(pres, i);
	}
	
	/*
	 * This is the second pass - we output the marshal/unmarshal stubs that
	 * we found were used while generating the regular presented stubs.
	 */
	if (!empty_list(state->get_mu_stub_list()))
		w_printf("\n"
			 "/* Mandatory marshaling/unmarshaling stubs. */\n\n");
	
	msi = (struct mu_stub_info_node *)state->get_mu_stub_list()->head;
	while( msi->link.succ ) {
		switch(msi->stub_kind) {
		case PRES_C_MARSHAL_STUB:
			w_marshal_stub(pres, *msi);
			break;
		case PRES_C_UNMARSHAL_STUB:
			w_unmarshal_stub(pres, *msi);
			break;
		default:
			warn(("Invalid stub kind %d found in set of used "
			       "marshaling/unmarshaling stubs."),
			      msi->stub_kind);
		}
		msi = (struct mu_stub_info_node *)msi->link.succ;
	}
	return( be );
}

struct be_handler be_file_gen_stubs("gen stubs", 0, be_file_gen_stubs_handler);

struct be_event *be_file_epilogue_handler(struct be_handler *bh,
					  struct be_event *be)
{
	struct be_file *file = (struct be_file *)bh->get_parent();
	struct be_state *state = file->get_state();
	
	switch( be->id ) {
	case BEFE_SHUTDOWN: {
		struct scml_scope *scope;
		
		if( (scope = state->get_scml_root()->
		     find_child(flick_asprintf("%s_file_defs",
					       file->get_name()))) ) {
			struct scml_context *sc;
			
			sc = new scml_context;
			sc->set_scope(scope);
			sc->exec_cmd("epilogue", 0, NULL);
			delete sc;
		}
		break;
	}
	default:
		break;
	}
	return( be );
}

struct be_handler be_file_epilogue("epilogue", -128, be_file_epilogue_handler);
