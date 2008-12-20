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

#include "ace/Get_Opt.h"
#include "tao/TAO.h"

#include "phone_i.h"

/*
 * This structure is used to track all the implementations the user asked us to
 * make.
 */
struct phone_impl {
	char *name;
	phone_i servant;
	struct phone_impl *next;
};

/* The list of implementations we should make. */
static struct phone_impl *the_impls = 0;

/*****************************************************************************/

static int parse_args(int argc, char **argv)
{
	ACE_Get_Opt get_opts(argc, argv, "I:");
	int c;
	
	while((c = get_opts ()) != -1)
		switch (c) {
		case 'I': {
			struct phone_impl *new_impl;
			
			/* Add a new implementation with the specified name. */
			new_impl = new phone_impl;
			new_impl->next = the_impls;
			new_impl->name = ACE_OS::strdup(get_opts.optarg);
			the_impls = new_impl;
			break;
		}
		case '?':
		default:
			ACE_ERROR_RETURN((LM_ERROR,
					  ("usage:  %s "
					   "-I <name>"
					   "\n"),
					  argv[0]),
					 -1);
		}
	
	/* Indicates sucessful parsing of the command line. */
	return 0;
}

int main(int argc, char **argv)
{
	TAO_ORB_Manager orb_manager;
	CORBA::Environment ACE_TRY_ENV;
	
	/* Use `try' because there might be an exception. */
	ACE_TRY {
		phone_impl *pi;
		
		/* Initialize the POA. */
		orb_manager.init_child_poa(argc, argv, "child_poa",
					   ACE_TRY_ENV);
		ACE_TRY_CHECK;
		
		/* Parse the command line; creates `the_impls' list. */
		if (parse_args(argc, argv) != 0)
			return 1;
		
		/*
		 * Walk through the list of implementations and attach each
		 * one to the server.
		 */
		for (pi = the_impls; pi; pi = pi->next) {
			/*
			 * Add the head of the `pi' list to the POA and get the
			 * IOR for the object.
			 */
			CORBA::String_var ior
				= (orb_manager.
				   activate_under_child_poa(pi->name,
							    &pi->servant,
							    ACE_TRY_ENV));
			ACE_TRY_CHECK;
			
			/* Give the IOR to the user. */
			ACE_DEBUG((LM_DEBUG, "Object `%s' is ready.\n",
				   pi->name));
			ACE_DEBUG((LM_DEBUG, "  IOR:  %s\n",
				   ior.in()));
		}
		/*
		 * Make sure there is at least one object and then start the
		 * server.
		 */
		if (the_impls)
			orb_manager.run(ACE_TRY_ENV);
		else
			ACE_ERROR_RETURN((LM_ERROR,
					  ("No implementations were "
					   "specified.\n")),
					 1);
		ACE_TRY_CHECK;
	}
	/* Catch any system exceptions. */
	ACE_CATCH (CORBA::SystemException, ex) {
		ex._tao_print_exception("System Exception");
		return 1;
	}
	ACE_ENDTRY;
	
	return 0;
}

/* End of file. */

