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

class mach3_be_state : public be_state
{
	
public:
	mach3_be_state();
	
protected:
	virtual be_flags get_default_be_flags();
	
};

struct mach3_mu_state : public mem_mu_state
{
	/* True if we're marshaling the elements of an array
	   that actually uses the array feature of the Mach3 message format
	   (i.e. msgt_number > 1).  */
	int mach3_array;

	/* True if we're marshaling forced out-of-line data. */
	int mach3_outofline;
	
	/* True if we're encoding an error.
	   This is used in combination with mach3_array to suppress the
	   "_array" appendage to back-end and encode names. */
	int mach3_error;
	
	/* True if we've seen anything that requires MACH_MSGH_BITS_COMPLEX
	   to be set for this message.  */
	cast_expr is_complex;

	/* Last cast_expr used for determining the last out-of-line check
	   variable */
	cast_expr last_ool_check;

	/* 1 if the next simple marshaled item is expected to be the IDL ID;
	   2 if it is expected to be the MIG message code;
	   3 if it is expected to be the MIG operation return code;
	   0 at any other time.  */
	int id_expected;

	/* True if we've seen the IDL ID
	   and discovered that we're handling a MIG message.  */
	int is_mig_message;

	/* True if we should *NOT* emit m/u code for the current entity. */
	int inhibit_marshal;
	
	/*
	 * if descending the type tag branch of an inline_typed,
	 * marshaling_inline_typed is true, causing mu_mapping_type_tag to
	 * simply store the tag's cast_expr into tag_cexpr for later use.
	 */
	int marshaling_inline_typed;
	cast_expr tag_cexpr;
	cast_expr msg_option_expr;
	cast_expr timeout_expr;
	
	mach3_mu_state(be_state *_state, mu_state_op _op, int _assumptions,
		       const char *which); 
	mach3_mu_state(const mach3_mu_state &must);
	mu_state *clone(); 
	mu_state *another(mu_state_op _op);
	
	virtual const char *get_be_name();
	virtual const char *get_encode_name();
	virtual const char *get_mu_stream_name();
	
	virtual int get_most_packable() 
	{
		return 1;
	}
	
	/*
	 * Set `id_expected' based on the type of the upcoming operation
	 * discriminator.
	 */
	virtual void set_id_expected(mint_ref operation_union_itype);
	
	virtual char *get_mach_port_type(pres_c_mapping_reference *rmap,
					 mint_ref itype);
	
	/* Make a marshaling stub */
	void mu_mapping_mstub(cast_expr expr, cast_type ctype, 
			      mint_ref itype, 
			      pres_c_marshal_stub *mstub);

	virtual void mu_mapping_simple(cast_expr expr, cast_type ctype,
				       mint_ref itype);

	virtual void mu_mapping_message_attribute(
		cast_expr expr,
		cast_type ctype,
		mint_ref itype,
		pres_c_mapping_message_attribute *attr_map);
	
	virtual void mu_mapping_argument(cast_expr expr, cast_type ctype,
					 mint_ref itype, pres_c_mapping map);
	
	virtual void mu_inline_typed(inline_state *ist,
				     mint_ref itype, pres_c_inline inl);
	
	virtual void mu_mapping_type_tag(cast_expr expr, cast_type ctype,
					 mint_ref itype);
	
	virtual void mu_mapping_reference(cast_expr expr, cast_type ctype,
					  mint_ref itype,
					  pres_c_mapping_reference *rmap);

	virtual void get_prim_params(mint_ref itype, int *size,
				     int *align_bits, char **macro_name);

	virtual cast_expr mu_get_sizeof(mint_ref itype, cast_type ctype,
					pres_c_mapping map,
					int *size, int *align_bits);
	
	/* special version of mu_pointer_free since mach_vm_deallocate() needs
	   the size passed when freeing as well as when allocating */
	virtual void mu_pointer_free(cast_expr expr, cast_type target_type,
				     char *cname);
	
	/* Helper functions for our version of mu_array() */
	void mu_array_type_descriptor(cast_expr len_expr, int fixed_elts,
				      char *prim_macro_name, int longform,
				      int out_of_line, cast_expr dealloc_expr,
				      cast_expr port_type, cast_expr oolcheck);
	
	void mu_aggregated_array(
		cast_expr array_expr, cast_type array_ctype,
		pres_c_allocation *array_alloc,
		cast_type elem_ctype, mint_ref elem_itype,
		pres_c_mapping elem_map,
		cast_expr len_expr, cast_type len_ctype,
		unsigned long len_min, unsigned long len_max,
		mint_ref prim_itype, int fixed_elts,
		int out_of_line, char *cname);
	
	virtual void mu_array(
		cast_expr array_expr, cast_type array_ctype,
		cast_type elem_ctype,
		mint_ref elem_itype,
		pres_c_mapping elem_map,
		char *cname);
	
	virtual int mu_array_encode_terminator(char *cname);
	
	virtual cast_stmt make_error(int err_val);
	
#if 0
	/* No longer used.  See comment in misc.cc */
	virtual void mu_union_case(functor *f, mint_const discrim_val);
#endif
	
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
	
	virtual void mu_inline_message_attribute(inline_state *ist,
						 mint_ref itype,
						 pres_c_inline inl);
	
	virtual void mu_server_func_client(pres_c_server_func *sfunc);
	
	virtual void mu_end();
};

struct mach3_target_mu_state : public target_mu_state
{
	/* The following variables are to store any special
           (polymorphic) parameters to the encode_target macro. */
	cast_expr target_remote;
	cast_expr target_local;
	
	mach3_target_mu_state(be_state *_state, mu_state_op _op,
			      int _assumptions, const char *which);
	mach3_target_mu_state(const mach3_target_mu_state &must);
	virtual mu_state *clone(); 
	virtual const char *get_be_name();
	virtual const char *get_encode_name();
	virtual void mu_mapping_type_tag(cast_expr expr, cast_type ctype,
					 mint_ref itype);
	virtual void mu_mapping_reference(cast_expr expr, cast_type ctype,
					  mint_ref itype,
					  pres_c_mapping_reference *rmap);
};

struct mach3_client_mu_state : public client_mu_state
{
	/* The following variables are to store any special
           (polymorphic) parameters to the encode_target macro. */
	cast_expr client_remote;
	cast_expr client_local;
	
	mach3_client_mu_state(be_state *_state, mu_state_op _op,
			      int _assumptions, const char *which);
	mach3_client_mu_state(const mach3_client_mu_state &must);
	virtual mu_state *clone(); 
	virtual const char *get_be_name();
	virtual const char *get_encode_name();
	virtual void mu_mapping_type_tag(cast_expr expr, cast_type ctype,
					 mint_ref itype);
	virtual void mu_mapping_reference(cast_expr expr, cast_type ctype,
					  mint_ref itype,
					  pres_c_mapping_reference *rmap);
};


/*****************************************************************************/

/*
 * `remove_idl_and_interface_ids' strips away the ``collapsed union'' stuff
 * that encodes IDL and interface information.  Mach3MIG client and server
 * stubs don't need to encode this information because it is manifest in the
 * object references.
 */

void remove_idl_and_interface_ids(
	pres_c_1 *pres,
	mint_ref in_itype, pres_c_inline in_inline,
	mint_ref *out_itype, pres_c_inline *out_inline);

#if 0
void remove_operation_id(
	pres_c_1 *pres,
	mint_ref in_itype, pres_c_inline in_inline,
	mint_ref *out_itype, pres_c_inline *out_inline);
#endif

/* End of file. */

