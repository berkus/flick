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

#ifndef _pg_sun_hh
#define _pg_sun_hh

#include <mom/c/pfe.hh>

class pg_sun:public pg_state {
public:
	/*
	 * Constructors.
	 */
	pg_sun();
	
private:
protected:
	virtual char *calc_client_stub_name(const char *basic_name);
	virtual char *calc_server_skel_name(const char *basic_name);
	virtual char *calc_server_func_name(const char *basic_name);
	
	/*
	 * Internal name-building functions used by the `calc_*_name'
	 * overrides listed above.
	 */
	virtual char *calc_operation_and_version_id(const char *basic_name);
	virtual char *calc_program_and_version_id();
	
	/* Internal functions. */
	virtual void p_interface_def_typedef(aoi_interface *ai);

	virtual void p_client_stub_special_params(aoi_operation *ao,
						  stub_special_params *s);
	virtual void p_client_stub_return_type(aoi_operation *ao,
					       mint_ref mr,
					       cast_type *out_ctype,
					       pres_c_mapping *out_mapping);
	
	virtual void p_server_func_special_params(aoi_operation *ao,
						  stub_special_params *s);
	virtual void p_server_func_return_type(aoi_operation *ao,
					       mint_ref mr,
					       cast_type *out_ctype,
					       pres_c_mapping *out_mapping);
	
	virtual void p_param_type(aoi_type at, mint_ref mr, aoi_direction dir,
				  cast_type *out_ctype,
				  pres_c_mapping *out_mapping);
	
public:
	virtual void p_variable_array_type(aoi_array *aa,
					   p_type_collection **out_ptc);
	virtual void p_any_type(p_type_collection **out_ptc);
	virtual void p_type_tag_type(p_type_collection **out_ptc);
	virtual void p_typed_type(aoi_typed *at,
				  p_type_collection **out_ptc);
	
	virtual void p_client_stub(aoi_interface *a, aoi_operation *ao);
	virtual pres_c_func p_server_func(aoi_interface *a,
					  aoi_operation *ao);
	virtual int p_skel(aoi_interface *a);
	
	virtual pres_c_allocation p_get_allocation(void);
	virtual void p_do_exceptional_case(
		pres_c_inline_virtual_union_case *vucase,
		mint_union_case *ucase,
		int icase,
		pres_c_inline_index errno_idx);

	virtual void make_prim_collections();
};

#endif /* _pg_sun_hh */

/* End of file */

