/*
 * Copyright (c) 1997, 1999 The University of Utah and
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

#include <mom/libaoi.h>

aoi_ref aoi_deref_fwd(aoi *a, aoi_ref parent_ref)
{
	aoi_ref _cur_pos = 0;
	
	if (a->defs.defs_val[parent_ref].binding->kind == AOI_FWD_INTRFC) {
		/*
		 * Dereference the forward interface declaration.
		 */
		for (; _cur_pos < ((aoi_ref) a->defs.defs_len); _cur_pos++) {
			if (a->defs.defs_val[_cur_pos].binding
			    && (a->defs.defs_val[_cur_pos].binding->kind
				== AOI_INTERFACE)
			    && (a->defs.defs_val[_cur_pos].scope
				== a->defs.defs_val[parent_ref].scope)
			    && (!strcmp(a->defs.defs_val[parent_ref].name,
					a->defs.defs_val[_cur_pos].name))
				) {
				int scope = a->defs.defs_val[_cur_pos].scope;
				int if_pos = _cur_pos;
				int fwd_pos = parent_ref;
				
				while (scope) {
					/*
					 * verify that the scoped names are the
					 * same
					 */
					while (a->defs.defs_val[if_pos].scope
					       >= scope)
						if_pos--;
					while (a->defs.defs_val[fwd_pos].scope
					       >= scope)
						fwd_pos--;
					if (strcmp(a->defs.
						   defs_val[if_pos].name,
						   a->defs.
						   defs_val[fwd_pos].name))
						break;
					scope--;
					assert((scope
						== (a->defs.
						    defs_val[if_pos].scope))
					       && (scope
						   == (a->defs.
						       defs_val[fwd_pos].
						       scope)));
				}
				if (!scope) {
					parent_ref = _cur_pos;
					break;
				}
			}
		}
	}

	/* Return -1 if the real interface wasn't found
	   and let the caller decide what to do */
	if( a->defs.defs_val[parent_ref].binding->kind != AOI_INTERFACE )
		parent_ref = -1;
	return parent_ref;
}

/* End of file. */

