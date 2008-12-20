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

#ifndef _iiop_h

#include <mom/c/pbe.hh>
#include <mom/c/be/be_state.hh>
#include <mom/c/be/mem_mu_state.hh>
#include <mom/c/be/target_mu_state.hh>
#include <mom/c/be/client_mu_state.hh>
#define IIOP_NO_SWAP 0
#define IIOP_SWAP 1

class iiop_be_state : public be_state
{

public:
	iiop_be_state();
	
};

struct iiop_mu_state : public mem_mu_state
{
	// This is whether or not the code should byteswap
	int should_swap;
	
	// This indicates where the ``principal'' is encoded
	int principal_mark;
	
	iiop_mu_state(be_state *_state,
		      mu_state_op _op, 
		      int _assumptions, 
		      const char *which, 
		      int swap = IIOP_NO_SWAP);
  
	iiop_mu_state(const iiop_mu_state &must);
	mu_state *clone();
	mu_state *another(mu_state_op op);

	virtual const char *get_be_name();
	virtual const char *get_encode_name();
	virtual int get_most_packable()
	{
		return 4;
	}
	
	virtual void mu_server_func_reply(pres_c_server_func *sfunc,
					  pres_c_skel *sskel);
	
	virtual target_mu_state *mu_make_target_mu_state(be_state *_state,
							 mu_state_op op,
							 int assumptions,
							 const char *which);
	virtual client_mu_state *mu_make_client_mu_state(be_state *_state,
							 mu_state_op op,
							 int assumptions,
							 const char *which);
	
	/*
	 * This returns a #if (0), #else (1), or #endif (2)
	 * for bit-translation checking for a particular MINT type.
	 */
	virtual cast_stmt mu_bit_translation_necessary(int, mint_ref);
	
	virtual void mu_prefix_params();
	
	virtual cast_scoped_name mu_mapping_stub_call_name(int stub_idx);
	
	/* This is used for passing object references other than the
           self reference */
	virtual void mu_mapping_reference(cast_expr expr, cast_type ctype,
					  mint_ref itype,
					  pres_c_mapping_reference *rmap);
	
	virtual int mu_array_encode_terminator(char *cname);
};

struct iiop_target_mu_state : public target_mu_state
{
	// This is whether or not the code should byteswap
	int should_swap;
	
	iiop_target_mu_state(be_state *_state, mu_state_op _op,
			     int _assumptions, const char *which,
			     int swap = IIOP_NO_SWAP);
	iiop_target_mu_state(const iiop_target_mu_state &must);
	virtual mu_state *clone(); 
	virtual const char *get_be_name();
	virtual const char *get_encode_name();
};

struct iiop_client_mu_state : public client_mu_state
{
	// This is whether or not the code should byteswap
	int should_swap;
	
	iiop_client_mu_state(be_state *_state, mu_state_op _op,
			     int _assumptions, const char *which,
			     int swap = IIOP_NO_SWAP);
	iiop_client_mu_state(const iiop_client_mu_state &must);
	virtual mu_state *clone(); 
	virtual const char *get_be_name();
	virtual const char *get_encode_name();
};

#endif /* _iiop_h */

/* End of file. */

