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

/* This code is no longer used. */
#if 0

#include <string.h>
#include <assert.h>
#include <mom/compiler.h>
#include <mom/libmint.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>
#include <mom/c/pbe.hh>

struct mu_decode_const_state
{
	mint_const *darray;
	int *darray_poss;
	int darray_len;

	void decode(mu_state *must, struct mu_decode_const_functor *f, mint_ref itype);
};

struct mu_decode_const_functor
{
	virtual void func(mu_state *must, int *darray_poss) = 0;
};

struct mu_decode_const_array_functor : public mu_decode_const_functor
{
	void func(mu_state *must, int *darray_poss);

	mu_decode_const_state *state;
	mu_decode_const_functor *sub;
	mint_array_def *adef;
	int el;
};

void mu_decode_const_array_functor::func(mu_state *must, int *darray_poss)
{
	int i;

	/* The darray_poss list contains the list of possible cases for this branch -
	   they all have the same array length an the same first `el' elements.  */

	/* If there are no remaining possibilities, terminate here.
	   Also find the length of the array we're dealing with.  */
	int array_len = -1;
	for (i = 0; i < state->darray_len; i++)
	{
		if (darray_poss[i])
		{
			assert(state->darray[i]->kind == MINT_CONST_ARRAY);
			mint_const_array *ac = &state->darray[i]->mint_const_u_u.const_array;
			if (array_len >= 0)
				assert(array_len == ac->mint_const_array_len);
			else
				array_len = ac->mint_const_array_len;
		}
	}
	if ((array_len < 0) || (el >= array_len))
	{
		/* We've finished the array, or else there are no more possibilities left.
		   Just call the subfunctor to get straight to th leaf
		   and set the cookie to -1.  */
		sub->func(must, darray_poss);
		return;
	}

	/* There are still one or more possibilities.
	   Check the next element of the array.  */

	/* Create a new darray containing the appropriate element constants.  */
	mint_const *new_darray = new mint_const[state->darray_len];
	for (i = 0; i < state->darray_len; i++)
	{
		if (darray_poss[i])
		{
			assert(state->darray[i]->kind == MINT_CONST_ARRAY);
			mint_const_array *ac = &state->darray[i]->mint_const_u_u.const_array;
			new_darray[i] = ac->mint_const_array_val[el];
		}
	}

	/* Unmarshal that element to narrow the possibilities further.  */
	mu_decode_const_state el_state = *state;
	el_state.darray = new_darray;
	mu_decode_const_array_functor ff;
	ff.state = state;
	ff.sub = sub;
	ff.adef = adef;
	ff.el = el+1;
	el_state.decode(must, &ff, adef->element_type);
}

void mu_decode_const_state::decode(mu_state *must, struct mu_decode_const_functor *f,
				   mint_ref itype)
{
	assert(itype >= 0); assert(itype < must->pres->mint.mint_1_len);
	mint_def *def = &must->pres->mint.mint_1_val[itype];

	switch (def->kind)
	{
		case MINT_INTEGER:
		case MINT_CHAR:
		{
			/* Unmarshal the simple itype into of a temporary variable.  */
			cast_type ctype = mint_to_ctype(&must->pres->mint, itype);
			cast_expr cexpr = must->add_temp_var("decode", ctype);
			must->mu_mapping_direct(cexpr, ctype, itype);

			/* Produce a C switch statement switched on that variable.  */
			cast_stmt old_c_block = must->c_block;
			must->c_block = 0;

			int *used_array = new int[darray_len];
			memset(used_array, 0, sizeof(used_array));
			for (int i = 0; i < darray_len; i++)
			{
				if (darray_poss[i] && !used_array[i])
				{
					/* Find the switch value for this case.  */
					int val;
					if (darray[i]->kind == MINT_CONST_INT)
						val = darray[i]->mint_const_u_u.const_int;
					else if (darray[i]->kind == MINT_CONST_CHAR)
						val = darray[i]->mint_const_u_u.const_char;
					else
						assert(0);

					/* Collect all the darray entries for the same case.  */
					int *new_poss = new int[darray_len];
					memset(new_poss, 0, sizeof(new_poss));
					for (int j = i; j < darray_len; j++)
					{
						if (darray_poss[j])
						{
							assert(darray[j]->kind == darray[i]->kind);
							int jval;
							if (darray[j]->kind == MINT_CONST_INT)
								jval = darray[j]->mint_const_u_u.const_int;
							else if (darray[j]->kind == MINT_CONST_CHAR)
								jval = darray[j]->mint_const_u_u.const_char;
							if (jval == val)
							{
								new_poss[j] = 1;
								used_array[j] = 1;
							}
						}
					}

					/* Add a switch case.  */
					cast_stmt old_c_block = must->c_block;
					must->c_block = 0;
					f->func(must, new_poss);
					must->add_stmt(cast_new_break());
					cast_stmt case_block = must->c_block;
					must->c_block = old_c_block;
					must->add_stmt(cast_new_case(
						def->kind == MINT_INTEGER
						? cast_new_expr_lit_int(val, 0)
						: cast_new_expr_lit_char(val, 0),
						case_block));
				}
			}

			/* Add a default case.
			   It always has an empty possibility list.  */
			{
				int *new_poss = new int[darray_len];
				memset(new_poss, 0, sizeof(new_poss));
				cast_stmt old_c_block = must->c_block;
				must->c_block = 0;
				f->func(must, new_poss);
				cast_stmt case_block = must->c_block;
				must->c_block = old_c_block;
				must->add_stmt(cast_new_default(case_block));
			}

			/* Finish off the switch statement.  */
			cast_stmt switch_body = must->c_block;
			must->c_block = old_c_block;
			must->add_stmt(cast_new_switch(cexpr, switch_body));
			break;
		}
		case MINT_ARRAY:
		{
			mint_array_def *adef = &def->mint_def_u.array_def;

			/* First narrow the possibilities using the array's length.
			   Our functor will be called back for each of the possible array lengths,
			   with a subset of the possibilities array,
			   and will recursively deal with the array elements from there.
			   By the time we get back, we're done.  */

			/* Create a new darray containing the array lengths.  */
			mint_const_u *new_darray_u = new mint_const_u[darray_len];
			mint_const *new_darray = new mint_const[darray_len];
			for (int i = 0; i < darray_len; i++)
			{
				if (darray_poss[i])
				{
					assert(darray[i]->kind == MINT_CONST_ARRAY);
					new_darray_u[i].kind = MINT_CONST_INT;
					new_darray_u[i].mint_const_u_u.const_int =
						darray[i]->mint_const_u_u.const_array.mint_const_array_len;
					new_darray[i] = &new_darray_u[i];
				}
			}

			/* Unmarshal the array length
			   and produce a switch statement on the possibilities.  */
			mu_decode_const_state len_state = *this;
			len_state.darray = new_darray;
			mu_decode_const_array_functor ff;
			ff.state = this;
			ff.sub = f;
			ff.adef = adef;
			ff.el = 0;
			len_state.decode(must, &ff, adef->length_type);

			break;
		}
		default:
			panic("mu_decode_const: unknown mint_def_kind %d", def->kind);
	}
}

struct mu_decode_const_leaf_functor : public mu_decode_const_functor
{
	void func(mu_state *must, int *darray_poss);

	cast_expr cookie_var;
	int darray_len;
};
void mu_decode_const_leaf_functor::func(mu_state *must, int *darray_poss)
{
	/* Find the appropriate cookie value
	   indicating the case eventually selected.
	   (There better be only one!)
	   If no possibilities remain, use the cookie value -1.  */
	int cookie = -1;
	for (int i = 0; i < darray_len; i++)
	{
		if (darray_poss[i])
		{
			assert(cookie < 0);
			cookie = i;
		}
	}

	/* Create a statement to assign the cookie value.  */
	must->add_stmt(cast_new_stmt_expr(
		cast_new_expr_assign(
			cookie_var,
			cast_new_expr_lit_int(cookie, 0))));
}


/* The darray is the list of the of constants to compare against
   The itype is the mint type of the constant to be decoded */
void mu_state::mu_decode_const(mint_const *darray, int darray_len, mint_ref itype,
			       cast_expr cookie_var)
{
	assert(op & MUST_DECODE);

	int *darray_poss = new int[darray_len];
	for (int i = 0; i < darray_len; i++)
		darray_poss[i] = 1;

	mu_decode_const_leaf_functor f;
	f.cookie_var = cookie_var;
	f.darray_len = darray_len;

	mu_decode_const_state st;
	st.darray = darray;
	st.darray_poss = darray_poss;
	st.darray_len = darray_len;
	st.decode(this, &f, itype);
}

#endif /* 0 */

