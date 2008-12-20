/*
 * Copyright (c) 1996, 1997, 1998, 1999 The University of Utah and
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

/*
 * These are the (symbolic) values that are returned by the server dispatch
 * functions.  I'm not sure where these symbols are actually defined in the
 * Fluke source tree...
 */

#define FLUKE_SERVER_NORMAL_REPLY_VALUE ("DISPATCH_ACK_SEND")
#define FLUKE_SERVER_NO_REPLY_VALUE     ("DISPATCH_NO_REPLY")

/*****************************************************************************/

class fluke_be_state : public be_state
{
	
public:
	fluke_be_state();
	
};

/*****************************************************************************/

struct fluke_mu_state : public mem_mu_state
{
	/*
	 * `gather_allocator' is the allocator that must be used in order for
	 * data to be marshaled by gathering.  See `mu_array.cc'.
	 *
	 * ANSI C++ forbids putting the initializer here, so we put it in
	 * `mu_array.cc', where it is used.  Blech!
	 */
	static const char * const gather_allocator /* = "fdev" */;
	
	/*********************************************************************/
	
	fluke_mu_state(be_state *_state, mu_state_op _op, int _assumptions,
		       const char *which);
	fluke_mu_state(const fluke_mu_state &must);
	mu_state *clone();
	mu_state *another(mu_state_op op);
	
	virtual const char *get_be_name();
	virtual const char *get_encode_name();
	
	virtual int get_most_packable() 
	{
		return 4;
	}
	
	virtual void get_prim_params(mint_ref itype, int *size,
				     int *align_bits, char **macro_name);
	
	/*****/
	
	virtual void mu_server_func_target(pres_c_server_func *sfunc);
	virtual void mu_server_func_reply(pres_c_server_func *sfunc,
					  pres_c_skel *sskel);
	virtual target_mu_state *mu_make_target_mu_state(be_state *_state,
							 mu_state_op op,
							 int assumptions,
	 						 const char *name);
	virtual client_mu_state *mu_make_client_mu_state(be_state *_state,
							 mu_state_op op,
							 int assumptions,
	 						 const char *name);
	/*
	 * This returns a #if (0), #else (1), or #endif (2)
	 * for bit-translation checking for a particular MINT type.
	 */
	virtual cast_stmt mu_bit_translation_necessary(int, mint_ref);
	
	/*****/
	
	virtual void mu_array(
		cast_expr array_expr, cast_type array_ctype,
		cast_type elem_ctype, mint_ref elem_itype,
		pres_c_mapping elem_map, char *cname);
	virtual int mu_array_encode_terminator(char *cname);
	
	virtual void mu_mapping_reference_get_attributes(
		mint_ref itype, pres_c_mapping_reference *rmap,
		int *ref_count_adjust, int *mark_for_cleanup);
	
	virtual void mu_mapping_sid(cast_expr expr,
				    cast_type ctype,
				    mint_ref itype,
				    pres_c_mapping_sid *sid_map);
};


/*****************************************************************************/

struct fluke_target_mu_state : public target_mu_state
{
	fluke_target_mu_state(be_state *_state, mu_state_op _op,
			      int _assumptions, const char *which);
	fluke_target_mu_state(const fluke_target_mu_state &must);
	
	virtual mu_state *clone(); 
	virtual const char *get_be_name();
	virtual const char *get_encode_name();
	
	/*****/
	
	/*
	 * We override this method so that we can find the target object for a
	 * client stub.  See the file `fluke_target_mu_mapping_reference.cc'.
	 */
	virtual void mu_mapping_reference(cast_expr expr, cast_type ctype,
					  mint_ref itype,
					  pres_c_mapping_reference *rmap);
	
	/*****/
	
	/*
	 * The `target_cast_expr' slot holds a CAST expression that refers to
	 * the target object passed to a client stub or server work function.
	 * `target_cast_type' is the CAST type of that object.  The special
	 * version of `mu_mapping_reference' sets these slots.
	 */
	cast_expr target_cast_expr;
	cast_type target_cast_type;
	
};

struct fluke_client_mu_state : public client_mu_state
{
	fluke_client_mu_state(be_state *_state, mu_state_op _op,
			      int _assumptions, const char *which);
	fluke_client_mu_state(const fluke_client_mu_state &must);
	
	virtual mu_state *clone(); 
	virtual const char *get_be_name();
	virtual const char *get_encode_name();
};

/* End of file. */

