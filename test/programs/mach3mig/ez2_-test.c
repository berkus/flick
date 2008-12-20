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
#include "ez2_-server.h"
#include "ez2_-client.h"

int call_server(mach_msg_header_t *request_ptr, mach_msg_header_t *reply_ptr) {
  return a_server(request_ptr, reply_ptr);
}

int call_client(mach_port_t right){
  int fail = 0;
  static char c[101];
  static int count = 5;
  int chr;
  char res;
  
  for(chr = 0; chr < count % 100; chr++) 
    c[chr] = (count + chr) % 96 + 32;
  
  c[chr] = 0;
  
  res = a_op(right, c);
  if (!chr) 
    fail = (res != ' ');
  else if (c[0] >= 'a' && c[0] <= 'z')
    res = (res == (c[0] - 'a' + 'A'));
  else
    res = (res == c[0]);
  
  count = (count + 1) % 1001;
  return fail;
}

char a_server_op(mom_ref_t op, char *s) {
  char t = s[0];
  if (t == 0) 
    return ' ';
  if (t >= 'a' && t <= 'z') 
    t = t - 'a' + 'A';
  return t;
}

/* End of file. */

