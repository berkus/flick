/*
 * Copyright (c) 1996 The University of Utah and
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
/* Purpose: To test the presentations of various `optional' types. */

/* A forward reference to a structure type. */

typedef struct foo *foo_opt;

struct foo {
    int     value;
    foo_opt next;
};

/* A second way to express optionality: A variable-length array. */

typedef struct foo foo_array<1>;

/* A third way to express optionality: A discriminated union. */

union foo_union switch (int present) {
    case 1:  struct foo element;
    default: void;
};

/* Finally, test the presentation of other counted types. */

struct mimsy {
    opaque fixed[100];
    string var<100>;
    int fixed_int[100];
    int var_int<100>;
    int unb_int<>;
};

/* End of file. */

