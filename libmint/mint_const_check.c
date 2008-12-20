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

#include <assert.h>
#include <mom/compiler.h>
#include <mom/libmint.h>

void mint_const_check(mint_const mc)
{
	if (mc == 0)
		panic("mint_const_check: null mint_const pointer");

	switch (mc->kind)
	{
		case MINT_CONST_INT:
		case MINT_CONST_CHAR:
		case MINT_CONST_FLOAT:
			break;
		case MINT_CONST_STRUCT:
		{
			mint_const_struct *mcs = &mc->mint_const_u_u.const_struct;
			unsigned int i;

			for (i = 0; i < mcs->mint_const_struct_len; i++)
				mint_const_check(mcs->mint_const_struct_val[i]);
			break;
		}
		case MINT_CONST_ARRAY:
		{
			mint_const_array *mca = &mc->mint_const_u_u.const_array;

			if (mca->mint_const_array_len > 0)
			{
				mint_const_kind subkind = mca->mint_const_array_val[0]->kind;
				unsigned int i;

				for (i = 1; i < mca->mint_const_array_len; i++)
				{
					if (mca->mint_const_array_val[i]->kind != subkind)
						panic("mint_const_check: inconsistent array element types");
					mint_const_check(mca->mint_const_array_val[i]);
				}
			}
			break;
		}
		default:
			panic("mint_const_check: unknown mint_const_kind %d\n", mc->kind);
	}
}

