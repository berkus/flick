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

#include <stdlib.h>

#include <mom/libaoi.h>
#include <mom/pres_c.h>
#include <mom/c/pbe.hh>

/* We use aoi_get_scoped_name here so we have
   to set this global up, otherwise it won't work */
extern aoi in_aoi;

void mark_interface_parents(aoi *the_aoi, aoi_interface *ai, int *parents)
{
	unsigned int j;
	
	for (j = 0; j < ai->parents.parents_len; j++) {
		parents[ai->parents.
		       parents_val[j]->aoi_type_u_u.indirect_ref] = 1;
		mark_interface_parents(the_aoi,
				       &the_aoi->defs.defs_val[ai->parents.
							parents_val[j]->
							aoi_type_u_u.
							indirect_ref].
				       binding->aoi_type_u_u.interface_def,
				       parents);
	}
}

/*
 * `make_interface_graph' takes care of putting run-time type information into
 * the `*-server.c' file.
 */
void make_interface_graph(pres_c_1 *pres)
{
	unsigned int i, j;
	aoi_def *ad;
	aoi_interface *ai;
	int *parents;
	
	in_aoi = pres->a;
	parents = (int *)mustcalloc(pres->a.defs.defs_len * sizeof(int));
	w_printf("/* This is the runtime type information */\n");
	for( i = 0; i < pres->a.defs.defs_len; i++ ) {
		ad = &pres->a.defs.defs_val[i];
		if (ad->binding->kind != AOI_INTERFACE)
			continue;
		/* Output an array of all the parents terminated by a NULL. */
		w_printf("char *flick_interface_parents_%s[] = {\n",
			 aoi_get_scoped_name(i, "_"));
		ai = &ad->binding->aoi_type_u_u.interface_def;
		w_printf("\t\"IDL:%s:1.0\",\n",
			 aoi_get_scoped_name(i, "/"));
		mark_interface_parents(&in_aoi, ai, parents);
		for( j = 0; j < pres->a.defs.defs_len; j++ ) {
			if( parents[j] ) {
				w_printf("\t\"IDL:%s:1.0\",\n",
					 aoi_get_scoped_name(j, "/"));
				parents[j] = 0;
			}
		}
		w_printf("\t0\n};\n\n");
	}
	w_printf("/* */\n\n");
	free(parents);
}

/* End of file. */

