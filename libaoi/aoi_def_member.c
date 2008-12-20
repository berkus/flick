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

#include <mom/libaoi.h>

int aoi_def_has_member(aoi *in_aoi, aoi_def *d, const char *name)
{
	aoi_def *out_d;
	int out_idx;
	
	return aoi_def_find_member(in_aoi, d, name, &out_d, &out_idx);
}

int aoi_def_find_member(aoi *in_aoi, aoi_def *d, const char *name,
			aoi_def **out_d, int *out_idx)
{
	unsigned int i;
	int retval = 0;
	
	switch (d->binding->kind) {
	case AOI_STRUCT:
		for (i = 0;
		     ((i < d->binding->aoi_type_u_u.struct_def.slots.slots_len)
		      && !retval);
		     i++) {
			if (!strcmp(name,
				    (d->binding->aoi_type_u_u.struct_def.
				     slots.slots_val[i].name))) {
				*out_d = d;
				*out_idx = i;
				retval = 1;
			}
		}
		break;
		
	case AOI_UNION:
		for (i = 0;
		     ((i < d->binding->aoi_type_u_u.union_def.cases.cases_len)
		      && !retval);
		     i++) {
			if (!strcmp(name,
				    (d->binding->aoi_type_u_u.union_def.
				     cases.cases_val[i].var.name))) {
				*out_d = d;
				*out_idx = i;
				retval = 1;
			}
		}
		break;
		
	case AOI_INTERFACE: {
		unsigned int cur_parent;
		aoi_def *par_d;
		
		for (cur_parent = 0;
		     ((cur_parent < (d->binding->aoi_type_u_u.interface_def.
				     parents.parents_len))
		      && !retval);
		     cur_parent++) {
			par_d = &in_aoi->defs.
				defs_val[d->binding->aoi_type_u_u.
					interface_def.parents.
					parents_val[cur_parent]->
					aoi_type_u_u.indirect_ref];
			retval = aoi_def_find_member(in_aoi, par_d, name,
						     out_d, out_idx);
		}
		for (i = 0;
		     ((i < d->binding->aoi_type_u_u.interface_def.ops.ops_len)
		      && !retval);
		     i++) {
			if (!strcmp(name,
				    (d->binding->aoi_type_u_u.interface_def.
				     ops.ops_val[i].name))) {
				*out_d = d;
				*out_idx = i;
				retval = 1;
			}
		}
		for (i = 0;
		     ((i < (d->binding->aoi_type_u_u.interface_def.attribs.
			    attribs_len))
		      && !retval);
		     i++) {
			if (!strcmp(name,
				    (d->binding->aoi_type_u_u.interface_def.
				     attribs.attribs_val[i].name))) {
				*out_d = d;
				*out_idx = (d->binding->aoi_type_u_u.
					    interface_def.ops.ops_len)
					   + i;
				retval = 1;
			}
		}
		break;
	}
	
	case AOI_EXCEPTION:
		for (i = 0;
		     ((i < (d->binding->aoi_type_u_u.exception_def.slots.
			    slots_len))
		      && !retval);
		     i++) {
			if (!strcmp(name,
				    (d->binding->aoi_type_u_u.exception_def.
				     slots.slots_val[i].name))) {
				*out_d = d;
				*out_idx = i;
				retval = 1;
			}
		}
		break;
		
	case AOI_ENUM:
		for (i = 0;
		     ((i < d->binding->aoi_type_u_u.enum_def.defs.defs_len)
		      && !retval);
		     i++) {
			if (!strcmp(name,
				    (d->binding->aoi_type_u_u.enum_def.defs.
				     defs_val[i].name))) {
				*out_d = d;
				*out_idx = i;
				retval = 1;
			}
		}
		break;
		
	default:
		break;
	}
	
	return retval;
}

/* End of file. */

