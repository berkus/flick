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

#ifndef _pg_fluke_hh
#define _pg_fluke_hh

#include <mom/c/pg_corba.hh>

extern "C" 
{
	mint_const build_op_const(aoi_interface *this_interface,
				  char *this_interface_name,
				  aoi_interface *derived_interface,
				  char *derived_interface_name,
				  aoi_operation *op,
				  int type_of_constant);
	
	mint_const build_exception_const_int(aoi_ref exception_ref,
					     unsigned int exception_num);
};

class pg_fluke : public pg_corba {
protected:
	/*
	 * `fdev_seq_type_name' is the presented name of the ``magic'' sequence
	 * type that is handled specially by the Fluke `p_variable_array_type'.
	 * See that function for more info.
	 *
	 * `fdev_seq_alloc_name' is the name of the allocator used for
	 * instances of the type named by `fdev_seq_type_name'.
	 *
	 * `fdev_seq_alloc_header' is the name of the `#include' file that
	 * contains the prototypes for the special allocator functions.
	 *
	 * ANSI C++ forbids putting the initializers here, so we put them in
	 * `pg_fluke.cc'.
	 *
	 * XXX --- These exist for the benefit of a special presentation hack.
	 * When we get a real presentation modification facility, these special
	 * values can go away.
	 */
	static const char * const fdev_seq_type_name    /* = "..." */;
	static const char * const fdev_seq_alloc_name   /* = "..." */;
	static const char * const fdev_seq_alloc_header /* = "..." */;
	
public:
	pg_fluke();
	
	/*****/
	
	virtual void      p_typedef_def(aoi_type at);
	virtual cast_type p_make_ctypename(aoi_ref ref);
	
	virtual void p_except_type(aoi_exception *as,
				   p_type_collection **out_ptc);
	virtual void p_interface_type(aoi_interface *ai,
				      p_type_collection **out_ptc);
	
	virtual void p_forward_type(p_type_collection **out_ptc);
	
	virtual void p_indirect_type(aoi_ref ref,
				     p_type_collection **out_ptc);
	
	virtual void p_variable_array_type(aoi_array *,
					   p_type_collection **out_ptc);
	virtual void p_any_type(p_type_collection **out_ptc);
	
	/*****/
	
	virtual cast_type p_get_env_struct_type();
	virtual int p_get_exception_discrim();
	cast_expr p_mint_exception_id_const_to_cast(mint_const mint_literal);
	virtual int p_get_exception_void();
	virtual pres_c_inline_atom p_get_user_discrim();
	virtual void p_do_return_union(aoi_operation *ao,
				       pres_c_inline* reply_l4_inl,
				       mint_ref reply_ref,
				       cast_ref cfunc,
				       pres_c_inline_index discrim_idx);
		
	/*****/
	
	virtual void p_param_type(aoi_type at, int mr,
				  aoi_direction dir,
				  cast_type *out_ctype,
				  pres_c_mapping *out_mapping);
	
	/*****/
	
	virtual void p_client_stub_special_params(aoi_operation *ao,
						  stub_special_params *s);
	virtual void p_server_func_special_params(aoi_operation *ao,
						  stub_special_params *s);
	
	/*****/
	
	virtual void preprocess();

	/* Since we use CORBA we need to stop it from adding it's own funcs */
	virtual void p_add_builtin_server_func(aoi_interface *ai,
					       char *name,
					       pres_c_skel *skel);

	virtual void gen_error_mappings();

	virtual void make_prim_collections();
};

#endif // _pg_fluke_hh

/* End of file. */

