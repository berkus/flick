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
#include "inherit-client.h"

void checkev(CORBA_Environment *ev, const char *action)
{
	if (ev->_major != CORBA_NO_EXCEPTION) {
		printf("Exception while %s: %s\n",
		       action,
		       CORBA_exception_id(ev));
	}
}

void tryobj(CORBA_Object obj)
{
	CORBA_Environment ev;
	CORBA_boolean chk;
	CORBA_boolean done = 0;
	
	printf("Using object `%s':\n",
	       CORBA_ORB_object_to_readable_string(0, obj, &ev));
	
	chk = CORBA_Object_is_a(obj, "IDL:one:1.0", &ev);
	checkev(&ev, "checking if `one'");
	if (chk) {
		printf("  identified as `one'\n");
		one_op_one(obj, &ev);
		checkev(&ev, "executing one::op_one");
		done++;
	}
	
	chk = CORBA_Object_is_a(obj, "IDL:two:1.0", &ev);
	checkev(&ev, "checking if `two'");
	if (chk) {
		printf("  identified as `two'\n");
		two_op_one(obj, &ev);
		checkev(&ev, "executing two::op_one");
		two_op_two(obj, &ev);
		checkev(&ev, "executing two::op_two");
		done++;
	}
	
	chk = CORBA_Object_is_a(obj, "IDL:three:1.0", &ev);
	checkev(&ev, "checking if `three'");
	if (chk) {
		printf("  identified as `three'\n");
		three_op_one(obj, &ev);
		checkev(&ev, "executing three::op_one");
		three_op_three(obj, &ev);
		checkev(&ev, "executing three::op_three");
		done++;
	}
	
	chk = CORBA_Object_is_a(obj, "IDL:four:1.0", &ev);
	checkev(&ev, "checking if `four'");
	if (chk) {
		printf("  identified as `four'\n");
		four_op_one(obj, &ev);
		checkev(&ev, "executing four::op_one");
		four_op_two(obj, &ev);
		checkev(&ev, "executing four::op_two");
		four_op_three(obj, &ev);
		checkev(&ev, "executing four::op_three");
		four_op_four(obj, &ev);
		checkev(&ev, "executing four::op_four");
		done++;
	}
	
	chk = CORBA_Object_is_a(obj, "IDL:five:1.0", &ev);
	checkev(&ev, "checking if `five'");
	if (chk) {
		printf("  identified as `five'\n");
		five_op_one(obj, &ev);
		checkev(&ev, "executing five::op_one");
		five_op_two(obj, &ev);
		checkev(&ev, "executing five::op_two");
		five_op_three(obj, &ev);
		checkev(&ev, "executing five::op_three");
		five_op_four(obj, &ev);
		checkev(&ev, "executing five::op_four");
		five_op_five(obj, &ev);
		checkev(&ev, "executing five::op_five");
		done++;
	}
	
	if (done < 1) {
	    printf("  Error: no objects called!\n");
	}
}

int main(int argc, char **argv)
{
	CORBA_ORB orb = 0;
	CORBA_Object factory;
	CORBA_Environment ev;
	CORBA_Object obj[5];
	int i;
	
	orb = CORBA_ORB_init(&argc, argv, 0 /* use default ORBid */, &ev);
	factory = CORBA_ORB_string_to_object(orb, argv[1], &ev);
	if (ev._major != CORBA_NO_EXCEPTION) {
		printf("%s exception: Problem converting `%s' to an object.\n",
		       CORBA_exception_id(&ev), argv[1]);
		exit(1);
	}
	printf("Beginning communication...\n");

	obj[0] = factory_get_one(factory, &ev);  checkev(&ev, "getting obj 1");
	obj[1] = factory_get_two(factory, &ev);  checkev(&ev, "getting obj 2");
	obj[2] = factory_get_three(factory, &ev);checkev(&ev, "getting obj 3");
	obj[3] = factory_get_four(factory, &ev); checkev(&ev, "getting obj 4");
	obj[4] = factory_get_five(factory, &ev); checkev(&ev, "getting obj 5");
	
	for (i = 0; i < 5; i++)
		tryobj(obj[i]);
	
	return 0;
}

/* End of file. */
