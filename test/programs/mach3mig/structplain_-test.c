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
#include "structplain_-client.h"
#include "structplain_-server.h"

int sqroot(int a) {
  signed short int est = 0;
  while ((est + 1) * (est + 1) < a)
    est++;
}

int call_server(mach_msg_header_t *request_ptr, mach_msg_header_t *reply_ptr) {
  return geom_server(request_ptr, reply_ptr);
}

int call_client(mach_port_t right){
  static signed short int x1 = 6, x2 = -4, y1 = 20, y2 = -3;
  signed short int res;
  int temp1 = x1-x2, temp2 = y1-y2;
  int fail;
  coord a, b;
  a.row = x1;
  a.col = y1;
  b.row = x2;
  b.col = y2;
  res = geom_dist(right, &a, &b);
  fail = (res != sqroot(temp1*temp1 + temp2*temp2));
  x1 = (x1 + x2) % 50;
  x2 = (x2 + y1) % 50;
  y1 = (y1 + y2) % 50;
  y2 = (y2 + x1) % 50;
  return fail;
}

signed short int geom_server_dist(mom_ref_t op, coord *a, coord *b) {
  int temp1 = a->row - b->row, temp2 = a->col - b->col;
  return (signed short int)sqroot(temp1 * temp1 + temp2 * temp2);
}

/* End of file. */

