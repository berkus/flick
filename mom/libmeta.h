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

/* This file describes the contents of the library that accompanies meta.x */

#ifndef _mom_meta_h_
#define _mom_meta_h_

#include <stdio.h>
#include <mom/meta.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Initialization, checking, and printing function for the root meta struct */
void init_meta(meta *m);
void check_meta(meta *m);
void print_meta(meta *m, FILE *file, int indent);

/* io_file */

/*
 * Add a file to the meta, if it already exists the previous one will be
 * returned
 */
io_file_index meta_add_file(meta *m, const char *id, int flags);
/*
 * Find a file in the meta.  If id is null then it will match only the flags,
 * and vice versa.  The absolute_path argument specifies whether 'id' is an
 * absolute file name, or just the file part of the name
 */
io_file_index meta_find_file(meta *m, const char *id, int flags,
			     int absolute_path);
/* Add an include file to another file */
void meta_include_file(meta *m, io_file_index file,
		       io_file_index included_file);
/* Check the file structure */
void meta_check_file(meta *m, io_file_index file);
/* Print the file structure */
void meta_print_file(meta *m, FILE *file, int indent, io_file_index idx);

/* Tags used by meta_make_file_mask() */
enum {
	FMA_TAG_DONE,
	FMA_MatchesID,        /* char * (matches on absolute path) */
	FMA_ExcludesID,       /* char * (matches on absolute path) */
	FMA_MatchesDirID,     /* char * (matches on dir part of the ID) */
	FMA_ExcludesDirID,    /* char * (matches on dir part of the ID) */
	FMA_MatchesFileID,    /* char * (matches on file part of the ID) */
	FMA_ExcludesFileID,   /* char * (matches on file part of the ID) */
	FMA_SetFlags,         /* unsigned int */
	FMA_UnsetFlags        /* unsigned int */
};
/* Construct an io_file_mask as described by the given tags */
io_file_mask meta_make_file_mask(int tag, ...);
/* Return whether or not a file matches a mask */
int meta_match_file_mask(meta *m, io_file_mask *ifm, io_file_index file);
/* Squelch a file and selected channels */
void meta_squelch_file(meta *m, io_file_index file, data_channel_mask *dcm);
/* Squelch the selected files and channels */
void meta_squelch_files(meta *m, io_file_mask *ifm, data_channel_mask *dcm);
/* Print a file mask */
void meta_print_file_mask(FILE *file, int indent, io_file_mask *ifm);
/* */

/* data_channel */

/*
 * Add a channel to the meta, if the it already exists the previous one will be
 * returned
 */
data_channel_index meta_add_channel(meta *m, io_file_index input,
				    const char *id);
/*
 * Finds a channel in the meta.  If 'id' is null, then it will match on the
 * flags, and vice versa
 */
data_channel_index meta_find_channel(meta *m, io_file_index input,
				     const char *id, int flags);
/* Add an output file to the channel */
void meta_add_channel_output(meta *m, data_channel_index channel,
			     io_file_index output);
/* Check the channel structure */
void meta_check_channel(meta *m, data_channel_index channel);
/* Print the channel structure */
void meta_print_channel(meta *m, FILE *file, int indent,
			data_channel_index channel);

/* Tags used by meta_make_channel_mask() */
enum {
	CMA_TAG_DONE,
	CMA_MatchesInput,    /* io_file_mask * */
	CMA_ExcludesInput,   /* io_file_mask * */
	CMA_MatchesID,       /* char * */
	CMA_ExcludesID,      /* char * */
	CMA_SetFlags,        /* unsigned int */
	CMA_UnsetFlags       /* unsigned int */
};
/* Construct a data_channel_mask as described by the given tags */
data_channel_mask meta_make_channel_mask(int tag, ...);
/* Return whether or not a channel matches a mask */
int meta_match_channel_mask(meta *m, data_channel_mask *dcm,
			    data_channel_index channel);
/* Squelch a channel */
void meta_squelch_channel(meta *m, data_channel_index channel);
/* Squelch a set of channels */
void meta_squelch_channels(meta *m, data_channel_mask mask);
/* Print the channel mask structure */
void meta_print_channel_mask(FILE *file, int indent, data_channel_mask dcm);
/* */

#ifdef __cplusplus
}
#endif

#endif /* _mom_meta_h_ */
