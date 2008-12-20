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
#include "structnestez_-client.h"
#include "structnestez_-server.h"

int call_server(mach_msg_header_t *request_ptr, mach_msg_header_t *reply_ptr) {
  return test_server(request_ptr, reply_ptr);
}

int call_client(mach_port_t right){
  static signed short int a = -1, z = 1;
  struct bar arg1, res1;
  struct b arg2, res2;
  int fail;
  arg1.f.f = a;
  arg2.f.f = z;
  res1 = test_op(right, &arg1);
  res2 = test_op2(right, &arg2);
  fail = !((res1.f.f == arg1.f.f) && (res2.f.f == arg2.f.f));
  a = (a - 1) % 5000;
  z = (z + 1) % 5000;
  return fail;
}

struct bar test_server_op(mom_ref_t op, struct bar *arg) {
  return *arg;
}

struct b test_server_op2(mom_ref_t op, struct b *arg) {
  return *arg;
}

/* End of file. */

