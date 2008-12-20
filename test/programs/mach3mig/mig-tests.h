/*
 * Copyright (c) 1997 The University of Utah and
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

#ifndef __MIG_TESTS_H__
#define __MIG_TESTS_H__

/********************************/
/*** TYPES for mig-tests.defs ***/
/********************************/

typedef unsigned  Iint;
typedef unsigned *Oint;
typedef struct {unsigned data[100];} IFstruct;
typedef struct {unsigned data[200];} *OFstruct;
typedef unsigned IFarray[300];
typedef unsigned (*OFarray)[400];
typedef unsigned IBarray[500];
typedef unsigned (*OBarray)[600];
typedef unsigned *Iarray;
typedef unsigned *Oarray;
typedef char IFstring[799];
typedef char (*OFstring)[800];
typedef char IVstring[4997];
typedef char (*OVstring)[1000];
typedef mach_port_t *Imach_port_array_t;
typedef mach_port_t *Imach_poly_array_t;
typedef mach_port_t *Omach_poly_array_t;

#endif /* __MIG_TESTS_H__ */

/* End of file. */

