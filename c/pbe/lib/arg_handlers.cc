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

#include <mom/compiler.h>

#include <mom/c/libcast.h>
#include <mom/c/scml.hh>
#include <mom/c/pbe.hh>

/*
 * This file contains most of the code for handling command line arguments.
 *
 * Each argument that requires some action to be taken has a handler function
 * followed by a be_handler structure for holding the handler and any
 * peripheral information.  The handlers are ordered based on priorities
 * so any arguments that may have dependencies on the others will have a
 * lower priority.  Note:  Priorities kind of suck, but its the best thing
 * short of explicitly declaring the dependencies...
 */

struct be_event *be_arg_scml_defs_handler(struct be_handler */*bh*/,
					  struct be_event *be)
{
	struct be_state *state = ((struct be_state_event *)be)->state;
	
	if( be->id != BESE_CLI_ARGS )
		return be;
	scml_parse_cmdline_defines(state->get_scml_root(),
				   &state->get_cli_args()->scml_defs);
	return( be );
}

struct be_handler be_arg_scml_defs("-D defs", 0, be_arg_scml_defs_handler);

struct be_event *be_arg_includes_handler(struct be_handler */*bh*/,
					 struct be_event *be)
{
	struct be_state *state = ((struct be_state_event *)be)->state;
	be_flags *res = state->get_cli_args();
	union tag_data_u data;
	unsigned int lpc;
	tag_item *ti;
	tag_list *tl;
	
	if( be->id != BESE_CLI_ARGS )
		return be;
	ti = find_tag(state->get_scml_root()->get_values(), "system_info");
	tl = find_tag(ti->data.tag_data_u.tl, "flags")->data.tag_data_u.tl;
	ti = find_tag(tl, "source_include_file");
	for( lpc = 0; lpc < res->src_includes.len; lpc++ ) {
		if( res->src_includes.values[lpc].string[0] != '<' ||
		    res->src_includes.values[lpc].string[0] != '\"' ) {
			res->src_includes.values[lpc].string =
				flick_asprintf("\"%s\"",
					       res->src_includes.
					       values[lpc].string);
			data.str = res->src_includes.values[lpc].string;
			set_tag_data(&ti->data, lpc, data);
		}
	}
	ti = find_tag(tl, "header_include_file");
	for( lpc = 0; lpc < res->hdr_includes.len; lpc++ ) {
		if( res->hdr_includes.values[lpc].string[0] != '<' ||
		    res->hdr_includes.values[lpc].string[0] != '\"' ) {
			res->hdr_includes.values[lpc].string =
				flick_asprintf("\"%s\"",
					       res->hdr_includes.
					       values[lpc].string);
			data.str = res->hdr_includes.values[lpc].string;
			set_tag_data(&ti->data, lpc, data);
		}
	}
	return( be );
}

struct be_handler be_arg_includes("includes", 0, be_arg_includes_handler);

static const char *stubs_file_suffix()
{
	const char *suffix;
	
	switch (cast_language) {
	default:
		warn("Unrecognized `cast_language'; "
		     "will use `.c' as the stub file suffix.");
		/* Fallthrough. */
	case CAST_C:
		suffix = ".c";
		break;
	case CAST_CXX:
		suffix = ".cc";
		break;
	}
	
	return suffix;
}

static const char *header_file_suffix()
{
	const char *suffix;
	
	switch (cast_language) {
	default:
		warn("Unrecognized `cast_language'; "
		     "will use `.h' as the header file suffix.");
		/* Fallthrough. */
	case CAST_C:
		suffix = ".h";
		break;
	case CAST_CXX:
		suffix = ".h";
		break;
	}
	
	return suffix;
}

struct be_event *be_arg_input_file_handler(struct be_handler */*bh*/,
					   struct be_event *be)
{
	struct be_state *state = ((struct be_state_event *)be)->state;
	be_flags *res = state->get_cli_args();
	data_channel_mask all_channels_mask;
	io_file_index builtin_file;
	io_file_mask builtin_mask;
	struct be_file *bef;
	
	if( be->id != BESE_CLI_ARGS )
		return be;
	bef = new be_file;
	bef->set_state(state);
	bef->set_name("input");
	bef->set_flags(bef->get_flags() | BEFF_INPUT);
	if( res->input ) {
		bef->set_path(res->input);
		bef->open();
	} else {
		bef->set_path("<stdin>");
		bef->set_file(stdin);
	}
	if( !bef->get_file() )
		panic("Unable to open input file `%s'.", bef->get_path());
	bef->export_to_scml();
	state->add_file(bef);
	
	/* Read our input data. */
	pres_c_1_readfh(state->get_pres(), bef->get_file());
	if( cast_language != state->get_pres()->cast_language )
		panic("Incompatible CAST languages\n");
	builtin_mask = meta_make_file_mask(FMA_SetFlags, IO_FILE_BUILTIN,
					   FMA_TAG_DONE);
	all_channels_mask = meta_make_channel_mask(CMA_TAG_DONE);
	meta_squelch_files(&state->get_pres()->meta_data,
			   &builtin_mask,
			   &all_channels_mask);
	builtin_file = meta_find_file(&state->get_pres()->meta_data, 0,
				      IO_FILE_BUILTIN, 1);
	assert(builtin_file != -1);
	state->set_implied_channel(meta_add_channel(&state->get_pres()->
						    meta_data,
						    builtin_file,
						    "(implied)"));
	return( be );
}

struct be_handler be_arg_input_file("input file",
				    0,
				    be_arg_input_file_handler);

struct be_event *be_arg_stubs_file_handler(struct be_handler */*bh*/,
					   struct be_event *be)
{
	struct be_state *state = ((struct be_state_event *)be)->state;
	be_flags *res = state->get_cli_args();
	struct be_file *bef;
	
	if( be->id != BESE_CLI_ARGS )
		return be;
	/* Determine the names of our output files. */
	if (res->input && !res->output)
		res->output = resuffix(res->input, stubs_file_suffix());
	
	bef = new be_file;
	bef->set_state(state);
	bef->set_name("stubs");
	bef->set_flags(bef->get_flags() | BEFF_OUTPUT);
	if( res->output ) {
		bef->set_path(res->output);
		bef->open();
	} else {
		bef->set_path("<stdout>");
	}
	if( !bef->get_file() )
		panic("Unable to open output file `%s'.", bef->get_path());
	bef->export_to_scml();
	bef->add_handler(new be_handler(be_file_prologue));
	bef->add_handler(new be_handler(be_file_pre_stubs));
	bef->add_handler(new be_handler(be_file_gen_stubs));
	bef->add_handler(new be_handler(be_file_epilogue));
	state->add_file(bef);
	return( be );
}

struct be_handler be_arg_stubs_file("stubs file",
				    0,
				    be_arg_stubs_file_handler);

struct be_event *be_arg_header_file_handler(struct be_handler */*bh*/,
					    struct be_event *be)
{
	struct be_state *state = ((struct be_state_event *)be)->state;
	be_flags *res = state->get_cli_args();
	struct be_file *bef;
	
	if( be->id != BESE_CLI_ARGS )
		return be;
	if (res->output && !res->header)
		res->header = resuffix(res->output, header_file_suffix());
	if (res->input && !res->header)
		res->header = resuffix(res->input, header_file_suffix());
	
	bef = new be_file;
	bef->set_state(state);
	bef->set_name("header");
	bef->set_flags(bef->get_flags() | BEFF_OUTPUT);
	if( res->header ) {
		bef->set_path(res->header);
	} else {
		bef->set_path("/dev/null");
	}
	if( !bef->open() )
		panic("Unable to open header file `%s'.", bef->get_path());
	bef->export_to_scml();
	bef->add_handler(new be_handler(be_file_prologue));
	bef->add_handler(new be_handler(be_file_header_defs));
	bef->add_handler(new be_handler(be_file_epilogue));
	state->add_file(bef);
	return( be );
}

struct be_handler be_arg_header_file("header file",
				     0,
				     be_arg_header_file_handler);

struct be_event *be_arg_inline_file_handler(struct be_handler */*bh*/,
					    struct be_event *be)
{
	struct be_state *state = ((struct be_state_event *)be)->state;
	be_flags *res = state->get_cli_args();
	struct be_file *bef;
	
	if( be->id != BESE_CLI_ARGS )
		return be;
	if( res->inline_file ) {
		bef = new be_file;
		bef->set_state(state);
		bef->set_name("inline");
		bef->set_flags(bef->get_flags() | BEFF_OUTPUT);
		bef->set_path(res->inline_file);
		if( !bef->open() )
			panic("Unable to open inline file `%s'.",
			      bef->get_path());
		bef->export_to_scml();
		bef->add_handler(new be_handler(be_file_prologue));
		bef->add_handler(new be_handler(be_file_epilogue));
		state->add_file(bef);
	}
	return( be );
}

struct be_handler be_arg_inline_file("inline file",
				     0,
				     be_arg_inline_file_handler);

struct be_event *be_arg_pres_impl_handler(struct be_handler */*bh*/,
					  struct be_event *be)
{
	struct be_state *state = ((struct be_state_event *)be)->state;
	be_flags *res = state->get_cli_args();
	struct scml_stream_pos *ssp;
	
	if( be->id != BESE_CLI_ARGS )
		return be;
	if( res->pres_impl ) {
		tag_item *ti;
		
		ssp = scml_execute_defs_file(state->get_scml_root(),
					     res->pres_impl_dirs,
					     res->pres_impl,
					     res->pres_impl);
		if( !ssp ) {
			panic("Couldn't open presentation "
			      "implementation file %s",
			      res->pres_impl);
		}
		state->set_scml_defs_stream(ssp);
		if( (ti = find_tag(state->get_scml_root()->get_values(),
				   "presentation_implementation_name")) ) {
			struct be_pres_impl *bpi;
			char *impl_name;
			
			if( !(impl_name = scml_string::tag_string(ti)) )
				panic("Couldn't get string from "
				      "presentation_implementation_name tag");
			if( !(bpi = state->find_pres_impl(impl_name)) )
				panic("Couldn't match "
				      "presentation_implementation_name '%s'"
				      " to any known implementation",
				      impl_name);
			state->set_pres_collection(bpi->handler(state));
		}
	}
	return( be );
}

struct be_handler be_arg_pres_impl("pres_impl",
				   REQUIRE_HANDLER(be_arg_scml_defs),
				   be_arg_pres_impl_handler);

struct be_event *be_arg_no_included_handler(struct be_handler */*bh*/,
					    struct be_event *be)
{
	struct be_state *state = ((struct be_state_event *)be)->state;
	io_file_mask root_mask;
	
	if( be->id != BESE_CLI_ARGS )
		return be;
	root_mask = meta_make_file_mask(FMA_SetFlags, IO_FILE_ROOT,
					FMA_TAG_DONE);
	
	if (state->get_cli_args()->no_included_declarations) {
		meta_squelch_channels(
			&state->get_pres()->meta_data,
			meta_make_channel_mask(CMA_ExcludesInput, &root_mask,
					       CMA_SetFlags, DATA_CHANNEL_DECL,
					       CMA_TAG_DONE));
	}
	if (state->get_cli_args()->no_included_implementations) {
		meta_squelch_channels(
			&state->get_pres()->meta_data,
			meta_make_channel_mask(CMA_ExcludesInput, &root_mask,
					       CMA_SetFlags, DATA_CHANNEL_IMPL,
					       CMA_TAG_DONE));
	}
	return( be );
}

struct be_handler be_arg_no_included("no_included_*",
				     REQUIRE_HANDLER(be_arg_input_file),
				     be_arg_no_included_handler);

struct be_event *be_arg_all_mu_stubs_handler(struct be_handler */*bh*/,
					     struct be_event *be)
{
	struct be_state *state = ((struct be_state_event *)be)->state;
	pres_c_1 *pres = state->get_pres();
	struct mu_stub_info_node *msi;
	unsigned int i;
	
	if( be->id != BESE_CLI_ARGS )
		return be;
	/*
	 * If we were told to generate all m/u stubs, add them all to the list
	 * of to-be-generated m/u stubs.
	 */
	if (state->get_cli_args()->all_mu_stubs) {
		for (i = 0; i < pres->stubs.stubs_len; i++) {
			switch (pres->stubs.stubs_val[i].kind) {
			case PRES_C_MARSHAL_STUB:
			case PRES_C_UNMARSHAL_STUB: {
				for (int j = 0; j < PRES_C_DIRECTIONS; j++) {
					if (j == PRES_C_DIRECTION_UNKNOWN)
						continue;
					msi = (mu_stub_info_node *) mustcalloc(
						sizeof(mu_stub_info_node));
					msi->stub_dir = (pres_c_direction) j;
					msi->stub_idx = i;
					msi->stub_kind =
						pres->stubs.stubs_val[i].kind;
					add_tail(state->
						 get_mu_stub_list(),
						 &msi->link);
				}
				break;
			}
			default:
				break;
			}
		}
	}
	return( be );
}

struct be_handler be_arg_all_mu_stubs("all_mu_stubs",
				      REQUIRE_HANDLER(be_arg_input_file),
				      be_arg_all_mu_stubs_handler);

/*
 * When we register with the state we just create a separate looper that
 * contains all of the argument handlers so that we don't interfere with
 * priorities of other state handlers.  If a backend wants to do something
 * else then they just grab the `arg looper' handler and add their own stuff.
 */

void register_arg_handlers(struct be_state *state)
{
	struct be_looper *arg_looper;
	
	arg_looper = new be_looper("arg looper", 10);
	arg_looper->add_handler(&be_arg_scml_defs);
	arg_looper->add_handler(&be_arg_pres_impl);
	arg_looper->add_handler(&be_arg_includes);
	arg_looper->add_handler(&be_arg_input_file);
	arg_looper->add_handler(&be_arg_inline_file);
	arg_looper->add_handler(&be_arg_stubs_file);
	arg_looper->add_handler(&be_arg_header_file);
	arg_looper->add_handler(&be_arg_no_included);
	arg_looper->add_handler(&be_arg_all_mu_stubs);
	
	state->add_handler(arg_looper);
}
