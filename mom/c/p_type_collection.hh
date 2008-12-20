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

#ifndef _p_type_collection_hh
#define _p_type_collection_hh

#include <mom/compiler.h>
#include <mom/aoi.h>
#include <mom/pres_c.h>

/* An IDL type can map to multiple implementation language types so the
   notion of a type collection was created to manage all of these mappings */

class p_scope_node;
class p_type_node;

/* p_type_collection is the main class for implementing type collections.
 * It maintains a list of p_scope_nodes and provides several operations for
 * operating on them in the context of this collection.
 */
class p_type_collection {
	
public:
	p_type_collection();
	~p_type_collection();
	
	/* Set and get a unique ID for this type */
	void set_id(const char *id);
	const char *get_id();
	
	/* Set and get the channel */
	void set_channel(data_channel_index channel);
	data_channel_index get_channel();
	
	/* Set and get the protection */
	void set_protection(cast_def_protection protection);
	cast_def_protection get_protection();
	
	/* Set and get the aoi ref */
	void set_ref(aoi_ref ref);
	aoi_ref get_ref();
	
	/* Set/get the index into the pres_type tag_list array in pres_attrs */
	void set_attr_index(int idx);
	int get_attr_index();
	
	/* Set and get the name */
	void set_name(const char *name);
	const char *get_name();
	
	/* Set and get the tag_list containing information about the type */
	void set_tag_list(tag_list *tl);
	tag_list *get_tag_list();
	
	/* Set and get the collection ref, this also goes through all
	   the type nodes in the reference collection and makes
	   reference types for them in the current collection. */
	void set_collection_ref(struct p_type_collection *ptc);
	struct p_type_collection *get_collection_ref();
	
	/* Define all the types present in this node in the given scope */
	void define_types();
	
	/* Find a type node in this collection */
	struct p_type_node *find_type(const char *name);
	struct p_scope_node *find_scope(const char *name);
	
	/* Add a type to the collection, if this collection references
	   the given type node will be put in the referenced collection
	   and a reference type node will be made for this collection. */
	struct p_type_node *add_type(const char *scope_name,
				     struct p_type_node *ptn,
				     int add_to_tail = 1,
				     int add_to_ref = 1);
	void add_scope(struct p_scope_node *psn);
	
	/* Find a collection in a list based on an aoi_ref */
	static p_type_collection *find_collection(struct dl_list *list,
						  aoi_ref ref);
	static p_type_collection *find_collection(struct dl_list *list,
						  const char *name,
						  cast_scope *scope);
	
	/* Our link into a doubly linked list */
	struct list_node link;
	/* The list of types in this collection */
	struct dl_list scopes;
	
private:
	/* A reference to another collection.  The referenced collection
	   is thought to be made up of definitons of types and this
	   collection is made up of named references to the definitions */
	struct p_type_collection *ptc_ref;
	/* An aoi_ref of the IDL type that we represent */
	aoi_ref ref;
	/* The name of this type collection.  This is used as the argument
	   to the format in the type node. */
	const char *name;
	const char *id;
	/* The data_channel_index of the IDL type, it is
	   applied to all cast_def's we make. */
	data_channel_index channel;
	/* The default protection for all the types we make */
	cast_def_protection protection;
	int attr_index;
	tag_list *tl;
};

/* A p_scope_node is a container for individual types.  Each scope node
   represents the scope where the individual types exist.  And each
   scope node is contained in the primary type collection. */
class p_scope_node {
	
public:
	p_scope_node();
	~p_scope_node();
	
	void set_collection(p_type_collection *ptc);
	p_type_collection *get_collection();
	
	void set_name(const char *name);
	const char *get_name();
	
	/* This specifies a prefix string to attach to names in this scope */
	void set_prefix(const char *prefix);
	const char *get_prefix();
	
	void set_scope_name(cast_scoped_name name);
	cast_scoped_name *get_scope_name();
	
	void set_scope(cast_scope *scope);
	cast_scope *get_scope();
	
	void add_type(struct p_type_node *ptn, int add_to_tail);
	struct p_type_node *find(const char *name);
	
	void ref_types(struct p_scope_node *psn);
	void add_defs();
	
	void set_channel(data_channel_index the_channel);
	data_channel_index get_channel();
	
	struct list_node link;
	struct dl_list types;
	
private:
	struct p_type_collection *collection;
	const char *name;
	const char *prefix;
	cast_scoped_name scope_name;
	cast_scope *scope;
	data_channel_index channel;
};

enum {
	PTB_NAME_REF,
	PTB_REF_ONLY,
	PTB_NO_REF,
	PTB_NO_DEF
};

enum {
	/* Causes this name to only be reference by name
	   and not qualified name (e.g. name instead of struct name) */
	PTF_NAME_REF = (1L << PTB_NAME_REF),
	PTF_REF_ONLY = (1L << PTB_REF_ONLY),
	PTF_NO_REF = (1L << PTB_NO_REF),
	PTF_NO_DEF = (1L << PTB_NO_DEF)
};

/* p_type_node is a support class for p_type_collection.  It only holds
 * individual types and anything related to them.
 */
class p_type_node {
	
public:
	p_type_node();
	~p_type_node();
	
	/* Set and get the flags for this node */
	void set_flags(unsigned int flags);
	unsigned int get_flags();
	
	/* Set and get the name for this node */
	void set_name(const char *name);
	const char *get_name();
	
	/* Set and get the format for this node */
	void set_format(const char *format);
	const char *get_format();
	
	/* Set and get the CAST type for this node */
	void set_type(cast_type type);
	cast_type get_type();
	
	/* Set and get the pres_c_mapping for this node */
	void set_mapping(pres_c_mapping map);
	pres_c_mapping get_mapping();
	
	void set_channel(data_channel_index channel);
	data_channel_index get_channel();
	
	const char *format_name(struct p_scope_node *psn);
	void add_def(struct p_scope_node *psn);
	struct p_type_node *ref_type(struct p_scope_node *psn);
	
	void remove(struct p_scope_node *psn);
	
	/* The link into the list */
	struct list_node link;
	
private:
	/* Flags for this node, denoted by PTF_ */
	unsigned int flags;
	/* Name of this node, it is expected to be defined elsewhere
	   and to have some semantic meaning to whoever is using this node */
	const char *name;
	/* Format refers to the format that any named references to this
	   type should have.  (e.g. format == "%s_ptr" and type is "foo *",
	   then any names used to reference this node will be of the form
	   <name>_ptr).  Note that a format of "" denotes an anonymous type
	   node.  This means that it will not be typedef'ed. */
	const char *format;
	/* The CAST type for this node */
	cast_type type;
	/* The pres_c_mapping for this type */
	pres_c_mapping map;
	data_channel_index channel;
};

#endif /* _p_type_collection_hh */

/* End of file */

