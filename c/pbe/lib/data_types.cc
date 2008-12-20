/*
 * Copyright (c) 1995, 1996, 1997, 1998, 1999 The University of Utah and
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

#include <string.h>

#include <mom/compiler.h>
#include "data_types.hh"

data_field::data_field(unsigned long int a)
{
	type = TYPE_UINT;
	category = TYPE_LITERAL;
	u.u = a;
}

data_field::data_field(long int a)
{
	type = TYPE_INT;
	category = TYPE_LITERAL;
	u.i = a;
}

data_field::data_field(unsigned int a)
{
	type = TYPE_UINT;
	category = TYPE_LITERAL;
	u.u = a;
}

data_field::data_field(int a)
{
	type = TYPE_INT;
	category = TYPE_LITERAL;
	u.i = a;
}

data_field::data_field(unsigned short int a)
{
	type = TYPE_USHORT;
	category = TYPE_LITERAL;
	u.s = a;
}

data_field::data_field(double a)
{
	type = TYPE_FLOAT;
	category = TYPE_LITERAL;
	u.f = a;
}

data_field::data_field(char a)
{
	type = TYPE_CHAR;
	category = TYPE_LITERAL;
	u.c = a;
}

data_field::data_field(mint_const_kind kind, char *name)
{
	switch (kind){
	case MINT_CONST_INT:
		type = TYPE_INT;
		break;
	case MINT_CONST_CHAR:
		type = TYPE_CHAR;
		break;
	case MINT_CONST_FLOAT:
		type = TYPE_FLOAT;
		break;
	default:
		panic("`data_field' can't construct from MINT const kind %d.",
		      kind);
		break;
	}
	category = TYPE_SYMBOLIC;
	u.name = name;
}

data_field::data_field(const data_field &d)
{
	type = d.type;
	category = d.category;
	u.i = d.u.i;
}

int data_field::operator==(const data_field &d) const
{
	if (d.type != type)
		return 0;
	if (d.category != category) {
		warn("A literal `data_field' was compared with a symbolic "
		     "`data_field'.  Assuming they are not equal.");
		return 0;
	}
	
	switch (category) {
	case TYPE_LITERAL:
		switch (type) {
		case TYPE_EMPTY:	return 1;
		case TYPE_CHAR:		return d.u.c == u.c;
		case TYPE_INT:		return d.u.i == u.i;
		case TYPE_UINT:		return d.u.u == u.u;
		case TYPE_USHORT:       return d.u.s == u.s;
		case TYPE_FLOAT:	return d.u.f == u.f;
		}
		break;
	case TYPE_SYMBOLIC:
		if (type == TYPE_EMPTY)
			/* All empty values are equal. */
			return 1;
		else if (strcmp(u.name, d.u.name))
			return 0;
		else
			return 1;
		break;
	}
	panic("`data_field::operator==' didn't return correctly!");
	return 0;
}

const data_field &
data_field::operator=(const data_field &d)
{
	type = d.type;
	category = d.category;
	/* XXX --- Not really "right."  Should find out what type of value we
	   are. */
	u.i = d.u.i;
	return *this;
}

data_type::data_type(const data_type &dt)
{
	length = dt.length;
	fields = new data_field[length];
	for (int i = 0; i < length; i++)
		fields[i] = dt.fields[i];
}

data_type::data_type(const data_field &df)
{
	length = 1;
	fields = new data_field(df);
}

data_type::~data_type()
{
	if (fields)
		delete[] fields;
}

data_type &
data_type::operator+(const data_type &dt) const
{
	int temp;
	data_type *d = new data_type;
	d->fields = new data_field[length + dt.length];
	d->length = length + dt.length;
	for (temp = 0; temp < length; temp++)
		d->fields[temp] = fields[temp];
	for (temp = 0; temp < dt.length; temp++)
		d->fields[temp+length] = dt.fields[temp];
	return *d;
}

data_type &
data_type::operator+(const data_field &df) const
{
	int temp;
	data_type *d = new data_type;
	d->fields = new data_field[length + 1];
	d->length = length + 1;
	for (temp = 0; temp < length; temp++)
		d->fields[temp] = fields[temp];
	d->fields[length] = df;
	return *d;
}

int
data_type::operator==(const data_type &dt) const
{
	if (length != dt.length)
		return 0;
	for (int i = 0; i < length; i++)
		if (!(fields[i] == dt.fields[i]))
			return 0;
	return 1;
}

const data_type &
data_type::operator=(const data_type &dt)
{
	if (fields)
		delete[] fields;
	length = dt.length;
	fields = new data_field[length];
	for (int i = 0; i < length; i++)
		fields[i] = dt.fields[i];
	return *this;
}

data_type &
data_type::crop() const
{
	data_type *d = new data_type;
	d->length = length - 1;
	d->fields = new data_field[d->length];
	for (int i = 1; i < length; i++)
		d->fields[i-1] = fields[i];
	return *d;
}

data_type &
build(mint_const_int_u val)
{
	data_type *d;
	
	switch (val.kind) {
	case MINT_CONST_LITERAL: {
		data_field f(val.mint_const_int_u_u.value);
		d = new data_type(f);
		break;
	}
	
	case MINT_CONST_SYMBOLIC: {
		data_field f(MINT_CONST_INT, val.mint_const_int_u_u.name);
		d = new data_type(f);
		break;
	}
	
	default:
		panic("MINT constant is neither literal nor symbolic.");
		break;
	}
	
	return *d;
}

data_type &
build(mint_const_char_u val)
{
	data_type *d;
	
	switch (val.kind) {
	case MINT_CONST_LITERAL: {
		data_field f(val.mint_const_char_u_u.value);
		d = new data_type(f);
		break;
	}
	
	case MINT_CONST_SYMBOLIC: {
		data_field f(MINT_CONST_CHAR, val.mint_const_char_u_u.name);
		d = new data_type(f);
		break;
	}
	
	default:
		panic("MINT constant is neither literal nor symbolic.");
		break;
	}
	
	return *d;
}

data_type &
build(mint_const_float_u val)
{
	data_type *d;
	
	switch (val.kind) {
	case MINT_CONST_LITERAL: {
		data_field f(val.mint_const_float_u_u.value);
		d = new data_type(f);
		break;
	}
	
	case MINT_CONST_SYMBOLIC: {
		data_field f(MINT_CONST_FLOAT, val.mint_const_float_u_u.name);
		d = new data_type(f);
		break;
	}
	
	default:
		panic("MINT constant is neither literal nor symbolic.");
		break;
	}
	
	return *d;
}

data_type &
build(mint_const_struct val)
{
	data_type *d = new data_type;
	for (unsigned int i = 0; i < val.mint_const_struct_len; i++)
		*d = *d + build(val.mint_const_struct_val[i]);
	return *d;
}

data_type &
build(mint_const_array val)
{
	data_field f(val.mint_const_array_len);
	data_type *d = new data_type(f);
	for (unsigned int i = 0; i < val.mint_const_array_len; i++)
		*d = *d + build(val.mint_const_array_val[i]);
	return *d;
}

data_type &
build(mint_const const_val)
{
	switch (const_val->kind) {
	case MINT_CONST_INT:
		return build(const_val->mint_const_u_u.const_int);
	case MINT_CONST_CHAR:
		return build(const_val->mint_const_u_u.const_char);
	case MINT_CONST_FLOAT:
		return build(const_val->mint_const_u_u.const_float);
	case MINT_CONST_ARRAY:
		return build(const_val->mint_const_u_u.const_array);
	case MINT_CONST_STRUCT:
		return build(const_val->mint_const_u_u.const_struct);
	default:
		panic("Incorrect MINT_CONST type in flattening code...\n");
	}
}

/* End of file. */

