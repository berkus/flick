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
#include "structnest_-client.h"
#include "structnest_-server.h"

int call_server(mach_msg_header_t *request_ptr, mach_msg_header_t *reply_ptr) {
  return test_server(request_ptr, reply_ptr);
}

int call_client(mach_port_t right){
  int fail;
  static int temp = 1;
  int value = temp;
  a arg;
  b res1, res2;

  arg.e.c = value++;
  arg.e.d = value++;
  arg.e.e.c = value++;
  arg.e.e.d = value++;
  arg.e.f = value++;
  arg.e.g.c = value++;
  arg.e.g.d = value++;
  arg.h.a = value++;
  arg.h.b = value++;
  arg.j.b.a.a = value++;
  arg.j.b.a.b = value++;
  arg.j.z.a = value++;
  arg.j.z.b = value++;
  arg.f = value++;
  arg.g.c = value++;
  arg.g.d = value++;
  arg.g.e.c = value++;
  arg.g.e.d = value++;
  arg.g.f = value++;
  arg.g.g.c = value++;
  arg.g.g.d = value++;
  arg.i.a = value++;
  arg.i.b = value;
  
  res1 = test_CopyAdd(right, &arg);
  res2 = test_server_CopyAdd(right, &arg);
  fail = (res1.c.c == arg.e.c);
  fail = fail && (res1.c.d == arg.e.d);
  fail = fail && (res1.c.e.c == arg.e.e.c);
  fail = fail && (res1.c.e.d == arg.e.e.d);
  fail = fail && (res1.c.f == arg.e.f);
  fail = fail && (res1.c.g.c == arg.e.g.c);
  fail = fail && (res1.c.g.d == arg.e.g.d);
  fail = fail && (res1.d == res2.d);
  
  temp = (temp + 1) % 500;
  return !fail;
}

struct b test_server_CopyAdd(mom_ref_t op, struct a *y) {
  struct b x;
  x.c.c = y->e.c;
  x.c.d = y->e.d;
  x.c.e.c = y->e.e.c;
  x.c.e.d = y->e.e.d;
  x.c.f = y->e.f;
  x.c.g.c = y->e.g.c;
  x.c.g.d = y->e.g.d;
  x.d = y->e.c;
  x.d += y->e.d;
  x.d += y->e.e.c;
  x.d += y->e.e.d;
  x.d += y->e.f;
  x.d += y->e.g.c;
  x.d += y->e.g.d;
  x.d += y->h.a;
  x.d += y->h.b;
  x.d += y->j.b.a.a;
  x.d += y->j.b.a.b;
  x.d += y->j.z.a;
  x.d += y->j.z.b;
  x.d += y->f;
  x.d += y->g.c;
  x.d += y->g.d;
  x.d += y->g.e.c;
  x.d += y->g.e.d;
  x.d += y->g.f;
  x.d += y->g.g.c;
  x.d += y->e.g.d;
  x.d += y->i.a;
  x.d += y->i.b;
  return x;
}

/* End of file. */

