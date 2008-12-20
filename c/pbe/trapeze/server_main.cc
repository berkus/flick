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

#include <assert.h>
#include "trapeze.h"
#include "mom/libmint.h"
#include <mom/c/libcast.h>

/* Output the bad `CORBA_BOA_create' examples?  No. */
#define BAD_EXAMPLE 0

void do_main_output_corba(pres_c_1 *p);
void do_main_output_sun(pres_c_1 *p);
void do_main_objinit_corba(pres_c_1 *p, int pos);
void do_main_objinit_sun(pres_c_1 *p, int pos);
void do_main_footer_corba(pres_c_1 *p);
void do_main_footer_sun(pres_c_1 *p);

void
do_main_output(pres_c_1 *p)
{
	trapeze_protocol_t protocol;
	int i, header = 0;
	
	protocol = what_protocol(p);
	
	// Build a list of funcs to work on
	for (i = 0; i < (signed int)p->stubs.stubs_len; i++) {
		if (p->stubs.stubs_val[i].kind == PRES_C_SERVER_SKEL) {
			if (!header) {
				w_printf("#include <assert.h>\n");
				w_printf("#include <stdio.h>\n");
				
				switch (protocol) {
				case TRAPEZE:
					w_printf("\n/* You need to implement "
						 "this function. */");
					w_printf("\nvoid register_objects("
						 "CORBA_ORB orb, "
						 "CORBA_BOA boa, "
						 "int argc, char **argv,\n");
					w_printf("                      "
						 "CORBA_Environment *ev);\n");
					break;
				case TRAPEZE_ONC:
					/* Fallthrough. */
				default:
					/* Do nothing. */
					break;
				}
				
				w_printf("\nint main(int argc, "
					 "char **argv)\n{\n");
				
				switch (protocol) {
				case TRAPEZE:
					do_main_output_corba(p);
					break;
				case TRAPEZE_ONC:
					do_main_output_sun(p);
					break;
				default:
					panic(("In `do_main_output', "
					       "unrecognized protocol."));
					break;
				}
				
				header = 1;
			}
			
			switch (protocol) {
			case TRAPEZE:
				do_main_objinit_corba(p, i);
				break;
			case TRAPEZE_ONC:
				do_main_objinit_sun(p, i);
				break;
			default:
				panic(("In `do_main_output', "
				       "unrecognized protocol."));
				break;
			}
		}
	}
	if (header) {
		switch (protocol) {
		case TRAPEZE:
			do_main_footer_corba(p);
			break;
		case TRAPEZE_ONC:
			do_main_footer_sun(p);
			break;
		default:
			panic("In `do_main_output', unrecognized protocol.");
			break;
		}
	}
}

/* ---------- CORBA/IIOP Protocol ---------- */
void
do_main_output_corba(pres_c_1 */*p*/)
{
	w_printf("\tCORBA_Environment ev;\n");
	w_printf("\tCORBA_ORB orb;\n");
	w_printf("\tCORBA_BOA boa;\n");
#if BAD_EXAMPLE
	w_printf("\tCORBA_ReferenceData key;\n");
	w_printf("\tCORBA_Object instance1, instance2;\n");
#endif /* BAD_EXAMPLE */
#if 0
	w_printf("\tif (argc < 3) {\n");
	w_printf("\t\tfprintf(stderr, \"usage: %%s [-ORBipaddr "
		 "(local host ip address)] -OAport (portnum) ...\\n\", "
		 "argv[0]);\n");
	w_printf("\t\texit(1);\n\t}\n");
#endif
	w_printf("\n\torb = CORBA_ORB_init(&argc, argv, 0, &ev);\n");
	w_printf("\tif (ev._major != "
		 "CORBA_NO_EXCEPTION) {\n");
	w_printf("\t\tfprintf(stderr,"
		 " \"%%s exception: unable"
		 " to initialize"
		 " ORB.\\n\", CORBA_exception_"
		 "id(&ev));\n");
	w_printf("\t\texit(1);\n");
	w_printf("\t}\n");
	w_printf("\tboa = CORBA_ORB_BOA_init(orb, &argc, argv, 0, &ev);\n");
	w_printf("\tif (ev._major != "
		 "CORBA_NO_EXCEPTION) {\n");
	w_printf("\t\tfprintf(stderr,"
		 " \"%%s exception: unable"
		 " to initialize"
		 " Object adapter.\\n\", "
		 "CORBA_exception_id(&ev));"
		 "\n");
	w_printf("\t\texit(1);\n");
	w_printf("\t}\n");
	w_printf("\tregister_objects(orb, boa, argc, argv, &ev);\n");
	w_printf("\tif (ev._major != CORBA_NO_EXCEPTION)\n");
	w_printf("\t\tfprintf(stderr, "
		 "\"ERROR: unable to register object"
		 " implementations.\\n\");\n");
	w_printf("\telse {\n");
#if BAD_EXAMPLE
/* XXX --- The `CORBA_BOA_create' example invocations are wrong! */
	w_printf("#if 0\n");
	w_printf("\t\t/* call CORBA_BOA_create for every object instance \n");
	w_printf("\t\t   of an interface you want.  "
		 "Here are some examples: */\n\n");
#endif /* BAD_EXAMPLE */
}

void
do_main_objinit_corba(pres_c_1 */*p*/, int /*pos*/)
{
#if BAD_EXAMPLE
	pres_c_skel skel = p->stubs.stubs_val[pos].pres_c_stub_u.sskel;
	cast_def *cdef = &p->stubs_cast.cast_scope_val[skel.c_def];
	
	w_printf("\t\tkey._maximum = 1024;\n");
	w_printf("\t\tkey._buffer = argv[++i];\n");
	w_printf("\t\tkey._length = strlen(key._buffer);\n");
	w_printf("\t\tinstance1 = CORBA_BOA_create(boa, &key,"
		 " \"");
	cast_w_scoped_name(&cdef->name);
	w_printf("_instance1\", &");
	cast_w_scoped_name(&cdef->name);
	w_printf(", &ev);\n");
	w_printf("\t\tinstance1 = CORBA_BOA_create(boa, &key,"
		 " \"");
	cast_w_scoped_name(&cdef->name);
	w_printf("_instance2\", &");
	cast_w_scoped_name(&cdef->name);
	w_printf(", &ev);\n");
#endif /* BAD_EXAMPLE */
}

void
do_main_footer_corba(pres_c_1 */*p*/)
{
#if BAD_EXAMPLE
	w_printf("#endif\n\n");
#endif /* BAD_EXAMPLE */
	w_printf("\t\tCORBA_BOA_impl_is_ready(boa, &ev);\n");
/* set exception */			
	w_printf("\t}\n");
	w_printf("\tif (ev._major != CORBA_NO_EXCEPTION) {\n");
	w_printf("\t\tfprintf(stderr, "
		 "\"%%s exception encountered.\\n\","
		 " CORBA_exception_id(&ev));\n");
	w_printf("\t\texit(1);\n");
	w_printf("\t}\n");
	w_printf("\treturn 0;\n}\n\n");
}

/* ---------- SUN/ONC Protocol ---------- */

struct prog_vers 
{
  char *prog;
  char *vers;
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
do_main_output_sun(pres_c_1 */*p*/)
{
#if 0
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
#endif
	w_printf("\tFLICK_SERVER_DESCRIPTOR s;\n");
	w_printf("\tint reg_success;\n\n");
}

void
do_main_objinit_sun(pres_c_1 *p, int pos)
{
	prog_vers item = GetInterface(pos, p);
	pres_c_skel sskel = p->stubs.stubs_val[pos].pres_c_stub_u.sskel;
	cast_def *cdef = &p->stubs_cast.cast_scope_val[sskel.c_def];
	w_printf("\n\ts.prog_num = %s;\n",item.prog);
	w_printf("\ts.vers_num = %s;\n\n",item.vers);
	w_printf("\treg_success = flick_server_register(s, &");
	cast_w_scoped_name(&cdef->name);
	w_printf(");\n");
	w_printf("\tassert(reg_success);\n\n");
}

void
do_main_footer_sun(pres_c_1 */*p*/)
{
	w_printf("\n\tflick_server_run();\n");
	w_printf("\t(void)fprintf(stderr, \"flick_server_run returned\\n\");\n");
	w_printf("\texit(1);\n");
	w_printf("}\n");
}

/* End of file. */

