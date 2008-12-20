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

#ifndef _pg_corbaxx_hh
#define _pg_corbaxx_hh

#include <mom/c/pfe.hh>
#include <mom/c/p_type_collection.hh>

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

class pg_corbaxx : public pg_state {
public:
	pg_corbaxx();
	
private:
	const char *sequence_type_string(aoi_type at, aoi *the_aoi,
					 const char *the_name);
protected:
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
	void p_const_def(aoi_const_def *ac);
	cast_expr p_const_translate(aoi_const_def *ac);

	virtual pres_c_inline p_inline_type(aoi_type at,
					    char *name,
					    p_type_collection *inl_ptc,
					    cast_type inl_ctype);
	virtual pres_c_inline p_inline_struct_union(aoi_union *au,
						    p_type_collection *inl_ptc,
						    cast_type inl_ctype);
	virtual void p_sequence_class(p_type_collection *ptc,
				      aoi_array *aa);
	virtual void p_array_dims(tag_list *tl, aoi_array *aa);
	virtual p_type_collection *p_anon_array(char *name,
						p_type_collection *ptc,
						aoi_type at);
	virtual void p_any_funcs(p_type_collection *ptc, aoi_type at);
	virtual void p_array_funcs(p_type_collection *ptc);
	virtual void p_union_case(p_type_collection *inl_ptc,
				  pres_c_inline_struct_union *suinl,
				  cast_scope *class_scope,
				  cast_scope *scope,
				  tag_list *main_tl,
				  aoi_const val,
				  aoi_field *var,
				  int index);
	virtual pres_c_inline p_inline_exception(aoi_exception *ae,
						 p_type_collection *inl_ptc,
						 cast_type inl_ctype);
	virtual void p_usable_type(aoi_type at,
				   p_type_collection **out_ptc,
				   cast_type *out_ctype,
				   pres_c_mapping *out_map);
	virtual void p_indirect_type(aoi_ref ref,
				     p_type_collection **ptc);
	virtual void p_interface_def(aoi_interface *ai);
	virtual void p_interface_funcs(p_type_collection *ptc,
				       cast_scope *scope);
	virtual void p_union_class(p_type_collection *ptc);
	virtual void p_poa_class(p_type_collection *ptc);
	virtual void p_poa_tie(p_type_collection *ptc);
	virtual void p_exception_class(p_type_collection *ptc);
	virtual void p_out_class(p_type_collection *ptc, aoi_type at);
	virtual void p_array_forany(p_type_collection *ptc, aoi_type at);
	virtual void p_type_var(p_type_collection *ptc, aoi_type at);
	virtual void p_namespace_def();
	virtual void p_union_type(aoi_union *au,
				   p_type_collection **out_ptc);
	virtual void p_struct_type(aoi_struct *as,
				   p_type_collection **out_ptc);
	virtual void p_typedef_def(aoi_type at);
	virtual void p_forward_type(p_type_collection **out_ptc);
	
	virtual void p_except_type(aoi_exception *as,
				   p_type_collection **out_ptc);
	virtual void p_fixed_array_type(aoi_array *aa,
					p_type_collection **out_ptc);
	virtual void p_variable_array_type(aoi_array *aa,
					   p_type_collection **out_ptc);
	virtual void p_interface_type(aoi_interface *ai,
				      p_type_collection **out_ptc);
	virtual void p_any_type(p_type_collection **out_ptc);
	virtual void p_type_tag_type(p_type_collection **out_ptc);
	virtual void p_typed_type(aoi_typed *at,
				  p_type_collection **out_ptc);
	
	/*****/
	
	virtual void p_interface_def_typedef(aoi_interface *a);
	
	virtual cast_type p_make_ctypename(aoi_ref ref);
	
	virtual void gen();
	virtual void gen_scope(int scope);
	
	virtual void p_param_type(aoi_type at, mint_ref mr, aoi_direction dir,
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
	
	virtual cast_ref p_skel_cdef(const char *skel_name,
				     const char *skel_type_name);
	virtual void p_server_func_special_params(aoi_operation *ao,
						  stub_special_params *s);
	virtual void p_server_func_return_type(aoi_operation *ao,
					       int mr,
					       cast_type *out_ctype,
					       pres_c_mapping *out_map);
	virtual void p_server_func_make_decl(aoi_interface *ai,
					     aoi_operation *ao,
					     char *opname,
					     cast_func_type *cfunc);
	
	virtual void process_server_params(
		cast_func_type *cp,
		stub_special_params *specials,
		mint_ref request_ref, mint_ref reply_ref,
		aoi_operation *ao,
		pres_c_inline request_l4_inl, pres_c_inline reply_l4_inl,
		pres_c_inline target_inl, pres_c_inline client_inl);
	
	virtual pres_c_allocation p_get_allocation(void);
	
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
	
	virtual p_type_collection *p_new_type_collection(
		const char *base_name);
	virtual void make_prim_collections();
	
	virtual char *add_poa_scope(char *name);
	struct ptr_stack *poa_scope_stack;
	cast_scoped_name current_poa_scope_name;
};

#endif // _pg_corbaxx_hh

/* End of file. */

