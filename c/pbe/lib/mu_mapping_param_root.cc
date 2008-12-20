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

#include <mom/compiler.h>
#include <mom/c/libcast.h>
#include <mom/c/pbe.hh>

/*
 * This function is a helper for `mu_state::mu_mapping_param_root' below.
 *
 * Briefly: `insert_actual_param' makes an actual-for-formal parameter
 * substitution in an expression that describes the invocation of a function.
 *
 * Not so briefly: `insert_actual_param' inserts `actual_param_cexpr', a CAST
 * expression describing an actual function parameter value, into `actual_func_
 * invocation_cexpr', which is a C expression describing a function invocation.
 * The `actual_param_cexpr' is inserted into `actual_func_invocation_cexpr' at
 * the point corresponding to the formal parameter `formal_param_cexpr', which
 * must be a CAST name.  The position of `formal_param_cexpr' in `formal_func_
 * invocation_cexpr' determines the corresponding position at which the actual
 * value `actual_param_cexpr' will be placed in `actual_func_invocation_cexpr'.
 *
 * The `formal_func_invocation_cexpr' and `actual_func_invocation_cexpr' are
 * supposed to be identical, except that `actual_func_invocation_cexpr' will
 * contain ``holes'' (null CAST expressions) in places at which no actual-for-
 * formal substitution has yet been made.
 */
static void insert_actual_param(
	cast_expr formal_func_invocation_cexpr,
	cast_expr actual_func_invocation_cexpr,
	cast_expr formal_param_cexpr,
	cast_expr actual_param_cexpr)
{
	cast_scoped_name	*formal_param_name;
	
	cast_expr		*formal_lvalue;
	cast_expr		formal_funcall;
	cast_expr		formal_func_cexpr;
	cast_expr_array		*formal_params;
	
	cast_expr		*actual_lvalue;
	cast_expr		actual_funcall;
	cast_expr		actual_func_cexpr;
	cast_expr_array		*actual_params;
	
	unsigned int i;
	int found;
	
	/*****/
	
	/*
	 * Sanity checks.
	 */
	if (formal_param_cexpr->kind != CAST_EXPR_NAME)
		panic("In `insert_actual_param', bad formal parameter name.");
	formal_param_name = &(formal_param_cexpr->cast_expr_u_u.name);
	
	if (!formal_func_invocation_cexpr)
		panic("In `insert_actual_param', "
		      "the formal function invocation expression is null.");
	if (!actual_func_invocation_cexpr)
		panic("In `insert_actual_param', "
		      "the actual function invocation expression is null.");
	
	if (formal_func_invocation_cexpr->kind
	    != actual_func_invocation_cexpr->kind)
		panic("In `insert_actual_param', "
		      "the actual and formal CAST expressions are of "
		      "different kinds.");
	
	/*
	 * Locate the components of the formal and actual invocation cexprs.
	 */
	switch (formal_func_invocation_cexpr->kind) {
	case CAST_EXPR_BINARY:
		/*
		 * _return = work_func(param, param, ...);
		 */
		formal_lvalue  = &(formal_func_invocation_cexpr->
				   cast_expr_u_u.binary.expr[0]);
		formal_funcall = formal_func_invocation_cexpr->
				 cast_expr_u_u.binary.expr[1];
		actual_lvalue  = &(actual_func_invocation_cexpr->
				   cast_expr_u_u.binary.expr[0]);
		actual_funcall = actual_func_invocation_cexpr->
				 cast_expr_u_u.binary.expr[1];
		
		break;
		
	case CAST_EXPR_CALL:
		/*
		 * work_func(param, param, ...);
		 */
		formal_lvalue  = 0;
		formal_funcall = formal_func_invocation_cexpr;
		actual_lvalue  = 0;
		actual_funcall = actual_func_invocation_cexpr;
		break;
		
	default:
		panic("In `insert_actual_param', "
		      "don't know how to handle invocation expression.");
		break;
	}
	
	/*
	 * More sanity checks.
	 */
	if (formal_lvalue
	    && ((*formal_lvalue)->kind != CAST_EXPR_NAME))
		panic("In `insert_actual_param', "
		      "the formal lvalue CAST expression is not a name.");
	
	if (formal_funcall->kind != CAST_EXPR_CALL)
		panic("In `insert_actual_param', "
		      "the formal funcall CAST expression is not a funcall.");
	
	if (actual_funcall->kind != CAST_EXPR_CALL)
		panic("In `insert_actual_param', "
		      "the actual funcall CAST expression is not a funcall.");
	
	/*
	 * Special case: we're making a substitution of the return value.
	 */
	if (formal_lvalue) {
		if (cast_cmp_scoped_names(
			formal_param_name,
			&((*formal_lvalue)->cast_expr_u_u.name))
		    != 0) {
			/* Our formal ``parameter'' is really the retval. */
			if (*actual_lvalue != 0)
				panic("In `insert_actual_param', "
				      "attempted to set an already-set actual "
				      "return value.");
			*actual_lvalue = actual_param_cexpr;
			return;
		}
	}
	
	/*
	 * Special case: we're making a substitution for the invoked object.
	 *
	 * XXX --- Our expression parsing should be more robust, and should be
	 * probably be handled in a separate function.  We currently handle
	 * only `obj.method' and `obj->method'.
	 */
	formal_func_cexpr = formal_funcall->cast_expr_u_u.call.func;
	actual_func_cexpr = actual_funcall->cast_expr_u_u.call.func;
	
	if (formal_func_cexpr->kind == CAST_EXPR_SEL) {
		found = 0;
		switch (formal_func_cexpr->cast_expr_u_u.sel.var->kind) {
		case CAST_EXPR_NAME:
			if (/* The object name matches our formal name... */
			    (cast_cmp_scoped_names(
				    formal_param_name,
				    &(formal_func_cexpr->
				      cast_expr_u_u.sel.var->
				      cast_expr_u_u.name))
			     != 0)
			    &&
			    /* ...and it's an actual hole. */
			    (actual_func_cexpr->cast_expr_u_u.sel.var == 0)
				) {
				/*
				 * Our formal ``parameter'' is really the
				 * object reference.
				 */
				actual_func_cexpr->cast_expr_u_u.sel.var
					= actual_param_cexpr;
				found = 1;
			}
			break;
			
		case CAST_EXPR_UNARY:
			if (/* The unary operand is a name... */
			    (formal_func_cexpr->
			     cast_expr_u_u.sel.var->
			     cast_expr_u_u.unary.expr->kind
			     == CAST_EXPR_NAME)
			    &&
			    /* ...and the name matches our formal name... */
			    (cast_cmp_scoped_names(
				    formal_param_name,
				    &(formal_func_cexpr->
				      cast_expr_u_u.sel.var->
				      cast_expr_u_u.unary.expr->
				      cast_expr_u_u.name))
			     != 0)
			    &&
			    /* ...and it's an actual hole. */
			    (actual_func_cexpr->
			     cast_expr_u_u.sel.var->
			     cast_expr_u_u.unary.expr
			     == 0)
				) {
				/*
				 * Our formal ``parameter'' is really the
				 * object reference.
				 */
				actual_func_cexpr->
					cast_expr_u_u.sel.var->
					cast_expr_u_u.unary.expr
					= actual_param_cexpr;
				found = 1;
			}
			break;
			
		default:
			panic("In `insert_actual_param', "
			      "can't parse fancy method invocation.");
			break;
		}
		
		/* If we made the actual-for-formal match, we're done. */
		if (found)
			return;
	}
	
	/*
	 * Regular case: search the formal parameters.
	 */
	formal_params = &(formal_funcall->cast_expr_u_u.call.params);
	actual_params = &(actual_funcall->cast_expr_u_u.call.params);
	
	if (formal_params->cast_expr_array_len
	    != actual_params->cast_expr_array_len)
		panic("In `insert_actual_param', "
		      "the formal and actual funcalls have arglists of "
		      "different lengths.");
	
	found = 0;
	for (i = 0; i < formal_params->cast_expr_array_len; ++i) {
		cast_expr this_param = formal_params->cast_expr_array_val[i];
		
		if (this_param->kind != CAST_EXPR_NAME)
			panic("In `insert_actual_param', "
			      "a formal parameter is not a CAST name.");
		
		if (cast_cmp_scoped_names(formal_param_name,
					  &(this_param->cast_expr_u_u.name))
		    != 0) {
			/* We've found the formal; fill in the actual. */
			if (actual_params->cast_expr_array_val[i] != 0)
				panic("In `insert_actual_param', "
				      "attempted to set an already-set actual "
				      "parameter.");
			
			actual_params->cast_expr_array_val[i]
				= actual_param_cexpr;
			found = 1;
		}
	}
	
	if (!found)
		panic("In `insert_actual_param', "
		      "the corresponding formal parameter was not found.");
	
	return;
}


/*****************************************************************************/

/*
 * A `pres_c_mapping_param_root' node indicates that the current CAST
 * expression and CAST type describe a formal parameter to a client stub or
 * server work function: i.e., the name and type of a parameter in the function
 * type signature.
 *
 * When the BE is generating some request-unmarshaling code within a server
 * dispatch function, a `pres_c_mapping_param_root' node tells the back end to
 * allocate the ``root'' of an actual parameter that will be given to the
 * server work function.  The type of the formal parameter may be transformed
 * in order to create a more efficient actual parameter: for instance, formal
 * array types may be transformed into actual pointer-to-element types in order
 * to allow for unmarshaling optimizations.
 *
 * When the BE is generating a client stub, a `pres_c_mapping_param_root' node
 * currently has no effect.
 *
 * XXX --- The initial implementation of this method simply replaces logic that
 * used to happen in the `mu_state::mu_server_func' auxiliary functions `whack_
 * on_server_cfunct' and `mu_state::mu_server_func_alloc_param'.  But soon,
 * this method will be enhanced to handle new kinds of parameter allocations,
 * i.e., C++ references and constrcuted objects.
 */

/*
 * XXX --- Below is the comment from `whack_on_server_work_cfunct', which has
 * been replaced by this method.  The comments that are still germane should be
 * folded into the comments above.
 */
/*
 * XXX --- The following function is a hack to change the names of the server
 * work function parameters and their types (to remove CAST_TYPE_QUALIFIERs,
 * and to change array types into pointer-to-element types).
 *
 * + Name changes are good because they help us avoid name clashes in the
 *   generated code (e.g., between the name of a type and the name of a local
 *   variable).
 *
 * + The removal of type qualifiers is necessary so that we don't try to
 *   allocate locals with `const' types!
 *
 * + Changing array types into pointer types helps `mu_server_func' avoid
 *   declaring unnecessary array locals.  Using a pointer helps the back end be
 *   smart, e.g., point into the message buffer when the encoded array is
 *   bitwise identical to the presented array.  This transformation works only
 *   because of the ``array slice hack'' that was implemented in `mu_state::
 *   mu_mapping_stub_inline', which allows us to associate pointer-to-element
 *   CAST types with the marshal/unmarshal stubs for the corresponding array
 *   CAST types.
 *
 * But really, we should do all of these things in a more principled way.
 */

#define LOCAL_PREFIX "_local_"

void mu_state::mu_mapping_param_root(
	cast_expr cexpr,
	cast_type ctype,
	mint_ref itype,
	pres_c_mapping_param_root *root_map)
{
	cast_scoped_name *formal_scoped_name;
	char *formal_simple_name;
	
	char *actual_simple_name;
	cast_expr actual_cexpr;
	cast_type actual_ctype;
	
	cast_type internal_ctype;
	
	/*****/
	
	if (cexpr->kind != CAST_EXPR_NAME)
		panic("In `mu_state::mu_mapping_param_root', "
		      "the parameter CAST expression is not a name.");
	if (cast_scoped_name_is_empty(&(cexpr->cast_expr_u_u.name)))
		panic("In `mu_state::mu_mapping_param_root', "
		      "the parameter name is empty.");
	
	formal_scoped_name = &(cexpr->cast_expr_u_u.name);
	formal_simple_name
		= (formal_scoped_name->
		   cast_scoped_name_val[
			   formal_scoped_name->cast_scoped_name_len - 1].
		   name);
	
	if (   !strcmp(get_which_stub(), "client")
	    || !strcmp(get_which_stub(), "send")
	    || !strcmp(get_which_stub(), "msg")) {
		/*
		 * Simple: use the formal parameter name.  If we were told to
		 * view the parameter as an instance of a specific type, do so;
		 * otherwise, just use the formal parameter type.
		 */
		actual_cexpr = cexpr;
		if (root_map->ctype != 0)
			actual_ctype = root_map->ctype;
		else
			actual_ctype = ctype;
		
	} else if (!strcmp(get_which_stub(), "server")) {
		/**************************************************************
		 *
		 * Logic stolen from `whack_on_server_work_cfunct', which has
		 * been replaced by this method.
		 */
		/*
		 * Determine the new name.  Ultimately, this name will be used
		 * to declare a local variable in the server dispatch function.
		 */
		if (formal_simple_name[0] == '_') {
			/*
			 * Don't munge the name if it starts with an `_'.  Some
			 * PG/BE code thinks that it knows the names of these
			 * parameters (e.g., that `_ev' is the environment.).
			 */
			actual_simple_name = formal_simple_name;
		} else {
			actual_simple_name
				= ((char *)
				   mustmalloc(sizeof(LOCAL_PREFIX)
					      + strlen(formal_simple_name)));
			strcpy(actual_simple_name, LOCAL_PREFIX);
			strcat(actual_simple_name, formal_simple_name);
		}
		
		actual_cexpr
			= cast_new_expr_scoped_name(
				cast_new_scoped_name(actual_simple_name,
						     NULL));
		
		/*
		 * Determine the new type.
		 *
		 * We strip away qualifiers and transform array types into
		 * pointer-to-element types.  By transforming array types into
		 * pointer types, we make it easier for `mu_server_func' to be
		 * intelligent.  (We don't want `mu_server_func' to declare a
		 * local array if it's not necessary!)
		 */
		if (root_map->ctype != 0)
			actual_ctype = root_map->ctype;
		else
			actual_ctype = ctype;
		
		while (actual_ctype->kind == CAST_TYPE_QUALIFIED)
			actual_ctype = actual_ctype->cast_type_u_u.qualified.
				       actual;
		
		if ((actual_ctype->kind == CAST_TYPE_POINTER)
		    && (actual_ctype->cast_type_u_u.pointer_type.target->kind
			== CAST_TYPE_QUALIFIED)
			) {
			/*
			 * Strip away modifiers from the target type, too!
			 */
			cast_type target_ctype = actual_ctype->cast_type_u_u.
						 pointer_type.target;
			
			while (target_ctype->kind == CAST_TYPE_QUALIFIED)
				target_ctype = target_ctype->cast_type_u_u.
					       qualified.actual;
			
			actual_ctype = cast_new_pointer_type(target_ctype);
		}
		
		if (actual_ctype->kind == CAST_TYPE_REFERENCE) {
			/*
			 * Strip away modifiers from the target type, too!
			 */
			cast_type target_ctype = actual_ctype->cast_type_u_u.
						 reference_type.target;
			
			while (target_ctype->kind == CAST_TYPE_QUALIFIED)
				target_ctype = target_ctype->cast_type_u_u.
					       qualified.actual;
			
			actual_ctype = cast_new_reference_type(target_ctype);
		}
		
		/*
		 * Transform array types into pointer-to-element types.  Note
		 * that reference types are not dereferenced here --- see the
		 * comments below.
		 */
		switch (actual_ctype->kind) {
		default:
			/*
			 * General case: the parameter has a type that is not
			 * an interesting type.
			 */
			break;
			
		case CAST_TYPE_ARRAY:
			/*
			 * Easy case: the parameter type is an explicit array
			 * type.
			 */
			actual_ctype =
				cast_new_pointer_type(actual_ctype->
						      cast_type_u_u.array_type.
						      element_type);
			break;
			
		case CAST_TYPE_NAME:
			/*
			 * Difficult case: the parameter type is a named type
			 * that we must resolve in order to see if it is an
			 * array or reference type.
			 */
			internal_ctype = cast_find_typedef_type(&(pres->cast),
								actual_ctype);
			
			if (!internal_ctype) {
				/* We hit an unresolvable name.  Punt. */
				
			} else if (internal_ctype->kind == CAST_TYPE_ARRAY) {
				/* We resolved to an array! */
				actual_ctype
					= cast_new_pointer_type(
						internal_ctype->
						cast_type_u_u.array_type.
						element_type);
				
			} else {
				/* What we finally found wasn't interesting. */
			}
			break;
		}
		
		/*************************************************************/
		
		if (op & MUST_ALLOCATE) {
			/*
			 * Allocate a local variable to act as our parameter
			 * root.  Logic stolen from `mu_state::mu_server_func
			 * _alloc_param', which has been replaced by the code
			 * here.
			 *
			 * If the parameter is a reference type, allocate the
			 * referent type instead of the `actual_ctype'.  We do
			 * *not* remove the reference from the `actual_ctype',
			 * however: that must be done by a subsequent MAPPING_
			 * VAR_REFERENCE node.
			 *
			 * XXX --- Technically, in the case of reference types,
			 * the `actual_ctype' and `actual_cexpr' that we pass
			 * to `mu_mapping' won't quite correspond.  This isn't
			 * a real problem, however.  The benefit of handling
			 * references in this way is that it makes the handling
			 * of references more uniform: references are always
			 * ``dereferenced'' by a MAPPING_VAR_REFERENCE node.
			 */
			if (actual_ctype->kind == CAST_TYPE_REFERENCE)
				add_var(actual_simple_name,
					(actual_ctype->cast_type_u_u.
					 reference_type.target),
					root_map->init,
					CAST_SC_NONE);
			else
				add_var(actual_simple_name,
					actual_ctype,
					root_map->init,
					CAST_SC_NONE);
			
			/*
			 * Adapted from `whack_...' again.
			 */
			insert_actual_param(formal_func_invocation_cexpr,
					    actual_func_invocation_cexpr,
					    cexpr,
					    actual_cexpr);
		}
		
	} else
		/* Handling neither client nor server?  Panic! */
		panic("In `mu_state::mu_mapping_param_root', "
		      "handling neither client stub nor server function.");
	
	/*********************************************************************/
	
	/* Finally, handle the parameter data. */
	mu_mapping(actual_cexpr, actual_ctype, itype, root_map->map);
}

/* End of file. */

