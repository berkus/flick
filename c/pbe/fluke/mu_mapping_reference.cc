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

#include <mom/compiler.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>

#include "fluke.h"

/*
 * The Fluke back end's version of `mu_mapping_reference_get_attributes' is
 * identical to `mu_state::mu_mapping_reference_attributes' --- and implements
 * the same reference counting policy (see `pbe/lib/mu_mapping_reference.cc'
 * for comments) --- EXCEPT that we account for the fact that some reference
 * deallocation is currently implemented by the MOM infrastructure or Flick's
 * Fluke runtime library.
 *
 * For cases in which reference deallocation is automatic, this method sets the
 * variable `automatic_cleanup'.  This inhibits the `mark_for_cleanup' macro
 * call in the generated code.
 */

void fluke_mu_state::mu_mapping_reference_get_attributes(
	mint_ref /*itype*/,
	pres_c_mapping_reference *rmap,
	int *ref_count_adjust,
	int *mark_for_cleanup)
{
	static int is_warned = 0;
	int automatic_cleanup;
	
	*ref_count_adjust = rmap->ref_count;
	*mark_for_cleanup = 0;
	automatic_cleanup = 0;
	
	if (op & MUST_ENCODE) {
		switch (rmap->kind) {
		case PRES_C_REFERENCE_COPY:
                        /* No local references are lost; no cleanup. */
			*ref_count_adjust = 0;
			break;
		case PRES_C_REFERENCE_MOVE:
                        /*
			 * We need to remember to cleanup references that are
			 * no longer used.
			 */
			*mark_for_cleanup = 1;
			/*
			 * With MOM on Fluke, parameters are managed by the MOM
			 * infrastructure (in the MOM module listener).
			 */
			automatic_cleanup = 1;
			break;
		case PRES_C_REFERENCE_COPY_AND_CONVERT:
                        /* No local references are lost; no cleanup. */
			*ref_count_adjust = 0;
			if (!is_warned++)
				warn("This back end doesn't support encoding "
				     "converted copies of object references.");
			break;
		default:
			panic("In `fluke_mu_state::"
			      "mu_mapping_reference_get_attributes', "
			      "invalid mapping reference!");
		}
		
	} else if (op & MUST_DECODE) {
		/*
		 * XXX --- Currently, m/u stubs for the server and client are
		 * the same, so cleanup *cannot* be done properly since it is
		 * different for each side.  Right now, no cleanup is done.
		 */
		if (!strcmp(get_which_stub(), "server")) {
                        /*
			 * We need to remember to cleanup references that will
			 * no longer be used.
			 */
			*mark_for_cleanup = 1;
			/*
			 * With MOM on Fluke, parameters are managed by the MOM
			 * infrastructure (in the MOM module listener).
			 */
			automatic_cleanup = 1;
		}
		
	} else 
		panic("In `fluke_mu_state::"
		      "mu_mapping_reference_get_attributes', "
		      "not decoding or encoding!");
	
	*mark_for_cleanup = (*mark_for_cleanup && !automatic_cleanup);
}

/* End of file. */

