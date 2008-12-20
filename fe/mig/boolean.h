/*
 * Copyright (c) 1995, 1996, 1999 The University of Utah and
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

#ifndef	_MIG_BOOLEAN_H_
#define	_MIG_BOOLEAN_H_

/*
 * This used to be just `boolean_t', but some systems (e.g., Solaris 2.5)
 * define `boolean_t' in <sys/types.h>.  Grrr.
 */
#ifndef mig_boolean_t
#define mig_boolean_t int
#endif

#ifndef	TRUE
#define TRUE	((mig_boolean_t) 1)
#endif

#ifndef	FALSE
#define FALSE	((mig_boolean_t) 0)
#endif

#endif	/* _MIG_BOOLEAN_H_ */

/* End of file. */

