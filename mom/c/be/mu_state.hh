/*
 * Copyright (c) 1999 The University of Utah and
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

#ifndef _mom_c_be_mu_state_hh_
#define _mom_c_be_mu_state_hh_

#include <mom/c/be/be_state.hh>
#include <mom/c/be/mu_abort_block.hh>
#include <mom/c/be/mu_msg_span.hh>

#include <runtime/headers/flick/pres/all.h>

/* Use these defines to _test_ assumption masks... */

/* Assume other side has same architecture/environment -
   no data format conversions necessary.  */
#define RPCA_SAME_ARCH		0x00000001

/* Assume the other side is on the same machine,
   which means we can use registerized parameters.
   Implies RPCA_SAME_ARCH.  */
#define RPCA_SAME_NODE		0x00000002

/* Assume the other side is trusted for security
   (but not necessarily for integrity).  */
#define RPCA_SECURE		0x00000004

/* Assume we fully trust the other side.
   RPCA_SECURE must also be set.  */
#define RPCA_TRUSTED		0x00000008

/* Assume we are fully dependent on the other side's continued existence -
   we would never be able to keep running sensibly if the other side fails,
   even if it doesn't directly damage our domain in doing so.
   Put another way, we assume that the target object will never go away
   until all references to it are gone - if it does, we die too.
   RPCA_TRUSTED must also be set.  */
#define RPCA_DEPEND		0x00000010

/* Assume client and server are in the same protection domain.
   RPCA_TRUSTED must also be set.  */
#define RPCA_SAME_DOMAIN	0x00000020

/* Use these defines to _build_ assumption masks... */
#define RPCM_SAME_ARCH		(RPCA_SAME_ARCH)
#define RPCM_SAME_NODE		(RPCA_SAME_NODE | RPCM_SAME_ARCH)
#define RPCM_SECURE		(RPCA_SECURE)
#define RPCM_TRUSTED		(RPCA_TRUSTED | RPCM_SECURE)
#define RPCM_DEPEND		(RPCA_DEPEND | RPCM_TRUSTED)
#define RPCM_SAME_DOMAIN	(RPCA_SAME_DOMAIN | RPCM_TRUSTED)


/* XXX machdep */
#define IPC_MAX_REGS	8
#define IPC_MAX_RIGHTS	1
#define IPC_MAX_BYTES	4096

#define IPC_REG_SIZE	32

// This is a little helper function...
cast_expr make_case_value_expr(mint_const mint_literal);

struct inline_state;
struct mu_state;
struct target_mu_state;
struct client_mu_state;

struct functor
{
	virtual void func(mu_state *must) = 0;
};

struct mu_inline_struct_union_error_functor : public functor
{
	virtual void func(mu_state *must);
};

struct inline_state
{
	/* Build an expression to access slot 'slot' of the inline_state 'ist'.
	   Also return the C type of the specified slot.  */
	virtual void slot_access(int slot, cast_expr *out_expr,
				 cast_type *out_type) = 0;
};

struct func_inline_state : public inline_state
{
	/* The function type itself.  */
	cast_func_type *func_type;

	inline func_inline_state(cast_func_type *_cfunct)
		{ func_type = _cfunct; }

	virtual void slot_access(int slot, cast_expr *out_expr,
				 cast_type *out_type);
};

struct struct_inline_state : public inline_state
{
	/* The structure type itself - must by CAST_TYPE_STRUCT.  */
	cast_aggregate_type *struct_type;

	/* This is the expression representing the struct variable.  */
	cast_expr var_expr;

	inline struct_inline_state(cast_aggregate_type *_struct_type,
				   cast_expr _var_expr)
		{ struct_type = _struct_type; var_expr = _var_expr; }

	virtual void slot_access(int slot, cast_expr *out_expr,
				 cast_type *out_type);
};

struct singleton_inline_state : public inline_state
{
	/* The CAST expr and type. */
	cast_expr cexpr;
	cast_type ctype;
	
	inline singleton_inline_state(cast_expr _cexpr,
				      cast_type _ctype)
		{
			cexpr = _cexpr;
			ctype = _ctype;
		};
	inline singleton_inline_state(singleton_inline_state &is)
		{
			cexpr = is.cexpr;
			ctype = is.ctype;
		};
	virtual void slot_access(int slot, cast_expr *out_expr,
				 cast_type *out_type);
};

/* Used by mu_decode_switch */
struct decode_switch_case
{
	pres_c_inline inl;
	pres_c_func *func;
	pres_c_skel *skel;
};


typedef int mu_state_op;

const int MUST_DECODE = 0x1;
const int MUST_ENCODE = 0x2;
const int MUST_ALLOCATE = 0x4;
const int MUST_DEALLOCATE = 0x8;
const int MUST_REQUEST = 0x10;
const int MUST_REPLY = 0x20;

/* Structure for holding m/u specialized m/u stubs that must be output. */
struct mu_stub_info_node {
	struct list_node	link;
	pres_c_stub_kind	stub_kind;
	int			stub_idx;
	pres_c_direction	stub_dir;
};

/* Base class for capturing memory allocation state. */
struct mu_memory_allocator_state
{
};

/* `MAX_STUB_RECURSIVE_INLINE_DEPTH' determines how many times the back end may
   recursively inline the body of any stub.
   
   If this value is zero, then the back end will never inline any stub; it will
   instead call every stub through a function call.
   
   If MAX_STUB_RECURSIVE_INLINE_DEPTH is one, then the back end will inline up
   to one level of every stub.  For example, if while inlining stub `X' the
   back end needs to invoke stub `X' (i.e., make a recursive call), then the
   recursive invocation of `X' will be a function call, not an inline.  Note
   that the MAX_STUB_RECURSIVE_INLINE_DEPTH limit only applies to `X' within
   `X' --- the back end may freely inline *other* stubs within `X' (unless
   doing so would violate some other limit, of course).
   
   Greater values mean that the back end will recursively inline stubs to
   greater depths.
    */
const int MAX_STUB_RECURSIVE_INLINE_DEPTH = 1;

/*
 * An `mu_state_arglist' is a generic mechanism for transporting CAST data from
 * child PRES_C nodes to parent PRES_C nodes.
 *
 * In order to process certain PRES_C nodes types, the back end must collect
 * CAST data from the children of the node, so that the CAST may be used in
 * other expressions.  For instance, in order to handle an INLINE_COUNTED_ARRAY
 * node, the back end needs to determine the CAST expression that describes the
 * array length.  The length expression is known only when the back end has
 * ``descended'' from the INLINE_COUNTED_ARRAY node, and is processing the
 * child node that corresponds to the length data.  At this point, the back end
 * needs to save the expression.  Then, once the back end has ``ascended'' back
 * up to the INLINE_COUNTED_ARRAY node, the length expression will be available
 * for use in processing the array contents.
 *
 * An `mu_state_arglist' contains a set of named arguments.  The methods for
 * handling some PRES_C nodes (e.g., `mu_state::mu_inline_allocation_context')
 * set up an arglist: with names, but no associated CAST values.  The values of
 * the arguments are later filled in by MAPPING_ARGUMENT nodes --- special
 * child nodes that save the then-current CAST data into the arglist.
 *
 * In order to support multiple (nested) arglists, each `mu_state_arglist' also
 * has a parent, usually the arglist that existed before the current arglist
 * was created.  Any `mu_state' method that creates a new argument list must
 * also take care to manage any already-existing arglist: first, by setting the
 * `parent' of the new arglist structure (passing it to the constructor for the
 * new arglist), and later, by restoring the original arglist when the new
 * arglist is no longer needed.
 */
class mu_state_arglist {
public:
	/*
	 * Although this type is used only privately, we make it `public' in
	 * order to avoid a warning from Sun's C++ compiler when compiling the
	 * definition of `findarg'.  If `argument' were `private', Sun's
	 * compiler would complain:
	 *
	 * ``Warning (Anachronism): mu_state_arglist::argument is not
	 * accessible from file level.''
	 */
	struct argument {
		char *		name;	/* Not `const' --- we free() this. */
		cast_expr	cexpr;
		cast_type	ctype;
		argument *	next;
	};
	
private:
	argument *			arg_list;
	struct mu_state_arglist *	parent;
	const char *			arglist_name;
	
	/*
	 * findarg() is meant mainly for internal use.  It finds the named
	 * argument in the current arglist (does NOT search parents).
	 */
	argument *findarg(const char *arg);
	
public:
	mu_state_arglist(const char *aname, mu_state_arglist *aparent = 0);
	~mu_state_arglist();
	
	void add(const char *aname, const char *arg);
	void remove(const char *aname, const char *arg);
	
	/* The return value is non-zero for success, zero for failure. */
	int getargs(const char *aname, const char *arg,
		    cast_expr *expr, cast_type *type);
	int setargs(const char *aname, const char *arg,
		    cast_expr expr, cast_type type);
};

/*
 * This structure is used to carry information from PRES_C inline nodes that
 * handle MINT_ARRAYs, down to the nodes that handle the array pieces.
 *
 * When we're processing a MINT_ARRAY with a PRES_C inline, the method that
 * handles the inline (e.g., `mu_mapping_internal_array') fills an instance of
 * this structure with information about the array.  This data is later used by
 * `mu_mapping_internal_array', which processes the array contents.
 */
struct mu_array_data {
	int		is_valid;	/* Is this data valid?       */
	unsigned	mint_len_min;	/* Minimum array length (from MINT). */
	unsigned	mint_len_max;	/* Maximum array length (from MINT). */
};

/*
 * This is used to carry information from PRES_C_INLINE_ALLOCATION_CONTEXT
 * nodes down to the nodes that actually handle allocation.
 */
struct mu_inline_alloc_context {
	char *	name;			/* The name of this context. */
	int	is_length;		/* True whenever handling a length */
	int	overwrite;		/* preallocated `out' buffer flag */
	allocation_owner	owner;	/* entity ownership */
	pres_c_allocation *	alloc;	/* the allocation semantics */

	mu_inline_alloc_context *parent_context; /* parent */
};

struct translation_handler_entry {
	struct h_entry entry;
	int (*handler)(mu_state *must,
		       cast_expr cexpr, cast_type ctype, mint_ref itype,
		       pres_c_mapping_xlate *xmap);
};

int mu_type_cast_mapping_handler(mu_state *must, cast_expr cexpr,
				 cast_type ctype, mint_ref itype,
				 pres_c_mapping_xlate *xmap);

int mu_ampersand_mapping_handler(mu_state *must, cast_expr cexpr,
				 cast_type ctype, mint_ref itype,
				 pres_c_mapping_xlate *xmap);

/*
Two base classes for code generation exist.
This class represents the general case - 
navigation through the pres_c file.
This class is abstract.
*/
struct mu_state
{
	struct be_state *state;
	/*
	 * This is a pointer to the PRES_C structure which is created by the
	 * presentation generator.  An `mu_state' operates over this structure.
	 */
	pres_c_1 *pres;
	
	/*
	 * The following CAST expressions describe the stub or function that is
	 * currently being handled by this `mu_state'.
	 *
	 * The `formal_func_invocation_cexpr' is a CAST expression that
	 * contains a function call with the formal parameter names (i.e., the
	 * names that appear in the function definition argument list).  Note
	 * that the `formal_func_invocation_cexpr' may not *be* a function
	 * call expression --- it may be (and usually is) a CAST assignment
	 * expression of the following form:
	 *
	 *   _return = func(param_1, param_2, ...);
	 *
	 * The `actual_func_invocation_cexpr' is a CAST expression identical to
	 * the formal expression, except that the parameters (and lvalue) are
	 * replaced with actual parameter values.  This expression is built up
	 * as the `mu_state' builds the body of the stub.
	 *
	 * See `mu_state::mu_mapping_param_root' to see how these expressions
	 * are used.
	 *
	 * Currently, these slots are used only when building a server dispatch
	 * function or a receive function.  In the future they may be used in
	 * the construction of other kinds of functions as well.
	 */
	cast_expr formal_func_invocation_cexpr;
	cast_expr actual_func_invocation_cexpr;
	
	/* The `stub_inline_depth' array keeps track of the "depth" at which we
	   are inlining each stub described in `pres'.  Every time we start
	   inlining (i.e., producing the C code within) the body of a stub, we
	   increment the counter associated with that stub.  When we finish, we
	   decrement the counter.  Certain functions such as `mu_mapping_stub'
	   examine these counters in order to determine whether we should
	   inline the body of a stub or produce a call to the stub.
	   */
	int *stub_inline_depth;
	const char *which_stub;
	
	/* Operation to be performed by the stub.  */
        mu_state_op op;

	/*
	 * The direction of the parameter for which we are currently processing
	 * data: `in', `inout', `out', `return', or `unknown'.
	 */
	pres_c_direction current_param_dir;
	
	// According to Nathan Dykman (6/10/96), this isn't used 
	int assumptions;
	
	/*
	 * The no-double-swap optimization. <Gnats #61>.
	 * 1 if currently packing several chars into a short or an int
	 * for discriminating upon.  This is done in hash_const.cc when
	 * doing a switch on a string.  See hash_const.cc for more details.
	 */
	int now_packing;	
	
	/* List of pres_c_def's for which we are doing inline marshaling.
	   This is used as a stack to detect loops
	   that we have to break with calls to out-of-line recursive stubs.  */
	/*...*/
	/* XXX for starters, don't bother doing any inline marshaling of
	   pres_c_def's (only pres_c_decl's).  */
	
	/*
	 * `c_block' is the C marshal/unmarshal code block we're currently
	 * building, null if none so far.
	 *
	 * XXX --- `cdecl_block' is not yet used.
	 *
	 * `cdecl_block' is a block statement used to carry local variable
	 * declarations up and out of the scope defined by `c_block', null if
	 * none so far.  When one `mu_state' object incorporates the `c_block'
	 * of another `mu_state', it must also absorb the declarations from the
	 * other `mu_state's `cdecl_block' into its own `cdecl_block'.  Before
	 * outputting code, the `cdecl_block' must be absorbed into the basic
	 * `c_block'.
	 */
	cast_stmt c_block;
	cast_stmt cdecl_block;
	
	/*
	 * These slots are used to communicate information from the nodes that
	 * handle MINT_ARRAYs, down to the nodes that handle the array pieces.
	 *
	 * When we're processing a MINT_ARRAY with `mu_mapping_internal_array',
	 * it fills the `array_data' with information about the array.  This
	 * data is later accessed by `mu_array', which actually processes the
	 * array contents.
	 */
	mu_array_data		array_data;
	
	/*
	 * This slot is used to communicate information from the allocation
	 * context nodes that specify allocation down to the nodes that
	 * actually handle the allocation.
	 */
	mu_inline_alloc_context	*inline_alloc_context;
	
	/*
	 * These are the CAST expressions for presented security identifiers
	 * (SIDs).  The base `mu_state' class knows when to set/clear these
	 * slots, but it does not know what to do with the actual expressions.
	 * Individual BE's must know what to do when any SID must be managed.
	 */
	cast_expr client_sid_cexpr;
	cast_expr server_sid_cexpr;
	
	/*
	 * The `arglist' is used by various PRES_C nodes to communicate data
	 * from children to parent nodes.  See `mu_state::mu_mapping_argument'.
	 */
	mu_state_arglist *arglist;
	
	/*********************************************************************/
	
	mu_state(be_state *_state, mu_state_op _op,
		 int _assumptions, const char *which);
	mu_state(const mu_state &must);
	/* clone & another operations are implementation specific.
	   They are used to create iterators over
	   the pres_c structure which gather info
	   to be used in the actual code gen phase. 
	   Nate Dykman said it's like trace scheduling,
	   but that seems like a stretch...
	   */
	virtual mu_state *clone() = 0;
	virtual mu_state *another(mu_state_op _op) = 0;
	virtual ~mu_state();

	/* Get an ASCII string describing the current operation -
	   generally used when constructing magic symbol names to spit out in
	   generated code.  */
	virtual const char *get_buf_name()
	{ 
		switch (op & (MUST_ENCODE | MUST_DECODE)) {
		case MUST_ENCODE:	return "encode";
		case MUST_DECODE:	return "decode";
		default:		return "memory";
		}
	};
	
	/* Returns the name of this backend,
	   for embedding in marshaling macro names and such.  */
	virtual const char *get_encode_name() = 0;
	virtual const char *get_be_name() = 0;
	virtual const char *get_which_stub();
	virtual const char *get_mu_stream_name()
	{
		return "_stream";
	};
	
	/* This should return the most characters an encoding can
	   pack to switch on for discriminators
	   */
	virtual int get_most_packable() = 0;
	
	/* These functions create statements and variables.
	   The reason you can't just add a variable as a statement
	   is that they must be shuffled forward to
	   the beginning of a code block
	   (C limitation, no 'inline' variables)
	   */
	
	void add_var(const char *name, cast_type type,
		     cast_init init = 0,
		     cast_storage_class sc = CAST_SC_NONE);
	void add_stmt(cast_stmt st);
	void add_initial_stmt(cast_stmt st);
	void absorb_stmt(cast_stmt st);
	
	/* This function adds a line of code with no indentation.  It is
	   currently used for "#if"'s, "#else"'s and "#endif"'s. */
	
	void add_direct_code(char *code_string);
	
	/* Create a temporary variable of the given type in the current
	   c_block, and return an expression representing its lvalue.
	   'tag' is a short constant string mangled into the name that
	   doesn't actually matter;
	   it's only to make the generated code more understandable.  */
	cast_expr add_temp_var(const char *tag, cast_type type,
			       cast_init init = 0,
			       cast_storage_class sc = CAST_SC_NONE);
	
	/* Make a unique label for use in the stub. */
	char *add_label();
	
	// This handles simple data types, and is very transport specific
	virtual void mu_mapping_simple(cast_expr expr, cast_type ctype,
				       mint_ref itype) = 0;
	
	// This just calls mu_mapping_simple, but can be overridden if desired
	virtual void mu_array_elem(cast_expr elem_expr, cast_type elem_ctype,
				   mint_ref elem_itype,
				   pres_c_mapping elem_mapping,
				   unsigned long len_min,
				   unsigned long len_max);

	// This just steps thru the array and calls mu_array_elem
	// It should probably be overridden to make it more efficient
	virtual void mu_array(cast_expr array_expr, cast_type array_ctype,
			      cast_type elem_ctype, mint_ref elem_itype,
			      pres_c_mapping elem_map, char *cname);
	
	// These attempt to determine the integer values of the array's bounds.
	virtual void mu_array_get_pres_bounds(unsigned *len_min,
					      cast_expr *min_len_expr,
					      unsigned *len_max,
					      cast_expr *max_len_expr,
					      char *cname);
	virtual void mu_array_get_encoded_bounds(unsigned *len_min,
						 unsigned *len_max,
						 char *cname);
	virtual void mu_array_get_pres_length(char *cname,
					      cast_expr *len_expr,
					      cast_type *len_ctype);
	// This checks the presented bounds on the array.
	virtual void mu_array_check_bounds(char *cname);
	// This terminates an array (e.g. NUL character for strings).
	virtual void mu_array_terminate(cast_expr expr, cast_type ctype,
					char *cname);
	virtual int mu_array_is_terminated(char *cname);
	virtual int mu_array_is_string(char *cname);
	virtual int mu_array_encode_terminator(char *cname) = 0;
	
	// This returns nil, if it's not a fixed size, or a (constant) size
	// expression otherwise
	virtual cast_expr mu_get_sizeof(mint_ref itype,
					cast_type ctype,
					pres_c_mapping map,
					int *size,
					int *align_bits);
  
	/*
	 * This returns a #if (0), #else (1), or #endif (2)
	 * for bit-translation checking for a particular MINT type.
	 */
	virtual cast_stmt mu_bit_translation_necessary(int, mint_ref);

	// The creates a union handler.
	// The functor should be the demux function.
	virtual void mu_union_case(functor *f);
	virtual void mu_union(functor *f);

	virtual void mu_pointer_alloc(cast_expr expr, cast_type target_type,
				      char *cname);
	virtual void mu_pointer_free(cast_expr expr, cast_type target_type,
				     char *cname);

	virtual void mu_mapping_direct(cast_expr expr, cast_type ctype,
				       mint_ref itype);
	virtual void mu_mapping_direction(cast_expr expr,
					  cast_type ctype,
					  mint_ref itype,
					  pres_c_mapping_direction *dir_map);
	virtual void mu_mapping_struct(cast_expr expr, cast_type ctype,
				       mint_ref itype,
				       pres_c_inline inl);
	virtual void mu_mapping_stub_inline(cast_expr expr, cast_type ctype,
					    mint_ref itype,
					    pres_c_mapping map);
	virtual cast_scoped_name mu_mapping_stub_call_name(int stub_idx);
	virtual void mu_mapping_stub_call(cast_expr expr, cast_type ctype,
					  mint_ref itype,
					  pres_c_mapping map);
	virtual void mu_mapping_stub(cast_expr expr, cast_type ctype,
				     mint_ref itype,
				     pres_c_mapping map);
	virtual void mu_mapping_pointer(cast_expr expr, cast_type ctype,
					mint_ref itype,
					pres_c_mapping_pointer *pmap);
	virtual void mu_mapping_var_reference(cast_expr expr, cast_type ctype,
					      mint_ref itype,
					      pres_c_mapping_var_reference *pmap);
	virtual void mu_mapping_xlate(cast_expr expr, cast_type ctype,
				      mint_ref itype,
				      pres_c_mapping_xlate *xmap);
	virtual void mu_mapping_reference_get_attributes(
		mint_ref itype, pres_c_mapping_reference *rmap,
		int *ref_count_adjust, int *mark_for_cleanup);
	virtual void mu_mapping_reference(cast_expr expr, cast_type ctype,
					  mint_ref itype,
					  pres_c_mapping_reference *rmap);
	virtual void mu_mapping_type_tag(cast_expr expr, cast_type ctype,
					 mint_ref itype);
	virtual void mu_mapping_typed(cast_expr expr, cast_type ctype,
				      mint_ref itype);
	
	virtual void mu_mapping_internal_array(
		cast_expr cexpr,
		cast_type ctype,
		mint_ref elem_itype,
		pres_c_mapping_internal_array *amap);
	virtual void mu_mapping_optional_pointer(
		cast_expr expr,
		cast_type ctype,
		mint_ref itype,
		pres_c_mapping_optional_pointer *ptr_map);
	virtual void mu_mapping_system_exception(cast_expr expr,
						 cast_type ctype,
						 mint_ref itype);
	
	virtual void mu_mapping_sid(cast_expr expr,
				    cast_type ctype,
				    mint_ref itype,
				    pres_c_mapping_sid *sid_map);
	
	virtual void mu_mapping_argument(cast_expr expr,
					 cast_type ctype,
					 mint_ref itype,
					 pres_c_mapping map);
	virtual void mu_mapping_selector(cast_expr expr,
					 cast_type ctype,
					 mint_ref itype,
					 pres_c_mapping_selector *smap);
	virtual void mu_mapping_temporary(cast_expr expr,
					  cast_type ctype,
					  mint_ref itype,
					  pres_c_temporary *temp);
	virtual void mu_mapping_singleton(cast_expr expr,
					  cast_type ctype,
					  mint_ref itype,
					  pres_c_mapping_singleton *singleton);
	
	virtual void mu_mapping_message_attribute(
		cast_expr expr,
		cast_type ctype,
		mint_ref itype,
		pres_c_mapping_message_attribute *attr_map);

	virtual void mu_mapping_initialize(cast_expr expr,
					   cast_type ctype,
					   mint_ref itype,
					   pres_c_mapping map);
	
	virtual void mu_mapping_illegal(cast_expr expr);
	
	virtual void mu_mapping_param_root(
		cast_expr cexpr,
		cast_type ctype,
		mint_ref itype,
		pres_c_mapping_param_root *root_map);
	
	virtual void mu_mapping(cast_expr expr,
				cast_type ctype,
				mint_ref itype,
				pres_c_mapping map);

	virtual void mu_inline_message_attribute(inline_state *ist,
						 mint_ref itype,
						 pres_c_inline inl);
	
	virtual void mu_inline_allocation_context(inline_state *ist,
						  mint_ref itype,
						  pres_c_inline inl);
	virtual void mu_inline_temporary(inline_state *ist,
					 mint_ref itype,
					 pres_c_inline inl);
	
	void mu_inline_struct_union(inline_state *ist, mint_ref itype,
				    pres_c_inline inl);
	void mu_inline_virtual_union(inline_state *ist, mint_ref itype,
				     pres_c_inline inl);
	void mu_inline_void_union(inline_state *ist, mint_ref itype,
				  pres_c_inline inl);
	void mu_inline_collapsed_union(inline_state *ist, mint_ref itype,
				       pres_c_inline inl);
	void mu_inline_typed(inline_state *ist, mint_ref itype,
			     pres_c_inline inl);
	void mu_inline_atom(inline_state *ist, mint_ref itype,
			    pres_c_inline inl);
	void mu_inline_struct(inline_state *ist, mint_ref itype,
			      pres_c_inline inl);
	void mu_inline_func_params_struct(inline_state *ist, mint_ref itype,
					  pres_c_inline inl);
	void mu_inline_handler_func(inline_state *ist, mint_ref itype,
				    pres_c_inline inl);
	void mu_inline_xlate(inline_state *ist, mint_ref itype,
			     pres_c_inline inl);
	void mu_inline_cond(inline_state *ist, mint_ref itype,
			    pres_c_inline inl);
	void mu_inline_assign(inline_state *ist, mint_ref itype,
			      pres_c_inline inl);
	void mu_inline_illegal(inline_state *ist, mint_ref itype,
			       pres_c_inline inl);
	void mu_inline(inline_state *ist, mint_ref itype,
		       pres_c_inline inl);
	
	void mu_func_params(cast_ref cfunc_idx, mint_ref itype,
			    pres_c_inline inl);
	
	/*
	  These functions handle constant values present in 
	  the pres_c structure.
	  Constants usually appear on the server stubs,
	  as a constant value,
	  which is used to determine if the client is calling the right server.
	  */
/*****
	virtual void mu_decode_const(mint_const *darray, int darray_len,
				     mint_ref n_r,
				     cast_expr cookie_var);
*****/
	virtual void mu_encode_const(mint_const iconst, mint_ref itype);
	virtual void mu_const(mint_const iconst, mint_ref itype);

        virtual void mu_hash_const(mint_const *domain,	// 'domain' of inputs
				   int domain_size,	// # of inputs
				   mint_ref const_type,	// type of the inputs
				   cast_expr var);	// variable to hash to
	/* The "new and improved" `mu_hash_const'. */
	virtual void mu_discriminate(mint_const *domain,
				     mint_ref domain_itype,
				     int domain_size,
				     functor **success_functors,
				     functor *failure_functor,
				     cast_type discrim_ctype, /* may be void */
				     cast_expr discrim_expr); /* may be void */
	
	/*
	  These functions are responsible for creating the server stub. For
	  each client function, a case inside a switch statement is made.
	  Calls to the server stub include a parameter identifying which
	  server function is to be called. These functions create that code,
	  and are the starting point for server stub generation.
	  */

	virtual void mu_decode_switch(decode_switch_case *darray,
				      int darray_len, mint_ref n_r);
	virtual void mu_server_func_target(pres_c_server_func *sfunc);
	virtual void mu_server_func_client(pres_c_server_func *sfunc);
	virtual void mu_server_func_reply(pres_c_server_func *sfunc,
					  pres_c_skel *skel);
	virtual void mu_server_func_get_invocation_names(
		cast_def *cfunc,
		/* OUT */ cast_expr *formal_func_cexpr,
		/* OUT */ cast_expr *actual_func_cexpr);
	void mu_server_func_set_invocation_cexprs(
		cast_def *cfunc);
	
	virtual void mu_server_func_call(cast_expr func_call_cexpr);
	virtual void mu_server_func(pres_c_inline inl, mint_ref tn_r,
				    pres_c_server_func *sfunc,
				    pres_c_skel *skel);
	
	virtual void mu_receive_func_target(pres_c_receive_func *sfunc);
	virtual void mu_receive_func_client(pres_c_receive_func *sfunc);
	virtual void mu_receive_func_get_invocation_names(
		cast_def *cfunc,
		/* OUT */ cast_expr *formal_func_cexpr,
		/* OUT */ cast_expr *actual_func_cexpr);
	void mu_receive_func_set_invocation_cexprs(
		cast_def *cfunc);
	
	virtual void mu_receive_func_call(cast_expr func_call_cexpr);
	virtual void mu_receive_func(pres_c_inline inl, mint_ref itype,
				     pres_c_receive_func *rfunc,
				     pres_c_skel *skel);
	
	/*
	 * This function gives one an appropriate "target" `mu_state' object,
	 * which is necessary for marshaling/unmarshaling references to objects
	 * that are the "targets" of RPC or RMI (remote method invocation).
	 *
	 * This method must be specialized for every class that is derived from
	 * `mu_state', because every transport deals with object references in
	 * a different way.
	 */
	virtual target_mu_state *mu_make_target_mu_state(
		be_state *state,
		mu_state_op op,
		int assumptions,
		const char *which_stub) = 0;
	
	/* Also for the "client" object. */
	virtual client_mu_state *mu_make_client_mu_state(
		be_state *state,
		mu_state_op op,
		int assumptions,
		const char *which_stub) = 0;
	
	/*
	 * Provides a means of adding code at the end of the block to do
	 * various cleanup and other tasks.
	 */
	virtual void mu_end();
	
	/*
	  These methods handle the building blocks of memory allocation:
	  "globs" and "chunks."  A glob is a contiguous region of memory that
	  has a known maximum size.  A chunk is a contiguous region of memory
	  within a glob; a chunk has an offset within its glob and a fixed
	  size.
	  
	  "Globs" and "chunks" may be realized in different ways by different
	  back ends.  The default implementations of these methods are no-ops.
	  Individual back ends are expected to override these methods.
	  */
	virtual void new_glob();
	virtual void make_glob();
	virtual void break_glob();
	virtual void end_glob();
	
	virtual void glob_grow(int amount);
	virtual void glob_prim(int needed_align_bits, int prim_size);
	
	virtual void new_chunk();
	virtual void new_chunk_align(int needed_bits);
	virtual void break_chunk();
	virtual void end_chunk();
	
	/*
	 * New methods to get and set the state of the memory allocator.  This
	 * allows us to do arbitrary rewinds, etc.
	 */
	virtual mu_memory_allocator_state *memory_allocator_state();
	virtual void set_memory_allocator_state(mu_memory_allocator_state *);
	virtual const char *get_deallocator(pres_c_allocation *alloc);
	virtual pres_c_allocator get_deallocator_kind(pres_c_allocation *);
	virtual pres_c_alloc_flags get_deallocator_flags(pres_c_allocation *);
	virtual const char *get_allocator(pres_c_allocation *alloc);
	virtual const char *get_abort_deallocator(pres_c_allocation *alloc);
	virtual pres_c_allocator get_allocator_kind(pres_c_allocation *);
	virtual pres_c_alloc_flags get_allocator_flags(pres_c_allocation *);
	virtual cast_expr get_allocation_length(char *cname,
						pres_c_allocation *palloc);
	virtual cast_stmt make_error(int err_val);
	virtual cast_stmt change_stub_state(int state);
	
	/* This will be called before we begin marshaling or unmarshaling
	   parameters */
	virtual void mu_prefix_params(void);
	
	struct mu_abort_block *abort_block;
	
	/* The current span node for this level of processing.  As
	   the mu_state descends through types and other things this
	   will point to the span node for this level.  */
	mu_msg_span *current_span;
	mu_msg_span *current_chunk_span;
	
	static struct h_table *translation_handlers;
};

/* This is for cleaning up the glop on the interface unions */
void remove_idl_and_interface_ids(
	pres_c_1 *pres,
	mint_ref in_itype, pres_c_inline in_inline,
	mint_ref *out_itype, pres_c_inline *out_inline);

void remove_operation_id(
	pres_c_1 *pres,
	mint_ref in_itype, pres_c_inline in_inline,
	mint_ref *out_itype, pres_c_inline *out_inline);

#endif
