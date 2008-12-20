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

/* This file handles the construction of any abort code needed */

#include <stdlib.h>
#include <assert.h>

#include <mom/c/libcast.h>
#include <mom/c/pbe.hh>

/* These are the default formats for the labels that are used. */
const char *mu_abort_block::label_format = "_flick_abort%d";
const char *mu_abort_block::block_format = "_flick_abort_block%d";

/* These are the counts for label and blocks, and
   are used in the above formats */
int mu_abort_block::label_count = 0;
int mu_abort_block::block_count = 0;

mu_abort_block::mu_abort_block()
{
	char *label_str;
	
	this->flags = 0;
	this->block_stmts = cast_new_block(0, 0);
	label_str = flick_asprintf(block_format, block_count++);
	this->block_label = cast_new_label(label_str, this->block_stmts);
	this->curr_label = 0;
	this->children.val = 0;
	this->children.len = 0;
	this->block_expr = 0;
	this->reaper_label = 0;
}

mu_abort_block::~mu_abort_block()
{
}

void mu_abort_block::set_kind(int kind)
{
	this->flags &= ~MABF_KIND;
	this->flags |= kind;
}

int mu_abort_block::get_kind()
{
	return( this->flags & MABF_KIND );
}

void mu_abort_block::set_expr(cast_expr expr)
{
	this->block_expr = expr;
}

cast_expr mu_abort_block::get_expr()
{
	return( this->block_expr );
}

void mu_abort_block::set_reaper_label(cast_stmt the_reaper_label)
{
	this->reaper_label = the_reaper_label;
	switch( this->flags & MABF_KIND ) {
	case MABK_CONTROL:
	case MABK_CONTROL_IF:
	case MABK_CONTROL_IF_ELSE:
	case MABK_CONTROL_SWITCH: {
		int lpc;
		
		/* If its a control block then we need to walk through
		   all the children and set their labels */
		for( lpc = 0; lpc < this->children.len; lpc++ ) {
			this->children.val[lpc]->
				set_reaper_label(the_reaper_label);
		}
		break;
	}
	case MABK_THREAD:
		/* If its a thread block then we just set the goto's label
		   to this one and increment its user count. */
		this->block_stmts->cast_stmt_u_u.block.stmts.stmts_val[0]->
			cast_stmt_u_u.s_label.stmt->cast_stmt_u_u.goto_label =
			this->reaper_label->cast_stmt_u_u.s_label.label;
		this->reaper_label->cast_stmt_u_u.s_label.users++;
		break;
	default:
		break;
	}
}

cast_stmt mu_abort_block::get_reaper_label()
{
	return( this->reaper_label );
}

void mu_abort_block::clear_reaper_label()
{
	switch( this->flags & MABF_KIND ) {
	case MABK_THREAD:
		this->block_stmts->cast_stmt_u_u.block.stmts.stmts_val[0]->
			cast_stmt_u_u.s_label.stmt->kind = CAST_STMT_EMPTY;
		if( this->reaper_label )
			this->reaper_label->cast_stmt_u_u.s_label.users--;
		break;
	default:
		break;
	}
}

cast_stmt mu_abort_block::get_block_label()
{
	return( this->block_label );
}

char *mu_abort_block::use_block_label()
{
	this->block_label->cast_stmt_u_u.s_label.users++;
	return( this->block_label->cast_stmt_u_u.s_label.label );
}

void mu_abort_block::drop_block_label()
{
	this->block_label->cast_stmt_u_u.s_label.users--;
}

void mu_abort_block::add_stmt(cast_stmt stmt)
{
	if( stmt ) {
		switch( this->flags & MABF_KIND ) {
		case MABK_THREAD: {
			char *label_str;
			
			/* In a thread block every statement needs to have
			   a label so it can be reached, so if it doesn't
			   have one we add one. */
			if( stmt->kind == CAST_STMT_LABEL ) {
				this->curr_label = stmt;
			}
			else {
				label_str = flick_asprintf(label_format,
							   label_count++);
				this->curr_label = cast_new_label(label_str,
								  stmt);
			}
			cast_block_add_stmt(&this->block_stmts->
					    cast_stmt_u_u.block,
					    this->curr_label);
			break;
		}
		case MABK_CONTROL:
		case MABK_CONTROL_IF:
		case MABK_CONTROL_IF_ELSE:
		case MABK_CONTROL_SWITCH:
			/* Just add the stmt without a label. */
			cast_block_add_stmt(&this->block_stmts->
					    cast_stmt_u_u.block,
					    stmt);
			break;
		default:
			break;
		}
	}
}

int mu_abort_block::stmt_count()
{
	int retval = 0;
	
	switch( this->flags & MABF_KIND ) {
	case MABK_THREAD:
		retval = this->block_stmts->cast_stmt_u_u.block.
			stmts.stmts_len - 1;
		break;
	case MABK_CONTROL:
	case MABK_CONTROL_IF:
	case MABK_CONTROL_IF_ELSE:
	case MABK_CONTROL_SWITCH:
		retval = this->block_stmts->cast_stmt_u_u.block.
			stmts.stmts_len;
		break;
	default:
		break;
	}
	return( retval );
}

void mu_abort_block::add_child(struct mu_abort_block *mab, int connection)
{
	int i;
	
	i = this->children.len++;
	this->children.val = (struct mu_abort_block **)
		mustrealloc(this->children.val,
			    this->children.len *
			    sizeof( struct mu_abort_block * ));
	this->children.val[i] = mab;
	switch( this->flags & MABF_KIND ) {
	case MABK_THREAD:
		/* The child is separate from this thread of execution
		   so we need to make a jump to the block and have
		   it end by going to the current label. */
		mab->set_reaper_label(this->get_current_label());
		if( (connection & MABF_OUT_OF_LINE) ) {
			if( mab->stmt_count() )
				this->add_stmt(cast_new_goto(
					mab->use_block_label()));
			else
				connection = 0;
		}
		break;
	case MABK_CONTROL:
	case MABK_CONTROL_IF:
	case MABK_CONTROL_IF_ELSE:
	case MABK_CONTROL_SWITCH:
		/* The flow of control isn't straightforward so we
		   construct it at a later time. */
		if( this->reaper_label )
			mab->set_reaper_label(this->reaper_label);
		break;
	default:
		panic( "mu_abort_block::add_child - "
		       "Can't handle kind %d",
		       this->flags & MABF_KIND );
		break;
	}
	if( connection & MABF_INLINE ) {
		this->add_stmt(mab->get_block_label());
		mab->block_stmts->cast_stmt_u_u.block.flags |=
			CAST_BLOCK_INLINE;
	}
	mab->flags |= connection;
}

struct mu_abort_block *mu_abort_block::find_child(char *child_label)
{
	struct mu_abort_block *retval = 0;
	int lpc;
	
	for( lpc = 0; (lpc < this->children.len) && !retval; lpc++ ) {
		if( !strcmp( child_label,
			     this->children.val[lpc]->get_block_label()->
			     cast_stmt_u_u.s_label.label ) ) {
			retval = this->children.val[lpc];
		}
	}
	return( retval );
}

cast_stmt mu_abort_block::get_current_label()
{
	return( this->curr_label );
}

char *mu_abort_block::use_current_label()
{
	this->curr_label->cast_stmt_u_u.s_label.users++;
	return( this->curr_label->cast_stmt_u_u.s_label.label );
}

void mu_abort_block::grab_label(char *label)
{
	int i;
	
	i = cast_find_label( &this->block_stmts->cast_stmt_u_u.block,
			     label );
	this->block_stmts->cast_stmt_u_u.block.stmts.stmts_val[i]->
		cast_stmt_u_u.s_label.users++;
}

void mu_abort_block::drop_label(char *label)
{
	int i;
	
	i = cast_find_label( &this->block_stmts->cast_stmt_u_u.block,
			     label );
	if( i >= 0 ) {
		this->block_stmts->cast_stmt_u_u.block.stmts.stmts_val[i]->
			cast_stmt_u_u.s_label.users--;
	}
	else {
		/* The label could be in one of our children so we search
		   through them as well. */
		for( i = 0; i < this->children.len; i++ ) {
			this->children.val[i]->drop_label(label);
		}
	}
}

void mu_abort_block::make_dispatch()
{
	switch( this->flags & MABF_KIND ) {
	case MABK_CONTROL_IF:
		if( this->children.val[0]->stmt_count() ) {
			cast_stmt if_stmt, t_stmt;
			cast_expr test_expr;
			
			test_expr = this->children.val[0]->get_expr();
			t_stmt = cast_new_goto(this->children.val[0]->
					       use_block_label());
			if_stmt = cast_new_if(test_expr, t_stmt, 0);
			this->add_stmt(if_stmt);
			this->add_stmt(cast_new_goto(""));
		}
		break;
	case MABK_CONTROL_IF_ELSE: {
		cast_stmt if_stmt, t_stmt = 0, f_stmt = 0;
		cast_expr test_expr;
		int zero;
		
		zero = this->children.val[0]->get_expr() == 0;
		test_expr = this->children.val[zero]->get_expr();
		assert(test_expr);
		if( this->children.val[zero]->stmt_count() )
			t_stmt = cast_new_goto(this->children.val[zero]->
					       use_block_label());
		if( this->children.val[!zero]->stmt_count() )
			f_stmt = cast_new_goto(this->children.val[!zero]->
					       use_block_label());
		if( !t_stmt ) {
			test_expr = cast_new_unary_expr(CAST_UNARY_LNOT,
							test_expr);
			t_stmt = f_stmt;
			f_stmt = 0;
		}
		if( t_stmt || f_stmt ) {
			if_stmt = cast_new_if(test_expr, t_stmt, f_stmt);
			this->add_stmt(if_stmt);
		} else {
			this->children.val[0]->flags &= ~MABF_CONNECTION;
			this->children.val[1]->flags &= ~MABF_CONNECTION;
			free(this->block_stmts->cast_stmt_u_u.block.stmts.
			     stmts_val);
			this->block_stmts->cast_stmt_u_u.block.stmts.
				stmts_val = 0;
			this->block_stmts->cast_stmt_u_u.block.stmts.
				stmts_len = 0;
		}
		break;
	}
	case MABK_CONTROL_SWITCH: {
		cast_stmt sw_stmt, sw_block, child_stmt;
		cast_expr child_value;
		cast_block *block;
		char *child_label;
		int found = 0;
		int lpc;
		
		sw_block = cast_new_block(0, 0);
		sw_stmt = cast_new_switch(this->block_expr, sw_block);
		block = &sw_block->cast_stmt_u_u.block;
		for( lpc = 0; lpc < this->children.len; lpc++ ) {
			child_value = this->children.val[lpc]->
				get_expr();
			if( this->children.val[lpc]->stmt_count() ) {
				found = 1;
				child_label = this->children.val[lpc]->
					use_block_label();
				child_stmt = cast_new_goto(child_label);
			}
			else {
				this->children.val[lpc]->flags &=
					~MABF_CONNECTION;
				child_stmt = cast_new_goto("");
			}
			if( child_value )
				cast_block_add_stmt(block,
						    cast_new_case(child_value,
								  child_stmt));
			else
				cast_block_add_stmt(
					block,
					cast_new_default(child_stmt));
		}
		if( found )
			this->add_stmt(sw_stmt);
		break;
	}
	default:
		break;
	}
}

void mu_abort_block::begin()
{
	switch( this->flags & MABF_KIND ) {
	case MABK_THREAD:
		/* Set the reverse flag for the block and add a
		   goto statement that will later point to the reaper label. */
		this->block_stmts->cast_stmt_u_u.block.flags |=
			CAST_BLOCK_REVERSE;
		this->add_stmt(cast_new_goto("XXX"));
		break;
	default:
		break;
	}
}

void mu_abort_block::end()
{
	switch( this->flags & MABF_KIND ) {
	case MABK_CONTROL:
	case MABK_CONTROL_IF:
	case MABK_CONTROL_IF_ELSE:
	case MABK_CONTROL_SWITCH:
		/* Construct the control flow needed for an abort
		   to properly back out of a conditional flow of control. */
		this->make_dispatch();
		break;
	default:
		break;
	}
}

int mu_abort_block::rollback(int has_path)
{
	cast_block *block;
	int lpc;
	
	for( lpc = 0; lpc < this->children.len; lpc++ ) {
		if( !(this->children.val[lpc]->flags & MABF_CONNECTION) ) {
			this->children.val[lpc]->rollback(0);
		}
	}
	switch( this->flags & MABF_KIND ) {
	case MABK_CONTROL: {
		int child_path = 0;
		int lpc;
		
		/* If we don't start with a path, check if our block
		   is used or not.  */
		if( has_path == 0 )
			has_path = this->block_label->
				cast_stmt_u_u.s_label.users != 0;
		/* Since there's nothing in the CAST to indicate
		   path relationships between the children we have
		   to go through them individually here. */
		for( lpc = 0; lpc < this->children.len; lpc++ ) {
			if( this->children.val[lpc]->flags &
			    MABF_INLINE ) {
				this->children.val[lpc]->clear_reaper_label();
			}
			if( this->children.val[lpc]->flags &
			    MABF_CONNECTION ) {
				child_path = this->children.val[lpc]->
					rollback(has_path) || child_path;
			}
		}
		has_path = child_path;
		break;
	}
	default:
		/* If we don't start with a path, check if our block
		   is used or not.  */
		if( has_path == 0 )
			has_path = this->block_label->
				cast_stmt_u_u.s_label.users != 0;
		/* Rollback our root block */
		block = &this->block_stmts->cast_stmt_u_u.block;
		has_path = this->rollback_block(block, 0,
						block->stmts.stmts_len,
						has_path);
		break;
	}
	return( has_path );
}

int mu_abort_block::rollback_stmt(cast_block */*block*/, int /*pos*/,
				  cast_stmt curr_stmt, int has_path)
{
	switch( curr_stmt->kind ) {
	case CAST_STMT_TEXT:
		/* Ignore text statements */
		curr_stmt = 0;
		break;
	case CAST_STMT_LABEL: {
		struct mu_abort_block *mab;
		
		/* If the label has users then we have a path, else
		   we need to rollback the statement. */
		if( curr_stmt->cast_stmt_u_u.s_label.users )
			has_path = 1;
		/* See if the label is from an inlined child,
		   if so rollback the child and check if it
		   has a path, else just rollback the statement. */
		if( (mab = this->find_child(curr_stmt->cast_stmt_u_u.
					    s_label.label)) ) {
			mab->clear_reaper_label();
			has_path = mab->rollback(has_path);
		} else {
			has_path = this->
				rollback_stmt(0, 0,
					      curr_stmt->cast_stmt_u_u.
					      s_label.stmt,
					      has_path);
			if( curr_stmt->cast_stmt_u_u.s_label.stmt->kind ==
			    CAST_STMT_RETURN )
				curr_stmt = 0;
		}
		break;
	}
	case CAST_STMT_GOTO: {
		struct mu_abort_block *mab;
		
		if( curr_stmt->cast_stmt_u_u.goto_label[0] == 0 ) {
			curr_stmt->cast_stmt_u_u.goto_label = this->
				reaper_label->cast_stmt_u_u.s_label.label;
			this->reaper_label->cast_stmt_u_u.s_label.users++;
		}
		if( has_path ) {
			if( (mab = this->find_child(curr_stmt->cast_stmt_u_u.
						    goto_label)) ) {
				if( (mab->flags & MABF_CONTROL) &&
				    mab->stmt_count() )
					mab->rollback(1);
				else if( mab->stmt_count() == 0 ) {
					curr_stmt->kind = CAST_STMT_EMPTY;
					mab->drop_block_label();
					mab->rollback(0);
				} else {
					mab->rollback(has_path);
				}
			}
		}
		else {
			if( (mab = this->find_child(curr_stmt->cast_stmt_u_u.
						    goto_label)) ) {
				/* If the goto jumps to a child then rollback
				   the child. */
				mab->drop_block_label();
				mab->rollback(has_path);
			}
			else if( !strcmp(curr_stmt->cast_stmt_u_u.goto_label,
					 this->reaper_label->
					 cast_stmt_u_u.s_label.label) ) {
				/* If its a jump to the reaper label then
				   we need to decrement its user count.
				   This is sort of a notable case since
				   the label doesn't exist in our block
				   or any of our children unlike the
				   previous cases. */
				this->reaper_label->cast_stmt_u_u.
					s_label.users--;
			}
		}
		break;
	}
	case CAST_STMT_BLOCK: {
		cast_block *nest_block;
		
		/* Just rollback the block like any other */
		nest_block = &curr_stmt->cast_stmt_u_u.block;
		has_path = this->rollback_block(nest_block, 0,
						nest_block->
						stmts.stmts_len,
						has_path);
		break;
	}
	case CAST_STMT_SWITCH:
		this->rollback_stmt(0, 0,
				    curr_stmt->
				    cast_stmt_u_u.
				    s_switch.stmt,
				    has_path);
		break;
	case CAST_STMT_CASE:
		this->rollback_stmt(0, 0,
				    curr_stmt->
				    cast_stmt_u_u.s_case.
				    stmt, has_path);
		break;
	case CAST_STMT_DEFAULT:
		this->rollback_stmt(0, 0,
				    curr_stmt->
				    cast_stmt_u_u.
				    default_stmt, has_path);
		break;
	case CAST_STMT_IF:
		this->rollback_stmt(0, 0,
				    curr_stmt->cast_stmt_u_u.s_if.
				    true_stmt, has_path);
		if( curr_stmt->cast_stmt_u_u.s_if.false_stmt )
			this->rollback_stmt(0, 0,
					    curr_stmt->
					    cast_stmt_u_u.s_if.
					    false_stmt, has_path);
		break;
	case CAST_STMT_RETURN:
		has_path = 0;
		curr_stmt = 0;
		break;
	default:
		break;
	}
	if( !has_path && curr_stmt )
		curr_stmt->kind = CAST_STMT_EMPTY;
	return( has_path );
}

int mu_abort_block::rollback_block(cast_block *block, int start,
				    int len, int has_path)
{
	cast_stmt curr_stmt;
	int lpc;
	
	if( block->flags & CAST_BLOCK_REVERSE ) {
		/* We need to walk through the block in reverse otherwise
		   the path information will be incorrect */
		for( lpc = (start + len) - 1; lpc >= 0; lpc-- ) {
			curr_stmt = block->stmts.stmts_val[lpc];
			has_path = this->rollback_stmt(block, lpc,
						       curr_stmt, has_path);
		}
	}
	else {
		for( lpc = start; lpc < len; lpc++ ) {
			curr_stmt = block->stmts.stmts_val[lpc];
			has_path = this->rollback_stmt(block, lpc,
						       curr_stmt, has_path);
		}
	}
	return( has_path );
}

/* End of file. */

