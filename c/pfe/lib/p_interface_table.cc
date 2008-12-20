/*
 * Copyright (c) 1997 The University of Utah and
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
#include <mom/libaoi.h>
#include <mom/c/pfe.hh>

#include "private.hh"

/*
 * Here is the implementation of the (interface, operation/attribute) mark
 * table.  It's just like every other C-implemented association table you've
 * ever seen.
 *
 * XXX --- This is a quickie implementation.  A good implementation would
 * hash the (interface, object) pair, or at the very least, use a vector of
 * vectors to first find the interface, then find the object.
 */

typedef struct {
	aoi_interface *interface;
	void *object; /* Really an `aoi_operation *' or `aoi_attribute *'. */
	int value;
} p_interface_table_elem;

#define P_INTERFACE_TABLE_INCREMENT (64)

static p_interface_table_elem *p_interface_table = 0;
static int p_interface_table_size = 0;
static int p_interface_table_count = 0;

void pg_state::p_interface_table_clear()
{
	if (!p_interface_table) {
		p_interface_table_size = P_INTERFACE_TABLE_INCREMENT;
		p_interface_table =
			(p_interface_table_elem *)
			mustmalloc(sizeof(p_interface_table_elem)
				   * p_interface_table_size);
	}
	p_interface_table_count = 0;
}

int pg_state::p_interface_table_get_value(aoi_interface *ai, void *object)
{
	int i;
	
	for (i = 0; i < p_interface_table_count; ++i)
		if ((p_interface_table[i].interface == ai)
		    && (p_interface_table[i].object == object))
			return p_interface_table[i].value;
	return 0;
}

void pg_state::p_interface_table_set_value(aoi_interface *ai, void *object,
					   int value)
{
	int i;
	
	for (i = 0; i < p_interface_table_count; ++i)
		if ((p_interface_table[i].interface == ai)
		    && (p_interface_table[i].object == object))
			break;
	
	if (i >= p_interface_table_count) {
		/*
		 * We must add a new entry to the table.
		 */
		if (i >= p_interface_table_size) {
			/*
			 * Grow the `p_interface_table'.
			 */
			p_interface_table_size =
				i + P_INTERFACE_TABLE_INCREMENT;
			p_interface_table =
				(p_interface_table_elem *)
				mustrealloc(p_interface_table,
					    (sizeof(p_interface_table_elem)
					     * p_interface_table_size));
		}
		
		p_interface_table[i].interface = ai;
		p_interface_table[i].object = object;
		
		++p_interface_table_count;
	}
	
	p_interface_table[i].value = value;
}

/* End of file. */

