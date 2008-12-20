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

#include <mom/c/pbe.hh>
#include <mom/c/be/be_state.hh>
#include <mom/c/be/mem_mu_state.hh>
#include <mom/c/be/target_mu_state.hh>
#include <mom/c/be/client_mu_state.hh>

class sun_be_state : public be_state
{

public:
	sun_be_state();
	
};

struct sun_mu_state : public mem_mu_state
{
	/* This is set when we're processing a packed array, so
	   get_mem_params() knows to marshal into 8-bit bytes instead
	   of 32-bit words.  */
	int in_packed_array;


	sun_mu_state(be_state *_state, mu_state_op _op,
		     int _assumptions, const char *which);
	sun_mu_state(const sun_mu_state &must);
	mu_state *clone();
	mu_state *another(mu_state_op op);

	virtual const char *get_encode_name();
	virtual const char *get_be_name();
	virtual int get_most_packable() {
		return 1;
	}
	
	virtual void get_prim_params(mint_ref itype, int *size,
				     int *align_bits, char **macro_name);

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
	
	/* We need to get control of array processing
	   so we can pack unsigned-byte and 8-bit-character arrays.  */
	virtual void mu_array(cast_expr array_expr, cast_type array_ctype,
			      cast_type elem_ctype, mint_ref elem_itype,
			      pres_c_mapping elem_map, char *cname);
	virtual void mu_array_get_pres_length(char *cname,
					      cast_expr *len_expr,
					      cast_type *len_ctype);
	virtual int mu_array_encode_terminator(char *cname);
	virtual void mu_array_do_msgptr(cast_expr ofs_expr,
					cast_expr ptr_expr,
					cast_type ptr_type,
					cast_type target_type,
					cast_expr len_expr,
					cast_expr size_expr,
					char *cname);
	
	/*
	 * This returns a #if (0), #else (1), or #endif (2)
	 * for bit-translation checking for a particular MINT type.
	 */
	virtual cast_stmt mu_bit_translation_necessary(int, mint_ref);
	
	/* This is used to tack on the opaque authorization crap... */
	virtual void mu_prefix_params();

#if 0
	/* XDR pads strings so we need to adjust the encoded
	   length to the actual length here. */
	virtual cast_expr mu_mapping_string_len(cast_expr ptr,
						cast_type ptr_ctype,
						mint_array_def *arr,
						pres_c_allocation *mem_alloc);
	/* XDR might have to do an alloc for an auto string
	   so we need to accomodate that and handle the abort for it. */
	void mu_mapping_auto_string(cast_expr ptr,
				    cast_type ptr_ctype,
				    mint_array_def *arr,
				    pres_c_allocation *mem_alloc,
				    cast_expr mem_alloc_func_cexpr,
				    cast_expr mem_alloc_abort_func_cexpr,
				    cast_expr /* max_string_length_cexpr */);
#endif
};

struct sun_target_mu_state : public target_mu_state
{
	sun_target_mu_state(be_state *_state, mu_state_op _op,
			    int _assumptions, const char *which);
	sun_target_mu_state(const sun_target_mu_state &must);
	virtual mu_state *clone(); 
	virtual const char *get_be_name();
	virtual const char *get_encode_name();
};

struct sun_client_mu_state : public client_mu_state
{
	sun_client_mu_state(be_state *_state, mu_state_op _op,
			    int _assumptions, const char *which);
	sun_client_mu_state(const sun_client_mu_state &must);
	virtual mu_state *clone(); 
	virtual const char *get_be_name();
	virtual const char *get_encode_name();
};

/* End of file. */

