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
#include "array-client.h"
#include "array-server.h"

int call_server(mach_msg_header_t *request_ptr, mach_msg_header_t *reply_ptr) {
  return A_i_server(request_ptr, reply_ptr);
}
  
int A_eq(A in, A inout) {
  int t1, t2, res;
  res = in.a1 == inout.a1;
  for (t1 = 0; t < 17; t++) {
    res = res && (in.a2[t1] == inout.a2[t1]);
  }
  for (t1 = 0; t1 < 10; t1++) {
    for (t2 = 0; t2 < 2; t2++) {
      res = res && (in.a3[t1][t2] == inout.a3[t1][t2]);
    }
  }
  return res;
}

int call_client(mach_port_t right) {
  static int which = 1;
  int fail = 0;
  switch (which) {
  case 1: {
    A in, inout, out, ret, temp;
    int t1, t2;
    in.a1 = rand() % 12345;
    inout.a1 = rand() % 12345;
    temp.a1 = inout.a1;
    for (t1 = 0; t < 17; t++) {
      in.a2[t1] = rand() % 128;
      inout.a2[t1] = rand() % 128;
      temp.a2[t1] = inout.a2[t1];
    }
    for (t1 = 0; t1 < 10; t1++) {
      for (t2 = 0; t2 < 2; t2++) {
	in.a3[t1][t2] = rand() % 128;
	inout.a3[t1][t2] = rand() % 128;
	temp.a3[t1][t2] = inout.a3[t1][t2];
      }
    }
    ret = A_i_op(right, &in, &inout, &out);
    fail = !(A_eq(ret, temp) && A_eq(ret, out) && A_eq(in, inout));
    break;
  }
  case 2:
    break;
  case 3:
    break;
  case 4:
    break;
  case 5:
    break;
  case 6:
    break;
  }
  return fail;
}

A
A_i_server_op(mom_ref_t o, A *in, A *inout, A *out) {
  *out = *inout;
  *inout = *in;
  return *out;
}

/* End of file. */

