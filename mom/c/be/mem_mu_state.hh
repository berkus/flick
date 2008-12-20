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

#ifndef _mom_c_pbe_mem_h_
#define _mom_c_pbe_mem_h_

#include <mom/types.h>
#include <mom/c/be/mu_state.hh>

struct mem_params
{
	int endian;	/* stream representation - see below */
	int size;	/* stream size in bytes */
	int align;	/* required alignments in bytes (power of two) */
};

struct mem_mu_memory_allocator_state : public mu_memory_allocator_state
{
	mem_mu_memory_allocator_state(maxuint mms,
				      int ab, int ao,
				      cast_expr *cse, int cs, int ck,
				      cast_expr *gse, int gs) :
		max_msg_size(mms),
		align_bits(ab), align_ofs(ao),
		chunk_size_expr(cse), chunk_size(cs), chunk_count(ck),
		glob_size_expr(gse), glob_size(gs) {}
	
	maxuint max_msg_size;
	
	int align_bits;
	int align_ofs;
	
	cast_expr *chunk_size_expr;
	int chunk_size;
	int chunk_count;
	
	cast_expr *glob_size_expr;
	int glob_size;
};

/* Memory buffer marshaler.

   Basically, marshaling code produced by this marshaler is divided into
   sections.  For each section, the amount of serialized buffer space needed
   is first computed, the buffer space is allocated by calling a runtime
   function/macro, and then the data is marshaled into the buffer by bumping a
   pointer through it.

   When converting bi-endian streams, endianness is tested only once per
   section, and the contents of the section is duplicated, once for each
   endian.  Where each section begins and ends depends on what more-specific
   code wants.  */
struct mem_mu_state : public mu_state
{
	/* Size limit of any one glob.
	   Set only during construction.  */
	const unsigned max_glob_size;

	/* Maximum possible total message size, in bytes.
	   For messages of indeterminate size, will be MAXUINT_MAX.  */
	maxuint max_msg_size;


	/*** Alignment ***/

	/* Number of low bits of the current stream offset known to be
	   constant.
	   For example, if this is 2, then the lowest two bits of the offset
	   are known.  */
	int align_bits;

	/* Value of the lowest `align_bits' bits of the current stream offset.
	   Must always be less than 2^align_bits.
	   For example, if align_bits is 2 and align_ofs is 3, then we know
	   that we are three bytes beyond a natural longword (4-byte)
	   boundary.  */
	int align_ofs;

	/* Normalize align_ofs according to align_bits.  */
	inline void align_ofs_normalize()
		{ align_ofs &= (1 << align_bits) - 1; }


	/*** Chunkification ***/

	/* If we've started a chunk, then this points to a cast_expr pointer
	   to fill in, when the chunk is finished, with the size of the chunk.
	   If we're between chunks, this is null.  */
	cast_expr *chunk_size_expr;

	/* Byte offset so far in the current chunk.
	   Always 0 if we're between chunks.  */
	int chunk_size;
	
	/* This is the number of chunks used, so far.  It's used by array
	 * stuff to verify that an array element fits in one chunk.
	 */
	int chunk_count;

	virtual void new_chunk();
	virtual void new_chunk_align(int needed_bits);
	virtual void end_chunk();

	/*
	 * Make sure there's a current chunk, creating a new one if necessary.
	 */
	virtual void make_chunk()
		{ if (!chunk_size_expr) new_chunk(); }
	
	/*
	 * Break the current chunk, if any, starting a new chunk next time we
	 * need one.
	 */
	virtual void break_chunk()
		{ if (chunk_size_expr) end_chunk(); }

	/*
	 * Makes room in the marshaling buffer for a new primitive.
	 * May start a new chunk and/or glob in the process.
	 * Returns the byte offset in the (new) current chunk
	 * at which the primitive should be placed.
	 */
	virtual int chunk_prim(int needed_align_bits, int prim_size);


	/*** Globification ***/

	/*
	 * If we've started a glob, then this points to a cast_expr pointer to
	 * fill in, when the glob is finished, with the maximum size of the
	 * glob.
	 *
	 * If we're between globs, this is null.
	 */
	cast_expr *glob_size_expr;

	/* Maximum size so far of the current glob.
	   Always 0 if we're between globs.  */
	int glob_size;
	
	virtual void new_glob();
	virtual void end_glob();

	/*
	 * Make sure there's a current glob, creating a new one if necessary.
	 */
	virtual void make_glob()
		{ if (!glob_size_expr) new_glob(); }

	/*
	 * Break the current glob, if any, starting a new glob next time we
	 * need one.
	 */
	virtual void break_glob()
		{ if (glob_size_expr) end_glob(); }

	/* Grow the current glob by `amount' bytes.
	   If there is no current glob, create one first.
	   If the current glob is too big, create a new one.  */
	void glob_grow(int amount);

	/* Do necessary globbing stuff before adding a new primitive.
	   Called by chunk_prim().  */
	virtual void glob_prim(int needed_align_bits, int prim_size);


	/*
	  New methods to get and set the state of the memory allocator.  This
	  allows us to do arbitrary rewinds, etc.
	  */
	virtual mu_memory_allocator_state *memory_allocator_state();
	virtual void set_memory_allocator_state(mu_memory_allocator_state *);
	
	/*** Union processing ***/

	int union_align_bits;
	int union_align_ofs;
	int union_glob_size;
	int union_one_glob;
	maxuint union_msg_size;
	

	virtual void mu_union_case(functor *f);
	virtual void mu_union(functor *f);

	

	/*** Array processing ***/

	/*
	 * This is set by mu_array() to true if the entire array is being
	 * marshaled into one glob.  mu_array_elem() uses it to determine
	 * whether to break_chunk() or break_glob() at the end of each
	 * iteration.
	 */
	int array_one_glob;
	int elem_one_chunk;
	
	/* These are for the initial conditions required by array elements */
	int array_elem_align_ofs;
	int array_elem_align_bits;
	 
	virtual void mu_array_elem(cast_expr elem_expr,
				   cast_type elem_ctype,
				   mint_ref elem_itype,
				   pres_c_mapping elem_mapping,
				   unsigned long len_min,
				   unsigned long len_max);
	
	/* This overrides the original, providing chunking and globbing,
	   as well as detecting optimizations such as out-of-line,
	   message-pointer, and bcopy */
	virtual void mu_array(
		cast_expr array_expr, cast_type array_ctype,
		cast_type elem_ctype, mint_ref elem_itype,
		pres_c_mapping elem_map, char *cname);
	virtual void mu_array_do_bcopy(cast_expr ofs_expr,
				       cast_expr ptr_expr,
				       cast_type ptr_type,
				       cast_type target_type,
				       cast_expr len_expr,
				       cast_expr size_expr,
				       char *cname);
	virtual void mu_array_do_msgptr(cast_expr ofs_expr,
					cast_expr ptr_expr,
					cast_type ptr_type,
					cast_type target_type,
					cast_expr len_expr,
					cast_expr size_expr,
					char *cname);
	
	/* Subclass code must implement these to provide marshaling stream
           parameters.  */
	virtual void get_prim_params(mint_ref itype, int *size,
				     int *align_bits, char **macro_name);
	virtual cast_expr mu_get_sizeof(mint_ref itype_name,
					cast_type ctype,
					pres_c_mapping map,
					int *size,
					int *align_bits);
	
	
	/* These explore the PRES_C/MINT type trees looking for anything
	   needing conversion.  If this item can be marshaled/unmarshaled
	   directly with no conversion, then these routines return an
	   expression representing the amount of memory to copy.  Otherwise,
	   they return 0 (null).  */
	virtual cast_expr mapping_noconv(cast_expr expr, cast_type ctype,
					 mint_ref itype, pres_c_mapping map);
	virtual cast_expr inline_noconv(inline_state *ist, mint_ref itype,
					pres_c_inline inl);
#if 0
	/* This routine actually spits out the code to perform a no-conversion
           marshal.  */
	virtual void mu_noconv(cast_expr expr, cast_type ctype,
			       mint_ref itype, cast_expr len);
#endif
	
	mem_mu_state(be_state *_state, mu_state_op _op, int _assumptions,
		     int init_align_bits, int init_align_ofs,
		     int init_max_glob_size, const char *which);
	mem_mu_state(const mem_mu_state &must);
	
	virtual void mu_mapping_reference(cast_expr expr,
					  cast_type ctype,
					  mint_ref itype,
					  pres_c_mapping_reference *rmap);
	
	virtual void mu_mapping_simple(cast_expr expr, cast_type ctype,
				       mint_ref itype);
	
	virtual void mu_mapping_type_tag(cast_expr expr, cast_type ctype,
					 mint_ref itype);
	
	virtual void mu_mapping_typed(cast_expr expr, cast_type ctype,
				      mint_ref itype);
	
	virtual void mu_end();
};

#endif /* _mom_c_pbe_mem_h_ */

/* End of file. */

