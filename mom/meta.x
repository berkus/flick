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

#ifdef RPC_HDR
%#ifndef _flick_meta_h
%#define _flick_meta_h
%
%#include <rpc/types.h>
%#include <rpc/xdr.h>
#endif

/*
 * The 'meta' structures contained within this file are used to store
 * information about whatever data we're processing.  Currently, this
 * just consists of storing information about input files and what 'classes'
 * of data we are going to output.
 *
 *
 * io_file
 *
 * This structure is used to record what files were used as input, and the
 * graph of `#includes'.  The structure holds the filename and various flags
 * indicating attributes of the file.  The graph of inclusions is done from
 * the top down, so the root file has a list of its includes, and the included
 * files track their includes, and so on.
 *
 *
 * data_channel
 *
 * A data_channel is a structure thats used to organize the contents of a file
 * into a series of classes.  This extra organization gives more information
 * about individual datums in the files and allows more control over their
 * processing.
 */

typedef int io_file_index;

const IO_FILE_INPUT		= 0x00000001; /* Input file */
const IO_FILE_OUTPUT		= 0x00000002; /* Output file */
const IO_FILE_BUILTIN		= 0x00000004; /*
					       * Not an actual file, but input
					       * is already builtin
					       */
const IO_FILE_ROOT		= 0x00000008; /* The root input file */
const IO_FILE_SYSTEM		= 0x00000010; /* A system file */
struct io_file {
	string		id<>;		/* Filename */
	unsigned int	flags;		/* Holds the above flags */
	int		references;	/*
					 * How many other files have included
					 * this one
					 */
	io_file_index	includes<>;	/* References to included files */
};

/* The io_file_mask structure is used to describe sets of io_file's */
const IO_FILE_MASK_HAS_ID	= 0x00000001; /* The io_file has the id */
const IO_FILE_MASK_FILE_PART	= 0x00000002; /*
					       * The id is only the file
					       * part of the filename
					       */
const IO_FILE_MASK_DIR_PART	= 0x00000004; /*
					       * The id is only the dir part
					       * of the filename
					       */
struct io_file_mask {
	unsigned int	mask_flags;	/* Flags specific to the mask */
	string		id<>;		/* An identifier to match */
	unsigned int	set_flags;	/*
					 * Match when all of these flags are 
					 * set in the io_file
					 */
	unsigned int	unset_flags;	/*
					 * Match when all of these flags aren't
					 * set in the io_file
					 */
};

typedef int data_channel_index;

/* Predefined channel, used when data just needs to be dumped out */
const PASSTHRU_DATA_CHANNEL	= 0;

const DATA_CHANNEL_SQUELCHED	= 0x00000001; /*
					       * Stop any output
					       * from the channel
					       */
const DATA_CHANNEL_DECL		= 0x00000002; /*
					       * The channel is used
					       * for declarations
					       */
const DATA_CHANNEL_IMPL		= 0x00000004; /*
					       * The channel is used
					       * for implementation
					       */
struct data_channel {
	io_file_index		input;		/* input for the channel */
	string			id<>;		/* semantic id */
	unsigned int		flags;		/* Holds the above flags */
	io_file_index		outputs<>;	/*
						 * List of output files for
						 * the channel (unused)
						 */
};

/* The data_channel_mask is used to describe sets of data_channel's */

 /* Match when data_channel has the same input */
const DATA_CHANNEL_MASK_HAS_INPUT	= 0x00000001;
/* Match when data_channel has the same id */
const DATA_CHANNEL_MASK_HAS_ID		= 0x00000002;
struct data_channel_mask {
	unsigned int	mask_flags;	/* Flags for the mask */
	io_file_mask	*input;		/* Input file mask */
	string		id<>;		/* ID to match on */
	unsigned int	set_flags;	/*
					 * Match when all of these flags are 
					 * set in the io_file
					 */
	unsigned int	unset_flags;	/*
					 * Match when all of these flags aren't
					 * set in the io_file
					 */
};


/* The root structure to hold meta data */
struct meta {
	io_file		files<>;
	data_channel	channels<>;
};

#ifdef RPC_HDR
%#endif /* _flick_meta_h */
#endif
