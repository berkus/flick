/*
 * Copyright (c) 1995, 1996, 1997 The University of Utah and
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
#include <mom/libmint.h>
#include <mom/c/libpres_c.h>

void descend_collapsed_union(pres_c_1 *pres, mint_ref *itypep,
			     pres_c_inline *inlp)
{
	mint_ref itype = *itypep;
	pres_c_inline inl = *inlp;
	
	assert(pres->mint.defs.defs_val[itype].kind == MINT_UNION);
	mint_union_def *udef = &(pres->mint.defs.defs_val[itype].mint_def_u.
				 union_def);
	
	assert(inl->kind == PRES_C_INLINE_COLLAPSED_UNION);
	pres_c_inline_collapsed_union *cuinl
		= &(inl->pres_c_inline_u_u.collapsed_union);
	
	/* Find the appropriate case. */
	mint_ref case_var_r;
	for (int case_num = 0; ; case_num++) {
		if (case_num >= (signed int) udef->cases.cases_len) {
			if (udef->dfault == mint_ref_null)
				panic("union collapsed on nonexistent case");
			case_var_r = udef->dfault;
			break;
		}
		if (mint_const_cmp(udef->cases.cases_val[case_num].val,
				   cuinl->discrim_val)
		    == 0) {
			case_var_r = udef->cases.cases_val[case_num].var;
			break;
		}
	}
	
	*itypep = case_var_r;
	*inlp = cuinl->selected_case;
}

/* End of file. */

