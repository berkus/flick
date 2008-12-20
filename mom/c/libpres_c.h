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

#ifndef _mom_libpres_c_h_
#define _mom_libpres_c_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#include <mom/libmeta.h>
#include <mom/pres_c.h>
#include <stdarg.h>

void pres_c_1_init(pres_c_1 *pres);
void pres_c_1_check(pres_c_1 *pres);
void pres_c_1_readfh(pres_c_1 *dest, FILE *fh);
void pres_c_1_writefh(pres_c_1 *src, FILE *fh);


/* Constructors for pres_c nodes.  */
pres_c_inline pres_c_new_inline_atom(pres_c_inline_index idx, pres_c_mapping map);
pres_c_inline pres_c_new_inline_struct(int slots);
pres_c_inline pres_c_new_inline_func_params_struct(int slots);
pres_c_inline pres_c_new_inline_struct_union(int mappings);
pres_c_inline pres_c_new_inline_virtual_union(int mappings);
pres_c_inline pres_c_new_inline_expanded_union(int mappings);
pres_c_inline pres_c_new_inline_void_union(int mappings);
pres_c_inline pres_c_new_inline(pres_c_inline_kind kind);
pres_c_mapping pres_c_new_mapping(pres_c_mapping_kind kind);

/* Constructors for pres_c_inline nodes.  Uses Tag notation (Attribute, Value)
   to specify the node's values */
pres_c_inline pres_c_set_inline_tags(pres_c_inline_kind kind,
				     pres_c_inline inl,
				     int tag, ...);
/* Constructors for pres_c_mapping nodes.  Uses Tag notation (Attribute, Value)
   to specify the node's values */
pres_c_mapping pres_c_set_mapping_tags(pres_c_mapping_kind kind,
				       pres_c_mapping map,
				       int tag, ...);

/* Add a new slot/case to the specified pres_c_inline_struct(or union),
   and return its index.  */
int pres_c_add_inline_struct_slot(pres_c_inline inl);
int pres_c_add_inline_union_case(pres_c_inline inl);


/* Given a simple MINT itype, this returns the name of a corresponding MOM ctype.  */
char *mint_to_ctype_name(mint_1 *mint, mint_ref itype);

/* Given a simple MINT type,
   produce a ctype appropriate for storing that itype.  */
cast_type mint_to_ctype(mint_1 *mint, mint_ref itype);

/* Given an initial itype which must be a MINT_UNION
   and a pres_c_inline which must be a PRES_C_INLINE_COLLAPSED_UNION,
   find the appropriate case in the union selected by the presentation
   and return the new itype and pres_c_inline corresponding to it.
   Note that `itypep' and `inlp' are in-out parameters.  */
void descend_collapsed_union(pres_c_1 *pres, mint_ref *itypep, pres_c_inline *inlp);

/* Upon encountering a PRES_C_MAPPING_STUB node while descending a mapping tree,
   this routine finds a stub that can marshal/unmarshal between the itype and the ctype.
   Returns -1 if none found.  */
int pres_c_find_mu_stub(pres_c_1 *pres, mint_ref itype, cast_type ctype, pres_c_mapping map,
			pres_c_stub_kind stub_kind);

/* Given an itype, ctype, and PRES_C_MAPPING_STUB,
   this routine finds a stub that can marshal/unmarshal between the itype and ctype,
   and bumps the mapping and ctype pointers to point into the stub's mapping and ctype,
   essentially dereferencing through the level of indirection.
   Note that this shouldn't be done blindly and unconditionally,
   because there can be recursion through stub mappings.  */
void pres_descend_mapping_stub(pres_c_1 *pres, mint_ref itype,
				cast_type *ctypep, pres_c_mapping *mapp);


/*
 * Given a ctype and mapping, return a new ctype and mapping which is identical
 * except that it imposes one additional level of pointer indirection.
 */
void pres_c_interpose_pointer(cast_type *inout_ctype,
			      pres_c_mapping *inout_mapping,
			      const char *arglist_name);

/*
 * Given a ctype and mapping, return a new ctype and mapping which is identical
 * except that it imposes one additional level of complete, presented pointer
 * indirection, including allocation context.  `ptr_alloc' defines the
 * allocation semantics for the new pointer.
 */
void pres_c_interpose_indirection_pointer(cast_type *inout_ctype,
					  pres_c_mapping *inout_mapping,
					  pres_c_allocation ptr_alloc);

/*
 * This is like `pres_c_interpose_pointer', except that the mapping for the
 * imposed pointer is PRES_C_MAPPING_INTERNAL_ARRAY, not ..._MAPPING_POINTER.
 * The special INTERNAL_ARRAY mapping is used to identify it as an array
 * instead of a simple pointer.
 */
void pres_c_interpose_internal_array(cast_type *inout_ctype,
				     pres_c_mapping *inout_mapping,
				     const char *arglist_name);
	
/*
 * This is like `pres_c_interpose_pointer', except that it interposes a C++
 * style reference.
 */
void pres_c_interpose_var_reference(cast_type *inout_ctype,
				    pres_c_mapping *inout_mapping);

/*
 * Given a mapping, return a new mapping that encapsulates the original mapping
 * but which tells the back end what kind of parameter the mapping corresponds
 * to: `in', `out', etc.
 */
void pres_c_interpose_direction(pres_c_mapping *inout_mapping,
				aoi_direction aoi_dir);

/*
 * Given a mapping, return a new mapping that encapsulates the original mapping
 * but which tells the back end that this PRES_C tree point corresponds to the
 * ``root'' of a client stub or server work function parameter.  This lets the
 * back end take special processing steps, e.g., allocating a local variable to
 * serve as the actual parameter in a call to a server work function.
 */
void pres_c_interpose_param_root(pres_c_mapping *inout_mapping,
				 cast_type ctype,
				 cast_init init);

/*
 * Given a mapping, return a new mapping that encapsulates the original mapping
 * but which tells the back end to save information about the CAST expression
 * and type being processed.  This information is then used by some parent of
 * the PRES_C mapping node.
 */
void pres_c_interpose_argument(pres_c_mapping *inout_mapping,
			       const char *arglist, const char *name);

/*
 * Given a mapping, return a new mapping that encapsulates the original so
 * that all operations are done on a temporary variable with the given hints.
 * This is useful for C++ or any other situations where the data being
 * marshaled/unmarshaled doesn't exactly match the structures/control flow of
 * the mapping.
 */
void pres_c_interpose_temporary(cast_type ctype,
				const char *name,
				cast_expr init,
				const char *prehandler,
				const char *posthandler,
				int is_const,
				pres_c_temporary_type temp_type,
				pres_c_mapping *inout_mapping);

void pres_c_add_unpresented_channel(pres_c_1 *pres, data_channel_mask dcm);

/*
 * Creates a unique new string (based on the given string `hint') to be used
 * for an arglist name.
 */
char *pres_c_make_arglist_name(const char *hint);

/* Return the string version of a PRES_C direction. */
const char *pres_c_dir_name(pres_c_direction dir);

/* tag_data_u is used to hold data items for tag's
   when passing a tag_data is too painful */
union tag_data_u {
	char b;
	const char *str;
	tag_list *tl;
	int i;
	float f;
	const char *ref;
	tag_object_array obj;
	cast_scoped_name scname;
	cast_def_t cdef;
	cast_type ctype;
	cast_expr cexpr;
	cast_stmt cstmt;
	cast_init cinit;
};

/* Functions for operating on tags */

/* create/delete a tag list */
tag_list *create_tag_list(int list_size);
void delete_tag_list(tag_list *tl);

/* deep copy a tag_list */
tag_list *copy_tag_list(tag_list *tl);
/* append tl2 onto tl1, tl2 will then be deleted */
void concat_tag_list(tag_list *tl1, tag_list *tl2);
/* fix all the parent pointers in tag_list's to point to their parents */
void relink_tag_list(tag_list *tl_root);

/* create/delete a standalone tag_item */
tag_item *create_tag_item(const char *tag, tag_data *data);
void delete_tag_item(tag_item *ti);
/* add/remove a tag from a tag_list */
tag_item *add_tag(tag_list *tl, const char *tag, tag_data_kind kind, ...);
void rem_tag(tag_list *tl, const char *tag);
/* find a tag in a tag_list */
tag_item *find_tag(tag_list *tl, const char *tag);
/* copy a tag_item */
tag_item copy_tag_item(tag_item *ti);

/* create and initialize a tag_data struct */
tag_data create_tag_data(tag_data_kind kind, int length);
/* delete a tag_data */
void delete_tag_data(tag_data *td);
/* copy a tag_data */
tag_data copy_tag_data(tag_data *td);
/* set a tag_data to the given value, if its an array
   then index specifies the index into the array */
void set_tag_data(tag_data *td, int index, union tag_data_u data);
/* get the value from a tag_data */
union tag_data_u get_tag_data(tag_data *td, unsigned int index);
/* get the length of a tag array */
unsigned int tag_data_length(tag_data *td);
/* append a value onto a tag_data thats an array */
int append_tag_data(tag_data *td, union tag_data_u data);

tag_data_kind get_base_tag_data_kind(tag_data_kind kind);
int tag_data_kind_size(tag_data_kind kind);

/* Convert a pointer into a TAG_REF style string */
char *ptr_to_tag_ref(const char *type, void *ptr);
/* Convert a TAG_REF style string into a pointer if the types match */
void *tag_ref_to_ptr(const char *type, const char *ref);

/* Compare the encoding of the ref string with ref_class */
int cmp_tag_ref_class(char *ref_class, char *ref);

/* print_tag_data will print the tag_data with w_printf */
void print_tag_data(int indent, tag_data_kind kind, union tag_data_u data);
/* print_tag will the whole tag as <tag_name>:<tag_kind>=<tag_data> */
void print_tag(int indent, tag_item *ti);


/* Tags for use in pres_function */
enum {
	PFA_TAG_DONE,
	
	PFA_Scope,                /* cast_scope * */
	PFA_ReturnType,           /* cast_type */
	PFA_Spec,                 /* int */
	PFA_Parameter,            /* cast_type type,
				     char *name,
				     cast_init default */
	PFA_StorageClass,         /* cast_storage_class */
	PFA_Template,             /* */
	PFA_TemplateParameter,    /* cast_template_arg_kind,
				     char *name,
				     cast_template_arg default_val */
	PFA_FunctionKind,         /* char * */
	PFA_Constructor,          /* */
	PFA_DeclChannel,          /* data_channel_index */
	PFA_ImplChannel,          /* data_channel_index */
	PFA_Tag,                  /* char *tag_name,
				     tag_data_kind kind,
				     union tag_data_u data */
	PFA_Model,                /* */
	PFA_Protection,           /* cast_def_protection */
	PFA_GlobalFunction        /* */
};

/* Pres function is used to create presentation only functions.  These
   functions exist only for presentation reasons and are described using
   tags, which the back end uses to produce the function bodies. */
int pres_function(pres_c_1 *out_pres,
		  tag_list *parent_tl,
		  cast_scoped_name current_scope_name,
		  cast_scoped_name fname,
		  int tag, ...);

int vpres_function(pres_c_1 *out_pres,
		   tag_list *parent_tl,
		   cast_scoped_name current_scope_name,
		   cast_scoped_name fname,
		   int tag,
		   va_list arg_addr);

/* Attributes ID's for pres_c nodes */
enum {
	PRES_C_TAG_DONE,

	/* pres_c_inline attributes */
	/* [Attribute ID]      [Values needed] */
	PIA_Index,          /* pres_c_inline_index */
	PIA_Mapping,        /* pres_c_mapping */
	PIA_Slot,           /* int, pres_c_inline */
	PIA_Flags,          /* pres_c_*_flags */
	PIA_Return,         /* int, pres_c_inline */
	PIA_Discrim,        /* depends on how union is structured */
	PIA_Default,        /* depends on how union is structured */
	PIA_Case,           /* depends on how union is structured */
	PIA_DiscrimVal,     /* mint_const */
	PIA_SelectedCase,   /* pres_c_inline */
	PIA_Tag,            /* pres_c_inline */
	PIA_Inl,            /* pres_c_inline */
	PIA_Offset,         /* pres_c_inline */
	PIA_Len,            /* pres_c_inline */
	PIA_MinLen,         /* pres_c_inline */
	PIA_MaxLen,         /* pres_c_inline */
	PIA_AllocLen,       /* pres_c_inline */
	PIA_MinAllocLen,    /* pres_c_inline */
	PIA_MaxAllocLen,    /* pres_c_inline */
	PIA_Release,        /* pres_c_inline */
	PIA_Terminator,     /* pres_c_inline */
	PIA_MustCopy,       /* pres_c_inline */
	PIA_Alloc,          /* pres_c_allocation */
	PIA_Overwrite,      /* int */
	PIA_Owner,          /* allocation_owner */
	PIA_Ptr,            /* pres_c_inline_atom */
	PIA_ArgList,        /* char * */
	PIA_Name,           /* char * */
	PIA_CType,          /* cast_type */
	PIA_Value,          /* cast_expr */
	PIA_PreHandler,     /* char * */
	PIA_PostHandler,    /* char * */
	PIA_IsConst,        /* int */
	PIA_TempType,       /* pres_c_temporary_type */

	/* pres_c_mapping attributes */
	PMA_Index,           /* int */
	PMA_Mapping,         /* pres_c_mapping */
	PMA_Dir,             /* pres_c_direction */
	PMA_Target,          /* pres_c_mapping or
				pres_c_inline for a STRUCT mapping */
	PMA_Length,          /* u_int */
	PMA_Max,             /* u_int */
	PMA_Discrim,         /* pres_c_mapping */
	PMA_Case,            /* pres_c_mapping */
	PMA_Default,         /* pres_c_mapping */
	PMA_ElementMapping,  /* pres_c_mapping */
	PMA_Kind,            /* Depends on the kind */
	PMA_RefCount,        /* int */
	PMA_Value,           /* cast_expr */
	PMA_CType,           /* cast_type */
	PMA_Name,            /* char * */
	PMA_PreHandler,      /* char * */
	PMA_PostHandler,     /* char * */
	PMA_IsConst,         /* int */
	PMA_TempType,        /* pres_c_temporary_type */
	PMA_ArgList,         /* char * */
	PMA_InternalCType,   /* cast_type */
	PMA_InternalMapping, /* pres_c_mapping */
	PMA_Translator,      /* char * */
	PMA_Destructor       /* char * */
};

/* Helper macros to make it easier to create/read pres_c nodes.  If
   you don't see a type here it isn't yet supported. */
#define PRES_C_I_ATOM pres_c_set_inline_tags( PRES_C_INLINE_ATOM, 0
#define PRES_C_I_STRUCT pres_c_set_inline_tags( PRES_C_INLINE_STRUCT, 0
#define PRES_C_I_FUNC_PARAMS_STRUCT \
pres_c_set_inline_tags( PRES_C_INLINE_FUNC_PARAMS_STRUCT, 0
#define PRES_C_I_STRUCT_UNION \
pres_c_set_inline_tags( PRES_C_INLINE_STRUCT_UNION, 0
#define PRES_C_I_VOID_UNION \
pres_c_set_inline_tags( PRES_C_INLINE_VOID_UNION, 0
#define PRES_C_I_EXPANDED_UNION \
pres_c_set_inline_tags( PRES_C_INLINE_EXPANDED_UNION, 0
#define PRES_C_I_VIRTUAL_UNION \
pres_c_set_inline_tags( PRES_C_INLINE_VIRTUAL_UNION, 0
#define PRES_C_I_COLLAPSED_UNION \
pres_c_set_inline_tags( PRES_C_INLINE_COLLAPSED_UNION, 0
#define PRES_C_I_TYPED pres_c_set_inline_tags( PRES_C_INLINE_TYPED, 0
#define PRES_C_I_ALLOCATION_CONTEXT \
pres_c_set_inline_tags( PRES_C_INLINE_ALLOCATION_CONTEXT, 0
#define PRES_C_I_THROWAWAY \
pres_c_set_inline_tags( PRES_C_INLINE_THROWAWAY, 0
#define PRES_C_I_TEMPORARY \
pres_c_set_inline_tags( PRES_C_INLINE_TEMPORARY, 0
#define PRES_C_I_ILLEGAL \
pres_c_set_inline_tags( PRES_C_INLINE_ILLEGAL, 0

#define PRES_C_M_DIRECT pres_c_set_mapping_tags( PRES_C_MAPPING_DIRECT, 0
#define PRES_C_M_STUB pres_c_set_mapping_tags( PRES_C_MAPPING_STUB, 0
#define PRES_C_M_DIRECTION pres_c_set_mapping_tags( PRES_C_MAPPING_DIRECTION, 0
#define PRES_C_M_ARGUMENT pres_c_set_mapping_tags( PRES_C_MAPPING_ARGUMENT, 0
#define PRES_C_M_POINTER pres_c_set_mapping_tags( PRES_C_MAPPING_POINTER, 0
#define PRES_C_M_OPTIONAL_POINTER \
pres_c_set_mapping_tags( PRES_C_MAPPING_OPTIONAL_POINTER, 0
#define PRES_C_M_INTERNAL_ARRAY \
pres_c_set_mapping_tags( PRES_C_MAPPING_INTERNAL_ARRAY, 0
#define PRES_C_M_REFERENCE \
pres_c_set_mapping_tags( PRES_C_MAPPING_REFERENCE, 0
#define PRES_C_M_TYPE_TAG pres_c_set_mapping_tags( PRES_C_MAPPING_TYPE_TAG, 0
#define PRES_C_M_TYPED pres_c_set_mapping_tags( PRES_C_MAPPING_TYPED, 0
#define PRES_C_M_IGNORE pres_c_set_mapping_tags( PRES_C_MAPPING_IGNORE, 0
#define PRES_C_M_SYSTEM_EXCEPTION \
pres_c_set_mapping_tags( PRES_C_MAPPING_SYSTEM_EXCEPTION, 0
#define PRES_C_M_STRUCT pres_c_set_mapping_tags( PRES_C_MAPPING_STRUCT, 0
#define PRES_C_M_INITIALIZE \
pres_c_set_mapping_tags( PRES_C_MAPPING_INITIALIZE, 0
#define PRES_C_M_ILLEGAL \
pres_c_set_mapping_tags( PRES_C_MAPPING_ILLEGAL, 0
#define PRES_C_M_VAR_REFERENCE \
pres_c_set_mapping_tags( PRES_C_MAPPING_VAR_REFERENCE, 0
#define PRES_C_M_PARAM_ROOT \
pres_c_set_mapping_tags( PRES_C_MAPPING_PARAM_ROOT, 0
#define PRES_C_M_SELECTOR \
pres_c_set_mapping_tags( PRES_C_MAPPING_SELECTOR, 0
#define PRES_C_M_ELSEWHERE \
pres_c_set_mapping_tags( PRES_C_MAPPING_ELSEWHERE, 0
#define PRES_C_M_TEMPORARY \
pres_c_set_mapping_tags( PRES_C_MAPPING_TEMPORARY, 0
#define PRES_C_M_SINGLETON \
pres_c_set_mapping_tags( PRES_C_MAPPING_SINGLETON, 0
#define PRES_C_M_XLATE \
pres_c_set_mapping_tags( PRES_C_MAPPING_XLATE, 0
#define END_PRES_C PRES_C_TAG_DONE )

#ifdef __cplusplus
}
#endif

#endif /* _mom_libpres_c_h_ */

/* End of file. */
			 
