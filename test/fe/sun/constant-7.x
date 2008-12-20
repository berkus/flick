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
/* Purpose: To test the handling of string constants containing escape
   sequences. */

/* This test doesn't work well because `rpc_scan.c' doesn't currently deal with
   C-style escapes.  Hence, for now, the double-quote characters are out. */

const lower_case_alphabet = "abcdefghijklmnopqrstuvxyz";
const upper_case_alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const digits = "0123456789";
const shift_digits = "!@#$%^&*()";
const other_punct = "`~[]{}\\|;:',<.>/?-_=+";
/* const other_punct = "`~[]{}\\|;:'\",<.>/?-_=+"; */

const c_escapes = "\a\b\f\n\r\t\v\'\\";
/* const c_escapes = "\a\b\f\n\r\t\v\'\"\\"; */

const octal_00 = "\001\002\003\004\005\006\007";
const octal_01 = "\010\011\012\013\014\015\016\017";
const octal_02 = "\020\021\022\023\024\025\026\027";
const octal_03 = "\030\031\032\033\034\035\036\037";
const octal_04 = "\040\041\042\043\044\045\046\047";
const octal_05 = "\050\051\052\053\054\055\056\057";
const octal_06 = "\060\061\062\063\064\065\066\067";
const octal_07 = "\070\071\072\073\074\075\076\077";
const octal_10 = "\100\101\102\103\104\105\106\107";
const octal_11 = "\110\111\112\113\114\115\116\117";
const octal_12 = "\120\121\122\123\124\125\126\127";
const octal_13 = "\130\131\132\133\134\135\136\137";
const octal_14 = "\140\141\142\143\144\145\146\147";
const octal_15 = "\150\151\152\153\154\155\156\157";
const octal_16 = "\160\161\162\163\164\165\166\167";
const octal_17 = "\170\171\172\173\174\175\176\177";

/* End of file. */

