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

#include <assert.h>

#include <mom/compiler.h>
#include <mom/libaoi.h>
#include <mom/libmint.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include <mom/c/pg_corba.hh>

void pg_corba::p_except_type(aoi_exception *as, p_type_collection **out_ptc)
{
	int cdef = cast_add_def((cast_scope *)top_ptr(scope_stack),
				cast_new_scoped_name(calc_exception_code_name(
					a(cur_aoi_idx).name), NULL),
				CAST_SC_NONE,
				CAST_DEFINE,
				ch(cur_aoi_idx, PG_CHANNEL_CLIENT_DECL),
				CAST_PROT_NONE);
	c(cdef).u.cast_def_u_u.define_as =
		cast_new_expr_lit_string(get_repository_id(cur_aoi_idx));
	pg_state::p_except_type(as, out_ptc);
}

cast_type pg_corba::p_get_env_struct_type() 
{
	static cast_type env = 0;
	int cdef;
	if (env)
		return env;
	env = cast_new_struct_type(0);

	cdef = cast_add_def(&env->cast_type_u_u.agg_type.scope,
			    cast_new_scoped_name("_major", NULL),
			    CAST_SC_NONE,
			    CAST_VAR_DEF,
			    PASSTHRU_DATA_CHANNEL,
			    CAST_PROT_NONE);
	env->cast_type_u_u.agg_type.scope.cast_scope_val[cdef].
		u.cast_def_u_u.var_def.type =
		cast_new_type_name("CORBA_exception_type");
#if 0
	/* XXX - This is what we should do, but the pres_c doesn't handle it */
	except.name = "_value";
	except.type = cast_new_union_type(2);
	cast_union_add_case(except, "_system_except", cast_new_type_name(
		"flick_system_exception"));
	cast_union_add_case(except, "_user_except", cast_new_pointer_type(
		cast_new_type(CAST_TYPE_VOID)));
#else
	/* XXX - This is what we do now */
	cdef = cast_add_def(&env->cast_type_u_u.agg_type.scope,
			    cast_new_scoped_name("_value._user_except", NULL),
			    CAST_SC_NONE,
			    CAST_VAR_DEF,
			    PASSTHRU_DATA_CHANNEL,
			    CAST_PROT_NONE);
	env->cast_type_u_u.agg_type.scope.cast_scope_val[cdef].
		u.cast_def_u_u.var_def.type =
		cast_new_pointer_type(cast_new_type(CAST_TYPE_VOID));
#endif

	cdef = cast_add_def(&env->cast_type_u_u.agg_type.scope,
			    cast_new_scoped_name("_id", NULL),
			    CAST_SC_NONE,
			    CAST_VAR_DEF,
			    PASSTHRU_DATA_CHANNEL,
			    CAST_PROT_NONE);
	env->cast_type_u_u.agg_type.scope.cast_scope_val[cdef].
		u.cast_def_u_u.var_def.type =
		cast_new_pointer_type(cast_new_prim_type(CAST_PRIM_CHAR, 0));
	
	if (gen_client)
		env->cast_type_u_u.agg_type.name
			= cast_new_scoped_name(
				calc_client_stub_environment_type_name(""),
				NULL);
	else if (gen_server)
		env->cast_type_u_u.agg_type.name
			= cast_new_scoped_name(
				calc_server_func_environment_type_name(""),
				NULL);
	else
		panic("In `pg_corba::p_get_env_struct_type', "
		      "generating neither client nor server.");
	
	return env;
}

int pg_corba::p_get_exception_discrim()
{
	return 0;
}

int pg_corba::p_get_exception_void()
{
	return 1;
}

pres_c_inline_atom pg_corba::p_get_user_discrim() 
{
#define ASSIGN_DISCRIM_STATICALLY
#ifdef ASSIGN_DISCRIM_STATICALLY
#undef ASSIGN_DISCRIM_STATICALLY
	/* The discriminator won't be unmarshaled.  It will be assigned the
	   exact literal value upon entering the chosen branch of the union. */
	pres_c_mapping ta
		= pres_c_new_mapping(PRES_C_MAPPING_ELSEWHERE);
#else
	pres_c_mapping ta
		= pres_c_new_mapping(PRES_C_MAPPING_TERMINATED_ARRAY);
	
	ta->pres_c_mapping_u_u.terminated_array.terminator
		= 0;
	ta->pres_c_mapping_u_u.terminated_array.max
		= 0;
	ta->pres_c_mapping_u_u.terminated_array.element_mapping
		= pres_c_new_mapping(PRES_C_MAPPING_DIRECT);
	/*
	 * When we encode, we don't free the string since we assume it to
	 * be a literal string (e.g. ex_CORBA_NO_MEM).  When we decode,
	 * we have to allocate space in order to process it, but we should
	 * quickly deallocate it after locating a static (literal) copy of
	 * that id in a table.  XXX - lookup not done yet.
	 */
	ta->pres_c_mapping_u_u.terminated_array.alloc
		= p_get_allocation();
#endif
	
	// Note the assumption that the identifier is in position 2!!!
	return pres_c_new_inline_atom(2, ta)->pres_c_inline_u_u.atom;
}

pres_c_inline_void_union_case *pg_corba::p_build_user_exceptions(int icase) 
{
	assert(m(icase).kind == MINT_UNION);
	
	mint_union_def *mu = &(m(icase).mint_def_u.union_def);
	int size = mu->cases.cases_len, i;
	aoi_ref j;
	
	pres_c_inline_void_union_case *vuc =
		(pres_c_inline_void_union_case *)
		mustcalloc(sizeof(pres_c_inline_void_union_case) * size);
	
	for (i = 0; i < size; i++) {
		mint_ref cur = mu->cases.cases_val[i].var;
		
		vuc[i].mapping
			= pres_c_new_mapping(PRES_C_MAPPING_STUB);
		
		int stub_idx = pres_c_find_mu_stub(out_pres, cur, 0,
						   vuc[i].mapping,
						   PRES_C_MARSHAL_STUB);
		
		vuc[i].mapping->pres_c_mapping_u_u.mapping_stub.
			mapping_stub_index = stub_idx;

		/*
		 * Now build the type of the exception (just a pointer to a
		 * name.
		 */
		for (j = 0; j < (signed int)in_aoi->defs.defs_len; j++) {
			if (aoi_to_mint_association[j] == cur)
				break;
		}
		assert(j < (signed int)in_aoi->defs.defs_len);
		vuc[i].type = p_make_ctypename(j);
		
		/* Interpose the pointer. */
		pres_c_interpose_indirection_pointer(&vuc[i].type,
						     &vuc[i].mapping,
						     p_get_allocation());
		
		/* Convert the MINT discriminator value for this branch. */
		vuc[i].case_value =
			p_mint_exception_id_const_to_cast(
				mu->cases.cases_val[i].val);
	}
	
	return vuc;
}

int pg_corba::p_count_user_exceptions(int icase)
{
	mint_union_def *mu = &(m(icase).mint_def_u.union_def);
	return mu->cases.cases_len;
}

void pg_corba::p_do_exceptional_case(pres_c_inline_virtual_union_case *vucase,
				     mint_union_case *ucase, int icase,
				     pres_c_inline_index env_idx)
{
	pres_c_mapping map = 0;
	
	mint_ref system_exception_ref
		= out_pres->mint.standard_refs.system_exception_ref;
	
	assert(vucase);
	if (!ucase
	    || (ucase->var != system_exception_ref)) {
		/*
		 * We have the default case, so this is the void union for the
		 * user exceptions.
		 */
		pres_c_inline void_inl
			= pres_c_new_inline(PRES_C_INLINE_VOID_UNION);
		
		void_inl->pres_c_inline_u_u.void_union.discrim
			= p_get_user_discrim();
		void_inl->pres_c_inline_u_u.void_union.void_index
			= p_get_exception_void();
		void_inl->pres_c_inline_u_u.void_union.cases.cases_val
			= p_build_user_exceptions(icase);
		void_inl->pres_c_inline_u_u.void_union.cases.cases_len
			= p_count_user_exceptions(icase);
		void_inl->pres_c_inline_u_u.void_union.dfault
			= 0;
		
		/* Create an allocation semantic suitable for the environment
		   variable. */
		pres_c_allocation alloc;
		
		/* Direction is Unknown, so other cases are invalid. */
		alloc.cases[PRES_C_DIRECTION_IN].allow
			= PRES_C_ALLOCATION_INVALID;
		alloc.cases[PRES_C_DIRECTION_INOUT].allow
			= PRES_C_ALLOCATION_INVALID;
		alloc.cases[PRES_C_DIRECTION_OUT].allow
			= PRES_C_ALLOCATION_INVALID;
		alloc.cases[PRES_C_DIRECTION_RETURN].allow
			= PRES_C_ALLOCATION_INVALID;
		
		/*
		 * This is an indirection pointer:
		 *   Client side never allocs nor deallocs.
		 *   Server side always allocs and deallocs.
		 */
		alloc.cases[PRES_C_DIRECTION_UNKNOWN].allow
			= PRES_C_ALLOCATION_ALLOW;
		if (gen_client)
			alloc.cases[PRES_C_DIRECTION_UNKNOWN].
				pres_c_allocation_u_u.val.flags
				= PRES_C_ALLOC_NEVER | PRES_C_DEALLOC_NEVER;
		else if (gen_server)
			alloc.cases[PRES_C_DIRECTION_UNKNOWN].
				pres_c_allocation_u_u.val.flags
				= PRES_C_ALLOC_ALWAYS | PRES_C_DEALLOC_ALWAYS;
		else
			panic("In pg_corba::p_do_exceptional_case: "
			      "Generating neither client nor server!");
		
		alloc.cases[PRES_C_DIRECTION_UNKNOWN].pres_c_allocation_u_u.
			val.allocator.kind = PRES_C_ALLOCATOR_DONTCARE;
		alloc.cases[PRES_C_DIRECTION_UNKNOWN].pres_c_allocation_u_u.
			val.alloc_init = 0;
		
		map = pres_c_new_mapping(PRES_C_MAPPING_STRUCT);
		map->pres_c_mapping_u_u.struct_i = void_inl;
		
		cast_type toss_type = cast_new_type(CAST_TYPE_VOID);
		pres_c_interpose_indirection_pointer(&toss_type, &map, alloc);
		
	} else if (ucase && (ucase->var == system_exception_ref)) {
		map = pres_c_new_mapping(PRES_C_MAPPING_SYSTEM_EXCEPTION);
		
	} else
		panic("Don't know how to deal with a user exception that is "
		      "not in the default slot!");
	
	*vucase = pres_c_new_inline_atom(env_idx, map);
}

pres_c_mapping pg_corba::p_make_exception_discrim_map(char *arglist_name)
{
	pres_c_mapping discrim_map = pres_c_new_mapping(PRES_C_MAPPING_DIRECT);
	pres_c_interpose_argument(&discrim_map, arglist_name, "discrim");
	
	/* Create an allocation semantic suitable for the environment
	   variable. */
	pres_c_allocation alloc;
	
	/* Direction is Unknown, so other cases are invalid. */
	alloc.cases[PRES_C_DIRECTION_IN].allow = PRES_C_ALLOCATION_INVALID;
	alloc.cases[PRES_C_DIRECTION_INOUT].allow = PRES_C_ALLOCATION_INVALID;
	alloc.cases[PRES_C_DIRECTION_OUT].allow = PRES_C_ALLOCATION_INVALID;
	alloc.cases[PRES_C_DIRECTION_RETURN].allow = PRES_C_ALLOCATION_INVALID;
	
	/*
	 * This is an indirection pointer:
	 *   Client side never allocs nor deallocs.
	 *   Server side always allocs and deallocs.
	 */
	alloc.cases[PRES_C_DIRECTION_UNKNOWN].allow
		= PRES_C_ALLOCATION_ALLOW;
	if (gen_client)
		alloc.cases[PRES_C_DIRECTION_UNKNOWN].
			pres_c_allocation_u_u.val.flags
			= PRES_C_ALLOC_NEVER | PRES_C_DEALLOC_NEVER;
	else if (gen_server)
		alloc.cases[PRES_C_DIRECTION_UNKNOWN].
			pres_c_allocation_u_u.val.flags
			= PRES_C_ALLOC_ALWAYS | PRES_C_DEALLOC_ALWAYS;
	else
		panic("In pg_corba::p_make_exception_discrim_map: "
		      "Generating neither client nor server!");
	
	alloc.cases[PRES_C_DIRECTION_UNKNOWN].pres_c_allocation_u_u.
		val.allocator.kind = PRES_C_ALLOCATOR_DONTCARE;
	alloc.cases[PRES_C_DIRECTION_UNKNOWN].pres_c_allocation_u_u.
		val.alloc_init = 0;
	
	pres_c_mapping map = pres_c_new_mapping(PRES_C_MAPPING_SELECTOR);
	map->pres_c_mapping_u_u.selector.index = 0; /* _major */
	map->pres_c_mapping_u_u.selector.mapping = discrim_map;
	
	cast_type toss_type = cast_new_type(CAST_TYPE_VOID);
	pres_c_interpose_indirection_pointer(&toss_type, &map, alloc);
	
	return map;
}

void pg_corba::p_do_return_union(aoi_operation *ao,
				 pres_c_inline* reply_l4_inl,
				 mint_ref reply_ref,
				 cast_ref cfunc,
				 pres_c_inline_index discrim_idx)
{
	mint_union_def *mu = &(m(reply_ref).mint_def_u.union_def);
	
	assert(mu->cases.cases_len == 2);
	
	mu->cases.cases_val[0].val
		= mint_new_symbolic_const(MINT_CONST_INT,
					  "CORBA_NO_EXCEPTION");
	mu->cases.cases_val[1].val
		= mint_new_symbolic_const(MINT_CONST_INT,
					  "CORBA_SYSTEM_EXCEPTION");
	
	pg_state::p_do_return_union(ao, reply_l4_inl, reply_ref, cfunc,
				    discrim_idx);
}

/* End of file. */

