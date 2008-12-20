/*
 * Copyright (c) 1998, 1999 The University of Utah and
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

#ifndef _mom_c_pbe_client_h_
#define _mom_c_pbe_client_h_

#include <mom/types.h>
#include <mom/c/be/mu_state.hh>

/*
 * Client reference marshaler.
 *
 * This is a very simple mu_state class that can be handy for dealing with the
 * `client_i' fields found in most of the stub presentations in PRES_C.  When
 * it digs the object reference out of the C function parameters (or whatever),
 * it simply invokes a macro/function "flick_<be>_<op>_client".
 */
struct client_mu_state : public mu_state
{
	client_mu_state(be_state *_state, mu_state_op _op, int _assumptions,
			const char *which);
	
	/*
	 * We should never need these when marshaling an object client, so
	 * provide implementations that simply panic when called.  That way
	 * subclasses don't have to define them needlessly.
	 */
	virtual mu_state *another(mu_state_op _op);
	virtual int get_most_packable() 
	{
		return 1;
	}
	
	virtual void mu_mapping_simple(cast_expr expr,
				       cast_type ctype,
				       mint_ref itype);
	
	virtual void mu_mapping_reference(cast_expr expr,
					  cast_type ctype,
					  mint_ref itype,
					  pres_c_mapping_reference *rmap);
	
	virtual int mu_array_encode_terminator(char */*cname*/) {return 0;};
	
	virtual target_mu_state *mu_make_target_mu_state(be_state *_state,
							 mu_state_op op,
							 int assumptions,
							 const char *which);
	virtual client_mu_state *mu_make_client_mu_state(be_state *_state,
							 mu_state_op op,
							 int assumptions,
							 const char *which);
};

#endif /* _mom_c_pbe_client_h_ */

/* End of file. */

