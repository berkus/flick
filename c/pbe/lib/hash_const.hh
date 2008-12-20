/*
 * Copyright (c) 1995, 1996, 1997, 1999 The University of Utah and
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

#ifndef _hash_const_hh
#define _hash_const_hh

#include <mom/c/be/mem_mu_state.hh>

class hash_const {
protected:
	mu_state	*must;			// The marshal/unmarshal obj.
	
	// The following data members are the responsibilty of the child.
	
	data_type	*domain;		// The domain of inputs.
	
	int		count;			// The number of inputs.
	functor		**success_functors;	// The values for success of
						//   the parallel domain index
						//   value.
	functor		*failure_functor;
	
	int		have_switch;		// 1 if `switch_var' was
						//   provided by the calling
						//   function.
	cast_type	switch_type;		// The type of the current
						//   switch variable.
	cast_expr	switch_var;		// The switch variable
	cast_expr	switch_slot;		// The current slot of the
						//   switch variable, used for
		                                //   arrays/structs, e.g.,
						//   `s[4]' or `t.a'.
	
protected:
	// This creates a temporary variable to switch upon (switch_type,
	// switch_var).
	virtual void create_switch_var(const data_field &t, mint_ref *itype);
	
	// This converts a data_field into a cast_expr
	virtual cast_expr get_case_val(const data_field &d,
				       unsigned int mostpackable,
				       cast_type *case_type);
	
	// This builds ALL code for the given case (INCLUDING the case <val>:
	// and INCLUDING the break;).
	virtual void add_case(data_type *, functor **, int, cast_expr,
			      cast_type, cast_expr, cast_expr);
	
	// This packs pairs and quadruplets of chars into shorts and ints for
	// unmarshaling.
	virtual data_type *get_optimized_domain(unsigned int *mostpackable);
	
	// This is used to nest hashers.
	hash_const(data_type *d, functor **succ_val, functor *fail_fun,
		   int count,
		   cast_type switch_type, cast_expr switch_var,
		   cast_expr switch_slot, hash_const *hc);
	
	hash_const() {}
	
public:
	hash_const(mu_state *, mint_const *, int, functor **, functor *,
		   cast_type, cast_expr);
	
	virtual ~hash_const() {}
	
	// This builds the code to assign the hash value.
	virtual void hash();
};

class hash_const_struct : public hash_const {
public:
	hash_const_struct(mu_state *, mint_const *, int, cast_expr);
};

class hash_const_int : public hash_const {
public:
	hash_const_int(mu_state *, mint_const *, int, cast_expr);
};

class hash_const_char : public hash_const {
public:
	hash_const_char(mu_state *, mint_const *, int, cast_expr);
};

class hash_const_float : public hash_const {
public:
	hash_const_float(mu_state *, mint_const *, int, cast_expr);
};

class hash_const_array : public hash_const {
public:
	hash_const_array(mu_state *, mint_const *, int, cast_expr);
};

#endif /* _hash_const_hh */

/* End of file. */

