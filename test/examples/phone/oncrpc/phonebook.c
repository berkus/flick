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

#include <stdlib.h>
#include <stdio.h>

#include "phone-client.h"

#define NAME_SIZE (200 + 1)
#define PHONE_SIZE (20 + 1)

void read_string(const char *prompt, char *buffer, int buffer_size);
void read_integer(const char *prompt, int *i);

void add_entry(CLIENT *c);
void remove_entry(CLIENT *c);
void find_entry(CLIENT *c);

/*****************************************************************************/

int main(int argc, char **argv) 
{
	CLIENT client_struct, *c;
        FLICK_SERVER_LOCATION s;
	int sel, done;
	
	c = &client_struct;
	
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <host>\n", argv[0]);
		exit(1);
	}
	
	s.server_name = argv[1];
	s.prog_num = netphone;
	s.vers_num = firstphone;
	create_client(c, s);
	
	done = 0;
	while (!done) {
		read_integer(("\n(1) Add an entry (2) Remove an entry "
			      "(3) Find a phone number (4) Exit: "),
			     &sel);
		switch(sel) {
		case 1:  add_entry(c); break;
		case 2:  remove_entry(c); break;
		case 3:  find_entry(c); break;
		case 4:  done = 1; break;
		default: printf("Please enter 1, 2, 3, or 4.\n");
		}
	}
	return 0;
}

/*****************************************************************************/

void read_string(const char *prompt, char *buffer, int buffer_size)
{
	int i, c, done;
	
	done = 0;
	while (!done) {
		printf("%s", prompt);
		i = 0;
		for (;;) {
			c = getchar();
			if ((c == EOF) || (c == '\n'))
				break;
			if (i < buffer_size)
				buffer[i++] = c;
		}
		if (i < buffer_size) {
			buffer[i] = 0;
			done = 1;
		} else {
			printf("Error: input too long.  Please enter no more "
			       "than %d characters.\n",
			       (buffer_size - 1));
		}
	}
}

void read_integer(const char *prompt, int *i)
{
	char number[101];
	
	read_string(prompt, number, 101);
	*i = atoi(number);
}

/*****************************************************************************/

void add_entry(CLIENT *c)
{
	entry e;
	char name_array[NAME_SIZE], phone_array[PHONE_SIZE];
	int *result;
	
	e.n = name_array;
	e.p = phone_array;
	
	read_string("Enter the name: ", e.n, NAME_SIZE);
	read_string("Enter the phone number: ", e.p, PHONE_SIZE);
	
	result = add_1(&e, c);
	if (!result)
		printf("Error: bad RPC call for add_1.\n");
	else if (*result)
		printf("Error: `%s' not added, error code = %d.\n",
		       e.n, *result);
	else
		printf("`%s' has been added.\n", e.n);
}

void remove_entry(CLIENT *c)
{
	char name_array[NAME_SIZE];
	name name_arg;
	int *result;
	
	read_string("Enter the name: ", name_array, NAME_SIZE);
	name_arg = name_array;
	
	result = remove_1(&name_arg, c);
	if (!result)
		printf("Error: bad RPC call for remove_1.\n");
	else if (*result)
		printf("Error: `%s' not removed, error code = %d.\n",
		       name_arg, *result);
	else
		printf("`%s' has been removed.\n", name_arg);
}

void find_entry(CLIENT *c) 
{
	char name_array[NAME_SIZE];
	name name_arg;
	phone *result;
	
	read_string("Enter the name: ", name_array, NAME_SIZE);
	name_arg = name_array;
	
	result = find_1(&name_arg, c);
	/*
	 * Note: If `result' is non-null, then `*result' is also non-null,
	 * because ONC RPC presentation doesn't allow for the return of null
	 * strings.  (Empty strings yes, null strings no.)
	 */
	if (!result)
		printf("Error: bad RPC call for find_1.\n");
	else if ((*result)[0] == 0)
		printf("Error: `%s' not found.\n", name_arg);
	else
		printf("`%s' was found.  Phone number is `%s'.\n",
		       name_arg, *result);
	
	if (result)
		free(*result);
}

/* End of file. */

