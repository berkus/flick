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

#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include <mom/c/pg_corba.hh>

/*
 * The method fills the `pg_state::prim_collections' array with descriptions
 * of this presentation generator's primitive and built-in presented types.
 */
void pg_corba::make_prim_collections()
{
	p_type_collection *ptc;
	p_scope_node *psn;
	p_type_node *ptn;
	
	pres_c_mapping direct_map = pres_c_new_mapping(PRES_C_MAPPING_DIRECT);
	
	int i;
	
	/* Initialize `prim_collections'. */
	for (i = 0; i < PRIM_COLLECTION_MAX; ++i)
		prim_collections[i] = 0;
	
	/*********************************************************************/
	
	/*
	 * Create the ``definition'' presentations of the types, i.e., the
	 * basic presentations of the types.
	 */
	
	/* Define the presented type corresponding to `void'. */
	ptc = new p_type_collection;
	ptc->set_name("void");
	
	psn = new p_scope_node;
	psn->set_name("default");
	ptc->add_scope(psn);
	
	ptn = new p_type_node;
	ptn->set_name("definition");
	ptn->set_format("%s");
	ptn->set_type(cast_new_type(CAST_TYPE_VOID));
	ptn->set_mapping(direct_map);
	ptc->add_type("default", ptn);
	
	prim_collections[PRIM_COLLECTION_VOID] = ptc;
	
	/* Define the presented type corresponding to `boolean'. */
	/* This is `CORBA_boolean'. */
	ptc = new p_type_collection;
	ptc->set_name("boolean");
	
	psn = new p_scope_node;
	psn->set_name("default");
	ptc->add_scope(psn);
	
	ptn = new p_type_node;
	ptn->set_name("definition");
	ptn->set_format("%s");
	ptn->set_type(cast_new_prim_alias(CAST_PRIM_CHAR,
					  CAST_MOD_UNSIGNED,
					  cast_new_scoped_name("CORBA_boolean",
							       NULL)
		));
	ptn->set_mapping(direct_map);
	ptc->add_type("default", ptn);
	
	prim_collections[PRIM_COLLECTION_BOOLEAN] = ptc;
	
	/* Define the presented type corresponding to `char'. */
	/* This is `CORBA_char'. */
	ptc = new p_type_collection;
	ptc->set_name("char");
	
	psn = new p_scope_node;
	psn->set_name("default");
	ptc->add_scope(psn);
	
	ptn = new p_type_node;
	ptn->set_name("definition");
	ptn->set_format("%s");
	ptn->set_type(cast_new_prim_alias(CAST_PRIM_CHAR,
					  CAST_MOD_SIGNED /* XXX? */,
					  cast_new_scoped_name("CORBA_char",
							       NULL)
		));
	ptn->set_mapping(direct_map);
	ptc->add_type("default", ptn);
	
	prim_collections[PRIM_COLLECTION_CHAR] = ptc;
	
	/* Define the presented type corresponding to `signed char'. */
	/* XXX --- This is also `CORBA_char'. */
	ptc = new p_type_collection;
	ptc->set_name("schar");
	
	psn = new p_scope_node;
	psn->set_name("default");
	ptc->add_scope(psn);
	
	ptn = new p_type_node;
	ptn->set_name("definition");
	ptn->set_format("%s");
	ptn->set_type(cast_new_prim_alias(CAST_PRIM_CHAR,
					  CAST_MOD_SIGNED,
					  cast_new_scoped_name("CORBA_char",
							       NULL)
		));
	ptn->set_mapping(direct_map);
	ptc->add_type("default", ptn);
	
	prim_collections[PRIM_COLLECTION_SCHAR] = ptc;
	
	/* Define the presented type corresponding to `unsigned char'. */
	/* XXX --- This is also `CORBA_char'. */
	ptc = new p_type_collection;
	ptc->set_name("uchar");
	
	psn = new p_scope_node;
	psn->set_name("default");
	ptc->add_scope(psn);
	
	ptn = new p_type_node;
	ptn->set_name("definition");
	ptn->set_format("%s");
	ptn->set_type(cast_new_prim_alias(CAST_PRIM_CHAR,
					  CAST_MOD_UNSIGNED,
					  cast_new_scoped_name("CORBA_char",
							       NULL)
		));
	ptn->set_mapping(direct_map);
	ptc->add_type("default", ptn);
	
	prim_collections[PRIM_COLLECTION_UCHAR] = ptc;
	
	/* Define the presented type corresponding to `octet'. */
	/* This is `CORBA_octet'. */
	ptc = new p_type_collection;
	ptc->set_name("octet");
	
	psn = new p_scope_node;
	psn->set_name("default");
	ptc->add_scope(psn);
	
	ptn = new p_type_node;
	ptn->set_name("definition");
	ptn->set_format("%s");
	ptn->set_type(cast_new_prim_alias(CAST_PRIM_CHAR,
					  CAST_MOD_UNSIGNED,
					  cast_new_scoped_name("CORBA_octet",
							       NULL)
		));
	ptn->set_mapping(direct_map);
	ptc->add_type("default", ptn);
	
	prim_collections[PRIM_COLLECTION_OCTET] = ptc;
	
	/* Define the presented type corresponding to `short'. */
	/* This is `CORBA_short'. */
	ptc = new p_type_collection;
	ptc->set_name("short");
	
	psn = new p_scope_node;
	psn->set_name("default");
	ptc->add_scope(psn);
	
	ptn = new p_type_node;
	ptn->set_name("definition");
	ptn->set_format("%s");
	ptn->set_type(cast_new_prim_alias(CAST_PRIM_INT,
					  (CAST_MOD_SIGNED | CAST_MOD_SHORT),
					  cast_new_scoped_name("CORBA_short",
							       NULL)
		));
	ptn->set_mapping(direct_map);
	ptc->add_type("default", ptn);
	
	prim_collections[PRIM_COLLECTION_SHORT] = ptc;
	
	/* Define the presented type corresponding to `unsigned short'. */
	/* This is `CORBA_unsigned_short'. */
	ptc = new p_type_collection;
	ptc->set_name("ushort");
	
	psn = new p_scope_node;
	psn->set_name("default");
	ptc->add_scope(psn);
	
	ptn = new p_type_node;
	ptn->set_name("definition");
	ptn->set_format("%s");
	ptn->set_type(cast_new_prim_alias(CAST_PRIM_INT,
					  (CAST_MOD_UNSIGNED | CAST_MOD_SHORT),
					  cast_new_scoped_name(
						  "CORBA_unsigned_short",
						  NULL)
		));
	ptn->set_mapping(direct_map);
	ptc->add_type("default", ptn);
	
	prim_collections[PRIM_COLLECTION_USHORT] = ptc;
	
	/* Define the presented type corresponding to `long'. */
	/* This is `CORBA_long'. */
	ptc = new p_type_collection;
	ptc->set_name("long");
	
	psn = new p_scope_node;
	psn->set_name("default");
	ptc->add_scope(psn);
	
	ptn = new p_type_node;
	ptn->set_name("definition");
	ptn->set_format("%s");
	ptn->set_type(cast_new_prim_alias(CAST_PRIM_INT,
					  (CAST_MOD_SIGNED | CAST_MOD_LONG),
					  cast_new_scoped_name("CORBA_long",
							       NULL)
		));
	ptn->set_mapping(direct_map);
	ptc->add_type("default", ptn);
	
	prim_collections[PRIM_COLLECTION_LONG] = ptc;
	
	/* Define the presented type corresponding to `unsigned long'. */
	/* This is `CORBA_unsigned_long'. */
	ptc = new p_type_collection;
	ptc->set_name("ulong");
	
	psn = new p_scope_node;
	psn->set_name("default");
	ptc->add_scope(psn);
	
	ptn = new p_type_node;
	ptn->set_name("definition");
	ptn->set_format("%s");
	ptn->set_type(cast_new_prim_alias(CAST_PRIM_INT,
					  (CAST_MOD_UNSIGNED | CAST_MOD_LONG),
					  cast_new_scoped_name(
						  "CORBA_unsigned_long",
						  NULL)
		));
	ptn->set_mapping(direct_map);
	ptc->add_type("default", ptn);
	
	prim_collections[PRIM_COLLECTION_ULONG] = ptc;
	
	/* Define the presented type corresponding to `long long'. */
	/* This is `CORBA_long_long'. */
	ptc = new p_type_collection;
	ptc->set_name("longlong");
	
	psn = new p_scope_node;
	psn->set_name("default");
	ptc->add_scope(psn);
	
	ptn = new p_type_node;
	ptn->set_name("definition");
	ptn->set_format("%s");
	ptn->set_type(cast_new_prim_alias(CAST_PRIM_INT,
					  (CAST_MOD_SIGNED
					   | CAST_MOD_LONG_LONG),
					  cast_new_scoped_name(
						  "CORBA_long_long",
						  NULL)
		));
	ptn->set_mapping(direct_map);
	ptc->add_type("default", ptn);
	
	prim_collections[PRIM_COLLECTION_LONGLONG] = ptc;
	
	/* Define the presented type corresponding to `unsigned long long'. */
	/* This is `CORBA_unsigned_long_long'. */
	ptc = new p_type_collection;
	ptc->set_name("ulonglong");
	
	psn = new p_scope_node;
	psn->set_name("default");
	ptc->add_scope(psn);
	
	ptn = new p_type_node;
	ptn->set_name("definition");
	ptn->set_format("%s");
	ptn->set_type(cast_new_prim_alias(CAST_PRIM_INT,
					  (CAST_MOD_UNSIGNED
					   | CAST_MOD_LONG_LONG),
					  cast_new_scoped_name(
						  "CORBA_unsigned_long_long",
						  NULL)
		));
	ptn->set_mapping(direct_map);
	ptc->add_type("default", ptn);
	
	prim_collections[PRIM_COLLECTION_ULONGLONG] = ptc;
	
	/* Define the presented type corresponding to `float'. */
	/* This is `CORBA_float'. */
	ptc = new p_type_collection;
	ptc->set_name("float");
	
	psn = new p_scope_node;
	psn->set_name("default");
	ptc->add_scope(psn);
	
	ptn = new p_type_node;
	ptn->set_name("definition");
	ptn->set_format("%s");
	ptn->set_type(cast_new_prim_alias(CAST_PRIM_FLOAT,
					  0,
					  cast_new_scoped_name("CORBA_float",
							       NULL)
		));
	ptn->set_mapping(direct_map);
	ptc->add_type("default", ptn);
	
	prim_collections[PRIM_COLLECTION_FLOAT] = ptc;
	
	/* Define the presented type corresponding to `double'. */
	/* This is `CORBA_double'. */
	ptc = new p_type_collection;
	ptc->set_name("double");
	
	psn = new p_scope_node;
	psn->set_name("default");
	ptc->add_scope(psn);
	
	ptn = new p_type_node;
	ptn->set_name("definition");
	ptn->set_format("%s");
	ptn->set_type(cast_new_prim_alias(CAST_PRIM_DOUBLE,
					  0,
					  cast_new_scoped_name("CORBA_double",
							       NULL)
		));
	ptn->set_mapping(direct_map);
	ptc->add_type("default", ptn);
	
	prim_collections[PRIM_COLLECTION_DOUBLE] = ptc;
	
	/* Define the presented type corresponding to enumeration types. */
	/* This is `CORBA_enum'. */ 
	ptc = new p_type_collection;
	ptc->set_name("CORBA_enum");
	
	psn = new p_scope_node;
	psn->set_name("default");
	ptc->add_scope(psn);
	
	ptn = new p_type_node;
	ptn->set_name("definition");
	ptn->set_format("%s");
	ptn->set_type(cast_new_prim_alias(CAST_PRIM_INT,
					  (CAST_MOD_UNSIGNED | CAST_MOD_LONG),
					  /*
					   * Do not call `calc_type_name' for
					   * this; that method is for
					   * typedef'ed names, not base type
					   * names.
					   */
					  cast_new_scoped_name("CORBA_enum",
							       NULL)
		));
	ptn->set_mapping(direct_map);
	ptc->add_type("default", ptn);
	
	prim_collections[PRIM_COLLECTION_ENUM] = ptc;
	
	/* Define the presented type corresponding to `type tag'. */
	/* This is `CORBA_TypeCode'. */
	ptc = new p_type_collection;
	ptc->set_name("type tag");
	
	psn = new p_scope_node;
	psn->set_name("default");
	ptc->add_scope(psn);
	
	ptn = new p_type_node;
	ptn->set_name("definition");
	ptn->set_format("%s");
	ptn->set_type(cast_new_type_name("CORBA_TypeCode"));
	ptn->set_mapping(pres_c_new_mapping(PRES_C_MAPPING_TYPE_TAG));
	ptc->add_type("default", ptn);
	
	prim_collections[PRIM_COLLECTION_TYPE_TAG] = ptc;
	
	/* Define the presented type corresponding to `typed any'. */
	/* This is `CORBA_any'. */
	ptc = new p_type_collection;
	ptc->set_name("any");
	
	psn = new p_scope_node;
	psn->set_name("default");
	ptc->add_scope(psn);
	
	ptn = new p_type_node;
	ptn->set_name("definition");
	ptn->set_format("%s");
	ptn->set_type(cast_new_type_name("CORBA_any"));
	ptn->set_mapping(pres_c_new_mapping(PRES_C_MAPPING_TYPED));
	ptc->add_type("default", ptn);
	
	prim_collections[PRIM_COLLECTION_TYPED_ANY] = ptc;
	
	/*********************************************************************/
	
	/*
	 * At this point, some presentation generators create additional
	 * presentations of the typed listed above (e.g., ``out_pointer''
	 * versions of the types).  But we don't need to do so.
	 */
}

/* End of file. */

