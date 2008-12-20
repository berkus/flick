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

#include <stdio.h>
#include "ace/Get_Opt.h"

#include "phoneC.h"

#define NAME_SIZE (200 + 1)
#define PHONE_SIZE (20 + 1)

static char *ior = 0;

void read_string(const char *prompt, char *buffer, int buffer_size);
void read_integer(const char *prompt, int *i);

void add_entry(data::phonebook_var c);
void remove_entry(data::phonebook_var c);
void find_entry(data::phonebook_var c);

/*****************************************************************************/

static int parse_args(int argc, char **argv)
{
	ACE_Get_Opt get_opts(argc, argv, "k:");
	int c;
	
	while((c = get_opts()) != -1) {
		switch (c) {
		case 'k':
			ior = get_opts.optarg;
			break;
		case '?':
		default:
			ACE_ERROR_RETURN((LM_ERROR,
					  ("usage:  %s "
					   "-k IOR"
					   "\n"),
					  argv[0]),
					 -1);
		}
	}
	
	if (ior == 0)
		ACE_ERROR_RETURN((LM_ERROR,
				  "Please specify the IOR for the servant.\n"),
				 -1);
	
	/* Indicates successful parsing of the command line. */
	return 0;
}

int main(int argc, char **argv)
{
	CORBA::ORB_var orb;
	CORBA::Object_var object;
	data::phonebook_var pb;
	CORBA::Environment ACE_TRY_ENV;
	const char *op_name;
	int sel, done;
	
	/* Parse the command line; initializes `ior'. */
	if (parse_args(argc, argv) != 0)
		return 1;
	
	ACE_TRY {
		/* Initialize the ORB. */
		op_name = "CORBA::ORB_init";
		orb = CORBA::ORB_init(argc, argv, 0, ACE_TRY_ENV);
		ACE_TRY_CHECK;
		
		/* Get the object reference with the IOR. */
		op_name = "CORBA::ORB::string_to_object";
		object = orb->string_to_object(ior, ACE_TRY_ENV);
		ACE_TRY_CHECK;
		
		/* Narrow the object to a `data::phonebook'. */
		op_name = "data::phonebook::_narrow";
		pb = data::phonebook::_narrow(object.in(), ACE_TRY_ENV);
		ACE_TRY_CHECK;
	}
	ACE_CATCH (CORBA::Exception, ex) {
		ex._tao_print_exception(op_name);
		return 1;
	}
	ACE_ENDTRY;
	
	done = 0;
	while (!done) {
		read_integer(("\n(1) Add an entry (2) Remove an entry "
			      "(3) Find a phone number (4) Exit: "),
			     &sel);
		switch (sel) {
		case 1:  add_entry(pb); break;
		case 2:  remove_entry(pb); break;
		case 3:  find_entry(pb); break;
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

void add_entry(data::phonebook_var obj)
{
	data::entry e;
	char name[NAME_SIZE], phone[PHONE_SIZE];
	CORBA::Environment ACE_TRY_ENV;
	
	read_string("Enter the name: ", name, NAME_SIZE);
	read_string("Enter the phone number: ", phone, PHONE_SIZE);
	/*
	 * Duplicate the strings; they will be freed when `e' is destroyed.
	 */
	e.n = CORBA::string_dup(name);
	e.p = CORBA::string_dup(phone);
	
	ACE_TRY {
		obj->add(e, ACE_TRY_ENV);
		/* Check for exceptions. */
		ACE_TRY_CHECK;
		printf("`%s' has been added.\n", name);
		break;
	}
	ACE_CATCH (data::duplicate, ex) {
		/*
		 * Catch a `data::duplicate' exception.  It contains the
		 * phone number that is already in the database.
		 */
		printf("A user exception was raised: ");
		printf("duplicate, phone = `%s'.\n", (const char *) ex.p);
	}
	ACE_CATCH (CORBA::Exception, ex) {
		/* Catch all other exceptions. */
		ex._tao_print_exception("data::phonebook::add");
	}
	ACE_ENDTRY;
}

void remove_entry(data::phonebook_var obj)
{
	char name[NAME_SIZE];
	CORBA::Environment ACE_TRY_ENV;
	
	read_string("Enter the name: ", name, NAME_SIZE);
	
	ACE_TRY {
		obj->remove((const char *) name, ACE_TRY_ENV);
		/* Check for exceptions. */
		ACE_TRY_CHECK;
		printf("`%s' has been removed.\n", name);
		break;
	}
	ACE_CATCH (data::notfound, ex) {
		/* Catch a `data::notfound' exception. */
		printf("A user exception was raised: ");
		printf("notfound.\n");
	}
	ACE_CATCH (CORBA::Exception, ex) {
		/* Catch all other exceptions. */
		ex._tao_print_exception("data::phonebook::remove");
	}
	ACE_ENDTRY;
}

void find_entry(data::phonebook_var obj)
{
	char name[NAME_SIZE];
	CORBA::Environment ACE_TRY_ENV;
	
	read_string("Enter the name: ", name, NAME_SIZE);
	
	ACE_TRY {
		/*
		 * Make the call and catch the result in a `String_var' so that
		 * allocation is taken care of for us.
		 */
		CORBA::String_var phone
			= obj->find((const char *) name, ACE_TRY_ENV);
		/* Check for exceptions. */
		ACE_TRY_CHECK;
		printf("`%s' was found.  Phone number is `%s'.\n",
		       name, (const char *) phone);
	}
	ACE_CATCH (data::notfound, ex) {
		/* Catch a `data::notfound' exception. */
		printf("A user exception was raised: ");
		printf("notfound.\n");
	}
	ACE_CATCH (CORBA::Exception, ex) {
		/* Catch all other exceptions. */
		ex._tao_print_exception("data::phonebook::find");
	}
	ACE_ENDTRY;
}

/* End of file. */

