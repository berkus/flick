/*
 * Copyright (c) 1997, 1998, 1999 The University of Utah and
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
#include <string.h>
#include <stdlib.h>

#include <mom/c/libcast.h>
#include <mom/c/pbe.hh>
#include <mom/compiler.h>

mu_state_arglist::mu_state_arglist(const char *aname,
				   mu_state_arglist *aparent)
{
	arg_list = NULL;
	parent = aparent;
	assert(aname);
	arglist_name = aname;
	
	while (aparent) {
		if (strcmp(aparent->arglist_name, aname) == 0) {
			warn(("In mu_state_arglist::mu_state_arglist(), "
			      "Adding duplicate arglist `%s'."), aname);
		}
		aparent = aparent->parent;
	}
}

mu_state_arglist::~mu_state_arglist()
{
	/*
	 * Delete each of the `argument' structures.  Delete the names (because
	 * we allocated them; see `mu_state_arglist::add') but do *not* delete
	 * CAST expressions or CAST types contained within those arguments.
	 * The CAST data are almost certainly referenced from elsewhere.
	 */
	argument *this_arg;
	argument *next_arg;
	
	for (this_arg = arg_list; this_arg; this_arg = next_arg) {
		next_arg = this_arg->next;
		/* These are `malloc'ed, not `new'ed!  See `add', below. */
		free(this_arg->name);
		free(this_arg);
	}
	
	/*
	 * Do *not* delete the `parent' argument list.  That argument list is
	 * shared.
	 */
}

void mu_state_arglist::add(const char *aname, const char *arg)
{
	mu_state_arglist *msa = this;
	argument *tmp = (argument *) mustmalloc(sizeof(argument));
	
	while (msa) {
		if (strcmp(msa->arglist_name, aname) == 0) {
			/* Make sure it doesn't already exist! */
			if (msa->findarg(arg)) {
				panic(("In mu_state_arglist::add(), arglist "
				       "`%s' already contains member `%s'."),
				      aname, arg);
			}
			tmp->name = flick_asprintf("%s", arg);
			tmp->next = msa->arg_list;
			tmp->cexpr = NULL;
			tmp->ctype = NULL;
			msa->arg_list = tmp;
			return;
		}
		msa = msa->parent;
	}
	panic("In mu_state_arglist::add(), arglist `%s' not found!", aname);
}

void mu_state_arglist::remove(const char *aname, const char *arg)
{
	mu_state_arglist *msa = this;
	
	while (msa) {
		if (strcmp(msa->arglist_name, aname) == 0) {
			argument **tmp = &(msa->arg_list);
			
			while (*tmp) {
				if (strcmp(arg, (*tmp)->name) == 0) {
					argument *match = *tmp;
					
					/*
					 * Remove the matched `argument' from
					 * the list.
					 */
					*tmp = (*tmp)->next;
					/*
					 * Free the data that we allocated for
					 * the matched `argument'.  See `add',
					 * above.
					 */
					free(match->name);
					free(match);
					
					return;
				}
				tmp = &((*tmp)->next);
			}
			panic(("In mu_state_arglist::remove(), arglist `%s' "
			       "does not contain member `%s'"), aname, arg);
		}
		msa = msa->parent;
	}
	panic("In mu_state_arglist::remove(), arglist `%s' not found!", aname);
}

int mu_state_arglist::getargs(const char *aname, const char *arg,
			      cast_expr *expr, cast_type *type)
{
	mu_state_arglist *msa = this;
	
	while (msa) {
		if (strcmp(msa->arglist_name, aname) == 0) {
			argument *tmp = msa->findarg(arg);
			
			if (tmp) {
				// Argument found, return the CAST expr/type.
				*expr = tmp->cexpr;
				*type = tmp->ctype;
				return TRUE;
				/*
				 * Note that the cexpr/ctype MAY still be NULL.
				 * This would mean the argument wasn't set yet.
				 * We still return success, however, since we
				 * did _find_ the argument.
				 */
			} else {
				// Argument not found, return failure.
				*expr = NULL;
				*type = NULL;
				return FALSE;
			}
		}
		msa = msa->parent;
	}
	panic("In mu_state_arglist::getargs(), arglist `%s' not found!",
	      aname);
}

int mu_state_arglist::setargs(const char *aname, const char *arg,
			      cast_expr expr, cast_type type)
{
	mu_state_arglist *msa = this;
	
	while (msa) {
		if (strcmp(msa->arglist_name, aname) == 0) {
			argument *tmp = msa->findarg(arg);
			
			if (tmp) {
				// Argument found, set the CAST expr/type.
				tmp->cexpr = expr;
				tmp->ctype = type;
				return TRUE;
			} else {
				// Argument not found, return failure.
				return FALSE;
			}
		}
		msa = msa->parent;
	}
	panic("In mu_state_arglist::setargs(), arglist `%s' not found!",
	      aname);
}

mu_state_arglist::argument *mu_state_arglist::findarg(const char *arg)
{
	argument *tmp = arg_list;
	
	while (tmp) {
		if (strcmp(arg, tmp->name) == 0)
			break;
		tmp = tmp->next;
	}
	return tmp;
}

/* End of file. */

