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
#include <mom/libmeta.h>

io_file_index meta_add_file(meta *m, const char *id, int flags)
{
	io_file_index retval;
	
	if( (retval = meta_find_file(m, id, flags, 1)) == -1 ) {
		retval = m->files.files_len++;
		m->files.files_val = (io_file *)
			mustrealloc(m->files.files_val,
				    m->files.files_len * sizeof(io_file));
		m->files.files_val[retval].id = ir_strlit(id);
		m->files.files_val[retval].flags = flags;
		m->files.files_val[retval].references = 0;
		m->files.files_val[retval].includes.includes_len = 0;
		m->files.files_val[retval].includes.includes_val = 0;
	}
	return( retval );
}

io_file_index meta_find_file(meta *m, const char *id, int flags,
			     int absolute_path)
{
	unsigned int lpc;
	io_file_index retval = -1;
	io_file *iofile;
	
	for( lpc = 0; (lpc < m->files.files_len) && (retval == -1); lpc++ ) {
		iofile = &m->files.files_val[lpc];
		if( (!id || !strcmp((absolute_path ?
				     iofile->id :
				     file_part(iofile->id)), id)) &&
		    (!flags || (iofile->flags & flags)) ) {
			retval = lpc;
		}
	}
	return( retval );
}

void meta_include_file(meta *m, io_file_index file,
		       io_file_index included_file)
{
	io_file *root_file;
	int i;
	
	m->files.files_val[included_file].references++;
	root_file = &m->files.files_val[file];
	i = root_file->includes.includes_len++;
	root_file->includes.includes_val =
		(io_file_index *)mustrealloc(root_file->includes.includes_val,
					     root_file->includes.includes_len *
					     sizeof(io_file_index));
	root_file->includes.includes_val[i] = included_file;
}

void meta_squelch_file(meta *m, io_file_index file, data_channel_mask *dcm)
{
	unsigned int lpc;
	io_file *iofile;
	
	iofile = &m->files.files_val[file];
	for( lpc = 0; lpc < m->channels.channels_len; lpc++ ) {
		if( (m->channels.channels_val[lpc].input == file) &&
		    meta_match_channel_mask(m, dcm, lpc) )
			meta_squelch_channel(m, lpc);
	}
	for( lpc = 0; lpc < iofile->includes.includes_len; lpc++ ) {
		m->files.files_val[iofile->includes.includes_val[lpc]].
			references--;
		if( m->files.files_val[iofile->includes.includes_val[lpc]].
		    references == 0 ) {
			meta_squelch_file(m,
					  iofile->includes.includes_val[lpc],
					  dcm);
		}
	}
}

void meta_squelch_files(meta *m, io_file_mask *ifm, data_channel_mask *dcm)
{
	unsigned int lpc;
	
	for( lpc = 0; lpc < m->files.files_len; lpc++ ) {
		if( meta_match_file_mask(m, ifm, lpc) )
			meta_squelch_file(m, lpc, dcm);
	}
}

io_file_mask meta_make_file_mask(int tag, ...)
{
	io_file_mask retval;
	va_list args;
	
	retval.mask_flags = 0;
	retval.id = "";
	retval.set_flags = 0;
	retval.unset_flags = 0;
	va_start(args, tag);
	while( tag != FMA_TAG_DONE ) {
		switch( tag ) {
		case FMA_MatchesID:
			retval.id = va_arg(args, char *);
			retval.mask_flags |= IO_FILE_MASK_HAS_ID;
			break;
		case FMA_ExcludesID:
			retval.id = va_arg(args, char *);
			break;
		case FMA_MatchesDirID:
			retval.id = va_arg(args, char *);
			retval.mask_flags |=
				IO_FILE_MASK_HAS_ID|
				IO_FILE_MASK_DIR_PART;
			break;
		case FMA_ExcludesDirID:
			retval.id = va_arg(args, char *);
			retval.mask_flags |= IO_FILE_MASK_DIR_PART;
			break;
		case FMA_MatchesFileID:
			retval.id = va_arg(args, char *);
			retval.mask_flags |=
				IO_FILE_MASK_HAS_ID|
				IO_FILE_MASK_FILE_PART;
			break;
		case FMA_ExcludesFileID:
			retval.id = va_arg(args, char *);
			retval.mask_flags |= IO_FILE_MASK_FILE_PART;
			break;
		case FMA_SetFlags:
			retval.set_flags = va_arg(args, unsigned int);
			break;
		case FMA_UnsetFlags:
			retval.unset_flags = va_arg(args, unsigned int);
			break;
		}
		tag = va_arg(args, int);
	}
	va_end(args);
	return( retval );
}

int meta_match_file_mask(meta *m, io_file_mask *ifm, io_file_index file)
{
	int retval = 1;
	io_file *ifile;
	
	ifile = &m->files.files_val[file];
	if( ifm->id[0] ) {
		if( ifm->mask_flags & IO_FILE_MASK_FILE_PART ) {
			if( ((ifm->mask_flags & IO_FILE_MASK_HAS_ID) &&
			     strcmp(ifm->id, file_part(ifile->id))) ||
			    (!(ifm->mask_flags & IO_FILE_MASK_HAS_ID) &&
			     !strcmp(ifm->id, file_part(ifile->id))) )
				retval = 0;
		} else {
			if( ((ifm->mask_flags & IO_FILE_MASK_HAS_ID) &&
			     strcmp(ifm->id, ifile->id)) ||
			    (!(ifm->mask_flags & IO_FILE_MASK_HAS_ID) &&
			     !strcmp(ifm->id, ifile->id)) )
				retval = 0;
		}
	}
	if( (ifile->flags & ifm->set_flags) != ifm->set_flags )
		retval = 0;
	if( ifile->flags & ifm->unset_flags )
		retval = 0;
	return( retval );
}

void meta_check_file(meta *m, io_file_index file)
{
	unsigned int lpc;
	io_file *ifile;
	
	ifile = &m->files.files_val[file];
	assert(ifile->id);
	assert(ifile->references >= 0);
	for( lpc = 0; lpc < ifile->includes.includes_len; lpc++ ) {
		assert(ifile->includes.includes_val[lpc] >= 0);
		assert(ifile->includes.includes_val[lpc] <
		       m->files.files_len);
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

void meta_print_file(meta *m, FILE *file, int indent, io_file_index idx)
{
	io_file *iofile;
	unsigned int lpc;
	
	iofile = &m->files.files_val[idx];
	my_i_fprintf(file, indent, "File #%d = {\n", idx);
	indent++;
	my_i_fprintf(file, indent, "ID:         %s\n", iofile->id);
	my_i_fprintf(file, indent, "Flags:     ");
	if( iofile->flags & IO_FILE_INPUT )
		fprintf(file, " IO_FILE_INPUT");
	if( iofile->flags & IO_FILE_OUTPUT )
		fprintf(file, " IO_FILE_OUTPUT");
	if( iofile->flags & IO_FILE_BUILTIN )
		fprintf(file, " IO_FILE_BUILTIN");
	if( iofile->flags & IO_FILE_ROOT )
		fprintf(file, " IO_FILE_ROOT");
	if( iofile->flags & IO_FILE_SYSTEM )
		fprintf(file, " IO_FILE_SYSTEM");
	fprintf(file, "\n");
	my_i_fprintf(file, indent, "References: %d\n", iofile->references);
	my_i_fprintf(file, indent, "Includes:  ");
	for( lpc = 0; lpc < iofile->includes.includes_len; lpc++ ) {
		fprintf(file, " [%d]", iofile->includes.includes_val[lpc]);
	}
	fprintf(file, "\n");
	indent--;
	my_i_fprintf(file, indent, "}\n");
}

void meta_print_file_mask(FILE *file, int indent, io_file_mask *ifm)
{
	my_i_fprintf(file, indent, "mask_flags: ");
	if( ifm->mask_flags & IO_FILE_MASK_HAS_ID )
		fprintf(file, " has_id");
	fprintf(file, "\n");
	my_i_fprintf(file, indent, "id:          ");
	if( ifm->id[0] )
		fprintf(file, "%s\n", ifm->id);
	else
		fprintf(file, "(ignored)\n");
	my_i_fprintf(file, indent, "set_flags:  ");
	
	fprintf(file, "\n");
	if( ifm->set_flags & IO_FILE_INPUT )
		fprintf(file, " IO_FILE_INPUT");
	if( ifm->set_flags & IO_FILE_OUTPUT )
		fprintf(file, " IO_FILE_OUTPUT");
	if( ifm->set_flags & IO_FILE_BUILTIN )
		fprintf(file, " IO_FILE_BUILTIN");
	if( ifm->set_flags & IO_FILE_ROOT )
		fprintf(file, " IO_FILE_ROOT");
	if( ifm->set_flags & IO_FILE_SYSTEM )
		fprintf(file, " IO_FILE_SYSTEM");
	my_i_fprintf(file, indent, "unset_flags:");
	if( ifm->unset_flags & IO_FILE_INPUT )
		fprintf(file, " IO_FILE_INPUT");
	if( ifm->unset_flags & IO_FILE_OUTPUT )
		fprintf(file, " IO_FILE_OUTPUT");
	if( ifm->unset_flags & IO_FILE_BUILTIN )
		fprintf(file, " IO_FILE_BUILTIN");
	if( ifm->unset_flags & IO_FILE_ROOT )
		fprintf(file, " IO_FILE_ROOT");
	if( ifm->unset_flags & IO_FILE_SYSTEM )
		fprintf(file, " IO_FILE_SYSTEM");
	fprintf(file, "\n");
}
