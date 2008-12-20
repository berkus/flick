/*
 * Copyright (c) 1996, 1997 The University of Utah and
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
#include "iiop.h"

/* Output the bad `CORBA_BOA_create' examples?  No. */
#define BAD_EXAMPLE 0

void
do_main_output(pres_c_1 *p)
{
	int i, header = 0;
	// Build a list of funcs to work on
	for (i = 0; i < (signed int)p->stubs.stubs_len; i++) {
		if (p->stubs.stubs_val[i].kind == PRES_C_SERVER_SKEL) {
			if (!header) {
				w_printf("#include <assert.h>\n");
				w_printf("#include <stdio.h>\n");
				w_printf("\n/* You need to implement this function. */");
				w_printf("\nvoid register_objects(CORBA_ORB orb, CORBA_BOA boa, int argc, char **argv,\n");
				w_printf("                      CORBA_Environment *ev);\n");
				w_printf("\nint main(int argc, char **argv)\n{\n");
				w_printf("\tCORBA_Environment ev;\n");
				w_printf("\tCORBA_ORB orb;\n");
				w_printf("\tCORBA_BOA boa;\n");
#if BAD_EXAMPLE
				w_printf("\tCORBA_ReferenceData key;\n");
				w_printf("\tCORBA_Object "
					 "instance1, instance2;\n");
#endif /* BAD_EXAMPLE */
				w_printf("\tif (argc < 3) {\n");
				w_printf("\t\tfprintf(stderr, \"usage: %%s [-ORBipaddr (local host ip address)] -OAport (portnum) ...\\n\", argv[0]);\n");
				w_printf("\t\texit(1);\n\t}\n");
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
					w_printf("\t\t   of an interface you want.  Here are some examples: */\n\n");
#endif /* BAD_EXAMPLE */
				header = 1;
			}
#if BAD_EXAMPLE
			w_printf("\t\tkey._maximum = 1024;\n");
			w_printf("\t\tkey._buffer = argv[++i];\n");
			w_printf("\t\tkey._length = strlen(key._buffer);\n");
			pres_c_skel skel = p->stubs.stubs_val[i].pres_c_stub_u.sskel;
			cast_def *cdef = &p->stubs_cast.cast_scope_val[skel.c_def];
			w_printf("\t\tinstance1 = CORBA_BOA_create(boa, &key,"
				 " \"%s_instance1\", &%s, &ev);\n",
				 cdef->name, cdef->name);
			w_printf("\t\tinstance2 = CORBA_BOA_create(boa, &key,"
				 " \"%s_instance2\", &%s, &ev);\n",
				 cdef->name, cdef->name);
#endif /* BAD_EXAMPLE */
		}
	}
	if (header) {
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
		w_printf("\treturn 0;\n}\n");
	}
}

/* End of file. */

