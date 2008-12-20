/*
 * Copyright (c) 1998 The University of Utah and
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

#ifndef __TIMER_H__
#define __TIMER_H__

#define CPU_HZ 100000000

/* Routine to read the CPU timer */
static inline long long
read_cpu_time (void)
{
#if 1 /* only use on a Pentium class machine! */
  long long time;
  __asm __volatile (
		    ".byte 0x0f; .byte 0x31  # RDTSC instruction
movl %%edx, %0          # High order 32 bits
movl %%eax, %1          # Low order 32 bits"
                    : "=g" (((long *)&time)[1]), "=g" (((long *)&time)[0]) :: "eax", "edx");
  return time;
#else
  return 0;
#endif
}

#define CLEAR_TIME(name) name = 0
#define START_TIME(name) name -= read_cpu_time()
#define STOP_TIME(name) name += read_cpu_time()

extern long long timer;

#endif /* __TIMER_H__ */
