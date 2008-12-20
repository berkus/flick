/*
 * Copyright (c) 1998, 1999 The University of Utah and
 * the Computer Systems Laboratory at the University of Utah (CSL).
 * All rights reserved.
 *
 * Permission to use, copy, modify and distribute this software is hereby
 * granted provided that (1) source code retains these copyright, permission,
 * and disclaimer notices, and (2) redistributions including binaries
 * reproduce the notices in supporting documentation.
 *
 * THE UNIVERSITY OF UTAH AND CSL ALLOW FREE USE OF THIS SOFTWARE IN ITS "AS
 * IS" CONDITION.  THE UNIVERSITY OF UTAH AND CSL DISCLAIM ANY LIABILITY OF
 * ANY KIND FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * CSL requests users of this software to return to csl-dist@cs.utah.edu any
 * improvements that they make and grant CSL redistribution rights.
 */

#include "mach3mig-link.h"

/* needed only for flick_check_byte_order */
#include <flick/encode/cdr.h>

CORBA_boolean CORBA_Object_is_nil(CORBA_Object ths, CORBA_Environment *ev)
{
	CORBA_BOA_set_exception(0 /* XXX */, ev,
				CORBA_NO_EXCEPTION, 0, 0);
	
	return !ths;
}

CORBA_Object CORBA_Object_duplicate(CORBA_Object ths, CORBA_Environment *ev)
{
	CORBA_BOA_set_exception(0 /* XXX */, ev,
				CORBA_NO_EXCEPTION, 0, 0);
	
	return ths;
}

void CORBA_Object_release(CORBA_Object ths, CORBA_Environment *ev)
{
	CORBA_BOA_set_exception(0 /* XXX */, ev,
				CORBA_NO_EXCEPTION, 0, 0);
	
	/* XXX - remove from nameserver? */
	mach_port_deallocate(mach_task_self(), ths);
}

CORBA_unsigned_long CORBA_Object_hash(CORBA_Object ths,
				      CORBA_unsigned_long maximum,
				      CORBA_Environment *ev)
{
	CORBA_BOA_set_exception(0 /* XXX */, ev, CORBA_NO_EXCEPTION, 0, 0);
	return ths % (maximum + 1);
}

CORBA_boolean CORBA_Object_is_equivalent(CORBA_Object ths,
					 CORBA_Object other_object,
					 CORBA_Environment *ev)
{
	/*
	 * XXX --- should null object references cause us to signal an error?
	 */
	CORBA_BOA_set_exception(0 /* XXX */, ev,
				CORBA_NO_EXCEPTION, 0, 0);
	
	return (ths == other_object);
}

/* not implemented yet. currently causes an exception */
CORBA_ImplementationDef CORBA_Object_get_implementation(CORBA_Object ths,
							CORBA_Environment *ev) 
{
	flick_set_exception(ths ? (ths->boa) : 0 /*XXX*/, ev,
			    ex_CORBA_BAD_OPERATION,
			    0, CORBA_COMPLETED_NO);
	return 0;
}

/* not implemented yet. currently causes an exception */
CORBA_InterfaceDef CORBA_Object_get_interface(CORBA_Object ths,
					      CORBA_Environment *ev) 
{
	flick_set_exception(ths ? (ths->boa) : 0 /*XXX*/, ev,
			    ex_CORBA_BAD_OPERATION,
			    0, CORBA_COMPLETED_NO);
	return 0;
}

/*
 * tests to see if an object is descended from a given type
 * XXX not implemented yet; currently returns true.
 * XXX needs to be implemented for remote calls
 */
CORBA_boolean CORBA_Object_is_a(CORBA_Object ths,
				CORBA_char *logical_type_id,
				CORBA_Environment *ev)
{
	CORBA_BOA_set_exception(0 /* XXX */, ev,
				CORBA_NO_EXCEPTION, 0, 0);
	return 1;
}

/* returns true if the object referred to no longer exists */
/* XXX Currently just returns true if the reference is CORBA_NIL */
/* XXX needs to be implemented to use a remote call */
CORBA_boolean CORBA_Object_non_existent(CORBA_Object ths,
					CORBA_Environment *ev)
{
	return CORBA_Object_is_nil(ths, ev);
}

/* End of file. */

