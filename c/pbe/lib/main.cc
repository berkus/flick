/*
 * Copyright (c) 1995, 1996, 1997, 1998, 1999 The University of Utah and
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

#include <stdio.h>
#include <string.h>

#include <mom/compiler.h>
#include <mom/c/libcast.h>
#include <mom/c/pbe.hh>

struct be_state *the_state;

cast_expr emergency_return_value;

char *progname;

int main(int argc, char **argv)
{
	int retval = 0;
	
	/* Initialize some modules */
	init_cast();
	init_scml();
	
	/* Get the be state (each be overrides for their own state) */
	the_state = get_be_state();
	/* Install the generic handlers */
	install_handlers(the_state->get_pres());
	
	/* Register handlers for the state */
	register_state_handlers(the_state);
	/* Register handlers for command line arguments */
	register_arg_handlers(the_state);
	
	/* Process the args */
	the_state->args(argc, argv);
	
	the_state->begin();
	/* Trigger the init event, any variable and what is done here */
	delete (the_state->handle(the_state->make_event(BESE_INIT)));
	/*
	 * Next, handle any arguments, this also results in the files being
	 * generated since their output is caused by the args
	 */
	delete (the_state->handle(the_state->make_event(BESE_CLI_ARGS,
							argc,
							argv)));
	/* Finally, signal shutdown */
	delete (the_state->handle(the_state->make_event(BESE_SHUTDOWN)));
	the_state->end();
	
	/* Delete the state and exit */
	delete the_state;
	return( retval );
}

/* End of file. */
