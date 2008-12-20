/*
 * Copyright (c) 1997, 1998, 1999 The University of Utah and
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
#include <unistd.h>
#include <sys/types.h>
#include <mach_init.h>
#include <mach/message.h>
#include <mach/port.h>
#include <mach/mig_errors.h>
#include <memory.h>
#include <servers/netname.h>

/* defined in "-use" client program */
int call_client(mach_port_t right);

int main(int argc, char *argv[])
{
	mach_port_t right;
	int i;
	char *servername = 0;
	
	/* Check for command line options. */
	for (i = 1; i < argc; i++) {
		switch (argv[i][0]) {
		case '-':
		  display_help:
			fprintf(stderr,
				("usage: %s [-h] <name>\n"
				 "  -h     - This help.\n"
				 "  <name> - Use server <name>.\n"),
				argv[0]);
			exit(0);
		default:
			if (servername) goto display_help;
			servername = argv[i];
			break;
		}
	}
	if (!servername) goto display_help;

	printf("Looking up server: %s\n", servername);
	if (netname_look_up(name_server_port, NULL, servername, &right)) {
		fprintf(stderr, "Couldn't find server!\n");
		return 1;
	}

	call_client(right);

	printf("\nClient Terminated.\n");
	
	return 0;
}

/* End of file. */

