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

#ifndef _mom_c_be_mu_abort_block_
#define _mom_c_be_mu_abort_block_

#include <mom/cast.h>

/*
 * mu_abort_block -
 *
 * mu_abort_block's are used to generate stub code that can clean up after
 * an error has occurred.  The abort blocks achieve this by providing
 * the back end with a place to put error handling code and labels which
 * can be used to jump to that code.  The basic type of a mu_abort_block
 * is a thread, this block just executes a series of statements in the
 * reverse order as they were added to the block.  Besides thread blocks,
 * there are also control blocks that can do conditional control flow.
 * These blocks are then organized in a hierarchical manner to match the
 * control flow of the worker code.  Unfortunately, after all of this
 * has been done there generally quite a bit of dead code or code that
 * does no real cleanup tasks.  Therefore, a rollback stage is done
 * at then end to remove as much of this code is possible.
 */

/* These are the bit numbers for the flags below */
enum {
	MABB_THREAD,
	MABB_CONTROL,
	MABB_IF,
	MABB_ELSE,
	MABB_SWITCH,
	
	MABB_INLINE,
	MABB_OUT_OF_LINE
};

/* These are flags used by mu_abort_block */
enum {
	/* These are used to represent the various
	   behaviours for an abort block */
	MABF_THREAD = (1L << MABB_THREAD),
	MABF_CONTROL = (1L << MABB_CONTROL),
	MABF_IF = (1L << MABB_IF),
	MABF_ELSE = (1L << MABB_ELSE),
	MABF_SWITCH = (1L << MABB_SWITCH),
	MABF_KIND = (MABF_THREAD|MABF_CONTROL|MABF_IF|MABF_ELSE|MABF_SWITCH),
	
	/* These are used to represent how a child connected to
	   its parent.  MABF_INLINE indicates that the child's
	   cast_block was inline'd directly with the parent.
	   MABF_OUT_OF_LINE indicates the block exists elsewhere
	   and a jump is needed to get to it.  A NULL would mean
	   that the parent never jumps up to the child. */
	MABF_INLINE = (1L << MABB_INLINE),
	MABF_OUT_OF_LINE = (1L << MABB_OUT_OF_LINE),
	MABF_CONNECTION = (MABF_INLINE|MABF_OUT_OF_LINE)
};

enum {
	MABK_NONE = 0,
	MABK_THREAD = MABF_THREAD,
	MABK_CONTROL = MABF_CONTROL,
	MABK_CONTROL_IF = MABF_CONTROL|MABF_IF,
	MABK_CONTROL_IF_ELSE = MABF_CONTROL|MABF_IF|MABF_ELSE,
	MABK_CONTROL_SWITCH = MABF_CONTROL|MABF_SWITCH
};

class mu_abort_block {

public:
	mu_abort_block();
	~mu_abort_block();
	
	/* Set/get the kind of the block */
	void set_kind(int kind);
	int get_kind();
	
	/* Set/get the expr value.  Its usage depends on the relationship
	   between this block and its parent.  If the parent is a MABK_THREAD
	   or MABK_CONTROL it is ignored.  However, if it is a MABK_CONTROL_IF
	   or MABK_CONTROL_IF_ELSE then the child with the expr is the true
	   arm and the child without an expr is the false arm, the expr
	   available is then used in the if test.  And, if it is a
	   MABK_CONTROL_SWITCH then the parents expression is used as the
	   switch expression and the children's are used as the case values. */
	void set_expr(cast_expr expr);
	cast_expr get_expr();
	
	/* Set/get the reaper label.  Generally, this shouldn't be set
	   unless the block has no parent since it is set when a child
	   is added to a parent. */
	void set_reaper_label(cast_stmt reaper_label);
	cast_stmt get_reaper_label();
	
	/* This returns the cast_label that is the block's label */
	cast_stmt get_block_label();
	/* This will add a reference to the block label
	   and return the label string. */
	char *use_block_label();
	/* This will drop a reference to the block label. */
	void drop_block_label();
	
	/* This will add a statement to the abort block.  If it's a thread then
	   statements will be executed in the reverse order that they were
	   added, otherwise they are executed in the same order.  Also, if the
	   statement isn't a label it will cause one to be created. */
	void add_stmt(cast_stmt stmt);
	/* Returns the number of statements added to the block */
	int stmt_count();
	/* This will add a child abort block to the current block.  The
	   connection argument needs to be NULL or one of the above flags. */
	void add_child(struct mu_abort_block *mab, int connection);
	
	/* Find a child block with the specified block label string. */
	struct mu_abort_block *find_child(char *child_label);
	
	/* Get the current cast_label */
	cast_stmt get_current_label();
	/* Add a reference to the current label and return its label string. */
	char *use_current_label();
	
	/* Add a reference to the given label. */
	void grab_label(char *label);
	/* Drop a reference to the given label. */
	void drop_label(char *label);
	
	/* Begin/end are used as bookends to any calls to add_stmt.  They're
	   mostly used to set up state that can't be done in
	   the constructor/destructor. */
	void begin();
	void end();
	
	/* Rollback is used to analyze the structure of the abort blocks
	   and remove any unused/useless code.  It does this based on whether
	   or not there is a path to the code in the first place, which
	   is what the pas_path parameters indicate. */
	int rollback(int has_path = 0);
	/* This rolls back an entire block */
	int rollback_block(cast_block *block, int start,
			    int len, int has_path);
	/* This rolls back a single statement */
	int rollback_stmt(cast_block *block, int pos,
			  cast_stmt curr_stmt, int has_path);
	
	/* Make dispatch generates the code for control blocks to
	   actually do their job.  Its generally run by end() so
	   should never anywhere else normally. */
	void make_dispatch();
private:
	/* Kind is used to determine what the block is really doing and
	   thus how it should be built. */
	int flags;
	/* The block_label has a label string that is taken to be the
	   name of this block and a pointer to a cast_block which
	   holds all the abort statements for this block. */
	cast_stmt block_label;
	/* Block_stmts is the cast_block which block_label points to. */
	cast_stmt block_stmts;
	/* Curr_label is a pointer to the current cast_label in the block. */
	cast_stmt curr_label;
	/* This is used to track all of the children of this block. */
	struct {
		struct mu_abort_block **val;
		int len;
	} children;
	/* The block_expr is used to associate an expression with this block */
	cast_expr block_expr;
	/* The reaper label points to where this block should jump to when
	   its done executing.  There's no logical reason why its called
	   'reaper', its mostly historical with another part of the code.
	   Basically, there was a need for a last ditch label that could
	   handle catastrophic failures, for some reason it was associated
	   with the Grim Reaper, maybe it was too much Blue Oyster Cult
	   at the time ("Don't fear the reaper")... */
	cast_stmt reaper_label;
	/* This will drop a reference to the reaper label and delete the
	   goto statement at the end of the block used to jump to the
	   reaper label.  This is only meant to be used on an inlined child */
	void clear_reaper_label();
	
	/* These are used to specify the formats when creating block labels
	   and regular labels attached to added statements. */
	static const char *label_format;
	static const char *block_format;
	/* The counters are just used to create unique labels. */
	static int label_count;
	static int block_count;
};

#endif
