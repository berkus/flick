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

#ifndef _mom_c_be_msg_span_hh_
#define _mom_c_be_msg_span_hh_

#include <mom/cast.h>
#include <mom/c/be/mu_abort_block.hh>

/*
 * The following infrastructure is used to check the 'spans' in an incoming
 * message.  Basically, this just outputs error checking code to make sure
 * that we've received all the data we expect to process.
 */

/*
 * MSK_* - mu_msg_span kind
 *
 * MSK_NONE - Just holds the length of a span in a message
 * MSK_SEQUENTIAL - Contains a list of other spans ordered sequentially
 * MSK_UNION - Contains a list of possible spans in a message
 * MSK_ARRAY - Contains a single span corresponding to an element in the array
 */
enum {
	MSK_NONE,
	MSK_SEQUENTIAL,
	MSK_UNION,
	MSK_ARRAY
};

/*
 * MSF_* - mu_msg_span flag
 *
 * MSF_ALIGN - Indicates this span is after an alignment
 */ 
enum {
	MSB_ALIGN
};

enum {
	MSF_ALIGN = (1L << MSB_ALIGN)
};

class mu_msg_span {

public:
	mu_msg_span();
	~mu_msg_span();
	
	/* set/get the flags */
	void set_flags(int flags);
	int get_flags();
	
	/* set/get the kind */
	void set_kind(int kind);
	int get_kind();
	
	/* set/get the cast_block where any checks will be added */
	void set_block(cast_stmt block);
	cast_stmt get_block();
	
	/* set/get the abort_block used to get labels for
	   where to jump to on an error */
	void set_abort(struct mu_abort_block *mab);
	struct mu_abort_block *get_abort();
	
	/* grow/shrink the size of this span */
	void grow(int size);
	void shrink(int size);
	
	/* set the size of this span as an expression */
	void set_size(cast_expr size);
	
	/* Add a child to this aggregate span (sequential, array, union...) */
	void add_child(struct mu_msg_span *mms);
	void rem_child(struct mu_msg_span *mms);
	
	/* Begin and end are used to bracket the use of the span */
	void begin();
	void end();
	
	void collapse();
	void drop();
	
	/* Commit will go back over the spans and commit their values */
	void commit();
	
	/* This is used to build the macro name for checking a span */
	static void set_be_name(const char *name);
	
private:
	int flags;
	int kind;
	struct mu_msg_span *parent;
	
	unsigned int size;
	cast_expr size_expr;
	
	cast_stmt check_stmt;
	
	cast_stmt block;
	int stmt_pos;
	struct mu_abort_block *a_block;
	
	struct {
		struct mu_msg_span **val;
		int len;
		unsigned int min_size;
	} children;
	
	void absorb_span(cast_binary_op op, struct mu_msg_span *mms);
	void mod_size(cast_binary_op op, struct mu_msg_span *mms);
	
	static cast_expr macro_name;
};

#endif
