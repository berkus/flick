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

#include <assert.h>

#include <mom/compiler.h>
#include <mom/libaoi.h>
#include <assert.h>

#include <mom/compiler.h>
#include <mom/libaoi.h>
#include <mom/libmint.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include <mom/c/pg_corbaxx.hh>

/* Inject built in functions/interfaces for CORBA */
void pg_corbaxx::p_add_builtin_server_func(aoi_interface *ai,
					   char */*name*/,
					   pres_c_skel *skel)
{
	aoi_type str_type, c_at, i_at, b_at;
	aoi_parameter *ap;
	aoi_operation *ao;
	
	ao = (aoi_operation *)mustmalloc(sizeof(aoi_operation));
	ao->name = ir_strlit("_is_a");
	ao->request_code = aoi_new_const_string("_is_a");
	ao->reply_code = aoi_new_const_string("$_is_a");
	ao->flags = 0;
	ap = (aoi_parameter *)mustmalloc(sizeof(aoi_parameter));
	
	c_at = (aoi_type)mustmalloc(sizeof(aoi_type_u));
	c_at->kind = AOI_CHAR;
	c_at->aoi_type_u_u.char_def.bits = 8;
	c_at->aoi_type_u_u.char_def.flags = AOI_CHAR_FLAG_NONE;
	
	i_at = (aoi_type)mustmalloc(sizeof(aoi_type_u));
	i_at->kind = AOI_INTEGER;
	i_at->aoi_type_u_u.integer_def.min = 0;
	i_at->aoi_type_u_u.integer_def.range = ~0;
	
	str_type = (aoi_type)mustmalloc(sizeof(aoi_type_u));
	str_type->kind = AOI_ARRAY;
	str_type->aoi_type_u_u.array_def.element_type = c_at;
	str_type->aoi_type_u_u.array_def.length_type = i_at;
	str_type->aoi_type_u_u.array_def.flgs =
		AOI_ARRAY_FLAG_NULL_TERMINATED_STRING;
	
	b_at = (aoi_type)mustmalloc(sizeof(aoi_type_u));
	b_at->kind = AOI_INTEGER;
	b_at->aoi_type_u_u.integer_def.min = 0;
	b_at->aoi_type_u_u.integer_def.range = 1;
	
	ap->name = ir_strlit("type_id");
	ap->direction = AOI_DIR_IN;
	ap->type = str_type;
	ao->params.params_val = ap;
	ao->params.params_len = 1;
	ao->return_type = b_at;
	ao->exceps.exceps_val = 0;
	ao->exceps.exceps_len = 0;
	
	mint_const idl_dis, int_dis;
	mint_ref int_ref, my_case, new_int, my_ref;
	mint_union_def *my_udef;
	mint_1 *mint = &out_pres->mint; /* This is used by the
					   MINT_*_REF macros */
	
	int_dis = mint_new_const_from_aoi_const( ai->code );
	/* Get the union of CORBA interfaces */
	assert( m(top_union).mint_def_u.union_def.cases.cases_len );
	idl_dis = mint_new_const_int( 1 );
	int_ref = mint_find_union_case( mint, top_union, idl_dis );
	
	/* Add our own interfaces */
	
	mint_const op_rep_dis, op_req_dis;
	mint_ref my_string_ref;
	unsigned int i;
	
	/* Setup MINT stuff */
	my_string_ref = MINT_ARRAY_REF,
		MDA_ElementType, MINT_CHAR_REF, END_REF,
		END_REF;
	
	/* Create the interface structures.
	 *
	 * The reason we separate these out as opposed to creating them in one
	 * big chunk is that the evaluation order of parameters in C is not
	 * specified.  Thus, we may actually create MINT in a different order
	 * across different platforms/compilers.  Although not tragic, it makes
	 * our IR data files different across these platforms, and may lead to
	 * some confusion.  We just make the order explicit here, so the IRs
	 * can remian identical.
	 */
	/* The request. */
	mint_ref req_ref = MINT_STRUCT_REF,
			     MDA_Slot, my_string_ref,
			     END_REF;
	/* The [normal] reply. */
	mint_ref rep_ref = MINT_STRUCT_REF,
			     MDA_Slot,
			       MINT_INTEGER_REF,
			         MDA_Min, 0,
			         MDA_Range, 1,
			         END_REF,
			     END_REF;
	/* The reply union (normal and exceptional). */
	mint_ref rep_u_ref = MINT_UNION_REF,
			       MDA_Discrim, MINT_INTEGER_REF, END_REF,
			       MDA_Case,
			         mint_new_symbolic_const(
					 MINT_CONST_INT,
					 "CORBA::NO_EXCEPTION"),
			         rep_ref,
			       MDA_Case,
			         mint_new_symbolic_const(
					 MINT_CONST_INT,
					 "CORBA::SYSTEM_EXCEPTION"),
			         mint->standard_refs.system_exception_ref,
			       END_REF;
	/* The operation. */
	new_int = MINT_UNION_REF,
		MDA_Discrim, my_string_ref,
		MDA_Case,
		  op_req_dis = mint_new_const_string( "_is_a" ),
		  req_ref,
		MDA_Case,
		  op_rep_dis = mint_new_const_string( "$_is_a" ),
		  rep_u_ref,
		END_REF;
	
	my_ref = mint_find_union_case( mint, int_ref, int_dis );
	for( i = 0;
	     i < m(new_int).mint_def_u.union_def.cases.cases_len;
	     i++ ) {
		/* Check to see if the functions are already there.
		   So either the user put them there or they are
		   there thru inheritance. */
		if( (mint_find_union_case( mint, my_ref,
					   m(new_int).mint_def_u.
					   union_def.cases.
					   cases_val[i].val )) != -1 )
			return;
		my_case = mint_add_union_case( mint, my_ref );
		my_udef = &m(my_ref).mint_def_u.union_def;
		my_udef->cases.cases_val[my_case].val =
			m(new_int).mint_def_u.union_def.cases.
			cases_val[i].val;
		my_udef->cases.cases_val[my_case].var =
			m(new_int).mint_def_u.union_def.cases.
			cases_val[i].var;
	}
	
	tam_interface_record_mint_discrims(ai, ao,
					   op_req_dis,
					   op_rep_dis);
	i = skel->funcs.funcs_len++;
	skel->funcs.funcs_val = (pres_c_func *)
	  mustrealloc(skel->funcs.funcs_val,
		      (sizeof(pres_c_func) * skel->funcs.funcs_len));
	skel->funcs.funcs_val[i] = p_server_func(ai, ao);
}
