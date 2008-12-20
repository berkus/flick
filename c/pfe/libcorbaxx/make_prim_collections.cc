/*
 * Copyright (c) 1995, 1996, 1998, 1999 The University of Utah and
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
#include <string.h>

#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include <mom/c/pg_corbaxx.hh>

/*
 * The method fills the `pg_state::prim_collections' array with descriptions
 * of this presentation generator's primitive and built-in presented types.
 */
void pg_corbaxx::make_prim_collections()
{
	p_type_collection *ptc, *ptc_ref;
	p_scope_node *psn;
	p_type_node *ptn, *new_ptn;
	cast_type type;
	
	cast_scoped_name corba_root = cast_new_scoped_name("CORBA", NULL);
	pres_c_mapping direct_map = pres_c_new_mapping(PRES_C_MAPPING_DIRECT);
	cast_scope *corba_scope;
	
	/* Extra stuff, used to build the `string' collection. */
	tag_list *tl, *main_tl;
	union tag_data_u data;
	tag_data *pt_td;
	
	int cdef, i;
	
	corba_scope = (cast_scope *)mustmalloc(sizeof(cast_scope));
	*corba_scope = cast_new_scope(0);
	cdef = cast_add_def(&out_pres->cast,
			    corba_root,
			    CAST_SC_NONE,
			    CAST_NAMESPACE,
			    pg_channel_maps[PG_CHANNEL_CLIENT_DECL]
			    [builtin_file],
			    CAST_PROT_NONE);
	out_pres->cast.cast_scope_val[cdef].u.cast_def_u_u.new_namespace =
		corba_scope;
	
	/* Initialize `prim_collections'. */
	for (i = 0; i < PRIM_COLLECTION_MAX; ++i)
		prim_collections[i] = 0;
	
	/*********************************************************************/
	
	/*
	 * Create the ``definition'' presentations of the types, i.e., the
	 * basic presentations of the types.  (For some of the more complex
	 * types, e.g., strings, we do a little more.)
	 */
	
	/* Define the presented type corresponding to `void'. */
	ptc = new p_type_collection;
	ptc->set_name("void");
	
	psn = new p_scope_node;
	psn->set_name("default");
	psn->set_scope(&out_pres->cast);
	psn->set_scope_name(corba_root);
	ptc->add_scope(psn);
	
	ptn = new p_type_node;
	ptn->set_name("definition");
	ptn->set_format("%s");
	ptn->set_type(cast_new_type(CAST_TYPE_VOID));
	ptn->set_mapping(direct_map);
	ptc->add_type("default", ptn);
	
	prim_collections[PRIM_COLLECTION_VOID] = ptc;
	
	/* Define the presented type corresponding to `boolean'. */
	/* This is `CORBA::Boolean'. */
	ptc = new p_type_collection;
	ptc->set_name("Boolean");
	
	psn = new p_scope_node;
	psn->set_name("default");
	psn->set_scope_name(corba_root);
	ptc->add_scope(psn);
	
	ptn = new p_type_node;
	ptn->set_name("definition");
	ptn->set_format("%s");
	ptn->set_type(cast_new_prim_alias(CAST_PRIM_CHAR,
					  CAST_MOD_UNSIGNED,
					  cast_new_scoped_name("CORBA",
							       "Boolean",
							       NULL)
		));
	ptn->set_mapping(direct_map);
	ptc->add_type("default", ptn);
	
	prim_collections[PRIM_COLLECTION_BOOLEAN] = ptc;
	
	/* Define the presented type corresponding to `char'. */
	/* This is `CORBA::Char'. */
	ptc = new p_type_collection;
	ptc->set_name("Char");
	
	psn = new p_scope_node;
	psn->set_name("default");
	psn->set_scope_name(corba_root);
	ptc->add_scope(psn);
	
	ptn = new p_type_node;
	ptn->set_name("definition");
	ptn->set_format("%s");
	ptn->set_type(cast_new_prim_alias(CAST_PRIM_CHAR,
					  CAST_MOD_SIGNED /* XXX? */,
					  cast_new_scoped_name("CORBA",
							       "Char",
							       NULL)
		));
	ptn->set_mapping(direct_map);
	ptc->add_type("default", ptn);
	
	prim_collections[PRIM_COLLECTION_CHAR] = ptc;
	
	/* Define the presented type corresponding to `signed char'. */
	/* XXX --- This is also `CORBA::Char'. */
	prim_collections[PRIM_COLLECTION_SCHAR] = ptc;
	
	/* Define the presented type corresponding to `unsigned char'. */
	/* XXX --- This is also `CORBA::Char'. */
	prim_collections[PRIM_COLLECTION_UCHAR] = ptc;
	
	/* Define the presented type corresponding to `octet'. */
	/* This is `CORBA::Octet'. */
	ptc = new p_type_collection;
	ptc->set_name("Octet");
	
	psn = new p_scope_node;
	psn->set_name("default");
	psn->set_scope_name(corba_root);
	ptc->add_scope(psn);
	
	ptn = new p_type_node;
	ptn->set_name("definition");
	ptn->set_format("%s");
	ptn->set_type(cast_new_prim_alias(CAST_PRIM_CHAR,
					  CAST_MOD_UNSIGNED,
					  cast_new_scoped_name("CORBA",
							       "Octet",
							       NULL)
		));
	ptn->set_mapping(direct_map);
	ptc->add_type("default", ptn);
	
	prim_collections[PRIM_COLLECTION_OCTET] = ptc;
	
	/* Define the presented type corresponding to `short'. */
	/* This is `CORBA::Short'. */
	ptc = new p_type_collection;
	ptc->set_name("Short");
	
	psn = new p_scope_node;
	psn->set_name("default");
	psn->set_scope_name(corba_root);
	ptc->add_scope(psn);
	
	ptn = new p_type_node;
	ptn->set_name("definition");
	ptn->set_format("%s");
	ptn->set_type(cast_new_prim_alias(CAST_PRIM_INT,
					  (CAST_MOD_SIGNED | CAST_MOD_SHORT),
					  cast_new_scoped_name("CORBA",
							       "Short",
							       NULL)
		));
	ptn->set_mapping(direct_map);
	ptc->add_type("default", ptn);
	
	prim_collections[PRIM_COLLECTION_SHORT] = ptc;
	
	/* Define the presented type corresponding to `unsigned short'. */
	/* This is `CORBA::UShort'. */
	ptc = new p_type_collection;
	ptc->set_name("UShort");
	
	psn = new p_scope_node;
	psn->set_name("default");
	psn->set_scope_name(corba_root);
	ptc->add_scope(psn);
	
	ptn = new p_type_node;
	ptn->set_name("definition");
	ptn->set_format("%s");
	ptn->set_type(cast_new_prim_alias(CAST_PRIM_INT,
					  (CAST_MOD_UNSIGNED | CAST_MOD_SHORT),
					  cast_new_scoped_name("CORBA",
							       "UShort",
							       NULL)
		));
	ptn->set_mapping(direct_map);
	ptc->add_type("default", ptn);
	
	prim_collections[PRIM_COLLECTION_USHORT] = ptc;
	
	/* Define the presented type corresponding to `long'. */
	/* This is `CORBA::Long'. */
	ptc = new p_type_collection;
	ptc->set_name("Long");
	
	psn = new p_scope_node;
	psn->set_name("default");
	psn->set_scope_name(corba_root);
	ptc->add_scope(psn);
	
	ptn = new p_type_node;
	ptn->set_name("definition");
	ptn->set_format("%s");
	ptn->set_type(cast_new_prim_alias(CAST_PRIM_INT,
					  (CAST_MOD_SIGNED | CAST_MOD_LONG),
					  cast_new_scoped_name("CORBA",
							       "Long",
							       NULL)
		));
	ptn->set_mapping(direct_map);
	ptc->add_type("default", ptn);
	
	prim_collections[PRIM_COLLECTION_LONG] = ptc;
	
	/* Define the presented type corresponding to `unsigned long'. */
	/* This is `CORBA::ULong'. */
	ptc = new p_type_collection;
	ptc->set_name("ULong");
	
	psn = new p_scope_node;
	psn->set_name("default");
	psn->set_scope_name(corba_root);
	ptc->add_scope(psn);
	
	ptn = new p_type_node;
	ptn->set_name("definition");
	ptn->set_format("%s");
	ptn->set_type(cast_new_prim_alias(CAST_PRIM_INT,
					  (CAST_MOD_UNSIGNED | CAST_MOD_LONG),
					  cast_new_scoped_name("CORBA",
							       "ULong",
							       NULL)
		));
	ptn->set_mapping(direct_map);
	ptc->add_type("default", ptn);
	
	prim_collections[PRIM_COLLECTION_ULONG] = ptc;
	
	/* Define the presented type corresponding to `long long'. */
	/* This is `CORBA::LongLong'. */
	ptc = new p_type_collection;
	ptc->set_name("LongLong");
	
	psn = new p_scope_node;
	psn->set_name("default");
	psn->set_scope_name(corba_root);
	ptc->add_scope(psn);
	
	ptn = new p_type_node;
	ptn->set_name("definition");
	ptn->set_format("%s");
	ptn->set_type(cast_new_prim_alias(CAST_PRIM_INT,
					  (CAST_MOD_SIGNED
					   | CAST_MOD_LONG_LONG),
					  cast_new_scoped_name("CORBA",
							       "LongLong",
							       NULL)
		));
	ptn->set_mapping(direct_map);
	ptc->add_type("default", ptn);
	
	prim_collections[PRIM_COLLECTION_LONGLONG] = ptc;
	
	/* Define the presented type corresponding to `unsigned long long'. */
	/* This is `CORBA::ULongLong'. */
	ptc = new p_type_collection;
	ptc->set_name("ULongLong");
	
	psn = new p_scope_node;
	psn->set_name("default");
	psn->set_scope_name(corba_root);
	ptc->add_scope(psn);
	
	ptn = new p_type_node;
	ptn->set_name("definition");
	ptn->set_format("%s");
	ptn->set_type(cast_new_prim_alias(CAST_PRIM_INT,
					  (CAST_MOD_UNSIGNED
					   | CAST_MOD_LONG_LONG),
					  cast_new_scoped_name("CORBA",
							       "ULongLong",
							       NULL)
		));
	ptn->set_mapping(direct_map);
	ptc->add_type("default", ptn);
	
	prim_collections[PRIM_COLLECTION_ULONGLONG] = ptc;
	
	/* Define the presented type corresponding to `float'. */
	/* This is `CORBA::Float'. */
	ptc = new p_type_collection;
	ptc->set_name("Float");
	
	psn = new p_scope_node;
	psn->set_name("default");
	psn->set_scope_name(corba_root);
	ptc->add_scope(psn);
	
	ptn = new p_type_node;
	ptn->set_name("definition");
	ptn->set_format("%s");
	ptn->set_type(cast_new_prim_alias(CAST_PRIM_FLOAT,
					  0,
					  cast_new_scoped_name("CORBA",
							       "Float",
							       NULL)
		));
	ptn->set_mapping(direct_map);
	ptc->add_type("default", ptn);
	
	prim_collections[PRIM_COLLECTION_FLOAT] = ptc;
	
	/* Define the presented type corresponding to `double'. */
	/* This is `CORBA::Double'. */
	ptc = new p_type_collection;
	ptc->set_name("Double");
	
	psn = new p_scope_node;
	psn->set_name("default");
	psn->set_scope_name(corba_root);
	ptc->add_scope(psn);
	
	ptn = new p_type_node;
	ptn->set_name("definition");
	ptn->set_format("%s");
	ptn->set_type(cast_new_prim_alias(CAST_PRIM_DOUBLE,
					  0,
					  cast_new_scoped_name("CORBA",
							       "Double",
							       NULL)
		));
	ptn->set_mapping(direct_map);
	ptc->add_type("default", ptn);
	
	prim_collections[PRIM_COLLECTION_DOUBLE] = ptc;
	
	/* Define the presented type corresponding to `string'. */
	/* This is `CORBA::String'. */
	ptc = new p_type_collection;
	ptc->set_name("String");
	
	psn = new p_scope_node;
	psn->set_name("default");
	psn->set_scope(corba_scope);
	psn->set_scope_name(corba_root);
	ptc->add_scope(psn);
	
	ptn = new p_type_node;
	ptn->set_name("definition");
	ptn->set_format("");
	ptn->set_type(cast_new_pointer_type(cast_new_prim_type(CAST_PRIM_CHAR,
							       0)
		));
	/* ptn->set_mapping(...); not required. */
	ptc->add_type("default", ptn);
	
	ptn = new p_type_node;
	ptn->set_name("smart_pointer");
	ptn->set_format("%s_var");
	ptn->set_type(cast_new_class_type(0));
	/* ptn->set_mapping(...); not required. */
	ptc->add_type("default", ptn);
	
	ptn = new p_type_node;
	ptn->set_name("out_pointer");
	ptn->set_format("%s_out");
	ptn->set_type(cast_new_class_type(0));
	/* ptn->set_mapping(...); not required. */
	ptc->add_type("default", ptn);
	
	pt_td = &(find_tag(out_pres->pres_attrs, "pres_type")->data);
	ptc_ref = new p_type_collection;
	tl = create_tag_list(0);
	data.tl = tl;
	ptc_ref->set_channel(pg_channel_maps[PG_CHANNEL_CLIENT_DECL]
			     [builtin_file]);
	ptc_ref->set_attr_index(append_tag_data(pt_td, data));
	ptc_ref->set_tag_list(tl);
	ptc_ref->set_id("IDL:CORBA/String:1.0");
	
	psn = new p_scope_node;
	psn->set_name("default");
	psn->set_scope(corba_scope);
	psn->set_scope_name(corba_root);
	
	ptc_ref->add_scope(psn);
	ptc_ref->set_name("String");
	ptc_ref->set_collection_ref(ptc);
	
	ptc->set_attr_index(ptc_ref->get_attr_index());
	main_tl = create_tag_list(0);
	add_tag(tl, "idl_type", TAG_STRING, "string");
	add_tag(tl, "size", TAG_STRING, "variable");
	add_tag(tl, "main", TAG_TAG_LIST, main_tl);
	add_tag(main_tl, "form", TAG_STRING, "string");
	
	type = cast_new_prim_type(CAST_PRIM_CHAR, 0);
	type = cast_new_pointer_type(type);
	current_scope_name = corba_root;
	add_function(main_tl,
		     cast_new_scoped_name("string_dup", NULL),
		     PFA_FunctionKind, "string_dup()",
		     PFA_Scope, corba_scope,
		     PFA_ReturnType, type,
		     PFA_StorageClass, CAST_SC_STATIC,
		     PFA_DeclChannel,
		     pg_channel_maps[PG_CHANNEL_CLIENT_DECL][builtin_file],
		     PFA_ImplChannel,
		     pg_channel_maps[PG_CHANNEL_CLIENT_IMPL][builtin_file],
		     PFA_TAG_DONE);
	add_function(main_tl,
		     cast_new_scoped_name("string_free", NULL),
		     PFA_FunctionKind, "string_free()",
		     PFA_Scope, corba_scope,
		     PFA_ReturnType, cast_new_type(CAST_TYPE_VOID),
		     PFA_StorageClass, CAST_SC_STATIC,
		     PFA_DeclChannel,
		     pg_channel_maps[PG_CHANNEL_CLIENT_DECL][builtin_file],
		     PFA_ImplChannel,
		     pg_channel_maps[PG_CHANNEL_CLIENT_IMPL][builtin_file],
		     PFA_TAG_DONE);
	current_scope_name = null_scope_name;
	
	ptc_ref->define_types();
	
	prim_collections[PRIM_COLLECTION_STRING] = ptc;
	
	/* Define the presented type corresponding to `type tag'. */
	/* This is `CORBA::TypeCode'. */
	ptc = new p_type_collection;
	ptc->set_name("TypeCode");
	
	psn = new p_scope_node;
	psn->set_name("default");
	psn->set_scope_name(corba_root);
	ptc->add_scope(psn);
	
	ptn = new p_type_node;
	ptn->set_name("definition");
	ptn->set_format("%s");
	ptn->set_type(
		cast_new_type_scoped_name(cast_new_scoped_name("CORBA",
							       "TypeCode",
							       NULL)
			));
	ptn->set_mapping(pres_c_new_mapping(PRES_C_MAPPING_TYPE_TAG));
	ptc->add_type("default", ptn);
	
	ptn = new p_type_node;
	ptn->set_name("smart_pointer");
	ptn->set_format("%s_var");
	ptn->set_type(
		cast_new_type_scoped_name(cast_new_scoped_name("CORBA",
							       "TypeCode_var",
							       NULL)
			));
	ptn->set_mapping(pres_c_new_mapping(PRES_C_MAPPING_TYPE_TAG));
	ptc->add_type("default", ptn);
	
	ptn = new p_type_node;
	ptn->set_name("out_pointer");
	ptn->set_format("%s_out");
	ptn->set_type(
		cast_new_type_scoped_name(cast_new_scoped_name("CORBA",
							       "TypeCode_out",
							       NULL)
			));
	/* ptn->set_mapping(...); not required. */
	ptc->add_type("default", ptn);
	
	ptn = new p_type_node;
	ptn->set_name("pointer");
	ptn->set_format("%s_ptr");
	ptn->set_type(
		cast_new_type_scoped_name(cast_new_scoped_name("CORBA",
							       "TypeCode_ptr",
							       NULL)
			));
	ptn->set_mapping(pres_c_new_mapping(PRES_C_MAPPING_TYPE_TAG));
	ptc->add_type("default", ptn);
	
	prim_collections[PRIM_COLLECTION_TYPE_TAG] = ptc;
	
	/* Define the presented type corresponding to `typed any'. */
	/* This is `CORBA::Any'. */
	ptc = new p_type_collection;
	ptc->set_name("Any");
	
	psn = new p_scope_node;
	psn->set_name("default");
	psn->set_scope_name(corba_root);
	ptc->add_scope(psn);
	
	ptn = new p_type_node;
	ptn->set_name("definition");
	ptn->set_format("%s");
	ptn->set_type(cast_new_type_scoped_name(cast_new_scoped_name("CORBA",
								     "Any",
								     NULL)
		));
	ptn->set_mapping(pres_c_new_mapping(PRES_C_MAPPING_TYPED));
	ptc->add_type("default", ptn);
	
	ptn = new p_type_node;
	ptn->set_name("out_pointer");
	ptn->set_format("%s_out");
	ptn->set_type(cast_new_type_scoped_name(cast_new_scoped_name("CORBA",
								     "Any_out",
								     NULL)
		));
	/* ptn->set_mapping(...); not required. */
	ptc->add_type("default", ptn);
	
	prim_collections[PRIM_COLLECTION_TYPED_ANY] = ptc;
	
	/*********************************************************************/
	
	/*
	 * Now make the `pres_c_presentation_type' structures for each type,
	 * and set up the basic set of tags about each type.
	 *
	 * Additionally, make the ``out_pointer'' presentations of each type.
	 * The ``out_pointer'' presentation type is used when values of a type
	 * are passed as `out' parameters; see `pg_corbaxx::p_param_type'.
	 * (Note that we do not need to make mappings for our ``out_pointers'';
	 * the appropriate mappings are built by `pg_corbaxx::p_param_type'
	 * from the ``definition'' mappings created above.
	 */
	for (i = 0; i < PRIM_COLLECTION_MAX; i++) {
		ptc_ref = prim_collections[i];
		if (!ptc_ref || (i == PRIM_COLLECTION_STRING))
			continue;
		
		ptc = new p_type_collection;
		
		ptc->set_name(ptc_ref->get_name());
		ptc->set_channel(pg_channel_maps[PG_CHANNEL_CLIENT_DECL]
				 [builtin_file]);
		ptc->set_protection(CAST_PROT_NONE);
		
		psn = new p_scope_node;
		psn->set_name("default");
		psn->set_scope(corba_scope);
		psn->set_scope_name(corba_root);
		ptc->add_scope(psn);
		
		tl = create_tag_list(0);
		data.tl = tl;
		ptc_ref->set_attr_index(append_tag_data(pt_td, data));
		ptc_ref->set_tag_list(tl);
		ptc->set_collection_ref(ptc_ref);
		add_tag(tl, "idl_type", TAG_STRING, ptc->get_name());
		if (!strcmp(ptc_ref->get_name(), "Any")
		    || !strcmp(ptc_ref->get_name(), "TypeCode"))
			/* `Any's and `TypeCode's are variable-size. */
			add_tag(tl, "size", TAG_STRING, "variable");
		else
			add_tag(tl, "size", TAG_STRING, "fixed");
		main_tl = create_tag_list(0);
		add_tag(main_tl, "form", TAG_STRING, "atomic");
		add_tag(tl, "main", TAG_TAG_LIST, main_tl);
		/*
		 * Create the ``out_pointer'' presentation of this type, unless
		 * it was made specially above.
		 */
		if (!ptc->find_type("out_pointer")) {
			ptn = ptc->find_type("definition");
			new_ptn = new p_type_node;
			new_ptn->set_name("out_pointer");
			new_ptn->set_format("%s_out");
			
			new_ptn->set_type(
				cast_new_reference_type(ptn->get_type())
				);
			/* new_ptn->set_mapping(...); not required. */
			
			ptc->add_type("default", new_ptn);
		}
		/* `TypeCode' is already set up. */
		if (strcmp(ptc_ref->get_name(), "TypeCode"))
			ptc->define_types();
	}
}

/* End of file. */

