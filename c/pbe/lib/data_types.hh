/*
 * Copyright (c) 1995, 1996, 1999 The University of Utah and
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

#ifndef _data_types_hh
#define _data_types_hh

#include <mom/mint.h>

class data_field {
public:
	enum {
		TYPE_INT,
		TYPE_UINT,
		TYPE_USHORT,
		TYPE_FLOAT,
		TYPE_CHAR,
		TYPE_EMPTY
	} type;
	
	enum {
		TYPE_LITERAL,
		TYPE_SYMBOLIC
	} category;
	
	union {
		signed long int i;
		unsigned long int u;
		unsigned short int s;
		double f;
		char c;
		char *name; /* For symbolic values. */
	} u;
	
	data_field() { type = TYPE_EMPTY; category = TYPE_LITERAL; }
	data_field(unsigned long int a);
	data_field(unsigned int a);
	data_field(long int a);
	data_field(int a);
	data_field(unsigned short int a);
	data_field(double a);
	data_field(char a);
	data_field(mint_const_kind kind, char *name); /* Symbolic values. */
	
	data_field(const data_field &d);
	
	int operator==(const data_field &d) const;
	const data_field &operator=(const data_field &d);
};

class data_type {
private:
	data_field *fields;
	int length;
	
public:
	data_type() : fields(0), length(0) { }
	data_type(const data_field &df);
	data_type(const data_type &dt);
	~data_type();
	
	const data_field &operator[](int i) const { return fields[i]; }
	int count() const { return length; }
	/* This will return a type with one fewer fields. */
	data_type &crop() const;
	
	data_type &operator+(const data_type &dt) const ;
	data_type &operator+(const data_field &df) const ;
	const data_type &operator=(const data_type &dt);
	int operator==(const data_type &dt) const ;
};

data_type &build(mint_const_int_u val);
data_type &build(mint_const_char_u val);
data_type &build(mint_const_float_u val);
data_type &build(mint_const_struct val);
data_type &build(mint_const_array val);
data_type &build(mint_const const_val);

#endif /* _data_types_hh */

/* End of file. */

