/*
 * Copyright (c) 1995, 1996, 1997, 1999 The University of Utah and
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
#include <mom/c/pbe.hh>

void mu_state::mu_mapping_reference(cast_expr /*expr*/,
				    cast_type /*ctype*/,
				    mint_ref /*itype*/,
				    pres_c_mapping_reference *rmap)
{
	switch (rmap->kind) {
	case PRES_C_REFERENCE_COPY:
		panic("This back end doesn't support passing "
		      "copies of object references.");
		break;
	case PRES_C_REFERENCE_MOVE:
		panic("This back end doesn't support moving "
		      "object references to other locations.");
		break;
	case PRES_C_REFERENCE_COPY_AND_CONVERT:
		panic("This back end doesn't support passing "
		      "converted copies of object references.");
		break;
	default:
		panic("In `mu_state::mu_mapping_reference', "
		      "invalid reference kind!");
	}
}

/*
 * This is an auxiliary method for `mu_mapping_reference' that determines:
 *
 *   + how many object references will be acquired or lost as part of the
 *     reference mapping, and
 *
 *   * if the object reference must be marked for special cleanup action.
 *     Special cleanup would be required, for instance, if we are told to
 *     ``move'' a reference over a transport facility that inherently supports
 *     only object copy semantics.
 */
void mu_state::mu_mapping_reference_get_attributes(
	mint_ref /*itype*/,
	pres_c_mapping_reference *rmap, 
	int *ref_count_adjust,
	int *mark_for_cleanup)
{
	static int is_warned = 0;
	*ref_count_adjust = rmap->ref_count;
	*mark_for_cleanup = 0;
	
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
			break;
		case PRES_C_REFERENCE_COPY_AND_CONVERT:
                        /* No local references are lost; no cleanup. */
			*ref_count_adjust = 0;
			if (!is_warned++)
				warn("This back end doesn't support encoding "
				     "converted copies of object references.");
			break;
		default:
			panic("In `mu_state::"
			      "mu_mapping_reference_get_attributes', "
			      "invalid mapping reference!");
		}
		
	} else if (op & MUST_DECODE) {
		/*
		 * XXX --- Should the references received by the server be
		 * cleaned up in the default case?  Should the user have to
		 * clean them up explicitly?   Right now, cleanup *is* done.
		 */
		if (!strcmp(get_which_stub(), "server")) {
                        /*
			 * We need to remember to cleanup references that will
			 * no longer be used.
			 */
			*mark_for_cleanup = 1;
		}
	} else 
		panic("In `mu_state::"
		      "mu_mapping_reference_get_attributes', "
		      "not decoding or encoding!");
}

/* End of file. */
