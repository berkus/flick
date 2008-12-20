/*
 * Copyright (c) 1995, 1996, 1998, 1999 The University of Utah and
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
#include "sun.h"
#include "mom/libmint.h"
#include <mom/c/libcast.h>

// This file outputs the main file for the server dispatch loop

struct prog_vers 
{
  char *prog;
  char *vers;
};

// This is a list of all server skeletons within a pres-c file
struct skel_group 
{
  skel_group(prog_vers itm, int p, skel_group *nxt) 
  {
    pos = p;
    next = nxt;
    item = itm;
  }
  int pos;		// The stub number within the pres-c file
  prog_vers item;	// The values for prog# & vers#
  skel_group *next;	// Link to the next item in the list
};

// This function finds the aoi_interface that corresponds to
// the server skeleton found at p->stubs.stubs_val[pos]
// then it returns the prognum & versnum (macros) for that skeleton
static prog_vers
GetInterface(int pos, pres_c_1 *p) 
{
  static aoi_ref *ifs = 0;
  static mint_const *ifc = 0;
  
  if (!ifs) {
    // Build a list of all aoi_interfaces
    // Also build a list of mint consts for the IDL code value
    int i, pos = 0;
    ifs = (aoi_ref *)mustcalloc((p->a.defs.defs_len+1) * sizeof(aoi_ref));
    ifc = (mint_const *)mustcalloc((p->a.defs.defs_len+1) *
				   sizeof(mint_const));
    for (i = 0; i < (signed int)p->a.defs.defs_len; i++) {
      if (p->a.defs.defs_val[i].binding->kind == AOI_INTERFACE) {
	aoi_const tmp = p->a.defs.defs_val[i].binding->aoi_type_u_u.
		interface_def.code;
	ifc[pos] = mint_new_const_from_aoi_const(tmp);
	ifs[pos++] = i;
      }
    }
    ifs[pos] = -1;
  }
  
  mint_const if_const = 0;
  pres_c_func *f = p->stubs.stubs_val[pos].pres_c_stub_u.sskel.funcs.funcs_val;
  pres_c_inline_collapsed_union col;

  switch (f->kind) {
  case PRES_C_SERVER_FUNC:
    {
      pres_c_server_func *sf = &p->stubs.stubs_val[pos].pres_c_stub_u.sskel.
			       funcs.funcs_val[0].pres_c_func_u.sfunc;
      if (!sf) {
	prog_vers res;
	res.prog = res.vers = 0;
	return res;
      }
      assert (sf->request_i->kind == PRES_C_INLINE_COLLAPSED_UNION);
      col = sf->request_i->pres_c_inline_u_u.collapsed_union;
      break;
    }
  case PRES_C_RECEIVE_FUNC:
    {
      pres_c_receive_func *rf = &p->stubs.stubs_val[pos].pres_c_stub_u.sskel.
				funcs.funcs_val[0].pres_c_func_u.rfunc;
      if (!rf) {
	prog_vers res;
	res.prog = res.vers = 0;
	return res;
      }
      assert (rf->msg_i->kind == PRES_C_INLINE_COLLAPSED_UNION);
      col = rf->msg_i->pres_c_inline_u_u.collapsed_union;
      break;
    }
  default:
    panic("Unknown function kind in GetInterface()");
  }
  
  assert (col.selected_case->kind == PRES_C_INLINE_COLLAPSED_UNION);
  col = col.selected_case->pres_c_inline_u_u.collapsed_union;
  if_const = col.discrim_val;
  
  for (int const_pos = 0; ifs[const_pos] >= 0; const_pos++)
    // Find which aoi_interface has the correct idl_code value
    if (!mint_const_cmp(if_const, ifc[const_pos])) {
      prog_vers res;
      aoi_interface this_if = p->a.defs.defs_val[ifs[const_pos]].binding->
	      aoi_type_u_u.interface_def;
      if (this_if.idl == AOI_IDL_SUN) {
	// If it's sun, then use the parent/child names for prog & vers
	assert (this_if.parents.parents_len == 1);
	aoi_type par = this_if.parents.parents_val[0];
	assert (par->kind == AOI_INDIRECT);
	res.prog = p->a.defs.defs_val[par->aoi_type_u_u.indirect_ref].name;
	res.vers = p->a.defs.defs_val[ifs[const_pos]].name;
      } else {
	// Otherwise, use the _PROG_ & _VERS_ stuff
	// IMPORTANT:  THIS STUFF IS ALSO USED IN MISC.CC
	//             IF YOU CHANGE IT HERE, CHANGE IT THERE, TOO
	res.prog = flick_asprintf("_PROG_%s",
				  p->a.defs.defs_val[ifs[const_pos]].name);
	res.vers = flick_asprintf("_VERS_%s",
				  p->a.defs.defs_val[ifs[const_pos]].name);
      }
      return res;
    }
  panic("Unable to find an interface to link to a server skeleton\n");
}

void
do_main_output(pres_c_1 *p)
{
	skel_group *top = 0;
	int i;
	// Build a list of funcs to work on
	for (i = 0; i < (signed int)p->stubs.stubs_len; i++) {
		if (p->stubs.stubs_val[i].kind == PRES_C_SERVER_SKEL) {
			prog_vers a = GetInterface(i, p);
			if (a.prog && a.vers)
				top = new skel_group(a, i, top);
		}
	}
  
  // If there are any server skeletons, output a main function
  if (top) {
    w_printf("#include <assert.h>\n");
    w_printf("#include <stdio.h>\n");
    w_printf("#include <flick/link/suntcp.h>\n");
    w_printf("#include <flick/encode/xdr.h>\n");
    
    w_printf("\nint main(int argc, char **argv)\n{\n");
    w_printf("\tFLICK_SERVER_DESCRIPTOR s;\n");
    w_printf("\tint reg_success;\n\n");
    
    while (top) {
      pres_c_skel sskel
	      = p->stubs.stubs_val[top->pos].pres_c_stub_u.sskel;
      cast_def *cdef = &p->stubs_cast.cast_scope_val[sskel.c_def];
      w_printf("\n\ts.prog_num = %s;\n",top->item.prog);
      w_printf("\ts.vers_num = %s;\n\n",top->item.vers);
      w_printf("\treg_success = flick_server_register(s, &");
      cast_w_scoped_name(&cdef->name);
      w_printf(");\n");
      w_printf("\tassert(reg_success);\n\n");
      top = top->next;
    }
    
    w_printf("\n\tflick_server_run();\n");
    w_printf("\t(void)fprintf(stderr, \"flick_server_run returned\\n\");\n");
    w_printf("\texit(1);\n");
    w_printf("}\n");
  }
}

