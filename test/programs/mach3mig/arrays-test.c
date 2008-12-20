/*
 * Copyright (c) 1996, 1997 The University of Utah and
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

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <mach/message.h>
#include <mach/port.h>
#include <mach/mig_errors.h>
#include "arrays-client.h"
#include "arrays-server.h"

#define _type arrays_a256
#define _use  arrays_test256
int *buffer;

int call_server(mach_msg_header_t *request_ptr, mach_msg_header_t *reply_ptr) {
  int res;
  res = arrays_server(request_ptr, reply_ptr);
  return res;
}

int call_client(mach_port_t right) {
  _type a;
  int result;
  
  result = _use(right, a);
  
  return 0;
}

#define fn(size)						\
int arrays_server_test##size(mom_ref_t o, arrays_a##size a)	\
{								\
  return 0;							\
}

fn(128);
fn(256);
fn(512);
fn(1024);
fn(2048);
fn(4096);
fn(8192);
fn(16384);
fn(32768);
fn(65536);

/* End of file. */

