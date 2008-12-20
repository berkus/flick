/*
 * Copyright (c) 1995, 1996, 1998 The University of Utah and
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
#include <mom/compiler.h>
#include <mom/c/pfe.hh>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include "private.hh"

void pg_state::p_shuffle_types(int org, int next) 
{
	if(org != next) {
		cast_scope *scope = (cast_scope *)top_ptr(scope_stack);
		cast_def temp = out_pres->cast.cast_scope_val[org];
		
		if (org < next) 
			for(int i = org; i < next; )
				scope->cast_scope_val[i] =
					scope->cast_scope_val[++i];
		else 
			for(int i = org; i > next; )
				scope->cast_scope_val[i] =
					scope->cast_scope_val[--i];
		scope->cast_scope_val[next] = temp;
	}
}

  
  
    
  
