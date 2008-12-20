/*
 * Copyright (c) 1997, 1998 The University of Utah and
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

#ifndef __MIG_ARRAYS_H__
#define __MIG_ARRAYS_H__

/* typdefs for the types in mig-arrays.defs */
typedef unsigned t64B[16];
typedef unsigned t128B[32];
typedef unsigned t256B[64];
typedef unsigned t512B[128];
typedef unsigned t1K[256];
typedef unsigned t2K[512];
typedef unsigned t4K[1024];
typedef unsigned t8K[2048];
typedef unsigned t16K[4096];
typedef unsigned t32K[8192];
typedef unsigned t64K[16384];
typedef unsigned t128K[32768];
typedef unsigned t256K[65536];
typedef unsigned t512K[131072];
typedef unsigned t1024K[262144];
typedef unsigned timervar[2];

#include "timer.h"
#if 0
#define PERF_Marshal_Client
#endif
#if 0
#define PERF_Unmarshal_Client
#endif
#if 0
#define PERF_Marshal_Server
#endif
#if 0
#define PERF_Unmarshal_Server
#endif
#if 0
#define PERF_MSG
#endif

#endif /* __MIG_ARRAYS_H__ */

/* End of file */
