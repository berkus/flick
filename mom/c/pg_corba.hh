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

#ifndef _pg_corba_hh
#define _pg_corba_hh

#include <mom/c/pfe.hh>

extern "C" 
{
	/* This computes the repository ID of an `aoi_ref'. */
	char *get_repository_id(aoi_ref aref);
	
	/*
	 * This overrides the default integer representation of an exception
	 * ID.
	 */
	mint_const build_exception_const_string(aoi_ref exception_ref,
						unsigned int exception_num);
};

class dep;

class pg_corba : public pg_state {
public:
	pg_corba();
	
private:
	const char *sequence_type_string(aoi_type at, aoi *the_aoi,
					 const char *the_name);
protected:
	/* Added by KBF - this stuff is for scoped name support in CORBA. */
	virtual char *getscopedname(int aoi_idx);
	
	// This builds a scoped operation name from an interface
	// (scope_Interface_operationname) virtual char
	// *build_scoped_op(aoi_interface *interface, char *intname,
	// char *op); This returns true if the type is a variable type
	// (CORBA 2.0 p14.9)
	virtual int isVariable(aoi_type);
	
	// This is called before we start generating the pres_c
	virtual void build_init_cast(void);
	
public:
	// This deals with Attribute conversion
	virtual void preprocess();
	cast_expr p_const_translate(aoi_const_def *ac);
	
	virtual void p_typedef_def(aoi_type at);
	virtual void p_typedef_array_slice_def(aoi_array *aa);
	
	virtual void p_except_type(aoi_exception *as,
				   p_type_collection **out_ptc);
	virtual void p_variable_array_type(aoi_array *aa,
					   p_type_collection **out_ptc);
	virtual void p_forward_type(p_type_collection **out_ptc);
	virtual void p_interface_type(aoi_interface *ai,
				      p_type_collection **out_ptc);
	virtual void p_enum_type(aoi_enum *ae,
				 p_type_collection **out_ptc);
	virtual void p_any_type(p_type_collection **out_ptc);
	virtual void p_type_tag_type(p_type_collection **out_ptc);
	virtual void p_typed_type(aoi_typed *at,
				  p_type_collection **out_ptc);
	
	/*****/
	
	virtual void p_interface_def_typedef(aoi_interface *a);
	
	virtual cast_type p_make_ctypename(aoi_ref ref);
	
	virtual void p_param_type(aoi_type at, int mr, aoi_direction dir,
				  cast_type *out_ctype,
				  pres_c_mapping *out_map);
	
	virtual void p_client_stub_special_params(aoi_operation *ao,
						  stub_special_params *s);
	
	virtual int  p_msg_marshal_stub_special_params(
		aoi_operation *ao,
		stub_special_params *s,
		int client,
		int request);
	
	virtual int  p_msg_send_stub_special_params(
		aoi_interface *ai,
		stub_special_params *s,
		int request);
	
	virtual void p_client_stub_return_type(aoi_operation *ao,
					       int mr,
					       cast_type *out_ctype,
					       pres_c_mapping *out_map);
	
	virtual void process_client_params(
		cast_func_type *cp,
		stub_special_params *specials,
		mint_ref request_ref, mint_ref reply_ref,
		aoi_operation *ao,
		pres_c_inline request_l4_inl, pres_c_inline reply_l4_inl,
		pres_c_inline target_inl, pres_c_inline client_inl);
	
	virtual void p_server_func_special_params(aoi_operation *ao,
						  stub_special_params *s);
	virtual void p_server_func_return_type(aoi_operation *ao,
					       int mr,
					       cast_type *out_ctype,
					       pres_c_mapping *out_map);
	
	virtual void process_server_params(
		cast_func_type *cp,
		stub_special_params *specials,
		mint_ref request_ref, mint_ref reply_ref,
		aoi_operation *ao,
		pres_c_inline request_l4_inl, pres_c_inline reply_l4_inl,
		pres_c_inline target_inl, pres_c_inline client_inl);
	
	virtual void p_do_exceptional_case(
		pres_c_inline_virtual_union_case *vucase,
		mint_union_case *ucase,
		int icase,
		pres_c_inline_index env_idx);
	
	virtual cast_type p_get_env_struct_type();
	virtual int p_get_exception_discrim();
	virtual int p_get_exception_void();
	virtual pres_c_inline_atom p_get_user_discrim();
	virtual pres_c_inline_void_union_case *p_build_user_exceptions(
		int icase);
	virtual int p_count_user_exceptions(int icase);
	virtual pres_c_mapping p_make_exception_discrim_map(
		char *arglist_name);
	virtual void p_do_return_union(aoi_operation *ao,
				       pres_c_inline* reply_l4_inl,
				       mint_ref reply_ref,
				       cast_ref cfunc,
				       pres_c_inline_index discrim_idx);
	
	/* Needed to add is_a() and other builtin functions */
        virtual void p_add_builtin_server_func(aoi_interface *ai,
					       char *name,
					       pres_c_skel *skel);

	virtual void gen_error_mappings();
	virtual void make_prim_collections();
};

#endif // _pg_corba_hh

/* End of file. */

