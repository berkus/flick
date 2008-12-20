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
#include "interface_-client.h"
#include "interface_-server.h"

int call_server(mach_msg_header_t *request_ptr, mach_msg_header_t *reply_ptr) {
  return math_server(request_ptr, reply_ptr);
}

int call_client(mach_port_t right) {
  static short int a = 50, b = 50;
  static int c = 500, d = 500;
  static int which = 1;
  int vals[10];
  CORBA_sequence_integer lst;
  short int res_short;
  int reslong;
  int fail;
  lst._buffer = &vals[0];
  
  switch (which) {
  case 1:
    res_short = math_addshort(right,a,b);
    fail = res_short!=(a+b);
    break;
  case 2:
    res_short = math_subshort(right,a,b);
    fail = res_short!=(a-b);
    break;
  case 3:
    res_short = math_multshort(right,a,b);
    fail = res_short!=(a*b);
    break;
  case 4:
    reslong = math_addlong(right,c,d);
    fail = reslong!=(c+d);
    break;
  case 5:
    reslong = math_sublong(right,c,d);
    fail = reslong!=(c-d);
    break;
  case 6:
    reslong = math_multlong(right,c,d);
    fail = reslong!=(c*d);
    break;
  case 7:
    lst._length = a%10;
    if (a%10 < 0)
      lst._length = -lst._length;
    for(res_short = 0; res_short < lst._length; res_short++) {
      lst._buffer[res_short] = (c+res_short)%127;
    }
    reslong = math_addlonglist(right,&lst);
    for(res_short = 0; res_short < lst._length; res_short++)
      reslong -= ((c+res_short)%127);
    fail = reslong;
    break;
  case 8:
    lst._length = a%10;
    if (a%10 < 0)
      lst._length = -lst._length;
    for(res_short = 0; res_short < lst._length; res_short++) {
      lst._buffer[res_short] = (c + res_short) % 13;
      if (!lst._buffer[res_short])
	lst._buffer[res_short] = 1;
    }
    reslong = math_multlonglist(right,&lst);
    for(res_short = 0; res_short < lst._length; res_short++)
      if ((c + res_short) % 13)
	reslong /= ((c + res_short) % 13);
    fail = (reslong != 1);
    break;
  }

  which =  which % 8 + 1;
  
  a = (a > -100) ? a - 1 : 100;
  b = (b < 100) ? b + 1 : -100;
  c = (c > -10000) ? c - 6 : 10000;
  d = (d < 10000) ? d + 6 : -10000;
  if (fail)
    printf("result: %d\n",(which > 3)?reslong:res_short);
  return fail;
}

short int math_server_addshort(mom_ref_t o, short int a, short int b) {
  return (a+b);
}

short int math_server_subshort(mom_ref_t o, short int a, short int b) {
  return (a-b);
}

short int math_server_multshort(mom_ref_t o, short int a, short int b) {
  return (a*b);
}

int math_server_addlong(mom_ref_t o, int a, int b) {
  return (a+b);
}

int math_server_sublong(mom_ref_t o, int a, int b) {
  return (a-b);
}

int math_server_multlong(mom_ref_t o, int a, int b) {
  return (a*b);
}

int math_server_addlonglist(mom_ref_t o, CORBA_sequence_integer *a) {
  int res = 0;
  int temp;
  for (temp = 0; temp < a->_length; temp++) {
    res += a->_buffer[temp];
  }
  return res;
}
int math_server_multlonglist(mom_ref_t o, CORBA_sequence_integer *a) {
  int res = 1;
  int temp;
  for (temp = 0; temp < a->_length; temp++) {
    res *= a->_buffer[temp];
  }
  return res;
}

/* End of file. */

