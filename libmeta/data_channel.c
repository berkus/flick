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

#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

#include <mom/compiler.h>
#include <mom/libmeta.h>

data_channel_index meta_add_channel(meta *m, io_file_index input,
				    const char *id)
{
	data_channel_index retval;
	
	retval = m->channels.channels_len++;
	m->channels.channels_val = (data_channel *)
		mustrealloc(m->channels.channels_val,
			    sizeof(data_channel) * m->channels.channels_len);
	m->channels.channels_val[retval].input = input;
	m->channels.channels_val[retval].id = ir_strlit(id);
	m->channels.channels_val[retval].flags = 0;
	m->channels.channels_val[retval].outputs.outputs_len = 0;
	m->channels.channels_val[retval].outputs.outputs_val = 0;
	return( retval );
}

data_channel_index meta_find_channel(meta *m, io_file_index input,
				     const char *id, int flags)
{
	data_channel_index retval = -1;
	unsigned int lpc;
	
	for( lpc = 0;
	     (lpc < m->channels.channels_len) && (retval == -1);
	     lpc++ ) {
		if( (m->channels.channels_val[lpc].input == input) &&
		    (m->channels.channels_val[lpc].flags & flags) &&
		    (!strcmp(m->channels.channels_val[lpc].id, id)) )
			retval = lpc;
	}
	return( retval );
}

void meta_add_channel_output(meta *m, data_channel_index channel,
			     io_file_index output)
{
	data_channel *dc;
	int i;
	
	dc = &m->channels.channels_val[channel];
	if( dc->flags & DATA_CHANNEL_SQUELCHED )
		return;
	i = dc->outputs.outputs_len++;
	dc->outputs.outputs_val = (io_file_index *)
		mustrealloc(dc->outputs.outputs_val,
			    sizeof(io_file_index) * dc->outputs.outputs_len);
	dc->outputs.outputs_val[i] = output;
}

data_channel_mask meta_make_channel_mask(int tag, ...)
{
	data_channel_mask retval;
	va_list args;
	
	retval.mask_flags = 0;
	retval.input = 0;
	retval.id = "";
	retval.set_flags = 0;
	retval.unset_flags = 0;
	va_start(args, tag);
	while( tag != CMA_TAG_DONE ) {
		switch( tag ) {
		case CMA_MatchesInput:
			retval.input = va_arg(args, io_file_mask *);
			retval.mask_flags |= DATA_CHANNEL_MASK_HAS_INPUT;
			break;
		case CMA_ExcludesInput:
			retval.input = va_arg(args, io_file_mask *);
			break;
		case CMA_MatchesID:
			retval.id = va_arg(args, char *);
			retval.mask_flags |= DATA_CHANNEL_MASK_HAS_ID;
			break;
		case CMA_ExcludesID:
			retval.id = va_arg(args, char *);
			break;
		case CMA_SetFlags:
			retval.set_flags = va_arg(args, unsigned int);
			break;
		case CMA_UnsetFlags:
			retval.unset_flags = va_arg(args, unsigned int);
			break;
		default:
			panic("CMA_ Tag %d not understood", tag);
			break;
		}
		tag = va_arg(args, int);
	}
	va_end(args);
	return( retval );
}

int meta_match_channel_mask(meta *m, data_channel_mask *dcm,
			    data_channel_index channel)
{
	data_channel *dc;
	int retval = 1;
	
	dc = &m->channels.channels_val[channel];
	if( dcm->input ) {
		if( dcm->mask_flags & DATA_CHANNEL_MASK_HAS_INPUT )
			retval = meta_match_file_mask(m, dcm->input,
						      dc->input);
		else
			retval = !meta_match_file_mask(m, dcm->input,
						       dc->input);
	}
	if( retval && dcm->id[0] ) {
		if( ((dcm->mask_flags & DATA_CHANNEL_MASK_HAS_ID) &&
		     strcmp(dcm->id, dc->id)) ||
		    (!(dcm->mask_flags & DATA_CHANNEL_MASK_HAS_ID) &&
		     !strcmp(dcm->id, dc->id)) )
			retval = 0;
	}
	if( retval && ((dc->flags & dcm->set_flags) != dcm->set_flags) )
		retval = 0;
	if( retval && (dc->flags & dcm->unset_flags) )
		retval = 0;
	return( retval );
}

void meta_squelch_channel(meta *m, data_channel_index channel)
{
	data_channel *dc;
	
	dc = &m->channels.channels_val[channel];
	dc->flags |= DATA_CHANNEL_SQUELCHED;
}

void meta_squelch_channels(meta *m, data_channel_mask dcm)
{
	int lpc;
	
	for( lpc = 0; lpc < m->channels.channels_len; lpc++ ) {
		if( meta_match_channel_mask(m, &dcm, lpc) )
			meta_squelch_channel(m, lpc);
	}
}

void meta_check_channel(meta *m, data_channel_index channel)
{
	unsigned int lpc;
	data_channel *dc;
	
	dc = &m->channels.channels_val[channel];
	assert(dc->input >= 0);
	assert(dc->input < m->files.files_len);
	assert(dc->id);
	for( lpc = 0; dc->outputs.outputs_len; lpc++ ) {
		assert( dc->outputs.outputs_val[lpc] >= 0 );
		assert( dc->outputs.outputs_val[lpc] < m->files.files_len);
	}
}

static void my_i_fprintf(FILE *file, int indent, char *format, ...)
{
	va_list args;
	int lpc;
	
	for( lpc = 0; lpc < indent; lpc++ )
		fprintf(file, " ");
	va_start(args, format);
	vfprintf(file, format, args);
	va_end(args);
}

void meta_print_channel(meta *m, FILE *file, int indent,
			data_channel_index channel)
{
	data_channel *dc;
	unsigned int lpc;
	
	dc = &m->channels.channels_val[channel];
	my_i_fprintf(file, indent, "Channel #%d = {\n", channel);
	indent++;
	my_i_fprintf(file, indent, "Input:      %d\n", dc->input);
	my_i_fprintf(file, indent, "ID:         %s\n", dc->id);
	my_i_fprintf(file, indent, "Flags:     ");
	if( dc->flags & DATA_CHANNEL_SQUELCHED )
		fprintf(file, " DATA_CHANNEL_SQUELCHED");
	if( dc->flags & DATA_CHANNEL_DECL )
		fprintf(file, " DATA_CHANNEL_DECL");
	if( dc->flags & DATA_CHANNEL_IMPL )
		fprintf(file, " DATA_CHANNEL_IMPL");
	fprintf(file, "\n");
	my_i_fprintf(file, indent, "Outputs:   ");
	for( lpc = 0; lpc < dc->outputs.outputs_len; lpc++ ) {
		fprintf(file, " [%d]", dc->outputs.outputs_val[lpc]);
	}
	fprintf(file, "\n");
	indent--;
	my_i_fprintf(file, indent, "}\n");
}

void meta_print_channel_mask(FILE *file, int indent, data_channel_mask dcm)
{
	my_i_fprintf(file, indent, "mask_flags: ");
	if( dcm.mask_flags & DATA_CHANNEL_MASK_HAS_INPUT )
		fprintf(file, " has_input");
	if( dcm.mask_flags & DATA_CHANNEL_MASK_HAS_ID )
		fprintf(file, " has_id");
	fprintf(file, "\n");
	my_i_fprintf(file, indent, "input:\n");
	if( dcm.input )
		meta_print_file_mask(file, indent + 1, dcm.input);
	else
		my_i_fprintf(file, indent + 1, "(ignored)\n");
	my_i_fprintf(file, indent, "id:               ");
	if( dcm.id[0] )
		fprintf(file, "(ignored)\n");
	else
		fprintf(file, "%s\n", dcm.id);
	my_i_fprintf(file, indent, "set_flags:        ");
	if( dcm.set_flags & DATA_CHANNEL_SQUELCHED )
		fprintf(file, " DATA_CHANNEL_SQUELCHED");
	if( dcm.set_flags & DATA_CHANNEL_DECL )
		fprintf(file, " DATA_CHANNEL_DECL");
	if( dcm.set_flags & DATA_CHANNEL_IMPL )
		fprintf(file, " DATA_CHANNEL_IMPL");
	fprintf(file, "\n");
	my_i_fprintf(file, indent, "unset_flags:      ");
	if( dcm.unset_flags & DATA_CHANNEL_SQUELCHED )
		fprintf(file, " DATA_CHANNEL_SQUELCHED");
	if( dcm.unset_flags & DATA_CHANNEL_DECL )
		fprintf(file, " DATA_CHANNEL_DECL");
	if( dcm.unset_flags & DATA_CHANNEL_IMPL )
		fprintf(file, " DATA_CHANNEL_IMPL");
	fprintf(file, "\n");
}
