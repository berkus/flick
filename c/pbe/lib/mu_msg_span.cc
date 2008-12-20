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

#include <assert.h>
#include <limits.h>

#include <mom/compiler.h>
#include <mom/c/libcast.h>
#include <mom/c/pbe.hh>

#define   min(a,b)        (a < b ? a : b)

cast_expr mu_msg_span::macro_name = 0;

void mu_msg_span::set_be_name(const char *be_name)
{
	macro_name = cast_new_expr_name(flick_asprintf("flick_%s_check_span",
						       be_name));
}

mu_msg_span::mu_msg_span()
{
	this->flags = 0;
	this->kind = MSK_NONE;
	this->parent = 0;
	this->size = 0;
	this->size_expr = 0;
	this->check_stmt =
		cast_new_stmt_expr(cast_new_expr_call_2(macro_name,0,0));
	this->block = 0;
	this->stmt_pos = -1;
	this->a_block = 0;
	this->children.val = 0;
	this->children.len = 0;
	this->children.min_size = UINT_MAX;
}

mu_msg_span::~mu_msg_span()
{
}

void mu_msg_span::set_flags(int the_flags)
{
	this->flags = the_flags;
}

int mu_msg_span::get_flags()
{
	return( this->flags );
}

void mu_msg_span::set_kind(int the_kind)
{
	this->kind = the_kind;
}

int mu_msg_span::get_kind()
{
	return( this->kind );
}

void mu_msg_span::set_block(cast_stmt the_block)
{
	this->block = the_block;
}

cast_stmt mu_msg_span::get_block()
{
	return( this->block );
}

void mu_msg_span::set_abort(struct mu_abort_block *mab)
{
	this->a_block = mab;
}

struct mu_abort_block *mu_msg_span::get_abort()
{
	return( this->a_block );
}

void mu_msg_span::grow(int increment)
{
	this->size += increment;
}

void mu_msg_span::shrink(int decrement)
{
	this->size -= decrement;
}

void mu_msg_span::set_size(cast_expr the_size)
{
	this->size_expr = the_size;
}

void mu_msg_span::mod_size(cast_binary_op op, struct mu_msg_span *mms)
{
	if( mms->size_expr ) {
		if( this->size_expr )
			this->size_expr = cast_new_binary_expr(op,
							       this->size_expr,
							       mms->size_expr);
		else
			this->size_expr = mms->size_expr;
	} else if( mms->size && this->size_expr ) {
		this->size_expr = cast_new_binary_expr(
			op,
			this->size_expr,
			cast_new_expr_lit_int(mms->size, 0));
		mms->size = 0;
	}
	this->size += mms->size;
}

void mu_msg_span::absorb_span(cast_binary_op op, struct mu_msg_span *mms)
{
	int absorbed = 0;
	int lpc;
	
	switch( this->kind ) {
	case MSK_UNION: {
		struct mu_msg_span mms_cp;
		
		for( lpc = 0; lpc < this->children.len; lpc++ ) {
			mms_cp = *mms;
			this->children.val[lpc]->absorb_span(op, &mms_cp);
			absorbed = 1;
		}
		break;
	}
	case MSK_ARRAY:
		this->children.val[0]->absorb_span(op, mms);
		absorbed = 1;
		break;
	case MSK_SEQUENTIAL:
		if( this->children.len ) {
			this->children.val[this->children.len - 1]->
				absorb_span(op, mms);
			absorbed = 1;
		}
		break;
	default:
		break;
	}
	if( !absorbed ) {
		this->mod_size(op, mms);
	}
}

void mu_msg_span::drop()
{
	if( (this->check_stmt->kind != CAST_STMT_EMPTY) ) {
		this->check_stmt->kind = CAST_STMT_EMPTY;
		if( this->a_block && (this->stmt_pos != -1) ) {
			this->a_block->
				drop_label(this->check_stmt->cast_stmt_u_u.
					   expr->cast_expr_u_u.call.params.
					   cast_expr_array_val[1]->
					   cast_expr_u_u.name.
					   cast_scoped_name_val[0].name);
		}
	}
}

void mu_msg_span::add_child(struct mu_msg_span *mms)
{
	int i;
	
	switch( this->kind ) {
	case MSK_ARRAY: {
		/*
		 * If the sequential that makes up the element is completely
		 * constant then we can just move the checks out of the loop.
		 */
		mms->collapse();
		if( (mms->kind == MSK_NONE) &&
		    !(mms->flags & MSF_ALIGN) &&
		    ((mms->size_expr && cast_expr_const(mms->size_expr)) ||
		     !mms->size_expr) && mms->size) {
			this->mod_size(CAST_BINARY_MUL, mms);
			this->kind = MSK_NONE;
			mms->drop();
		} else {
			this->size_expr = 0;
			this->drop();
		}
		break;
	}
	default:
		break;
	}
	i = this->children.len++;
	this->children.val = (struct mu_msg_span **)
		mustrealloc(this->children.val,
			    this->children.len *
			    sizeof(struct mu_msg_span *));
	this->children.val[i] = mms;
	mms->parent = this;
}

void mu_msg_span::rem_child(struct mu_msg_span *mms)
{
	int found = 0, lpc;
	
	for( lpc = 0; (lpc < this->children.len) && !found; lpc++ ) {
		if( mms == this->children.val[lpc] )
			found = 1;
	}
	for( ; lpc < this->children.len; lpc++ ) {
		this->children.val[lpc - 1] = this->children.val[lpc];
	}
	this->children.len--;
}

void mu_msg_span::begin()
{
	if( this->kind == MSK_SEQUENTIAL )
		return;
	assert(this->block);
	/* Add the check */
	this->check_stmt->cast_stmt_u_u.expr->cast_expr_u_u.call.
		params.cast_expr_array_val[1] =
		cast_new_expr_name(a_block->use_current_label());
	cast_block_add_stmt(&this->block->cast_stmt_u_u.block,
			    this->check_stmt);
	this->stmt_pos = this->block->cast_stmt_u_u.block.stmts.
		stmts_len - 1;
}

void mu_msg_span::end()
{
	/* Resolve two sizes into an expression if needed */
	if( this->size_expr && this->size ) {
		this->size_expr = cast_new_binary_expr(
			CAST_BINARY_ADD,
			this->size_expr,
			cast_new_expr_lit_int(this->size, 0));
		this->size = 0;
	}
}

void mu_msg_span::collapse()
{
	switch( this->kind ) {
	case MSK_UNION: {
		int lpc;
		
		/*
		 * Figure out which of our children we can drop and then
		 * grow ourself to compensate.  We can only drop those with
		 * the same size as the min since the others still have to
		 * check the rest of the span.
		 */
		for( lpc = 0; lpc < this->children.len; lpc++ ) {
			this->children.val[lpc]->collapse();
			if( this->children.val[lpc]->size_expr )
				this->children.min_size = 0;
			else
				this->children.min_size =
					min(this->children.min_size,
					    this->children.val[lpc]->size);
		}
		for( lpc = 0; lpc < this->children.len; lpc++ ) {
			if( !this->children.val[lpc]->size_expr &&
			    (this->children.min_size ==
			     this->children.val[lpc]->size) ) {
				this->children.val[lpc]->drop();
			}
		}
		if( this->children.len == 1 ) {
			struct mu_msg_span *child;
			
			child = this->children.val[0];
			if( !(child->flags & MSF_ALIGN) ) {
				this->size = this->children.val[0]->size;
				this->size_expr =
					this->children.val[0]->size_expr;
				this->children.val[0]->drop();
			}
		} else if( this->children.len )
			this->grow(this->children.min_size);
		else
			this->drop();
		break;
	}
	case MSK_SEQUENTIAL: {
		struct mu_msg_span *child_pred, *child;
		int lpc;
		
		/*
		 * Walk backwards through the list of children and try to
		 * combine them.  We can only do this if the values are
		 * constant though, since otherwise they might be variables
		 * which haven't been initialized yet.
		 */
		for( lpc = this->children.len - 1; lpc > 0; lpc-- ) {
			child = this->children.val[lpc];
			child_pred = this->children.val[lpc - 1];
			child->collapse();
			if( !(child->flags & MSF_ALIGN) &&
			    (child_pred->check_stmt->kind !=
			     CAST_STMT_EMPTY) &&
			    ((child->size_expr &&
			      cast_expr_const(child->size_expr)) ||
			     !child->size_expr) ) {
				child_pred->
					absorb_span(CAST_BINARY_ADD,
						    child);
				child->drop();
			}
		}
		if( this->children.len == 1 ) {
			if( !(this->children.val[0]->flags & MSF_ALIGN) )
				*this = *this->children.val[0];
		} else if( this->children.len ) {
			struct mu_msg_span *child;
			
			child = this->children.val[0];
			if( !(child->flags & MSF_ALIGN) &&
			    ((child->size_expr &&
			      cast_expr_const(child->size_expr)) ||
			     !child->size_expr) ) {
				this->size = child->size;
				this->size_expr = child->size_expr;
				this->block = child->block;
				this->check_stmt = child->check_stmt;
				this->stmt_pos = child->stmt_pos;
				this->a_block = child->a_block;
			}
		} else {
			this->kind = MSK_NONE;
			this->size = 0;
			this->size_expr = 0;
		}
		break;
	}
	default:
		break;
	}
}

void mu_msg_span::commit()
{
	int lpc;
	
	for( lpc = 0; lpc < this->children.len; lpc++ ) {
		this->children.val[lpc]->commit();
	}
	if( this->size ) {
		cast_expr expr = cast_new_expr_lit_int(this->size, 0);
		
		if( this->size_expr )
			this->size_expr = cast_new_binary_expr(CAST_BINARY_ADD,
							       this->size_expr,
							       expr);
		else
			this->size_expr = expr;
	}
	if( this->size_expr )
		this->check_stmt->cast_stmt_u_u.expr->cast_expr_u_u.call.
			params.cast_expr_array_val[0] = this->size_expr;
	else
		this->drop();
}

/* End of file. */

