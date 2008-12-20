/*
 * Copyright (c) 1995, 1996 The University of Utah and
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

#ifndef _mom_types_h_
#define _mom_types_h_

#include <limits.h>


/* Maximum possible integer type that the compiler can handle.
   XXX should be long long.  */
typedef long		maxint;
typedef unsigned long	maxuint;
#define MAXINT_MAX	LONG_MAX
#define MAXINT_MIN	LONG_MIN
#define MAXUINT_MAX	ULONG_MAX
#define MAXUINT_MIN	ULONG_MIN


#endif /* _mom_types_h_ */
