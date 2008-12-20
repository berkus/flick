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

#ifndef _trapeze_h

#include <mom/c/pbe.hh>
#include <mom/c/be/mem_mu_state.hh>
#include <mom/c/be/target_mu_state.hh>
#include <mom/c/be/client_mu_state.hh>
#define TRAPEZE_NO_SWAP 0
#define TRAPEZE_SWAP 1

#if 0
/* We should define them this way... */
#  define TRAPEZE_MAX_CONTROL_SIZE	(SHMEM_PAYLOAD_BYTES)
#  define TRAPEZE_MAX_PAYLOAD_SIZE	(LANAI_BUFFER_SIZE)
#else
/* ...but we don't want to need the Trapeze headers here, so... */
#  define TRAPEZE_MAX_CONTROL_SIZE	(120)
#  define TRAPEZE_MAX_PAYLOAD_SIZE	(8192)
#endif

class trapeze_be_state : public be_state
{
	
public:
	trapeze_be_state();
	
};

/*
 * The Trapeze back end implements two different message formats: an IIOP-like
 * format that uses CDR data encoding, and an ONC RPC-like format that uses
 * XDR encoding.
 */
typedef enum {
	TRAPEZE,	/* IIOP-like message format with CDR encoding. */
	TRAPEZE_ONC	/* ONC RPC-like message format with XDR encoding. */
} trapeze_protocol_t;

struct trapeze_mu_state : public mem_mu_state
{
	// This is whether or not the code should byteswap.
	int should_swap;
	cast_expr has_payload;
	trapeze_protocol_t protocol;
	
	trapeze_mu_state(be_state *_state,
			 mu_state_op _op, 
			 int _assumptions, 
			 const char *which, 
			 int swap = TRAPEZE_NO_SWAP);
	
	trapeze_mu_state(const trapeze_mu_state &must);
	mu_state *clone();
	mu_state *another(mu_state_op op);
	
	virtual const char *get_be_name();
	virtual const char *get_encode_name();
	virtual const char *get_mu_stream_name();
	
	virtual unsigned int get_replytoken_index();
	virtual unsigned int get_control_msg_data_size();
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
	
	/* This is used for passing object references other than
	   the self reference */
	virtual void mu_mapping_reference(cast_expr expr, cast_type ctype,
					  mint_ref itype,
					  pres_c_mapping_reference *rmap);
	
	virtual void mu_array(cast_expr array_expr,
			      cast_type array_ctype,
			      cast_type elem_ctype,
			      mint_ref elem_itype,
			      pres_c_mapping elem_map,
			      char *cname);
	
	virtual int mu_array_encode_terminator(char *cname);
	
	virtual void mu_server_func(pres_c_inline inl, mint_ref tn_r,
				    pres_c_server_func *sfunc,
				    pres_c_skel *sskel);
	
	virtual void mu_mapping_system_exception(cast_expr expr,
						 cast_type ctype,
						 mint_ref itype);
	
	/*
	 * The on-the-wire format for system exception data is two four-byte,
	 * network-byte-order integers.  The first value indicates the kind of
	 * exception (basically like the `id' value of a CORBA exception), and
	 * the second indicates whether or not the operation completed
	 * successfully (basically like the `completed' value of a CORBA system
	 * exception).
	 */
	enum { TRAPEZE_SYSTEM_EXCEPTION_ALIGN_BITS = 2 };
	enum { TRAPEZE_SYSTEM_EXCEPTION_BYTES = 8 };
};

struct trapeze_target_mu_state : public target_mu_state
{
	trapeze_protocol_t protocol;

	trapeze_target_mu_state(be_state *_state, mu_state_op _op, int _assumptions, const char *which);
	trapeze_target_mu_state(const trapeze_target_mu_state &must);
	virtual mu_state *clone(); 
	virtual const char *get_be_name();
	virtual const char *get_encode_name();
};

struct trapeze_client_mu_state : public client_mu_state
{
	trapeze_protocol_t protocol;

	trapeze_client_mu_state(be_state *_state, mu_state_op _op, int _assumptions, const char *which);
	trapeze_client_mu_state(const trapeze_client_mu_state &must);
	virtual mu_state *clone(); 
	virtual const char *get_be_name();
	virtual const char *get_encode_name();
};

void replace_operation_ids(pres_c_1 *pres,
			   mint_ref itype,
			   pres_c_inline inl);

trapeze_protocol_t what_protocol(pres_c_1 *pres);

#endif /* _trapeze_h */

/* End of file. */

