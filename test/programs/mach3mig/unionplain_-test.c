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
#include "unionplain_-client.h"
#include "unionplain_-server.h"

int call_server(mach_msg_header_t *request_ptr, mach_msg_header_t *reply_ptr) {
  return un_server(request_ptr, reply_ptr);
}

int call_client(mach_port_t right) {
  int fail;
  static int which = 0;
  static int sel = 0;
  
  switch (which) {
  case 0:
    {
      Foo2 parm;
      Foo res;
      switch (sel) {
      case 0:
	parm._d = 'a';
	parm._u.x = rand()%123456;
	break;
      case 1:
	parm._d = 'b';
	parm._u.y = rand()%1234;	
	break;
      case 2:
	parm._d = 'c';
	parm._u.z = rand()%123;	
	break;
      }
      res = un_one(right, &parm);
      switch (sel) {
      case 0:
	fail = ((res._d != 1) || (res._u.x != parm._u.x));
	break;
      case 1:
	fail = ((res._d != 2) || (res._u.y != parm._u.y));
	break;
      case 2:
	fail = (res._u.z != parm._u.z);
	break;
      }
    }
    break;
  case 1:
    {
      Foo4 parm;
      Foo3 res;
      char s[10];
      long int li[10], total = 0;
      int temp;
      switch (sel) {
      case 0:
	parm._d = 1;
	temp = rand()%10;
	
	parm._u.a = rand()%123456;
	break;
      case 1:
	parm._d = 2;
	parm._u.b._length = rand()%10;	
	for (temp = 0; temp < parm._u.b._length; temp++) {
	  int zzz = rand()%12345;
	  total += zzz;
	  li[temp] = zzz;
	}
	parm._u.b._buffer = &li[0];
	break;
      case 2:
	parm._d = rand()%123 + 3;
	parm._u.c = rand()%123456;	
	break;
      }
      res = un_one(right, &parm);
      switch (sel) {
      case 0:
	fail = ((res._d != 1) || (res._u.x != parm._u.x));
	break;
      case 1:
	fail = ((res._d != 2) || (res._u.y != parm._u.y));
	break;
      case 2:
	fail = (res._u.z != parm._u.z);
	break;
      }      
    }
    break;
  }
  
  which =  1 - which;
  sel = (sel + 1) % 3;
  
  return fail;
}

Foo un_server_one(un o, Foo2 *a) {
}

Foo3 un_server_two(un o, Foo4 *a) {
}

/* End of file. */

