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

#include "pg_fluke.hh"

/*
 * The method fills the `pg_state::prim_collections' array with descriptions
 * of this presentation generator's primitive and built-in presented types.
 */
void pg_fluke::make_prim_collections()
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
	/* This is `oskit_bool_t'. */
	/* `oskit_bool_t' may or may not be a 32-bit integer. */
	ptc = new p_type_collection;
	ptc->set_name("boolean");
	
	psn = new p_scope_node;
	psn->set_name("default");
	ptc->add_scope(psn);
	
	ptn = new p_type_node;
	ptn->set_name("definition");
	ptn->set_format("%s");
	ptn->set_type(cast_new_type_name("oskit_bool_t"));
	ptn->set_mapping(direct_map);
	ptc->add_type("default", ptn);
	
	prim_collections[PRIM_COLLECTION_BOOLEAN] = ptc;
	
	/* Define the presented type corresponding to `char'. */
	/* This is simply `char'. */
	ptc = new p_type_collection;
	ptc->set_name("char");
	
	psn = new p_scope_node;
	psn->set_name("default");
	ptc->add_scope(psn);
	
	ptn = new p_type_node;
	ptn->set_name("definition");
	ptn->set_format("%s");
	ptn->set_type(cast_new_prim_type(CAST_PRIM_CHAR, 0));
	ptn->set_mapping(direct_map);
	ptc->add_type("default", ptn);
	
	prim_collections[PRIM_COLLECTION_CHAR] = ptc;
	
	/* Define the presented type corresponding to `signed char'. */
	/* This is simply `signed char'. */
	ptc = new p_type_collection;
	ptc->set_name("schar");
	
	psn = new p_scope_node;
	psn->set_name("default");
	ptc->add_scope(psn);
	
	ptn = new p_type_node;
	ptn->set_name("definition");
	ptn->set_format("%s");
	ptn->set_type(cast_new_prim_type(CAST_PRIM_CHAR, CAST_MOD_SIGNED));
	ptn->set_mapping(direct_map);
	ptc->add_type("default", ptn);
	
	prim_collections[PRIM_COLLECTION_SCHAR] = ptc;
	
	/* Define the presented type corresponding to `unsigned char'. */
	/* This is simply `unsigned char'. */
	ptc = new p_type_collection;
	ptc->set_name("uchar");
	
	psn = new p_scope_node;
	psn->set_name("default");
	ptc->add_scope(psn);
	
	ptn = new p_type_node;
	ptn->set_name("definition");
	ptn->set_format("%s");
	ptn->set_type(cast_new_prim_type(CAST_PRIM_CHAR, CAST_MOD_UNSIGNED));
	ptn->set_mapping(direct_map);
	ptc->add_type("default", ptn);
	
	prim_collections[PRIM_COLLECTION_UCHAR] = ptc;
	
	/* Define the presented type corresponding to `octet'. */
	/* This is `oskit_u8_t'. */
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
					  cast_new_scoped_name("oskit_u8_t",
							       NULL)
		));
	ptn->set_mapping(direct_map);
	ptc->add_type("default", ptn);
	
	prim_collections[PRIM_COLLECTION_OCTET] = ptc;
	
	/* Define the presented type corresponding to `short'. */
	/* This is `oskit_s16_t'. */
	ptc = new p_type_collection;
	ptc->set_name("short");
	
	psn = new p_scope_node;
	psn->set_name("default");
	ptc->add_scope(psn);
	
	ptn = new p_type_node;
	ptn->set_name("definition");
	ptn->set_format("%s");
	ptn->set_type(cast_new_prim_alias(CAST_PRIM_INT,
					  CAST_MOD_SHORT,
					  cast_new_scoped_name("oskit_s16_t",
							       NULL)
		));
	ptn->set_mapping(direct_map);
	ptc->add_type("default", ptn);
	
	prim_collections[PRIM_COLLECTION_SHORT] = ptc;
	
	/* Define the presented type corresponding to `unsigned short'. */
	/* This is `oskit_u16_t'. */
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
					  cast_new_scoped_name("oskit_u16_t",
							       NULL)
		));
	ptn->set_mapping(direct_map);
	ptc->add_type("default", ptn);
	
	prim_collections[PRIM_COLLECTION_USHORT] = ptc;
	
	/* Define the presented type corresponding to `long'. */
	/* This is `oskit_s32_t'. */
	ptc = new p_type_collection;
	ptc->set_name("long");
	
	psn = new p_scope_node;
	psn->set_name("default");
	ptc->add_scope(psn);
	
	ptn = new p_type_node;
	ptn->set_name("definition");
	ptn->set_format("%s");
	ptn->set_type(cast_new_prim_alias(CAST_PRIM_INT,
					  CAST_MOD_LONG,
					  cast_new_scoped_name("oskit_s32_t",
							       NULL)
		));
	ptn->set_mapping(direct_map);
	ptc->add_type("default", ptn);
	
	prim_collections[PRIM_COLLECTION_LONG] = ptc;
	
	/* Define the presented type corresponding to `unsigned long'. */
	/* This is `oskit_u32_t'. */
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
					  cast_new_scoped_name("oskit_u32_t",
							       NULL)
		));
	ptn->set_mapping(direct_map);
	ptc->add_type("default", ptn);
	
	prim_collections[PRIM_COLLECTION_ULONG] = ptc;
	
	/* Define the presented type corresponding to `long long'. */
	/* This is `oskit_s64_t'. */
	ptc = new p_type_collection;
	ptc->set_name("longlong");
	
	psn = new p_scope_node;
	psn->set_name("default");
	ptc->add_scope(psn);
	
	ptn = new p_type_node;
	ptn->set_name("definition");
	ptn->set_format("%s");
	ptn->set_type(cast_new_prim_alias(CAST_PRIM_INT,
					  CAST_MOD_LONG_LONG,
					  cast_new_scoped_name("oskit_s64_t",
							       NULL)
		));
	ptn->set_mapping(direct_map);
	ptc->add_type("default", ptn);
	
	prim_collections[PRIM_COLLECTION_LONGLONG] = ptc;
	
	/* Define the presented type corresponding to `unsigned long long'. */
	/* This is `oskit_u64_t'. */
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
					  cast_new_scoped_name("oskit_u64_t",
							       NULL)
		));
	ptn->set_mapping(direct_map);
	ptc->add_type("default", ptn);
	
	prim_collections[PRIM_COLLECTION_ULONGLONG] = ptc;
	
	/* Define the presented type corresponding to `float'. */
	/* This is simply `float'. */
	ptc = new p_type_collection;
	ptc->set_name("float");
	
	psn = new p_scope_node;
	psn->set_name("default");
	ptc->add_scope(psn);
	
	ptn = new p_type_node;
	ptn->set_name("definition");
	ptn->set_format("%s");
	ptn->set_type(cast_new_prim_type(CAST_PRIM_FLOAT, 0));
	ptn->set_mapping(direct_map);
	ptc->add_type("default", ptn);
	
	prim_collections[PRIM_COLLECTION_FLOAT] = ptc;
	
	/* Define the presented type corresponding to `double'. */
	/* This is simply `double'. */
	ptc = new p_type_collection;
	ptc->set_name("double");
	
	psn = new p_scope_node;
	psn->set_name("default");
	ptc->add_scope(psn);
	
	ptn = new p_type_node;
	ptn->set_name("definition");
	ptn->set_format("%s");
	ptn->set_type(cast_new_prim_type(CAST_PRIM_DOUBLE, 0));
	ptn->set_mapping(direct_map);
	ptc->add_type("default", ptn);
	
	prim_collections[PRIM_COLLECTION_DOUBLE] = ptc;
	
	/* Define the presented type corresponding to enumeration types. */
	/* This is simply `unsigned long'. */ 
	ptc = new p_type_collection;
	ptc->set_name("enum");
	
	psn = new p_scope_node;
	psn->set_name("default");
	ptc->add_scope(psn);
	
	ptn = new p_type_node;
	ptn->set_name("definition");
	ptn->set_format("%s");
	ptn->set_type(cast_new_prim_type(CAST_PRIM_INT,
					 (CAST_MOD_UNSIGNED | CAST_MOD_LONG)));
	ptn->set_mapping(direct_map);
	ptc->add_type("default", ptn);
	
	prim_collections[PRIM_COLLECTION_ENUM] = ptc;
	
	/* Define the presented type corresponding to `type tag'. */
	/* This is `mom_typecode'. */
	ptc = new p_type_collection;
	ptc->set_name("type tag");
	
	psn = new p_scope_node;
	psn->set_name("default");
	ptc->add_scope(psn);
	
	ptn = new p_type_node;
	ptn->set_name("definition");
	ptn->set_format("%s");
	ptn->set_type(cast_new_type_name("mom_typecode"));
	ptn->set_mapping(pres_c_new_mapping(PRES_C_MAPPING_TYPE_TAG));
	ptc->add_type("default", ptn);
	
	prim_collections[PRIM_COLLECTION_TYPE_TAG] = ptc;
	
	/* Define the presented type corresponding to `typed any'. */
	/* This is `mom_any'. */
	ptc = new p_type_collection;
	ptc->set_name("any");
	
	psn = new p_scope_node;
	psn->set_name("default");
	ptc->add_scope(psn);
	
	ptn = new p_type_node;
	ptn->set_name("definition");
	ptn->set_format("%s");
	ptn->set_type(cast_new_type_name("mom_any"));
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

