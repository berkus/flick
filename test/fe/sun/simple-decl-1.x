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

/* This file conatins test input for `flick-fe-sun'. */
/* Purpose: To test the presentations of all "simple" datatypes. */

struct one_of_each {
              bool my_bool;

              char my_char;
unsigned      char my_unsigned_char;

               int my_int;
         short int my_short_int;
         long  int my_long_int;
unsigned       int my_unsigned_int;
unsigned short int my_unsigned_short_int;
unsigned long  int my_unsigned_long_int;

             float my_float;
            double my_double;

            string my_bounded_string<256>;
            string my_unbounded_string<>;

            opaque my_fixed_opaque[256];
            opaque my_bounded_opaque<256>;
            opaque my_unbounded_opaque<>;
};

/* End of file. */

