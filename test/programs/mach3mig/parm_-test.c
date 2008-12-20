/*
 * Copyright (c) 1995, 1996 The University of Utah and
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
#include "interface_.client.h"
#include "interface_.server.h"

int call_server(mach_msg_header_t *request_ptr, mach_msg_header_t *reply_ptr) {
  return test_server(request_ptr, reply_ptr);
}

int call_client(mach_port_t right) {
  static int which = 0;
  int fail = 0;

  switch (which) {
  case 1: {
    signed short int a = rand()%3000-1500, b = rand()%3000-1500, c, d, res;
    d = b;
    res = test_Short(right,a,&b,&c);
    fail = (b != a) || (c != d) || (res != a+d);
    break;
  }
  case 2: {
    signed long int a = rand()%300000-150000, b = rand()%300000-150000, c, d, res;
    d = b;
    res = test_Long(right,a,&b,&c);
    fail = (b != a) || (c != d) || (res != a+d);
    break;
  }
  case 3: {
    unsigned short int a = rand()%3000, b = rand()%3000, c, d, res;
    d = b;
    res = test_UShort(right,a,&b,&c);
    fail = (b != a) || (c != d) || (res != a+d);
    break;
  }
  case 4: {
    unsigned long int a = rand()%3000, b = rand()%3000, c, d, res;
    d = b;
    res = test_ULong(right,a,&b,&c);
    fail = (b != a) || (c != d) || (res != a+d);
    break;
  }
  case 5: {
    /*    float a = rand()/15.34, b = rand()/15.34, c, d, res;
	  d = b;
	  res = test_Float(right,a,&b,&c);
	  fail = (b != a) || (c != d) || (res != a+d);
	  */
    fail = 0;
    break;
  }
  case 6: {
    unsigned char a = rand()%2, b = rand()%2, c, d, res;
    d = b;
    res = test_Boolean(right,a,&b,&c);
    fail = (b != a) || (c != d) || (res != a && d);
    break;
  }
  case 7: {
    char a = rand()%128, b = rand()%128, c, d, res;
    d = b;
    res = test_Char(right,a,&b,&c);
    fail = (b != a) || (c != d) || (res != ((a>d) ? a : d));
    break;
  }
  case 8: {
    char a = rand()%64, b = rand()%64, c, d, res;
    d = b;
    res = test_Octet(right,a,&b,&c);
    fail = (b != a) || (c != d) || (res != a+d);
    break;
  }
  }

  which =  which % 8 + 1;

  if (fail)
    printf("Failed case %d\n",which);
  return fail;
}

signed short int test_server_Short(mom_ref_t o, signed short int a, signed short int *b, signed short int *c) {
  *c = *b;
  *b = a;
  return a + *c;
}

unsigned short int test_server_UShort(mom_ref_t o, unsigned short int a, unsigned short int *b, unsigned short int *c) {
  *c = *b;
  *b = a;
  return a + *c;
}

signed long int test_server_Long(mom_ref_t o, signed long int a, signed long int *b, signed long int *c) {
  *c = *b;
  *b = a;
  return a + *c;
}

unsigned long int test_server_ULong(mom_ref_t o, unsigned long int a, unsigned long int *b, unsigned long int *c) {
  *c = *b;
  *b = a;
  return a + *c;
}

float test_server_Float(mom_ref_t o, float a, float *b, float *c) {
  *c = *b;
  *b = a;
  return a + *c;
}

unsigned char test_server_Boolean(mom_ref_t o, unsigned char a, unsigned char *b, unsigned char *c) {
  *c = *b;
  *b = a;
  return a && *c;
}

char test_server_Char(mom_ref_t o, char a, char *b, char *c) {
  *c = *b;
  *b = a;
  return (a > *c ? a : *c);
}

unsigned char test_server_Octet(mom_ref_t o, unsigned char a, unsigned char *b, unsigned char *c) {
  *c = *b;
  *b = a;
  return a + *c;
}
