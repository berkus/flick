/*
 * Copyright (c) 1997, 1999 The University of Utah and
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

/* This is my attempt at defining a 'stat'
 * like structure to be used for complex data testing
 */

const my_ST_FSTYPSZ = 16;

typedef unsigned long my_ulong_t;

typedef my_ulong_t my_dev_t;
typedef my_ulong_t my_ino_t;
typedef my_ulong_t my_mode_t;
typedef my_ulong_t my_nlink_t;
typedef long my_uid_t;
typedef my_uid_t my_gid_t;
typedef long my_off_t;
typedef long my_time_t;

struct my_timespec {			/* definition per POSIX.4 */
        my_time_t	tv_sec;		/* seconds */
        long            tv_nsec;        /* and nanoseconds */
};
 
typedef my_timespec	my_timestruc_t;		/* definition per SVr4 */
struct my_stat {
	my_dev_t	st_dev;
	long		st_pad1[3];		/* reserve for dev expansion, */
	/* sysid definition */
	my_ino_t	st_ino;
	my_mode_t	st_mode;
	my_nlink_t	st_nlink;
	my_uid_t	st_uid;
	my_gid_t	st_gid;
	my_dev_t	st_rdev;
	long		st_pad2[2];
	my_off_t	st_size;
	long		st_pad3;		/* reserve pad for future off_t expansion */
	my_timestruc_t	st_atime;
	my_timestruc_t	st_mtime;
	my_timestruc_t	st_ctime;
	long		st_blksize;
	long		st_blocks;
	char		st_fstype[my_ST_FSTYPSZ];
	long		st_pad4[8];		/* expansion area */
};

struct entry {
	string		filename<255>;
	my_stat		info;
};

struct inner {
	long a;
	long b;
};

struct outer {
	inner a;
	inner b;
};

typedef entry	directory<>;
typedef long	longlist<>;
typedef outer	structlist<>;

program PROGNAME {
	version VERSNAME {
		long dirlst(directory) = 1;
		long lng(longlist) = 2;
		long strct(structlist) = 3;
	} = 2;
} = 3;

/* End of file. */

