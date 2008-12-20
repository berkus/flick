/*
 * Copyright (c) 1997, 1998 The University of Utah and
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

void add_entry(data_phonebook c);
void remove_entry(data_phonebook c);
void find_entry(data_phonebook c);

void handle_exception(CORBA_Environment *ev);

/*****************************************************************************/

int main(int argc, char **argv) 
{
	CORBA_ORB orb = 0;
	CORBA_Environment ev;
	data_phonebook obj;
	int sel, done;
	
	if (argc != 2) {
		fprintf(stderr,
			"Usage: %s <phone obj reference>\n", argv[0]);
		exit(1);
	}
	
	orb = CORBA_ORB_init(&argc, argv, 0, &ev);
	if (ev._major != CORBA_NO_EXCEPTION) {
		printf("Can't initialize the ORB.\n");
		exit(1);
	}
	
	obj = CORBA_ORB_string_to_object(orb, argv[1], &ev);
	if (ev._major != CORBA_NO_EXCEPTION) {
		printf("Can't convert `%s' into an object reference.\n",
		       argv[1]);
		exit(1);
	}
	
	done = 0;
	while (!done) {
		read_integer(("\n(1) Add an entry (2) Remove an entry "
			      "(3) Find a phone number (4) Exit: "),
			     &sel);
		switch(sel) {
		case 1:  add_entry(obj); break;
		case 2:  remove_entry(obj); break;
		case 3:  find_entry(obj); break;
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

void add_entry(data_phonebook obj)
{
	data_entry e;
	char name[NAME_SIZE], phone[PHONE_SIZE];
	CORBA_Environment ev;
	
	e.n = name;
	e.p = phone;
	
	read_string("Enter the name: ", e.n, NAME_SIZE);
	read_string("Enter the phone number: ", e.p, PHONE_SIZE);
	
	data_phonebook_add(obj, &e, &ev);
	if (ev._major != CORBA_NO_EXCEPTION)
		handle_exception(&ev);
	else
		printf("`%s' has been added.\n", name);
}

void remove_entry(data_phonebook obj)
{
	char name[NAME_SIZE];
	CORBA_Environment ev;
	
	read_string("Enter the name: ", name, NAME_SIZE);
	
	data_phonebook_remove(obj, name, &ev);
	if (ev._major != CORBA_NO_EXCEPTION)
		handle_exception(&ev);
	else
		printf("`%s' has been removed.\n", name);
}

void find_entry(data_phonebook obj) 
{
	char name[NAME_SIZE];
	char *phone;
	CORBA_Environment ev;
	
	read_string("Enter the name: ", name, NAME_SIZE);
	
	phone = data_phonebook_find(obj, name, &ev);
	if (ev._major != CORBA_NO_EXCEPTION)
		handle_exception(&ev);
	else {
		printf("`%s' was found.  Phone number is `%s'.\n",
		       name, phone);
		CORBA_free(phone);
	}
}

/*****************************************************************************/

void handle_exception(CORBA_Environment *ev)
{
	switch (ev->_major) {
	case CORBA_NO_EXCEPTION:
		break;
		
	case CORBA_SYSTEM_EXCEPTION: {
		/*
		 * All CORBA system exceptions have the same fields, so we
		 * arbitrarily cast to `CORBA_UNKNOWN *'.
		 */
		CORBA_UNKNOWN *e = ((CORBA_UNKNOWN *)
				    CORBA_exception_value(ev));
		
		printf(("A system exception was raised: "
			"id = %s, minor = %d, completed = %d.\n"),
		       CORBA_exception_id(ev),
		       e->minor,
		       e->completed);
		break;
	}
	
	case CORBA_USER_EXCEPTION:
		printf("A user exception was raised: ");
		
		if (!strcmp(CORBA_exception_id(ev), ex_data_duplicate)) {
			data_duplicate *dup
				= ((data_duplicate *)
				   CORBA_exception_value(ev));
			
			printf("duplicate, phone = `%s'.\n", dup->p);
			/*
			 * XXX --- Flick's `CORBA_exception_free'
			 * doesn't yet do a ``deep free.''
			 */
			CORBA_free(dup->p);
				
		} else if (!strcmp(CORBA_exception_id(ev), ex_data_notfound))
			printf("notfound.\n");
		
		else
			printf("unknown exception, id = %s.\n",
			       CORBA_exception_id(ev));
		break;
		
	default:
		printf("Error: unknown exception class %d!\n", ev->_major);
		break;
	}
	
	CORBA_exception_free(ev);
}

/* End of file. */

