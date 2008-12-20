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

#include <ctype.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <mom/c/scml.hh>

scml_parser::scml_parser()
{
	this->ssp = 0;
}

scml_parser::~scml_parser()
{
}

void scml_parser::set_stream_pos(struct scml_stream_pos *the_ssp)
{
	this->ssp = the_ssp;
}

struct scml_stream_pos *scml_parser::get_stream_pos()
{
	return( this->ssp );
}

struct scml_token scml_parser::collapse(struct scml_token_stack *values,
					struct scml_token *oper)
{
	struct scml_token retval;
	
	retval.kind = SCML_ERROR;
	switch( oper->kind ) {
	case SCML_NT_COLON:
		if( (values->count() >= 2) &&
		    values->index(-1)->is_expr() &&
		    values->index(0)->is_expr() ) {
			retval.kind = SCML_NT_TAG;
			retval.value.children = new scml_token[2];
			retval.value.children[1] = *values->index(0);
			values->pop();
			retval.value.children[0] = *values->index(0);
			values->pop();
		}
		break;
	case SCML_TERM_RPAREN:
		retval.kind = SCML_NT_EXPR;
		retval.value.children = new scml_token[1];
		retval.value.children[0] = *values->index(0);
		values->pop();
		break;
	case SCML_TERM_RBRACE:
		if( (values->count() >= 2) &&
		    values->index(-1)->is_expr() &&
		    values->index(0)->is_expr() ) {
			retval.kind = SCML_NT_SEL;
			retval.value.children = new scml_token[2];
			retval.value.children[1] = *values->index(0);
			values->pop();
			retval.value.children[0] = *values->index(0);
			values->pop();
		}
		break;
	case SCML_TERM_RCURLY: {
		int lpc, args;
		
		for( args = 0; values->index(-args)->kind != SCML_TERM_LCURLY;
		     args++ );
		retval.kind = SCML_NT_INIT;
		retval.value.children = new scml_token[args + 1];
		retval.value.children[args].kind = SCML_NONE;
		for( lpc = 0; lpc < args; lpc++ ) {
			retval.value.children[args - lpc - 1] =
				*values->index(0);
			values->pop();
		}
		values->pop();
		break;
	}
	case SCML_TERM_GT: {
		int lpc, args;
		
		for( args = 0; values->index(-args)->kind != SCML_TERM_LT;
		     args++ );
		if( args ) {
			retval.kind = SCML_NT_COMMAND;
			retval.value.children = new scml_token[args + 1];
			retval.value.children[args].kind = SCML_NONE;
			for( lpc = 0; lpc < args; lpc++ ) {
				retval.value.children[args - lpc - 1] =
					*values->index(0);
				values->pop();
			}
			values->pop();
		}
		break;
	}
	case SCML_NT_SCOPE_RES:
		if( (values->count() >= 2) ) {
			oper->value.children = new scml_token[2];
			oper->value.children[1] = *values->index(0);
			values->pop();
			oper->value.children[0] = *values->index(0);
			values->pop();
			retval.kind = SCML_NT_NAME;
			retval.value.children = new scml_token;
			retval.value.children[0] = *oper;
		}
		break;
	case SCML_NT_ASSIGN:
		if( (values->count() >= 2) &&
		    (values->index(-1)->is_expr()) &&
		    values->index(0)->is_expr() ) {
			oper->kind = SCML_NT_SET;
			oper->value.children = new scml_token[2];
			oper->value.children[1] = *values->index(0);
			values->pop();
			oper->value.children[0] = *values->index(0);
			values->pop();
			retval.kind = SCML_NT_EXPR;
			retval.value.children = new scml_token;
			retval.value.children[0] = *oper;
		}
		break;
	case SCML_NT_NOT:
		if( (values->count() >= 1) &&
		    (values->index(0)->is_expr()) ) {
			oper->value.children = new scml_token[1];
			oper->value.children[0] = *values->index(0);
			values->pop();
			retval.kind = SCML_NT_EXPR;
			retval.value.children = new scml_token;
			retval.value.children[0] = *oper;
		}
		break;
	case SCML_NT_PLUS:
	case SCML_NT_MINUS:
	case SCML_NT_DIV:
	case SCML_NT_MOD:
	case SCML_NT_MULT:
	case SCML_NT_EQUAL:
	case SCML_NT_NOT_EQUAL:
	case SCML_NT_LT:
	case SCML_NT_GT:
	case SCML_NT_LE:
	case SCML_NT_GE:
	case SCML_NT_AND:
	case SCML_NT_LAND:
	case SCML_NT_OR:
	case SCML_NT_LOR:
	case SCML_NT_DOT:
		if( (values->count() >= 2) &&
		    values->index(0)->is_expr() &&
		    values->index(-1)->is_expr() ) {
			oper->value.children = new scml_token[2];
			oper->value.children[1] = *values->index(0);
			values->pop();
			oper->value.children[0] = *values->index(0);
			values->pop();
			retval.kind = SCML_NT_EXPR;
			retval.value.children = new scml_token;
			retval.value.children[0] = *oper;
		} else {
			scml_alert(this->ssp, SAF_ERROR|SAF_GENERAL,
				   "Bad args for %s (%d:%d:%d)",
				   scml_kind_map[oper->kind],
				   values->count(),
				   values->index(0)->is_expr(),
				   values->index(-1)->is_expr());
			printf("index(0) "); values->index(0)->print(); printf("\n");
			printf("index(-1) "); values->index(-1)->print(); printf("\n");
		}
		break;
	default:
		break;
	}
	values->push(retval);
	return( retval );
}

struct scml_token_sequence *scml_parser::parse()
{
	struct scml_token_stack values, operators;
	struct scml_token_sequence *retval = 0;
	struct scml_token curr, last;
	int lpc, done = 0, error = 0;
	
	last.kind = SCML_NONE;
	while( !done ) {
		curr = this->ssp->get_token();
		if( curr.kind == SCML_DONE ) {
			done = 1;
		} else if( curr.is_value() ) {
			while( last.is_value() &&
			       operators.count() &&
			       !operators.index(0)->is_container() ) {
				this->collapse(&values,
					       operators.index(0));
				operators.pop();
			}
			values.push(curr);
			last = curr;
		}
		else if( curr.is_operator() ) {
			if( (curr.kind == SCML_NT_DIV) && values.count() &&
			    (values.index(0)->kind == SCML_TERM_LT) ) {
				curr.kind = SCML_TERM_SLASH;
				values.push(curr);
			} else {
				if( operators.count() &&
				    !operators.index(0)->is_container() &&
				    (curr.get_prec() <=
				     operators.index(0)->get_prec()) ) {
					this->collapse(&values,
						       operators.index(0));
					operators.pop();
				}
				operators.push(curr);
			}
			last = curr;
		}
		else if( curr.is_container() ) {
			switch( curr.kind ) {
			case SCML_TERM_LT:
				values.push(curr);
				break;
			case SCML_TERM_GT:
				while( operators.count() ) {
					this->collapse(&values,
						       operators.index(0));
					operators.pop();
				}
				last = this->collapse(&values,
						      &curr);
				break;
			case SCML_TERM_LPAREN:
				operators.push(curr);
				break;
			case SCML_TERM_RPAREN:
				while( operators.count() &&
				       (operators.index(0)->kind !=
					SCML_TERM_LPAREN) ) {
					this->collapse(&values,
						       operators.index(0));
					operators.pop();
				}
				if( operators.count() ) {
					operators.pop();
					last = this->collapse(&values, &curr);
				} else {
					scml_alert(this->ssp,
						   SAF_ERROR|SAF_LEXICAL,
						   "Mismatched parentheses");
					error = 1;
					done = 1;
				}
				break;
			case SCML_TERM_LBRACE:
				if( operators.count() &&
				    !operators.index(0)->is_container() &&
				    (curr.get_prec() <=
				     operators.index(0)->get_prec()) ) {
					this->collapse(&values,
						       operators.index(0));
					operators.pop();
				}
				operators.push(curr);
				break;
			case SCML_TERM_RBRACE:
				while( operators.count() &&
				       (operators.index(0)->kind !=
					SCML_TERM_LBRACE) ) {
					this->collapse(&values,
						       operators.index(0));
					operators.pop();
				}
				if( operators.count() ) {
					operators.pop();
					last = this->collapse(&values, &curr);
				} else {
					scml_alert(this->ssp,
						   SAF_ERROR|SAF_LEXICAL,
						   "Mismatched braces");
					error = 1;
					done = 1;
				}
				break;
			case SCML_TERM_LCURLY:
				values.push(curr);
				operators.push(curr);
				break;
			case SCML_TERM_RCURLY:
				while( operators.count() &&
				       operators.index(0)->kind !=
				       SCML_TERM_LCURLY ) {
					this->collapse(&values,
						       operators.index(0));
					operators.pop();
				}
				if( operators.count() ) {
					operators.pop();
					last = this->collapse(&values, &curr);
				} else {
					scml_alert(this->ssp,
						   SAF_ERROR|SAF_LEXICAL,
						   "Mismatched braces");
					error = 1;
					done = 1;
				}
				break;
			default:
				break;
			}
		} else {
			switch( curr.kind ) {
			case SCML_IGNORE:
				break;
			case SCML_COL_POS:
				values.push(curr);
				break;
			case SCML_ROW_POS:
				values.push(curr);
				break;
			case SCML_ERROR:
				error = 1;
				scml_alert(this->ssp, SAF_ERROR|SAF_GENERAL,
					   "Invalid token");
				break;
			default:
				scml_alert(this->ssp, SAF_ERROR|SAF_GENERAL,
					   "Token not handled");
				curr.print(); printf("\n");
				error = 1;
				done = 1;
				break;
			}
		}
	}
	if( !error ) {
		retval = new scml_token_sequence();
		retval->set_value(new scml_token[values.count()]);
		retval->set_length(values.count());
		retval->set_stream_pos(this->ssp);
		for( lpc = 0; lpc < values.count(); lpc++ ) {
			retval->get_value()[values.count() - lpc - 1] =
				*values.index(-lpc);
		}
	} else {
		scml_alert(this->ssp, SAF_ERROR|SAF_GENERAL,
			   "Too many errors, bailing out");
	}
	return( retval );
}
