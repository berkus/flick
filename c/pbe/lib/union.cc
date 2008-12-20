/*
 * Copyright (c) 1995, 1996 The University of Utah and
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

/* This file provides default do-nothing versions of the
   mu_union() and mu_union_case() operations for unions.
   They generally need to be overridden by backends
   in order to handle unions correctly.
   See mem_mu_state::mu_union*() for a typical implementation.

   Basically, when a union is being processed,
   after the discriminator value is processed
   but before any actual switch code or other union-case-handling code is generated,
   the routine processing the union bundles its state into a functor and calls mu_union().
   The default implementation of mu_union(), defined here,
   is simply to invoke that functor in turn.
   Backends can override mu_union() to perform any necessary actions surrounding union handling.

   Similarly, when a particular union case is about to be processed,
   the union processing code creates a functor and calls mu_union_case() with it.
   Note that it is _not_ guaranteed
   that mu_union_case() will be called
   for every possible branch of the union.
   In some cases (e.g. when processing a pres_c_inline_collapsed_union),
   only one or a small subset of the possible branches
   may need code generated for them,
   so mu_union_case() will only be called for those branches.
   Of course, mu_union_case will always be called at least once,
   because it's impossible to marshal a union without marshaling one of its cases.
   (That one case may be the default case, but it's always there.)
*/

#include <mom/c/pbe.hh>

void mu_state::mu_union_case(functor *f)
{
	f->func(this);
}

void mu_state::mu_union(functor *f)
{
	f->func(this);
}

