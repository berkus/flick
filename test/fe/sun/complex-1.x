/*
 * Copyright (c) 1996, 1997, 1999 The University of Utah and
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

/* This file contains test input for `flick-fe-sun'. */
/* Purpose: To test the generation of code for a complex data type. */
/* This file is also useful for timing tests. */

struct sexnage {
	char sex;
	float age;
};

struct person {
	string name<50>;
	int zip;
	char a;
	int ssn;
	sexnage sa;
};

typedef struct person people<70000>;

program PROG {
    version VERS {
	double op(people) = 1;
    } = 2;
} = 3;

/*
#include <sys/time.h>
double curtime;

  struct timeval starttime, endtime;
  gettimeofday(&starttime, NULL);

  gettimeofday(&endtime, NULL);
  curtime += (((endtime.tv_sec - starttime.tv_sec) * 1000.0) +
	     ((endtime.tv_usec - starttime.tv_usec) / 1000.0));
*/

/* End of file. */

